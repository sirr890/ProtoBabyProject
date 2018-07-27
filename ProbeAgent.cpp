#include "ProbeAgent.h"

using namespace sc2;

ProbeAgent::ProbeAgent()
{
	fsm = new FSM(PROBE_STATE::TOTAL_STATES, PROBE_PERCEPTIONS::TOTAL_PERCEPTIONS);
	init_state_machine();
	vespene = false;
	building_assimilator = false;
}

ProbeAgent::ProbeAgent(const sc2::ObservationInterface * _enviroment, MessageClass * _message, sc2::ActionInterface * _action_, sc2::QueryInterface* query, const sc2::Unit * _unit, std::vector<sc2::Point3D> expantion_points) :BaseAgent(_enviroment, _message, _action_, query, _unit, expantion_points)
{
	fsm = new FSM(PROBE_STATE::TOTAL_STATES, PROBE_PERCEPTIONS::TOTAL_PERCEPTIONS);
	init_state_machine();
	vespene = false;
	position = unit->pos;
	building_assimilator = false;
}


ProbeAgent::~ProbeAgent()
{
}

/*Iniciar a máquina de estado*/
void ProbeAgent::init_state_machine()
{
	fsm->set_transition(PROBE_STATE::WORKING, PROBE_PERCEPTIONS::ATTACKED, transition(PROBE_ACTIONS::MOVE, PROBE_STATE::RUNNING));
	fsm->set_transition(PROBE_STATE::WORKING, PROBE_PERCEPTIONS::BUILD_ORDER, transition(PROBE_ACTIONS::BUILD, PROBE_STATE::BUILDING));
	fsm->set_transition(PROBE_STATE::WORKING, PROBE_PERCEPTIONS::SCOUT_ORDER, transition(PROBE_ACTIONS::MOVE, PROBE_STATE::SCOUTING));
	fsm->set_transition(PROBE_STATE::WORKING, PROBE_PERCEPTIONS::VESPENE_ORDER, transition(PROBE_ACTIONS::GATHER, PROBE_STATE::WORKING));
	fsm->set_transition(PROBE_STATE::WORKING, PROBE_PERCEPTIONS::NORMAL, transition(PROBE_ACTIONS::NOTHING, PROBE_STATE::WORKING));
	fsm->set_transition(PROBE_STATE::WORKING, PROBE_PERCEPTIONS::IDLE, transition(PROBE_ACTIONS::GATHER, PROBE_STATE::WORKING));

	fsm->set_transition(PROBE_STATE::BUILD_COMPLETE, PROBE_PERCEPTIONS::IDLE, transition(PROBE_ACTIONS::GATHER, PROBE_STATE::WORKING));
	fsm->set_transition(PROBE_STATE::BUILD_COMPLETE, PROBE_PERCEPTIONS::ATTACKED, transition(PROBE_ACTIONS::MOVE, PROBE_STATE::RUNNING));
	fsm->set_transition(PROBE_STATE::BUILD_COMPLETE, PROBE_PERCEPTIONS::BUILD_ORDER, transition(PROBE_ACTIONS::BUILD, PROBE_STATE::BUILDING));
	fsm->set_transition(PROBE_STATE::BUILD_COMPLETE, PROBE_PERCEPTIONS::SCOUT_ORDER, transition(PROBE_ACTIONS::MOVE, PROBE_STATE::SCOUTING));
	fsm->set_transition(PROBE_STATE::BUILD_COMPLETE, PROBE_PERCEPTIONS::VESPENE_ORDER, transition(PROBE_ACTIONS::GATHER, PROBE_STATE::WORKING));
	fsm->set_transition(PROBE_STATE::BUILD_COMPLETE, PROBE_PERCEPTIONS::NORMAL, transition(PROBE_ACTIONS::GATHER, PROBE_STATE::WORKING));

	fsm->set_transition(PROBE_STATE::RUNNING, PROBE_PERCEPTIONS::IDLE, transition(PROBE_ACTIONS::GATHER, PROBE_STATE::WORKING));
	fsm->set_transition(PROBE_STATE::RUNNING, PROBE_PERCEPTIONS::ATTACKED, transition(PROBE_ACTIONS::MOVE, PROBE_STATE::RUNNING));
	fsm->set_transition(PROBE_STATE::RUNNING, PROBE_PERCEPTIONS::NORMAL, transition(PROBE_ACTIONS::NOTHING, PROBE_STATE::RUNNING));
	fsm->set_transition(PROBE_STATE::RUNNING, PROBE_PERCEPTIONS::BUILD_ORDER, transition(PROBE_ACTIONS::BUILD, PROBE_STATE::BUILDING));
	fsm->set_transition(PROBE_STATE::RUNNING, PROBE_PERCEPTIONS::SCOUT_ORDER, transition(PROBE_ACTIONS::MOVE, PROBE_STATE::SCOUTING));
	fsm->set_transition(PROBE_STATE::RUNNING, PROBE_PERCEPTIONS::VESPENE_ORDER, transition(PROBE_ACTIONS::GATHER, PROBE_STATE::WORKING));

	fsm->set_transition(PROBE_STATE::BUILDING, PROBE_PERCEPTIONS::IDLE, transition(PROBE_ACTIONS::NOTHING, PROBE_STATE::BUILD_COMPLETE));
	fsm->set_transition(PROBE_STATE::BUILDING, PROBE_PERCEPTIONS::NORMAL, transition(PROBE_ACTIONS::NOTHING, PROBE_STATE::BUILDING));
	fsm->set_transition(PROBE_STATE::BUILDING, PROBE_PERCEPTIONS::BUILD_ORDER, transition(PROBE_ACTIONS::BUILD, PROBE_STATE::BUILDING));
	fsm->set_transition(PROBE_STATE::BUILDING, PROBE_PERCEPTIONS::SCOUT_ORDER, transition(PROBE_ACTIONS::MOVE, PROBE_STATE::SCOUTING));
	fsm->set_transition(PROBE_STATE::BUILDING, PROBE_PERCEPTIONS::VESPENE_ORDER, transition(PROBE_ACTIONS::GATHER, PROBE_STATE::WORKING));
	fsm->set_transition(PROBE_STATE::BUILDING, PROBE_PERCEPTIONS::ATTACKED, transition(PROBE_ACTIONS::NOTHING, PROBE_STATE::BUILDING));

	fsm->set_transition(PROBE_STATE::SCOUTING, PROBE_PERCEPTIONS::ATTACKED, transition(PROBE_ACTIONS::NOTHING, PROBE_STATE::SCOUTING));
	fsm->set_transition(PROBE_STATE::SCOUTING, PROBE_PERCEPTIONS::IDLE, transition(PROBE_ACTIONS::GATHER, PROBE_STATE::WORKING));
	fsm->set_transition(PROBE_STATE::SCOUTING, PROBE_PERCEPTIONS::NORMAL, transition(PROBE_ACTIONS::NOTHING, PROBE_STATE::SCOUTING));
	fsm->set_transition(PROBE_STATE::SCOUTING, PROBE_PERCEPTIONS::BUILD_ORDER, transition(PROBE_ACTIONS::BUILD, PROBE_STATE::BUILDING));
	fsm->set_transition(PROBE_STATE::SCOUTING, PROBE_PERCEPTIONS::SCOUT_ORDER, transition(PROBE_ACTIONS::MOVE, PROBE_STATE::SCOUTING));
	fsm->set_transition(PROBE_STATE::SCOUTING, PROBE_PERCEPTIONS::VESPENE_ORDER, transition(PROBE_ACTIONS::GATHER, PROBE_STATE::WORKING));

	fsm->current_state = PROBE_STATE::WORKING;
}

/*Função de percepção do agente sonda*/
Message ProbeAgent::Function_Perception(Message msg)
{
	if(fsm->current_state==PROBE_STATE::WORKING){
	const Unit* target_structure=nullptr;
	const Unit* mineral = FindNearestMineralPatch(position, target_structure);
	if(mineral!=nullptr)
		position = mineral->pos;
	}
	if (msg["type"] == "request")
	{
		if (msg["content"]["type_request"] == "build")
		{
			sc2::ABILITY_ID structure = msg["content"]["structure"];
			float target_x = msg["content"]["target_x"];
			float target_y = msg["content"]["target_y"];
			return	{
				{ "perception_id", PROBE_PERCEPTIONS::BUILD_ORDER },
			{ "structure",structure },
			{ "target_x",target_x },
			{ "target_y",target_y }
			};
		}
		if (msg["content"]["type_request"] == "scout")
			return	{
				{ "perception_id", PROBE_PERCEPTIONS::SCOUT_ORDER },
		{ "target_x",msg["content"]["target_x"] },
		{ "target_y",msg["content"]["target_y"] }
		};
		if (msg["content"]["type_request"] == "vespene")
			return	{
				{ "perception_id", PROBE_PERCEPTIONS::VESPENE_ORDER },
		{ "target_x",msg["content"]["target_x"] },
		{ "target_y",msg["content"]["target_y"] }
		};
	}

	if (unit->health < current_health || unit->shield < current_shield)
	{
		current_health = unit->health;
		current_shield = unit->shield;
		const Unit * u = FindNearestEnemy(unit->pos);
		sc2::Point2D point = Running_Function(u->pos);
		return	{ { "perception_id", PROBE_PERCEPTIONS::ATTACKED },
		{ "target_x",point.x },{ "target_y",point.y } };
	}

	current_shield = unit->shield;

	if (unit->orders.empty() || (building_assimilator && unit->orders[0].ability_id != sc2::ABILITY_ID::BUILD_ASSIMILATOR))//
	{
		if (building_assimilator) {
			building_assimilator = false;
		}

		return {
			{ "perception_id", PROBE_PERCEPTIONS::IDLE },
		{ "target_x",-1 },{ "target_y",-1 } };
	};

	return { { "perception_id", PROBE_PERCEPTIONS::NORMAL },
	{ "target_x",-1 },{ "target_y",-1 } };
}

/*Função da sonda de acordo com a percepção*/
void ProbeAgent::Function_Action(Message m_action)
{
	switch ((action)m_action["action_id"])
	{
	case PROBE_ACTIONS::GATHER:
		GatherFunction((float)m_action["target_x"], (float)m_action["target_y"]);
		break;
	case PROBE_ACTIONS::BUILD:
		Build_Structure((sc2::ABILITY_ID)m_action["structure"], (float)m_action["target_x"], (float)m_action["target_y"]);
	break;
	case PROBE_ACTIONS::MOVE:
		action_->UnitCommand(unit, sc2::ABILITY_ID::MOVE, sc2::Point2D((float)m_action["target_x"], (float)m_action["target_y"]), true);
		break;
	case PROBE_ACTIONS::NOTHING:
		break;
	default:
		break;
	}
}

/*Função para que a sonda ache o mineral o gas para minerar depois de terminar uma ação*/
void ProbeAgent::GatherFunction(float target_x, float target_y)
{
	sc2::Point2D build_location = sc2::Point2D(target_x, target_y);
	if (target_x == -1 || target_y == -1)
	{
		build_location = position;
	}
	else
	{
		build_location = sc2::Point2D(target_x, target_y);
	}
	const sc2::Unit* target_structure = nullptr;
	const sc2::Unit* resources_patch = FindNearestMineralPatch(build_location, target_structure);
	position = build_location;
	if (target_structure == nullptr)
		return;
	target_tag = target_structure->tag;
	if (resources_patch->unit_type == sc2::UNIT_TYPEID::PROTOSS_ASSIMILATOR)
	{
		vespene = true;
	}
	else
	{
		vespene = false;
	}
	action_->UnitCommand(unit, sc2::ABILITY_ID::SMART, resources_patch, true);
}

/*Função para criar uma estrutura*/
void ProbeAgent::Build_Structure(sc2::ABILITY_ID id, float target_x, float target_y)
{
	bool build_succed = false;
	switch (id)
	{
	case sc2::ABILITY_ID::BUILD_ASSIMILATOR:
		build_succed = TryBuildAssimilator(target_x, target_y);
		break;
	case sc2::ABILITY_ID::BUILD_NEXUS:
		build_succed = TryBuildNexus(target_x, target_y);
		break;
	case sc2::ABILITY_ID::BUILD_PYLON:
		build_succed = TryBuildPylon(target_x, target_y);
		break;
	default:
		build_succed = TryBuildStructureNearPylon(id, target_x, target_y);
		if (!build_succed)
		{
			message->PushMessage({
				{ "type","request" },
				{ "content",{ { "type_request","build" },{ "structure",id },{ "target_x",target_x },
				{ "target_y",target_y } } },{ "receiver",unit->tag }
				});
		}
		break;
	}
}

/*Função para achar o mineral ou gás mais próximo*/
const sc2::Unit* ProbeAgent::FindNearestMineralPatch(sc2::Point2D start, const sc2::Unit* &target_structure) {
	Units nexus = enviroment->GetUnits(Unit::Alliance::Self, IsUnit(sc2::UNIT_TYPEID::PROTOSS_NEXUS));
	Units assimilators = enviroment->GetUnits(Unit::Alliance::Self, IsUnit(sc2::UNIT_TYPEID::PROTOSS_ASSIMILATOR));
	Units mineral = enviroment->GetUnits(Unit::Alliance::Neutral, IsMinieral());
	float distance = std::numeric_limits<float>::max();
	const Unit* target = nullptr;
	if (nexus.empty()) {
		return target;
	}

	for (const auto& base : nexus) {
		if (base->ideal_harvesters == 0 || base->build_progress != 1) 
			continue;
			
			float d = DistanceSquared2D(base->pos, start);
			if (d < distance) {
				distance = d;
				target_structure = base;
			}
	}
	if(target_structure!=nullptr){
	distance = std::numeric_limits<float>::max();
	for (const auto& u : mineral) {
		float d = DistanceSquared2D(target_structure->pos, u->pos);
		if (d < distance) {
			distance = d;
			target = u;
		}
	}
	}
	for (const auto& u : assimilators) {
		if (u->ideal_harvesters == 0 || u->build_progress != 1) {
			continue;
		}
		if (u->assigned_harvesters < u->ideal_harvesters) {
			float d = DistanceSquared2D(u->pos, start);
			if (d < distance) {
				distance = d;
				target = u;
				target_structure = u;
			}
		}
	}
	if (distance == std::numeric_limits<float>::max()) {
		target = mineral[0];
		target_structure = mineral[0];
	}
	return target;
}

/*Função para achar o nexus mas próximo dada uma posição*/
const sc2::Unit* ProbeAgent::FindNearestNexus(sc2::Point2D start) {
	Units units = enviroment->GetUnits(Unit::Alliance::Self);
	float distance = std::numeric_limits<float>::max();
	const Unit* target = nullptr;
	for (const auto& u : units) {
		if (u->unit_type.ToType() == sc2::UNIT_TYPEID::PROTOSS_NEXUS) {
			float d = DistanceSquared2D(u->pos, start);
			if (d < distance) {
				distance = d;
				target = u;
			}
		}
	}
	if (distance == std::numeric_limits<float>::max()) {
		return target;
	}
	return target;
}

/*Função de fugir se uma sonda está sendo atacada*/
sc2::Point2D ProbeAgent::Running_Function(sc2::Point2D point)
{
	float x = unit->pos.x + ((sc2::GetRandomScalar() + 1) * 5)*(int((unit->pos.x - point.x) > 0) * 2 - 1);
	float y = unit->pos.y + ((sc2::GetRandomScalar() + 1) * 5)*(int((unit->pos.y - point.y) > 0) * 2 - 1);
	return sc2::Point2D(x, y);
}

/*Acha o enemigo mais próximo dado uma localização*/
const sc2::Unit* ProbeAgent::FindNearestEnemy(sc2::Point2D start) {
	Units units = enviroment->GetUnits(Unit::Alliance::Enemy);
	float distance = std::numeric_limits<float>::max();
	const Unit* target = nullptr;
	for (const auto& u : units) {
		float d = DistanceSquared2D(u->pos, start);
		if (d < distance) {
			distance = d;
			target = u;
		}
	}
	if (distance == std::numeric_limits<float>::max()) {
		return target;
	}
	return target;
}

/*Função para criar um assimilator*/
bool ProbeAgent::TryBuildAssimilator(float target_x, float target_y) {
	Units bases = enviroment->GetUnits(Unit::Alliance::Self, IsTownHall());
	if (target_x == -1 || target_y == -1)
	{
		sc2::Point2D localization;
		if (bases.size() > 0)
		{
			localization = bases[0]->pos;
		}
		else
		{
			localization = position;
		}
		if (TryBuildGas(ABILITY_ID::BUILD_ASSIMILATOR, UNIT_TYPEID::PROTOSS_PROBE, localization)) {
			return true;
		}
	}
	else
	{
		if (TryBuildGas(ABILITY_ID::BUILD_ASSIMILATOR, UNIT_TYPEID::PROTOSS_PROBE, sc2::Point2D(target_x, target_y))) {
			return true;
		}
	}
	return false;
}

bool ProbeAgent::TryBuildGas(sc2::AbilityID build_ability, sc2::UnitTypeID worker_type, sc2::Point2D base_location) {
	Units all_nexus = enviroment->GetUnits(Unit::Alliance::Self, IsUnit(sc2::UNIT_TYPEID::PROTOSS_NEXUS));
	sc2::Tag closestGeyser;
	closestGeyser = get_geyser(build_ability, base_location);
	bool nexus_geyser = false;
	if (closestGeyser == 0 ) {
		for (int i = 0; i < all_nexus.size();i++)
		{
			closestGeyser = get_geyser(build_ability, all_nexus[i]->pos);
			if (closestGeyser != 0) {
				nexus_geyser = true;
				break;
			}
		}
		if (!nexus_geyser)
			return false;
	}
	const Unit* target = enviroment->GetUnit(closestGeyser);
	building_assimilator = TryBuildStructure(build_ability, worker_type, target->pos, target);
	return building_assimilator;
}

/*Função para obter o ponto de gás mas próximo de uma posição dada*/
sc2::Tag ProbeAgent::get_geyser(sc2::AbilityID build_ability, sc2::Point2D base_location)
{
	Units geysers = enviroment->GetUnits(Unit::Alliance::Neutral, IsVespeneGeyser());
	float minimum_distance = 15.0f;
	sc2::Tag closestGeyser=0;
	for (const auto& geyser : geysers) {
		float current_distance = Distance2D(base_location, geyser->pos);
		if (current_distance < minimum_distance) {
			if (query->Placement(build_ability, geyser->pos)) {
				minimum_distance = current_distance;
				closestGeyser = geyser->tag;
			}
		}
	}
	return closestGeyser;
}

/*Função para construir uma estrutura dado uma posição*/
bool ProbeAgent::TryBuildStructure(sc2::AbilityID ability_type_for_structure, sc2::UnitTypeID unit_type, sc2::Point2D location, const Unit* target) {
	if (target != nullptr && query->Placement(ability_type_for_structure, target->pos))
	{
		action_->UnitCommand(unit, ability_type_for_structure, target);//, true);
		return true;
	}
	if (query->Placement(ability_type_for_structure, location)) {
		action_->UnitCommand(unit, ability_type_for_structure, location);//, true);
		return true;
	}
	return false;
}

/*Função para criar um pilar*/
bool ProbeAgent::TryBuildPylon(float target_x, float target_y) {
	sc2::Units units = enviroment->GetUnits(Unit::Alliance::Self, IsUnit(UNIT_TYPEID::PROTOSS_PYLON));
	for (const auto& unit : units) {
		if (unit->build_progress != 1) {
			return false;
		}
	}
	sc2::Point2D build_location;
	if (target_x != -1 && target_y != -1)
	{
		build_location = sc2::Point2D(target_x, target_y);
	}
	else
	{
		float rx = GetRandomScalar();
		float ry = GetRandomScalar();
		const sc2::Unit* neares_nexus = FindNearestNexus(position);
		if (neares_nexus != nullptr)
			build_location = sc2::Point2D(neares_nexus->pos.x + rx * 15, neares_nexus->pos.y + ry * 15);
		else
			build_location = unit->pos;
	}
	return TryBuildStructure(ABILITY_ID::BUILD_PYLON, UNIT_TYPEID::PROTOSS_PROBE, build_location, nullptr);
}

/*Função para criar um nexus*/
bool ProbeAgent::TryBuildNexus(float target_x, float target_y)
{
	sc2::Point2D build_location;
	if (target_x != -1 && target_y != -1)
	{
		build_location = sc2::Point2D(target_x, target_y);
		return TryBuildStructure(ABILITY_ID::BUILD_NEXUS, UNIT_TYPEID::PROTOSS_PROBE, build_location, nullptr);
	}
	return TryExpand(ABILITY_ID::BUILD_NEXUS, UNIT_TYPEID::PROTOSS_PROBE);
}

/*Função para tentar expansão*/
bool ProbeAgent::TryExpand(sc2::AbilityID build_ability, sc2::UnitTypeID worker_type) {
	float minimum_distance = std::numeric_limits<float>::max();
	Point3D closest_expansion;
	std::vector<Point3D> expansions_ = expantion_points;
	for (const auto& expansion : expansions_) {
		float current_distance = Distance2D(enviroment->GetStartLocation(), expansion);
		if (current_distance < .01f) {
			continue;
		}

		if (current_distance < minimum_distance) {
			if (query->Placement(build_ability, expansion)) {
				closest_expansion = expansion;
				minimum_distance = current_distance;
			}
		}
	}
	return TryBuildStructure(build_ability, worker_type, closest_expansion, nullptr);
}

/*Função para contruir uma estrutura próximo ao nexus*/
bool ProbeAgent::TryBuildStructureNearPylon(sc2::AbilityID ability_type_for_structure, float target_x, float target_y) {
	std::vector<PowerSource> power_sources = enviroment->GetPowerSources();
	if (power_sources.empty()) {
		return false;
	}
	const PowerSource& random_power_source = GetRandomEntry(power_sources);
	if (enviroment->GetUnit(random_power_source.tag) != nullptr) {
		if (enviroment->GetUnit(random_power_source.tag)->unit_type == UNIT_TYPEID::PROTOSS_WARPPRISM) {
			return false;
		}
	}
	else {
		return false;
	}
	sc2::Point2D build_location;
	if (target_x != -1 && target_y != -1)
	{
		build_location = sc2::Point2D(target_x, target_y);
	}
	else
	{
		float radius = random_power_source.radius;
		float rx = GetRandomScalar();
		float ry = GetRandomScalar();
		build_location = sc2::Point2D(random_power_source.position.x + rx * radius, random_power_source.position.y + ry * radius);
	}
	return TryBuildStructure(ability_type_for_structure, UNIT_TYPEID::PROTOSS_PROBE, build_location, nullptr);
}