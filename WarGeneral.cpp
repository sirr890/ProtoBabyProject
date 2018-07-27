#include "WarGeneral.h"



WarGeneral::WarGeneral()
{
}

WarGeneral::WarGeneral(const sc2::ObservationInterface * _enviroment, MessageClass * _message,
	sc2::ActionInterface * _action_, sc2::QueryInterface* query, const sc2::Unit * _unit, std::vector<sc2::Point3D> expantion_points,
	std::vector<sc2::Point3D> enemy_points_) :BaseAgent(_enviroment, _message, _action_, query, _unit, expantion_points)
{
	fsm = new FSM(GENERAL_STATES::GP_TOTAL_STATES, GENERAL_PERCEPTIONS::GP_TOTAL_PERCEPTIONS);
	init_state_machine();
	air_build_ = true;
	target_worker_count_ = 15;
	max_colossus_count_ = 5;
	max_sentry_count_ = 2;
	max_stalker_count_ = 20;
	thr_dist_enemy = 50;
	thr_min_dist = 100;
	thr_largearmy = 25;
	enemy_points = enemy_points_;
	float distance = std::numeric_limits<float>::max();
	for (auto&expantions : expantion_points)
	{
		float d = DistanceSquared2D(expantions, enviroment->GetGameInfo().start_locations[0]);
		if (d < distance && d>1) {
			distance = d;
			staging_location_ = expantions;
		}
	}

	staging_location_.x = (enviroment->GetGameInfo().start_locations[0].x + staging_location_.x) / 2;
	staging_location_.y = (enviroment->GetGameInfo().start_locations[0].y + staging_location_.y) / 2;
}


WarGeneral::~WarGeneral()
{
	soldier.clear();
}

void WarGeneral::init_state_machine()
{
	fsm->set_transition(GENERAL_STATES::GP_PREPARING_ARMY, GENERAL_PERCEPTIONS::GP_CLOSED_GATEWAY,
		transition(GENERAL_ACTIONS::GP_OPEN_GATEWAY, GENERAL_STATES::GP_PREPARING_ARMY));
	fsm->set_transition(GENERAL_STATES::GP_PREPARING_ARMY, GENERAL_PERCEPTIONS::GP_ENEMYBASE,
		transition(GENERAL_ACTIONS::GP_UPDATE_ENEMYBASE, GENERAL_STATES::GP_PREPARING_ARMY));
	fsm->set_transition(GENERAL_STATES::GP_PREPARING_ARMY, GENERAL_PERCEPTIONS::GP_NEED_ARMY,
		transition(GENERAL_ACTIONS::GP_CREATE_SOLDIER, GENERAL_STATES::GP_PREPARING_ARMY));
	fsm->set_transition(GENERAL_STATES::GP_PREPARING_ARMY, GENERAL_PERCEPTIONS::GP_SCOUT_ORDER,
		transition(GENERAL_ACTIONS::GP_SCOUT, GENERAL_STATES::GP_PREPARING_ARMY));
	fsm->set_transition(GENERAL_STATES::GP_PREPARING_ARMY, GENERAL_PERCEPTIONS::GP_NORMAL,
		transition(GENERAL_ACTIONS::GP_NOTHING, GENERAL_STATES::GP_PREPARING_ARMY));
	fsm->set_transition(GENERAL_STATES::GP_PREPARING_ARMY, GENERAL_PERCEPTIONS::GP_LARGE_ARMY,
		transition(GENERAL_ACTIONS::GP_ATTACK, GENERAL_STATES::GP_ATTACKING));
	fsm->set_transition(GENERAL_STATES::GP_PREPARING_ARMY, GENERAL_PERCEPTIONS::GP_BASE_ATTACKED,
		transition(GENERAL_ACTIONS::GP_DEFEND_BASE, GENERAL_STATES::GP_DEFENDING));

	fsm->set_transition(GENERAL_STATES::GP_ATTACKING, GENERAL_PERCEPTIONS::GP_CLOSED_GATEWAY,
		transition(GENERAL_ACTIONS::GP_OPEN_GATEWAY, GENERAL_STATES::GP_ATTACKING));
	fsm->set_transition(GENERAL_STATES::GP_ATTACKING, GENERAL_PERCEPTIONS::GP_ENEMYBASE,
		transition(GENERAL_ACTIONS::GP_UPDATE_ENEMYBASE, GENERAL_STATES::GP_ATTACKING));
	fsm->set_transition(GENERAL_STATES::GP_ATTACKING, GENERAL_PERCEPTIONS::GP_NEED_ARMY,
		transition(GENERAL_ACTIONS::GP_CREATE_SOLDIER, GENERAL_STATES::GP_ATTACKING));
	fsm->set_transition(GENERAL_STATES::GP_ATTACKING, GENERAL_PERCEPTIONS::GP_IDLE_ARMY,
		transition(GENERAL_ACTIONS::GP_ATTACK, GENERAL_STATES::GP_ATTACKING));
	fsm->set_transition(GENERAL_STATES::GP_ATTACKING, GENERAL_PERCEPTIONS::GP_BASE_ATTACKED,
		transition(GENERAL_ACTIONS::GP_DEFEND_BASE, GENERAL_STATES::GP_DEFENDING));
	fsm->set_transition(GENERAL_STATES::GP_ATTACKING, GENERAL_PERCEPTIONS::GP_NONE_ENEMY_VISIBLE,
		transition(GENERAL_ACTIONS::GP_NOTHING, GENERAL_STATES::GP_PREPARING_ARMY));
	fsm->set_transition(GENERAL_STATES::GP_ATTACKING, GENERAL_PERCEPTIONS::GP_NORMAL,
		transition(GENERAL_ACTIONS::GP_NOTHING, GENERAL_STATES::GP_ATTACKING));

	fsm->set_transition(GENERAL_STATES::GP_DEFENDING, GENERAL_PERCEPTIONS::GP_CLOSED_GATEWAY,
		transition(GENERAL_ACTIONS::GP_OPEN_GATEWAY, GENERAL_STATES::GP_DEFENDING));
	fsm->set_transition(GENERAL_STATES::GP_DEFENDING, GENERAL_PERCEPTIONS::GP_ENEMYBASE,
		transition(GENERAL_ACTIONS::GP_UPDATE_ENEMYBASE, GENERAL_STATES::GP_DEFENDING));
	fsm->set_transition(GENERAL_STATES::GP_DEFENDING, GENERAL_PERCEPTIONS::GP_NEED_ARMY,
		transition(GENERAL_ACTIONS::GP_CREATE_SOLDIER, GENERAL_STATES::GP_DEFENDING));
	fsm->set_transition(GENERAL_STATES::GP_DEFENDING, GENERAL_PERCEPTIONS::GP_BASEOK_LARGEARMY,
		transition(GENERAL_ACTIONS::GP_ATTACK, GENERAL_STATES::GP_ATTACKING));
	fsm->set_transition(GENERAL_STATES::GP_DEFENDING, GENERAL_PERCEPTIONS::GP_BASEOK_SMALLARMY,
		transition(GENERAL_ACTIONS::GP_GROUP_ARMY, GENERAL_STATES::GP_PREPARING_ARMY));
	fsm->set_transition(GENERAL_STATES::GP_DEFENDING, GENERAL_PERCEPTIONS::GP_BASE_ATTACKED,
		transition(GENERAL_ACTIONS::GP_DEFEND_BASE, GENERAL_STATES::GP_DEFENDING));
	fsm->set_transition(GENERAL_STATES::GP_DEFENDING, GENERAL_PERCEPTIONS::GP_NORMAL,
		transition(GENERAL_ACTIONS::GP_NOTHING, GENERAL_STATES::GP_DEFENDING));
}

Message WarGeneral::Function_Perception(Message msg)
{
	auto all_upgrades = enviroment->GetUpgrades();
	upgrades.clear();
	for (auto& upg : all_upgrades)
		upgrades[upg] = true;
	if (msg["type"] == "request")
	{
		if (msg["content"]["type_request"] == "scout")
			return { { "perception_id", GENERAL_PERCEPTIONS::GP_SCOUT_ORDER },{ "target_x",msg["content"]["target_x"] },
		{ "target_y", msg["content"]["target_y"] } };
	}

	const sc2::Unit* n;
	static int cooldowndefend = 500;
	if (--cooldowndefend <= 0 && BaseAgentAttacked(n)) {
		cooldowndefend = 500;
		return { { "perception_id", GENERAL_PERCEPTIONS::GP_BASE_ATTACKED } ,{ "target_x",n->pos.x },
		{ "target_y", n->pos.y } };
	}

	static int cooldbaseok1 = 300;
	if (--cooldbaseok1 <= 0 && fsm->current_state == GENERAL_STATES::GP_DEFENDING && !BaseAgentAttacked(n) && soldier.size() < thr_largearmy)
	{
		cooldbaseok1 = 300;
		return { { "perception_id", GENERAL_PERCEPTIONS::GP_BASEOK_SMALLARMY },{ "target_x",staging_location_.x },
		{ "target_y", staging_location_.y } };
	}

	static int cooldbaseok2 = 300;
	if (--cooldbaseok2 <= 0 && fsm->current_state == GENERAL_STATES::GP_DEFENDING && !BaseAgentAttacked(n) && soldier.size() >= thr_largearmy)
	{
		cooldbaseok2 = 300;
		return { { "perception_id", GENERAL_PERCEPTIONS::GP_BASEOK_LARGEARMY },{ "target_x",enemy_points[0].x },
		{ "target_y", enemy_points[0].y } };
	}

	static int cooldownattack = 80;
	if (fsm->current_state == GENERAL_STATES::GP_PREPARING_ARMY && --cooldownattack <= 0 && LargeArmy()) {
		cooldownattack = 80;
		return { { "perception_id", GENERAL_PERCEPTIONS::GP_LARGE_ARMY },{ "all_soldier",true } };
	}

	static int cooldownarmy = 300;
	if (--cooldownarmy <= 0 && NeedArmy())
	{
		sc2::Units nexus = enviroment->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnit(sc2::UNIT_TYPEID::PROTOSS_NEXUS));
		if(nexus.size()>1)
			cooldownarmy = 200;
		else
			cooldownarmy = 300;
		return { { "perception_id", GENERAL_PERCEPTIONS::GP_NEED_ARMY } };
	}

	//static int cooldownenemyempty = 500;
	if (NoneEnemy())
	{
		//cooldownenemyempty = 500;
		static int pos = 0;
		if (expantion_points.size() > ++pos) {
			enemy_points[0] = expantion_points[pos - 1];
			return { { "perception_id", GENERAL_PERCEPTIONS::GP_NONE_ENEMY_VISIBLE } };
		}
		else
		{
			pos = 0;
		}
	}

	if (fsm->current_state == GENERAL_STATES::GP_ATTACKING && IdleArmy())
	{
		return { { "perception_id", GENERAL_PERCEPTIONS::GP_IDLE_ARMY },{ "all_soldier",false } };
	}

	if (CloseGateway()) {
		return { { "perception_id", GENERAL_PERCEPTIONS::GP_CLOSED_GATEWAY } };
	}
	const sc2::Unit*enemy;

	static int cooldownenemy = 500;
	if (--cooldownenemy <= 0 && EnemyBase(enemy))
	{
		cooldownenemy = 500;
		return { { "perception_id", GENERAL_PERCEPTIONS::GP_ENEMYBASE },{ "target_x",enemy->pos.x },
		{ "target_y", enemy->pos.y } };
	}

	return { { "perception_id", GENERAL_PERCEPTIONS::GP_NORMAL } };
}

void WarGeneral::Function_Action(Message m_action)
{
	switch ((action)m_action["action_id"]) {
	case GENERAL_ACTIONS::GP_OPEN_GATEWAY:
		OpenGateway();
		break;
	case GENERAL_ACTIONS::GP_CREATE_SOLDIER:
		TryBuildArmy();
		break;
	case GENERAL_ACTIONS::GP_DEFEND_BASE:
		DefendBase(sc2::Point2D((float)m_action["target_x"], (float)m_action["target_y"]));
		break;
	case GENERAL_ACTIONS::GP_SCOUT:

		break;
	case GENERAL_ACTIONS::GP_GROUP_ARMY:
		for (auto&sold : soldier)
		{
			message->PushMessage(
				{
					{ "type","request" },
				{ "content",{ { "type_request","move" },{ "target_x",staging_location_.x },{ "target_y",staging_location_.y } } },
				{ "receiver",sold->unit->tag }
				}
			);
		}
		break;
	case GENERAL_ACTIONS::GP_ATTACK:
		AttackOrden((bool)m_action["all_soldier"]);
		break;
	case GENERAL_ACTIONS::GP_UPDATE_ENEMYBASE:
		enemy_points.push_back(sc2::Point3D((float)m_action["target_x"], (float)m_action["target_y"],0));
		break;
	case GENERAL_ACTIONS::GP_NOTHING:
		break;
	default:
		break;
	}
}

bool WarGeneral::NoneEnemy()
{
	sc2::Units enemy = enviroment->GetUnits(sc2::Unit::Alliance::Enemy);
	if (fsm->current_state == GENERAL_STATES::GP_ATTACKING && enemy.size() == 0)
		return true;
	return false;
}

bool WarGeneral::EnemyBase(const sc2::Unit* &result)
{
	sc2::Units enemy_base = enviroment->GetUnits(sc2::Unit::Alliance::Enemy, IsEnemyBase());
	std::vector<sc2::Point2D> interset;
	for (auto& enemy : enemy_base)
	{
		bool flag = false;
		for (auto& point : enemy_points)
		{
			if (enemy->pos.x == point.x && enemy->pos.y == point.y)
			{
				flag = true;
			}			
		}
		if (!flag)
		{
			result = enemy;
			return true;
		}
	}
	return false;
}

bool WarGeneral::IdleArmy()
{
	for (auto& army : soldier) {
		if (army->fsm->current_state != SOLDIER_STATES::SB_ATTACKING)
			return true;
	}
	return false;
}

bool WarGeneral::CloseGateway()
{
	if (upgrades.find(sc2::UPGRADE_ID::WARPGATERESEARCH) != upgrades.end()) {
		sc2::Units gateway = enviroment->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnit(sc2::UNIT_TYPEID::PROTOSS_GATEWAY));
		if (gateway.size() > 0)
			return true;
	}
	return false;
}

void WarGeneral::OpenGateway()
{
	sc2::Units gateway = enviroment->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnit(sc2::UNIT_TYPEID::PROTOSS_GATEWAY));
	if (gateway.size() > 0)
	{
		for (auto&gt : gateway) {
			if (gt->build_progress == 1) {
				action_->UnitCommand(gt, sc2::ABILITY_ID::MORPH_WARPGATE);
			}
		}
	}
}

bool WarGeneral::GetRandomUnit(const sc2::Unit*& unit_out, sc2::UnitTypeID unit_type) {
	sc2::Units my_units = enviroment->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnit(unit_type));
	if (!my_units.empty()) {
		unit_out = GetRandomEntry(my_units);
		return true;
	}
	return false;
}

bool WarGeneral::TryBuildUnit(sc2::AbilityID ability_type_for_unit, sc2::UnitTypeID unit_type) {
	if (enviroment->GetFoodUsed() >= enviroment->GetFoodCap() && ability_type_for_unit != sc2::ABILITY_ID::TRAIN_OVERLORD) {
		return false;
	}
	const sc2::Unit* unit = nullptr;

	sc2::Units structures = enviroment->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnit(unit_type));
	for (auto & str : structures)
	{
		if (str->orders.empty() && str->build_progress == 1)
		{
			unit = str;
			break;
		}
	}

	if (unit != nullptr) {
		action_->UnitCommand(unit, ability_type_for_unit);
		if (ability_type_for_unit == sc2::ABILITY_ID::TRAIN_VOIDRAY || ability_type_for_unit == sc2::ABILITY_ID::TRAIN_CARRIER)
		{
			sc2::Units nexus = enviroment->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnit(sc2::UNIT_TYPEID::PROTOSS_NEXUS));
			for (auto&nx : nexus)
			{
				if (nx->energy >= 50)
				{
					
					action_->UnitCommand(nx, sc2::ABILITY_ID::EFFECT_CHRONOBOOST,unit );
					break;
				}
			}
		}

		return true;
	}

	return false;
}

size_t WarGeneral::CountUnitTypeTotal(sc2::UNIT_TYPEID unit_type, sc2::UNIT_TYPEID production, sc2::ABILITY_ID ability) {
	return CountUnitType(unit_type) + CountUnitTypeBuilding(production, ability);
}

size_t WarGeneral::CountUnitTypeBuilding(sc2::UNIT_TYPEID production_building, sc2::ABILITY_ID ability) {
	int building_count = 0;
	sc2::Units buildings = enviroment->GetUnits(sc2::Unit::Self, sc2::IsUnit(production_building));

	for (const auto& building : buildings) {
		if (building->orders.empty()) {
			continue;
		}

		for (const auto order : building->orders) {
			if (order.ability_id == ability) {
				building_count++;
			}
		}
	}

	return building_count;
}

bool WarGeneral::TryBuildArmy() {
	size_t mothership_count = CountUnitType(sc2::UNIT_TYPEID::PROTOSS_MOTHERSHIPCORE);
	mothership_count += CountUnitType(sc2::UNIT_TYPEID::PROTOSS_MOTHERSHIP);

	if (enviroment->GetFoodWorkers() > target_worker_count_ && mothership_count < 1) {
		if (CountUnitType(sc2::UNIT_TYPEID::PROTOSS_CYBERNETICSCORE) > 0) {
			TryBuildUnit(sc2::ABILITY_ID::TRAIN_MOTHERSHIPCORE, sc2::UNIT_TYPEID::PROTOSS_NEXUS);
		}
	}
	size_t colossus_count = CountUnitType(sc2::UNIT_TYPEID::PROTOSS_COLOSSUS);
	size_t carrier_count = CountUnitType(sc2::UNIT_TYPEID::PROTOSS_CARRIER);
	sc2::Units templar = enviroment->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnit(sc2::UNIT_TYPEID::PROTOSS_HIGHTEMPLAR));
	if (templar.size() > 1) {
		sc2::Units templar_merge;
		for (int i = 0; i < 2; ++i) {
			templar_merge.push_back(templar.at(i));
		}
		action_->UnitCommand(templar_merge, sc2::ABILITY_ID::MORPH_ARCHON);
	}

	if (air_build_) {
		//If we have a fleet beacon, and haven't hit our carrier count, build more carriers
		if (CountUnitType(sc2::UNIT_TYPEID::PROTOSS_FLEETBEACON) > 0 && carrier_count < max_colossus_count_) {
			//After the first carrier try and make a Mothership
			if (CountUnitType(sc2::UNIT_TYPEID::PROTOSS_MOTHERSHIP) < 1 && mothership_count > 0) {
				if (TryBuildUnit(sc2::ABILITY_ID::MORPH_MOTHERSHIP, sc2::UNIT_TYPEID::PROTOSS_MOTHERSHIPCORE)) {
					return true;
				}
				return false;
			}

			if (enviroment->GetMinerals() > 350 && enviroment->GetVespene() > 250) {
				if (TryBuildUnit(sc2::ABILITY_ID::TRAIN_CARRIER, sc2::UNIT_TYPEID::PROTOSS_STARGATE)) {
					return true;
				}
			}
			/*else if (CountUnitType(sc2::UNIT_TYPEID::PROTOSS_TEMPEST) < 1) {
				TryBuildUnit(sc2::ABILITY_ID::TRAIN_TEMPEST, sc2::UNIT_TYPEID::PROTOSS_STARGATE);
			}*/
			else if (carrier_count < 1) { //Try to build at least 1
				return false;
			}
		}
		else {
			// If we can't build Carrier, try to build voidrays
			if (enviroment->GetMinerals() > 250 && enviroment->GetVespene() > 150) {
				TryBuildUnit(sc2::ABILITY_ID::TRAIN_VOIDRAY, sc2::UNIT_TYPEID::PROTOSS_STARGATE);

			}
		}
	}
	else {
		//If we have a robotics bay, and haven't hit our colossus count, build more colossus
		if (CountUnitType(sc2::UNIT_TYPEID::PROTOSS_OBSERVER) < 1) {
			TryBuildUnit(sc2::ABILITY_ID::TRAIN_OBSERVER, sc2::UNIT_TYPEID::PROTOSS_ROBOTICSFACILITY);
		}
		if (CountUnitType(sc2::UNIT_TYPEID::PROTOSS_ROBOTICSBAY) > 0 && colossus_count < max_colossus_count_) {
			if (enviroment->GetMinerals() > 300 && enviroment->GetVespene() > 200) {
				if (TryBuildUnit(sc2::ABILITY_ID::TRAIN_COLOSSUS, sc2::UNIT_TYPEID::PROTOSS_ROBOTICSFACILITY)) {
					return true;
				}
			}
			else if (CountUnitTypeTotal(sc2::UNIT_TYPEID::PROTOSS_DISRUPTOR, sc2::UNIT_TYPEID::PROTOSS_ROBOTICSFACILITY, sc2::ABILITY_ID::TRAIN_DISRUPTOR) < 2) {
				TryBuildUnit(sc2::ABILITY_ID::TRAIN_DISRUPTOR, sc2::UNIT_TYPEID::PROTOSS_ROBOTICSFACILITY);
			}
		}
		else {
			// If we can't build Colossus, try to build immortals
			if (enviroment->GetMinerals() > 250 && enviroment->GetVespene() > 100) {
				if (TryBuildUnit(sc2::ABILITY_ID::TRAIN_IMMORTAL, sc2::UNIT_TYPEID::PROTOSS_ROBOTICSFACILITY)) {
					return true;
				}
			}
		}
	}

	if (upgrades.find(sc2::UPGRADE_ID::WARPGATERESEARCH) != upgrades.end() && CountUnitType(sc2::UNIT_TYPEID::PROTOSS_WARPGATE) > 0) {
		if (enviroment->GetMinerals() > 200 && enviroment->GetVespene() < 200) {
			return TryWarpInUnit(sc2::ABILITY_ID::TRAINWARP_ZEALOT);
		}
		if (CountUnitType(sc2::UNIT_TYPEID::PROTOSS_STALKER) > max_stalker_count_) {
			return false;
		}

		if (CountUnitType(sc2::UNIT_TYPEID::PROTOSS_ADEPT) > max_stalker_count_) {
			return false;
		}

		if (!air_build_) {
			if (CountUnitType(sc2::UNIT_TYPEID::PROTOSS_SENTRY) < max_sentry_count_) {
				return TryWarpInUnit(sc2::ABILITY_ID::TRAINWARP_SENTRY);
			}

			if (CountUnitType(sc2::UNIT_TYPEID::PROTOSS_HIGHTEMPLAR) < 2 && CountUnitType(sc2::UNIT_TYPEID::PROTOSS_ARCHON) < 2) {
				return TryWarpInUnit(sc2::ABILITY_ID::TRAINWARP_HIGHTEMPLAR);
			}
		}
		//build adepts until we have robotics facility, then switch to stalkers.
		if (CountUnitType(sc2::UNIT_TYPEID::PROTOSS_ROBOTICSFACILITY) > 0) {
			return TryWarpInUnit(sc2::ABILITY_ID::TRAINWARP_STALKER);
		}
		else {
			return TryWarpInUnit(sc2::ABILITY_ID::TRAINWARP_ZEALOT);
		}
	}
	else {
		if (enviroment->GetMinerals() > 100) {
			return TryBuildUnit(sc2::ABILITY_ID::TRAIN_ZEALOT, sc2::UNIT_TYPEID::PROTOSS_GATEWAY);
		}
		return false;
	}
}

sc2::PowerSource* WarGeneral::NearPower(std::vector<sc2::PowerSource>& power_sources)
{
	float distance = std::numeric_limits<float>::max();
	sc2::PowerSource* result = nullptr;
	for (auto&power : power_sources)
	{
		float d = DistanceSquared2D(power.position, staging_location_);
		if (d < distance) {
			distance = d;
			result = &power;
		}
	}
	return result;
}

bool WarGeneral::TryWarpInUnit(sc2::ABILITY_ID ability_type_for_unit) {
	std::vector<sc2::PowerSource> power_sources = enviroment->GetPowerSources();
	sc2::Units warpgates = enviroment->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnit(sc2::UNIT_TYPEID::PROTOSS_WARPGATE));

	if (power_sources.empty()) {
		return false;
	}
	sc2::PowerSource* random_power_source = NearPower(power_sources);
	float radius = random_power_source->radius;
	float rx = sc2::GetRandomScalar();
	float ry = sc2::GetRandomScalar();
	sc2::Point2D build_location = sc2::Point2D(random_power_source->position.x + rx * radius, random_power_source->position.y + ry * radius);
	for (const auto& warpgate : warpgates) {
		if (warpgate->build_progress == 1) {
			sc2::AvailableAbilities abilities = query->GetAbilitiesForUnit(warpgate);
			for (const auto& ability : abilities.abilities) {
				if (ability.ability_id == ability_type_for_unit) {
					action_->UnitCommand(warpgate, ability_type_for_unit, build_location);
					return true;
				}
			}
		}
	}
	return false;
}

int WarGeneral::CountUnitType(sc2::UnitTypeID unit_type) {
	int count = 0;
	sc2::Units my_units = enviroment->GetUnits(sc2::Unit::Alliance::Self);
	for (const auto unit : my_units) {
		if (unit->unit_type == unit_type)
			++count;
	}

	return count;
}

bool WarGeneral::NeedArmy()
{
	sc2::Units my_units = enviroment->GetUnits(sc2::Unit::Alliance::Self, IsArmyStructure());

	if (enviroment->GetArmyCount() > 200 || my_units.size() < 1)
		return false;

	return true;
}

bool WarGeneral::BaseAgentAttacked(const sc2::Unit* &n)
{
	sc2::Units enemy_units = enviroment->GetUnits(sc2::Unit::Alliance::Enemy);
	sc2::Units bases = enviroment->GetUnits(sc2::Unit::Alliance::Self, IsNexusPylon());

	for (auto& base : bases)
		for (auto& enemy : enemy_units)
			if (enemy->unit_type!= sc2::UNIT_TYPEID::ZERG_CHANGELINGZEALOT && enemy->display_type == sc2::Unit::DisplayType::Visible && sc2::DistanceSquared2D(enemy->pos, base->pos) < thr_dist_enemy && soldier.size() > 3)
				{
					n = base;
					return true;
				}
	return false;
}

void WarGeneral::AttackFunction(SoldadoBot* sold)
{
	message->PushMessage(
		{
			{ "type","request" },
		{ "content",{ { "type_request","attack" },{ "target_x",staging_location_.x },{ "target_y",staging_location_.y } } },
		{ "receiver",sold->unit->tag }
		}
	);
}

void WarGeneral::DefendBase(sc2::Point2D point)
{
	staging_location_.x = point.x;
	staging_location_.y = point.y;
	//sc2::Units enemy = enviroment->GetUnits(sc2::Unit::Alliance::Enemy, sc2::IsUnit(sc2::UNIT_TYPEID::ZERG_CHANGELINGZEALOT));
	
	for (auto& sold : soldier)
		AttackFunction(sold);
	/*for (auto& en : enemy)
		if (en->display_type == sc2::Unit::DisplayType::Visible)
		{
			int index = rand()%(soldier.size());
			if(index>=0 && index<soldier.size())
				action_->UnitCommand(soldier[index]->unit, sc2::ABILITY_ID::ATTACK, en);
		}	*/

}

bool WarGeneral::LargeArmy()
{
	sc2::Units voidray = enviroment->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnit(sc2::UNIT_TYPEID::PROTOSS_VOIDRAY));
	sc2::Units carrier = enviroment->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnit(sc2::UNIT_TYPEID::PROTOSS_CARRIER));
	if (fsm->current_state == GENERAL_STATES::GP_ATTACKING || enviroment->GetArmyCount() > thr_largearmy || (voidray.size()+ carrier.size())>4)
		return true;
	return false;
}

void WarGeneral::AttackOrden(bool all)
{
	staging_location_ = enemy_points[0];
	for (auto& army : soldier) {
		if (all || army->fsm->current_state != SOLDIER_STATES::SB_ATTACKING)
			AttackFunction(army);
	}
}


