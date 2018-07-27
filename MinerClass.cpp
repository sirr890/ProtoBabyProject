#include "MinerClass.h"



MinerClass::MinerClass()
{
}

MinerClass::MinerClass(const sc2::ObservationInterface * _enviroment, MessageClass * _message, sc2::ActionInterface * _action_, sc2::QueryInterface* query, const sc2::Unit * _unit, std::vector<sc2::Point3D> expantion_points) :BaseAgent(_enviroment, _message, _action_, query, _unit, expantion_points)
{
	fsm = new FSM(MINER_STATES::MC_TOTAL_STATES, MINER_PERCEPTIONS::MC_TOTAL_PERCEPTIONS);
	init_state_machine();
	num_nexus = 1;
	thr_need_pylon = 0.9f;
	thr_desbalance = 3.0f;
	thr_army = 10;
	thr_expansion = 6000;
	cooldown = new int[MINER_PERCEPTIONS::MC_TOTAL_PERCEPTIONS];
	for (int i = 0; i < MINER_PERCEPTIONS::MC_TOTAL_PERCEPTIONS;i++)
		cooldown[i] = 0;
}


MinerClass::~MinerClass()
{
}

void MinerClass::Decrese()
{
	for (int i = 0; i < MINER_PERCEPTIONS::MC_TOTAL_PERCEPTIONS;i++)
		cooldown[i]--;
}

void MinerClass::init_state_machine()
{
	fsm->set_transition(MINER_STATES::MC_ONE_BASE, MINER_PERCEPTIONS::MC_STANDBY_ORDER, transition(MINER_ACTIONS::MC_NOTHING, MINER_STATES::MC_STANDBY));
	fsm->set_transition(MINER_STATES::MC_ONE_BASE, MINER_PERCEPTIONS::MC_NEED_PYLON, transition(MINER_ACTIONS::MC_BUILD, MINER_STATES::MC_ONE_BASE));
	fsm->set_transition(MINER_STATES::MC_ONE_BASE, MINER_PERCEPTIONS::MC_BUILD_ORDER, transition(MINER_ACTIONS::MC_BUILD, MINER_STATES::MC_ONE_BASE));
	fsm->set_transition(MINER_STATES::MC_ONE_BASE, MINER_PERCEPTIONS::MC_NEED_PROBE, transition(MINER_ACTIONS::MC_CREATE_PROBE, MINER_STATES::MC_ONE_BASE));
	fsm->set_transition(MINER_STATES::MC_ONE_BASE, MINER_PERCEPTIONS::MC_SCOUT_ORDER, transition(MINER_ACTIONS::MC_SCOUT, MINER_STATES::MC_ONE_BASE));
	fsm->set_transition(MINER_STATES::MC_ONE_BASE, MINER_PERCEPTIONS::MC_DESBALANCE_BASE, transition(MINER_ACTIONS::MC_BALANCE_BASE, MINER_STATES::MC_ONE_BASE));
	fsm->set_transition(MINER_STATES::MC_ONE_BASE, MINER_PERCEPTIONS::MC_2EXPAND, transition(MINER_ACTIONS::MC_BUILD, MINER_STATES::MC_ONE_BASE));
	fsm->set_transition(MINER_STATES::MC_ONE_BASE, MINER_PERCEPTIONS::MC_ADDED_BASE, transition(MINER_ACTIONS::MC_NOTHING, MINER_STATES::MC_TWO_BASE));
	fsm->set_transition(MINER_STATES::MC_ONE_BASE, MINER_PERCEPTIONS::MC_LOSTED_BASE, transition(MINER_ACTIONS::MC_NOTHING, MINER_STATES::MC_STANDBY));
	fsm->set_transition(MINER_STATES::MC_ONE_BASE, MINER_PERCEPTIONS::MC_WORKING_ORDER, transition(MINER_ACTIONS::MC_NOTHING, MINER_STATES::MC_ONE_BASE));
	fsm->set_transition(MINER_STATES::MC_ONE_BASE, MINER_PERCEPTIONS::MC_NORMAL, transition(MINER_ACTIONS::MC_NOTHING, MINER_STATES::MC_ONE_BASE));

	fsm->set_transition(MINER_STATES::MC_TWO_BASE, MINER_PERCEPTIONS::MC_STANDBY_ORDER, transition(MINER_ACTIONS::MC_NOTHING, MINER_STATES::MC_STANDBY));
	fsm->set_transition(MINER_STATES::MC_TWO_BASE, MINER_PERCEPTIONS::MC_NEED_PYLON, transition(MINER_ACTIONS::MC_BUILD, MINER_STATES::MC_TWO_BASE));
	fsm->set_transition(MINER_STATES::MC_TWO_BASE, MINER_PERCEPTIONS::MC_BUILD_ORDER, transition(MINER_ACTIONS::MC_BUILD, MINER_STATES::MC_TWO_BASE));
	fsm->set_transition(MINER_STATES::MC_TWO_BASE, MINER_PERCEPTIONS::MC_NEED_PROBE, transition(MINER_ACTIONS::MC_CREATE_PROBE, MINER_STATES::MC_TWO_BASE));
	fsm->set_transition(MINER_STATES::MC_TWO_BASE, MINER_PERCEPTIONS::MC_SCOUT_ORDER, transition(MINER_ACTIONS::MC_SCOUT, MINER_STATES::MC_TWO_BASE));
	fsm->set_transition(MINER_STATES::MC_TWO_BASE, MINER_PERCEPTIONS::MC_DESBALANCE_BASE, transition(MINER_ACTIONS::MC_BALANCE_BASE, MINER_STATES::MC_TWO_BASE));
	fsm->set_transition(MINER_STATES::MC_TWO_BASE, MINER_PERCEPTIONS::MC_3EXPAND, transition(MINER_ACTIONS::MC_BUILD, MINER_STATES::MC_TWO_BASE));
	fsm->set_transition(MINER_STATES::MC_TWO_BASE, MINER_PERCEPTIONS::MC_LOSTED_BASE, transition(MINER_ACTIONS::MC_NOTHING, MINER_STATES::MC_ONE_BASE));
	fsm->set_transition(MINER_STATES::MC_TWO_BASE, MINER_PERCEPTIONS::MC_ADDED_BASE, transition(MINER_ACTIONS::MC_NOTHING, MINER_STATES::MC_THREE_BASE));
	fsm->set_transition(MINER_STATES::MC_TWO_BASE, MINER_PERCEPTIONS::MC_WORKING_ORDER, transition(MINER_ACTIONS::MC_NOTHING, MINER_STATES::MC_TWO_BASE));
	fsm->set_transition(MINER_STATES::MC_TWO_BASE, MINER_PERCEPTIONS::MC_NORMAL, transition(MINER_ACTIONS::MC_NOTHING, MINER_STATES::MC_TWO_BASE));


	fsm->set_transition(MINER_STATES::MC_THREE_BASE, MINER_PERCEPTIONS::MC_STANDBY_ORDER, transition(MINER_ACTIONS::MC_NOTHING, MINER_STATES::MC_STANDBY));
	fsm->set_transition(MINER_STATES::MC_THREE_BASE, MINER_PERCEPTIONS::MC_NEED_PYLON, transition(MINER_ACTIONS::MC_BUILD, MINER_STATES::MC_THREE_BASE));
	fsm->set_transition(MINER_STATES::MC_THREE_BASE, MINER_PERCEPTIONS::MC_BUILD_ORDER, transition(MINER_ACTIONS::MC_BUILD, MINER_STATES::MC_THREE_BASE));
	fsm->set_transition(MINER_STATES::MC_THREE_BASE, MINER_PERCEPTIONS::MC_NEED_PROBE, transition(MINER_ACTIONS::MC_CREATE_PROBE, MINER_STATES::MC_THREE_BASE));
	fsm->set_transition(MINER_STATES::MC_THREE_BASE, MINER_PERCEPTIONS::MC_SCOUT_ORDER, transition(MINER_ACTIONS::MC_SCOUT, MINER_STATES::MC_THREE_BASE));
	fsm->set_transition(MINER_STATES::MC_THREE_BASE, MINER_PERCEPTIONS::MC_DESBALANCE_BASE, transition(MINER_ACTIONS::MC_BALANCE_BASE, MINER_STATES::MC_THREE_BASE));
	fsm->set_transition(MINER_STATES::MC_THREE_BASE, MINER_PERCEPTIONS::MC_4EXPAND, transition(MINER_ACTIONS::MC_BUILD, MINER_STATES::MC_THREE_BASE));
	fsm->set_transition(MINER_STATES::MC_THREE_BASE, MINER_PERCEPTIONS::MC_LOSTED_BASE, transition(MINER_ACTIONS::MC_NOTHING, MINER_STATES::MC_TWO_BASE));
	fsm->set_transition(MINER_STATES::MC_THREE_BASE, MINER_PERCEPTIONS::MC_ADDED_BASE, transition(MINER_ACTIONS::MC_NOTHING, MINER_STATES::MC_FOUR_BASE));
	fsm->set_transition(MINER_STATES::MC_THREE_BASE, MINER_PERCEPTIONS::MC_WORKING_ORDER, transition(MINER_ACTIONS::MC_NOTHING, MINER_STATES::MC_THREE_BASE));
	fsm->set_transition(MINER_STATES::MC_THREE_BASE, MINER_PERCEPTIONS::MC_NORMAL, transition(MINER_ACTIONS::MC_NOTHING, MINER_STATES::MC_THREE_BASE));

	fsm->set_transition(MINER_STATES::MC_FOUR_BASE, MINER_PERCEPTIONS::MC_STANDBY_ORDER, transition(MINER_ACTIONS::MC_NOTHING, MINER_STATES::MC_STANDBY));
	fsm->set_transition(MINER_STATES::MC_FOUR_BASE, MINER_PERCEPTIONS::MC_NEED_PYLON, transition(MINER_ACTIONS::MC_BUILD, MINER_STATES::MC_FOUR_BASE));
	fsm->set_transition(MINER_STATES::MC_FOUR_BASE, MINER_PERCEPTIONS::MC_BUILD_ORDER, transition(MINER_ACTIONS::MC_BUILD, MINER_STATES::MC_FOUR_BASE));
	fsm->set_transition(MINER_STATES::MC_FOUR_BASE, MINER_PERCEPTIONS::MC_NEED_PROBE, transition(MINER_ACTIONS::MC_CREATE_PROBE, MINER_STATES::MC_FOUR_BASE));
	fsm->set_transition(MINER_STATES::MC_FOUR_BASE, MINER_PERCEPTIONS::MC_SCOUT_ORDER, transition(MINER_ACTIONS::MC_SCOUT, MINER_STATES::MC_FOUR_BASE));
	fsm->set_transition(MINER_STATES::MC_FOUR_BASE, MINER_PERCEPTIONS::MC_DESBALANCE_BASE, transition(MINER_ACTIONS::MC_BALANCE_BASE, MINER_STATES::MC_FOUR_BASE));
	fsm->set_transition(MINER_STATES::MC_FOUR_BASE, MINER_PERCEPTIONS::MC_LOSTED_BASE, transition(MINER_ACTIONS::MC_NOTHING, MINER_STATES::MC_THREE_BASE));
	fsm->set_transition(MINER_STATES::MC_FOUR_BASE, MINER_PERCEPTIONS::MC_ADDED_BASE, transition(MINER_ACTIONS::MC_NOTHING, MINER_STATES::MC_FOUR_BASE));
	fsm->set_transition(MINER_STATES::MC_FOUR_BASE, MINER_PERCEPTIONS::MC_WORKING_ORDER, transition(MINER_ACTIONS::MC_NOTHING, MINER_STATES::MC_FOUR_BASE));
	fsm->set_transition(MINER_STATES::MC_FOUR_BASE, MINER_PERCEPTIONS::MC_NORMAL, transition(MINER_ACTIONS::MC_NOTHING, MINER_STATES::MC_FOUR_BASE));

	fsm->set_transition(MINER_STATES::MC_STANDBY, MINER_PERCEPTIONS::MC_WORKING_ORDER, transition(MINER_ACTIONS::MC_NOTHING, MINER_STATES::MC_ONE_BASE));
	fsm->set_transition(MINER_STATES::MC_STANDBY, MINER_PERCEPTIONS::MC_BUILD_ORDER, transition(MINER_ACTIONS::MC_BUILD, MINER_STATES::MC_STANDBY));
	fsm->set_transition(MINER_STATES::MC_STANDBY, MINER_PERCEPTIONS::MC_SCOUT_ORDER, transition(MINER_ACTIONS::MC_SCOUT, MINER_STATES::MC_STANDBY));
	fsm->set_transition(MINER_STATES::MC_STANDBY, MINER_PERCEPTIONS::MC_STANDBY_ORDER, transition(MINER_ACTIONS::MC_NOTHING, MINER_STATES::MC_STANDBY));
	fsm->set_transition(MINER_STATES::MC_STANDBY, MINER_PERCEPTIONS::MC_NORMAL, transition(MINER_ACTIONS::MC_NOTHING, MINER_STATES::MC_STANDBY));

	fsm->current_state = MINER_STATES::MC_ONE_BASE;
}

Message MinerClass::Function_Perception(Message msg)
{
	int expWork = GetExpectedWorkers();
	int foodWork = enviroment->GetFoodWorkers();
	Decrese();
	if (msg["type"] == "request")
	{
		if (msg["content"]["type_request"] == "standby")
		{
			return	{
				{ "perception_id", MINER_PERCEPTIONS::MC_STANDBY_ORDER }
			};
		}
		if (msg["content"]["type_request"] == "working")
		{
			return	{
				{ "perception_id", MINER_PERCEPTIONS::MC_WORKING_ORDER }
			};
		}
		if (probes.size()>0 && msg["content"]["type_request"] == "build")
		{
			return	{
				{ "perception_id", MINER_PERCEPTIONS::MC_BUILD_ORDER },
				{ "structure",msg["content"]["structure"] },
				{ "target_x",msg["content"]["target_x"] },
				{ "target_y",msg["content"]["target_y"] }
			};
		}
		if (probes.size()>0 && msg["content"]["type_request"] == "scout")
		{
			return	{
				{ "perception_id", MINER_PERCEPTIONS::MC_SCOUT_ORDER },
				{ "target_x",msg["content"]["target_x"] },
				{ "target_y",msg["content"]["target_y"] }
			};
		}

	}
	if (cooldown[MINER_PERCEPTIONS::MC_NEED_PYLON] <= 0 && enviroment->GetFoodUsed() >= enviroment->GetFoodCap()*thr_need_pylon && enviroment->GetMinerals() > 100)
	{
		cooldown[MINER_PERCEPTIONS::MC_NEED_PYLON] = 300;
		return	{
			{ "perception_id", MINER_PERCEPTIONS::MC_NEED_PYLON },
		{"structure",sc2::ABILITY_ID::BUILD_PYLON},
		{ "target_x",-1 },{ "target_y",-1 }
		};
	}
	sc2::Units bases = enviroment->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnit(sc2::UNIT_TYPEID::PROTOSS_NEXUS));
	if (bases.size() < num_nexus)
	{
		num_nexus--;
		return	{
			{ "perception_id", MINER_PERCEPTIONS::MC_LOSTED_BASE}
		};
	}
	if (bases.size() > num_nexus)
	{
		num_nexus++;
		return	{
			{ "perception_id", MINER_PERCEPTIONS::MC_ADDED_BASE }
		};
	}
	//need probe
	if (cooldown[MINER_PERCEPTIONS::MC_NEED_PROBE] <= 0 && enviroment->GetMinerals() >= 50 && expWork > foodWork) {
		cooldown[MINER_PERCEPTIONS::MC_NEED_PROBE] = 100;
		return	{
			{ "perception_id", MINER_PERCEPTIONS::MC_NEED_PROBE }
		};
	}

	//expansions
	if (cooldown[MINER_PERCEPTIONS::MC_2EXPAND] <= 0 && ((enviroment->GetArmyCount() > thr_army) || 
		enviroment->GetGameLoop()>thr_expansion)
		&& fsm->current_state == MINER_STATES::MC_ONE_BASE)
	{
		cooldown[MINER_PERCEPTIONS::MC_2EXPAND] = 200;
		return	{
			{ "perception_id", MINER_PERCEPTIONS::MC_2EXPAND },
		{ "structure",sc2::ABILITY_ID::BUILD_NEXUS },
		{ "target_x",-1 },{ "target_y",-1 }
		};
	}

	if (cooldown[MINER_PERCEPTIONS::MC_3EXPAND] <= 0 && ((enviroment->GetArmyCount() > thr_army*2) ||
		enviroment->GetGameLoop()>thr_expansion*2)
		&&	fsm->current_state == MINER_STATES::MC_TWO_BASE)
	{
		cooldown[MINER_PERCEPTIONS::MC_3EXPAND] = 200;
		return	{
			{ "perception_id", MINER_PERCEPTIONS::MC_3EXPAND },{ "structure",sc2::ABILITY_ID::BUILD_NEXUS },
		{ "target_x",-1 },{ "target_y",-1 }
		};
	}
	if (cooldown[MINER_PERCEPTIONS::MC_4EXPAND] <= 0 && ((enviroment->GetArmyCount() > thr_army*3) ||
		enviroment->GetGameLoop()>thr_expansion*3) && fsm->current_state == MINER_STATES::MC_THREE_BASE)
	{
		cooldown[MINER_PERCEPTIONS::MC_4EXPAND] = 10000000000000;
		return	{
			{ "perception_id", MINER_PERCEPTIONS::MC_4EXPAND },{ "structure",sc2::ABILITY_ID::BUILD_NEXUS },
		{ "target_x",-1 },{ "target_y",-1 }
		};
	}

	//desbalance vespene
	if (cooldown[MINER_PERCEPTIONS::MC_DESBALANCE_BASE] <= 0 && AssimilatorDesbalance())
	{
		cooldown[MINER_PERCEPTIONS::MC_DESBALANCE_BASE] = 100;
		return	{
			{ "perception_id", MINER_PERCEPTIONS::MC_DESBALANCE_BASE },
		{ "desbalance","assimilator" }
		};
	}
	if (cooldown[MINER_PERCEPTIONS::MC_DESBALANCE_BASE] <= 0 && NexusDesbalance())
	{
		cooldown[MINER_PERCEPTIONS::MC_DESBALANCE_BASE] = 100;
		return	{
			{ "perception_id", MINER_PERCEPTIONS::MC_DESBALANCE_BASE },
		{ "desbalance","nexus" }
		};
	}

	if (cooldown[MINER_PERCEPTIONS::MC_DESBALANCE_BASE] <= 0 && VespenDesbalance())
	{
		cooldown[MINER_PERCEPTIONS::MC_DESBALANCE_BASE] = 100;
		return	{
			{ "perception_id", MINER_PERCEPTIONS::MC_DESBALANCE_BASE },
		{ "desbalance","vespene" }
		};
	}

	if (cooldown[MINER_PERCEPTIONS::MC_DESBALANCE_BASE] <= 0 && MineralDesbalance())
	{
		cooldown[MINER_PERCEPTIONS::MC_DESBALANCE_BASE] = 100;
		return	{
			{ "perception_id", MINER_PERCEPTIONS::MC_DESBALANCE_BASE },
		{ "desbalance","minerals" }
		};
	}

	return	{
		{ "perception_id", MINER_PERCEPTIONS::MC_NORMAL }
	};
}

void MinerClass::Function_Action(Message m_action)
{
	switch ((action)m_action["action_id"]) {
	case MINER_ACTIONS::MC_BUILD:
	{
		Build_Order(m_action);
		break;
	}
	case MINER_ACTIONS::MC_CREATE_PROBE:
	{
		CreateProbe();
		break;
	}
	case MINER_ACTIONS::MC_SCOUT:
	{
		ScoutFunction(m_action["target_x"], (float)m_action["target_y"]);
		break;
	}
	case MINER_ACTIONS::MC_BALANCE_BASE:
	{
		if (m_action["desbalance"] == "vespene")
		{
			VespeneBalance();
			break;
		}
		if (m_action["desbalance"] == "minerals")
		{
			MineralBalance();
			break;
		}
		if (m_action["desbalance"] == "assimilator" || m_action["desbalance"] == "nexus")
		{
			BaseBalance();
			break;
		}
	}
	default:
		break;
	}
}

bool MinerClass::MineralDesbalance()
{
	sc2::Units nexus = enviroment->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnit(sc2::UNIT_TYPEID::PROTOSS_NEXUS));
	bool flag = false;
	for (int i = 0; i < nexus.size(); i++)
	{
		if (nexus[i]->ideal_harvesters > nexus[i]->assigned_harvesters)
		{
			flag = true;
			break;
		}
	}
	return (enviroment->GetVespene() / (1 + enviroment->GetMinerals())) > thr_desbalance && flag;
}

bool MinerClass::VespenDesbalance()
{
	sc2::Units assimilators = enviroment->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnit(sc2::UNIT_TYPEID::PROTOSS_ASSIMILATOR));
	bool flag = false;
	for (int i = 0; i < assimilators.size(); i++)
	{
		if (assimilators[i]->ideal_harvesters > assimilators[i]->assigned_harvesters)
		{
			flag = true;
			break;
		}
	}
	return (enviroment->GetMinerals() / (1 + enviroment->GetVespene())) > thr_desbalance && assimilators.size() > 0 && flag;
}

bool MinerClass::AssimilatorDesbalance()
{
	sc2::Units assimilators = enviroment->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnit(sc2::UNIT_TYPEID::PROTOSS_ASSIMILATOR));
	sc2::Units allbases = enviroment->GetUnits(sc2::Unit::Alliance::Self, IsBase());
	for (const auto& assm : assimilators)
	{
		if (assm->assigned_harvesters > assm->ideal_harvesters)
		{
			for (const auto& base : allbases)
			{
				if (base->assigned_harvesters < base->ideal_harvesters)
					return true;
			}
		}
	}
	return false;
}

bool MinerClass::NexusDesbalance()
{
	sc2::Units nexus = enviroment->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnit(sc2::UNIT_TYPEID::PROTOSS_NEXUS));
	sc2::Units allbases = enviroment->GetUnits(sc2::Unit::Alliance::Self, IsBase());
	for (const auto& nx : nexus)
	{
		if (nx->assigned_harvesters > nx->ideal_harvesters)
		{
			for (const auto& base : allbases)
			{
				if (base->assigned_harvesters < base->ideal_harvesters)
					return true;
			}
		}
	}
	return false;
}

bool MinerClass::IsNexusAssimilator(sc2::ABILITY_ID type)
{
	if (type == sc2::ABILITY_ID::BUILD_NEXUS)
		return true;
	if (type == sc2::ABILITY_ID::BUILD_PYLON)
		return true;
	if (type == sc2::ABILITY_ID::BUILD_ASSIMILATOR)
		return true;
	return false;
}

sc2::Units MinerClass::GetMineralProbes()
{
	sc2::Units result;
	for (const auto& worker : probes)
	{
		if (!worker->vespene)
		{
			result.push_back(worker->unit);
		}
	}
	return result;
}

sc2::Units MinerClass::GetVespeneProbes()
{
	sc2::Units result;
	for (const auto& worker : probes)
	{
		if (worker->vespene)
		{
			result.push_back(worker->unit);
		}
	}
	return result;
}


void MinerClass::Build_Order(Message m_action)
{
	size_t min = 1;

	sc2::Units probes_mineral = GetMineralProbes();
	sc2::Units probes= enviroment->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnit(sc2::UNIT_TYPEID::PROTOSS_PROBE));
	size_t max = probes_mineral.size();
	if (probes.size() < 1)
		return;

	if (max == 0) {
		probes_mineral = enviroment->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnit(sc2::UNIT_TYPEID::PROTOSS_PROBE));
		max = probes_mineral.size();
	}

	size_t num_rand = (min + (rand() % (size_t)(max - min + 1)));
	sc2::Units power = enviroment->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnit(sc2::UNIT_TYPEID::PROTOSS_PYLON));
	if (power.size() < 1 && !IsNexusAssimilator((sc2::ABILITY_ID) m_action["structure"]))
	{
		message->PushMessage({
			{ "type","request" },
			{ "content",{ { "type_request","build" },{ "structure",sc2::ABILITY_ID::BUILD_PYLON },
			{ "target_x",-1 },{ "target_y",-1 } } },{ "receiver",probes_mineral[num_rand - 1]->tag }
			});
	}
	message->PushMessage({
		{ "type","request" },
		{ "content",{ { "type_request","build" },{ "structure",m_action["structure"] },{ "target_x",m_action["target_x"] },
		{ "target_y",m_action["target_y"] } } },{ "receiver",probes_mineral[num_rand - 1]->tag }
		});
}

void MinerClass::CreateProbe()
{
	sc2::Units nexus = enviroment->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnit(sc2::UNIT_TYPEID::PROTOSS_NEXUS));
	for (int i = 0; i < nexus.size();i++)
	{
		int total = abs(nexus[i]->ideal_harvesters - nexus[i]->assigned_harvesters);
		if (total > 0)
		{
			TryBuildUnit(sc2::ABILITY_ID::TRAIN_PROBE, nexus[i]);
		}
	}
}

void MinerClass::VespeneBalance()
{
	sc2::Units assimilator = enviroment->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnit(sc2::UNIT_TYPEID::PROTOSS_ASSIMILATOR));
	sc2::Units probes_mineral = GetMineralProbes();
	for (int i = 0; i < assimilator.size(); i++)
	{
		if (assimilator[i]->assigned_harvesters < assimilator[i]->ideal_harvesters && assimilator[i]->build_progress == 1.0 && probes_mineral.size() > 0)
		{
			message->PushMessage(
				{
					{ "type","request" },
				{ "content",{ { "type_request","vespene" },{ "target_x",assimilator[i]->pos.x},
				{ "target_y",assimilator[i]->pos.y} } },
				{ "receiver",probes_mineral[probes_mineral.size() - 1]->tag }
				}
			);
			probes_mineral.pop_back();
			break;
		}
	}
}

void MinerClass::MineralBalance() {
	sc2::Units nexus = enviroment->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnit(sc2::UNIT_TYPEID::PROTOSS_NEXUS));
	sc2::Units probes_mineral = GetVespeneProbes();
	for (int i = 0; i < nexus.size(); i++)
	{
		if (nexus[i]->assigned_harvesters < nexus[i]->ideal_harvesters && nexus[i]->build_progress == 1.0 && probes_mineral.size() > 0)
		{
			const sc2::Unit* u = FindNearestMineralPatch(nexus[i]->pos);
			message->PushMessage(
				{
					{ "type","request" },
				{ "content",{ { "type_request","vespene" },{ "target_x",u->pos.x },
				{ "target_y",u->pos.y } } },
				{ "receiver",probes_mineral[probes_mineral.size() - 1]->tag }
				}
			);
			probes_mineral.pop_back();
			break;
		}
	}
}

void MinerClass::BaseBalance()
{
	sc2::Units all_bases = enviroment->GetUnits(sc2::Unit::Alliance::Self, IsBase());
	for (const auto&base : all_bases)
	{
		if (base->assigned_harvesters <= base->ideal_harvesters)
			continue;		
		for (const auto& worker : probes) {
			if(worker->unit->orders.size()>0 && worker->unit->orders.front().target_unit_tag == base->tag)
			{
				for (const auto& new_base : all_bases)
				{
					if (new_base->ideal_harvesters > new_base->assigned_harvesters) {
						message->PushMessage(
							{
								{ "type","request" },
							{ "content",{ { "type_request","vespene" },{ "target_x",new_base->pos.x },{ "target_y",new_base->pos.y } } },
							{ "receiver",worker->unit->tag }
							}
						);
						return;
					}
				}
			}
		}
	}
}

const sc2::Unit* MinerClass::FindNearestMineralPatch(sc2::Point2D start) {
	sc2::Units nexus = enviroment->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnit(sc2::UNIT_TYPEID::PROTOSS_NEXUS));
	sc2::Units mineral = enviroment->GetUnits(sc2::Unit::Alliance::Neutral, IsMinieral());
	float distance = std::numeric_limits<float>::max();
	const sc2::Unit* target = nullptr;
	if (nexus.empty()) {
		return mineral[0];
	}
	for (const auto& base : nexus) {
		if (base->ideal_harvesters == 0 || base->build_progress != 1) {
			continue;
		}
		//if (base->assigned_harvesters < base->ideal_harvesters) {
		for (const auto& u : mineral) {
			float d = DistanceSquared2D(u->pos, base->pos);
			if (d < distance) {
				distance = d;
				target = u;
			}
		}
		//}
	}
	if (distance == std::numeric_limits<float>::max()) {
		target = mineral[0];
	}
	return target;
}

void MinerClass::ScoutFunction(float target_x, float target_y)
{
	/*size_t min = 1;
	size_t max = probes.size();
	size_t num_rand = (min + (rand() % (size_t)(max - min + 1)));*/
	message->PushMessage(
		{
			{ "type","request" },
		{ "content",{ { "type_request","scout" },{ "target_x",  target_x },{ "target_y",target_y } } },
		{ "receiver",probes[0]->unit->tag }
		}
	);
}

bool MinerClass::TryBuildUnit(sc2::AbilityID ability_type_for_unit, const sc2::Unit* unit) {
	if (!unit->orders.empty()) {
		return false;
	}

	if (unit->build_progress != 1) {
		return false;
	}

	action_->UnitCommand(unit, ability_type_for_unit);
	return true;
}

int MinerClass::GetExpectedWorkers() {
	sc2::Units bases = enviroment->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnit(sc2::UNIT_TYPEID::PROTOSS_NEXUS));
	sc2::Units geysers = enviroment->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnit(sc2::UNIT_TYPEID::PROTOSS_ASSIMILATOR));
	int expected_workers = 0;
	for (const auto& base : bases) {
		if (base->build_progress != 1) {
			continue;
		}
		expected_workers += base->ideal_harvesters;
	}

	for (const auto& geyser : geysers) {
		if (geyser->vespene_contents > 0) {
			if (geyser->build_progress != 1) {
				continue;
			}
			expected_workers += geyser->ideal_harvesters;
		}
	}

	return expected_workers;
}