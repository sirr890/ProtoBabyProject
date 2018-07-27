#pragma once
#include "BaseAgent.h"
#include "ProbeAgent.h"
#include <stdlib.h> 
#include <cmath>
#include"CoordinatorAgent.h"

struct IsBase {
	bool operator()(const sc2::Unit& unit) {
		switch (unit.unit_type.ToType()) {
		case sc2::UNIT_TYPEID::PROTOSS_NEXUS: return true;
		case sc2::UNIT_TYPEID::PROTOSS_ASSIMILATOR: return true;
		default: return false;
		}
	}
};

enum MINER_PERCEPTIONS {
	MC_STANDBY_ORDER, MC_WORKING_ORDER, 
	MC_NEED_PYLON, MC_BUILD_ORDER, MC_NEED_PROBE, MC_SCOUT_ORDER, MC_DESBALANCE_BASE,
	MC_ADDED_BASE, MC_LOSTED_BASE, MC_NORMAL, MC_2EXPAND, MC_3EXPAND, MC_4EXPAND , MC_TOTAL_PERCEPTIONS
};

enum MINER_ACTIONS
{
	MC_NOTHING, MC_BUILD, MC_CREATE_PROBE, MC_SCOUT, MC_BALANCE_BASE, MC_TOTAL_ACTIONS
};

enum MINER_STATES
{
	MC_STANDBY, 
	MC_ONE_BASE, MC_TWO_BASE, MC_THREE_BASE, MC_FOUR_BASE, MC_TOTAL_STATES
};

class MinerClass : public BaseAgent
{
public:
	int num_nexus;
	float thr_need_pylon;
	float thr_desbalance;
	float thr_expansion;
	int thr_army;
	std::vector<ProbeAgent*> probes;
	int* cooldown;
	
	MinerClass();
	MinerClass(const sc2::ObservationInterface* enviroment, MessageClass* message, sc2::ActionInterface* action_, sc2::QueryInterface* query, const sc2::Unit* unit, std::vector<sc2::Point3D> expantion_points);
	~MinerClass();
	void Decrese();
	void init_state_machine();
	virtual Message Function_Perception(Message msg) override;
	bool MineralDesbalance();
	bool VespenDesbalance();
	bool AssimilatorDesbalance();
	bool NexusDesbalance();
	bool IsNexusAssimilator(sc2::ABILITY_ID type);
	sc2::Units GetMineralProbes();
	sc2::Units GetVespeneProbes();
	void Build_Order(Message m_action);
	void CreateProbe();
	void VespeneBalance();
	void MineralBalance();
	void BaseBalance();
	const sc2::Unit * FindNearestMineralPatch(sc2::Point2D start);
	void ScoutFunction(float target_x, float target_y);
	virtual void Function_Action(Message m_action) override;
	bool TryBuildUnit(sc2::AbilityID ability_type_for_unit, const sc2::Unit* unit);
	int GetExpectedWorkers();
};

