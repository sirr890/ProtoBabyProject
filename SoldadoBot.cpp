#include "SoldadoBot.h"



SoldadoBot::SoldadoBot()
{
}


SoldadoBot::~SoldadoBot()
{
}

SoldadoBot::SoldadoBot(const sc2::ObservationInterface * _enviroment, MessageClass * _message, sc2::ActionInterface * _action_, sc2::QueryInterface* query, const sc2::Unit * _unit, std::vector<sc2::Point3D> expantion_points) :BaseAgent(_enviroment, _message, _action_, query, _unit, expantion_points)
{
	fsm = new FSM(SOLDIER_STATES::SB_TOTAL_STATES, SOLDIER_PERCEPTIONS::SB_TOTAL_PERCEPTIONS);
	init_state_machine();
	thr_min_dist = 10.0f;
	sight_ = Sight().getSight(unit);
}

void SoldadoBot::init_state_machine()
{
	fsm->set_transition(SOLDIER_STATES::SB_WAIT, SOLDIER_PERCEPTIONS::SB_SCOUT_ORDER, transition(SOLDIER_ACTIONS::SB_SCOUT, SOLDIER_STATES::SB_WAIT));
	fsm->set_transition(SOLDIER_STATES::SB_WAIT, SOLDIER_PERCEPTIONS::SB_MOVE_ORDER, transition(SOLDIER_ACTIONS::SB_MOVE, SOLDIER_STATES::SB_WAIT));
	fsm->set_transition(SOLDIER_STATES::SB_WAIT, SOLDIER_PERCEPTIONS::SB_ATTACK_ORDER, transition(SOLDIER_ACTIONS::SB_ATTACK, SOLDIER_STATES::SB_ATTACKING));
	fsm->set_transition(SOLDIER_STATES::SB_WAIT, SOLDIER_PERCEPTIONS::SB_NORMAL, transition(SOLDIER_ACTIONS::SB_NOTHING, SOLDIER_STATES::SB_WAIT));
	fsm->set_transition(SOLDIER_STATES::SB_WAIT, SOLDIER_PERCEPTIONS::SB_LOW_SHIELD, transition(SOLDIER_ACTIONS::SB_HEALT, SOLDIER_STATES::SB_WAIT));

	fsm->set_transition(SOLDIER_STATES::SB_ATTACKING, SOLDIER_PERCEPTIONS::SB_NORMAL, transition(SOLDIER_ACTIONS::SB_NOTHING, SOLDIER_STATES::SB_ATTACKING));
	fsm->set_transition(SOLDIER_STATES::SB_ATTACKING, SOLDIER_PERCEPTIONS::SB_NONE_ENEMY, transition(SOLDIER_ACTIONS::SB_MOVE, SOLDIER_STATES::SB_WAIT));
	fsm->set_transition(SOLDIER_STATES::SB_ATTACKING, SOLDIER_PERCEPTIONS::SB_WAIT_ORDER, transition(SOLDIER_ACTIONS::SB_MOVE, SOLDIER_STATES::SB_WAIT));
	fsm->set_transition(SOLDIER_STATES::SB_ATTACKING, SOLDIER_PERCEPTIONS::SB_ATTACK_ORDER, transition(SOLDIER_ACTIONS::SB_ATTACK, SOLDIER_STATES::SB_ATTACKING));
	fsm->set_transition(SOLDIER_STATES::SB_ATTACKING, SOLDIER_PERCEPTIONS::SB_MOVE_ORDER, transition(SOLDIER_ACTIONS::SB_MOVE, SOLDIER_STATES::SB_ATTACKING));
	fsm->set_transition(SOLDIER_STATES::SB_ATTACKING, SOLDIER_PERCEPTIONS::SB_IDLE, transition(SOLDIER_ACTIONS::SB_ATTACK, SOLDIER_STATES::SB_ATTACKING));

	fsm->current_state = SOLDIER_STATES::SB_WAIT;
}

Message SoldadoBot::Function_Perception(Message msg)
{
	if (msg["type"] == "request")
	{
		if (msg["content"]["type_request"] == "scout")
			return { { "perception_id", SOLDIER_PERCEPTIONS::SB_SCOUT_ORDER },{"target_x",msg["content"]["target_x"]},
		{ "target_y", msg["content"]["target_y"] } };

		if (msg["content"]["type_request"] == "attack")
			return { { "perception_id", SOLDIER_PERCEPTIONS::SB_ATTACK_ORDER },{ "target_x",msg["content"]["target_x"] },
		{ "target_y", msg["content"]["target_y"] } };

		if (msg["content"]["type_request"] == "move")
			return { { "perception_id", SOLDIER_PERCEPTIONS::SB_MOVE_ORDER },{ "target_x",msg["content"]["target_x"] },
		{ "target_y", msg["content"]["target_y"] } };
		if (msg["content"]["type_request"] == "wait")
			return { { "perception_id", SOLDIER_PERCEPTIONS::SB_WAIT_ORDER },{ "target_x",msg["content"]["target_x"] },
		{ "target_y", msg["content"]["target_y"] } };
	}

	sc2::Units enemies = enviroment->GetUnits(sc2::Unit::Alliance::Enemy);
	if (fsm->current_state == SOLDIER_STATES::SB_ATTACKING && IdleUnit())
	{		
		float distance = std::numeric_limits<float>::max();
		const sc2::Unit* target = nullptr;
		for (const auto& enemy : enemies) {
			float d = sc2::DistanceSquared2D(enemy->pos, unit->pos);
			if (d < distance && enemy->display_type == sc2::Unit::DisplayType::Visible) {
				distance = d;
				target = enemy;
			}
		}
		if(target!=nullptr)
		return { { "perception_id", SOLDIER_PERCEPTIONS::SB_IDLE } ,{ "target_x",target->pos.x },
		{ "target_y", target->pos.y } };
		else
			return { { "perception_id", SOLDIER_PERCEPTIONS::SB_NONE_ENEMY },{ "target_x",poisition.x },
		{ "target_y", poisition.y } };
	}

	if (fsm->current_state == SOLDIER_STATES::SB_WAIT && LowShield())
		return { { "perception_id", SOLDIER_PERCEPTIONS::SB_LOW_SHIELD } };

	return { { "perception_id", SOLDIER_PERCEPTIONS::SB_NORMAL } };
}

bool SoldadoBot::LowShield()
{
	sc2::Units batery = enviroment->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnit(sc2::UNIT_TYPEID::PROTOSS_SHIELDBATTERY));
	if (batery.size() > 0 && unit->shield < unit->shield_max)
		return true;
	return false;
}

bool SoldadoBot::IdleUnit()
{
	if (unit->orders.empty())
		return true;

	return false;
}

void SoldadoBot::Function_Action(Message m_action)
{
	switch ((action)m_action["action_id"])
	{
	case SOLDIER_ACTIONS::SB_ATTACK:
	{
		sc2::Point2D target = sc2::Point2D((float)m_action["target_x"], (float)m_action["target_y"]);
		UnitAttack(target);
		poisition = unit->pos;
	}
	break;
	case SOLDIER_ACTIONS::SB_MOVE:
	{
		sc2::Point2D point = sc2::Point2D((float)m_action["target_x"], (float)m_action["target_y"]);
		action_->UnitCommand(unit, sc2::ABILITY_ID::MOVE, point);
		poisition = point;
	}
	break;
	case SOLDIER_ACTIONS::SB_SCOUT:
	{
		sc2::Point2D point = sc2::Point2D((float)m_action["target_x"], (float)m_action["target_y"]);
		action_->UnitCommand(unit, sc2::ABILITY_ID::MOVE, point);
		action_->UnitCommand(unit, sc2::ABILITY_ID::MOVE, poisition, true);
	}
	break;
	case SOLDIER_ACTIONS::SB_HEALT:
	{
		HealtFunction();
	}
	break;
	case SOLDIER_ACTIONS::SB_NOTHING:
		break;
	default:
		break;
	}
}

void SoldadoBot::HealtFunction()
{
	sc2::Units batery = enviroment->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnit(sc2::UNIT_TYPEID::PROTOSS_SHIELDBATTERY));
	if (batery.size() > 0)
	{
		const sc2::Unit* near_batery = nullptr;
		float distance = std::numeric_limits<float>::max();
		for (const auto& u : batery) {
			float d = Distance2D(u->pos, unit->pos);
			if (d < distance) {
				distance = d;
				near_batery = u;
			}
		}
		action_->UnitCommand(unit, sc2::ABILITY_ID::MOVE, near_batery->pos);
	}
}

void SoldadoBot::UnitAttack(sc2::Point2D target)
{
	sc2::Units enemy_units = enviroment->GetUnits(sc2::Unit::Alliance::Enemy);
	action_->UnitCommand(unit, sc2::ABILITY_ID::ATTACK, target);
	switch (unit->unit_type.ToType()) {
	case (sc2::UNIT_TYPEID::PROTOSS_SENTRY): {
		if (!unit->orders.empty()) {
			if (unit->orders.front().ability_id == sc2::ABILITY_ID::ATTACK) {
				float distance = std::numeric_limits<float>::max();
				for (const auto& u : enemy_units) {
					float d = Distance2D(u->pos, unit->pos);
					if (d < distance) {
						distance = d;
					}
				}
				if (distance < 6 && unit->energy >= 75) {
					action_->UnitCommand(unit, sc2::ABILITY_ID::EFFECT_GUARDIANSHIELD);
				}
			}
		}
		
	}break;
	case (sc2::UNIT_TYPEID::PROTOSS_VOIDRAY): {
		if (!unit->orders.empty()) {
			if (unit->orders.front().ability_id == sc2::ABILITY_ID::ATTACK) {
				float distance = std::numeric_limits<float>::max();
				for (const auto& u : enemy_units) {
					float d = Distance2D(u->pos, unit->pos);
					if (d < distance) {
						distance = d;
					}
				}
				if (distance < 8) {
					action_->UnitCommand(unit, sc2::ABILITY_ID::EFFECT_VOIDRAYPRISMATICALIGNMENT);
				}
			}
		}
		break;
	}
	case (sc2::UNIT_TYPEID::PROTOSS_ORACLE): {
		if (!unit->orders.empty()) {
			float distance = std::numeric_limits<float>::max();
			for (const auto& u : enemy_units) {
				float d = Distance2D(u->pos, unit->pos);
				if (d < distance) {
					distance = d;
				}
			}
			if (distance < 6 && unit->energy >= 25) {
				action_->UnitCommand(unit, sc2::ABILITY_ID::BEHAVIOR_PULSARBEAMON);
			}
		}
		break;
	}

	case (sc2::UNIT_TYPEID::PROTOSS_DISRUPTOR): {
		float distance = std::numeric_limits<float>::max();
		sc2::Point2D closest_unit;
		for (const auto& u : enemy_units) {
			float d = Distance2D(u->pos, unit->pos);
			if (d < distance) {
				distance = d;
				closest_unit = u->pos;
			}
		}
		if (distance < 7) {
			action_->UnitCommand(unit, sc2::ABILITY_ID::EFFECT_PURIFICATIONNOVA, closest_unit);
		}
		else {
			action_->UnitCommand(unit, sc2::ABILITY_ID::ATTACK, closest_unit);
		}
		break;
	}
	case (sc2::UNIT_TYPEID::PROTOSS_DISRUPTORPHASED): {
		float distance = std::numeric_limits<float>::max();
		sc2::Point2D closest_unit;
		for (const auto& u : enemy_units) {
			if (u->is_flying) {
				continue;
			}
			float d = DistanceSquared2D(u->pos, unit->pos);
			if (d < distance) {
				distance = d;
				closest_unit = u->pos;
			}
		}
		action_->UnitCommand(unit, sc2::ABILITY_ID::MOVE, closest_unit);
		break;
	}
	default:
		break;

	}
}