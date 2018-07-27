#pragma once
#include "BaseAgent.h"
#include <limits>

#undef max

enum SOLDIER_PERCEPTIONS {
	SB_SCOUT_ORDER, SB_ATTACK_ORDER, SB_WAIT_ORDER, SB_NORMAL, SB_IDLE, SB_MOVE_ORDER, SB_LOW_SHIELD, SB_NONE_ENEMY, SB_TOTAL_PERCEPTIONS
};

enum SOLDIER_ACTIONS {
	SB_ATTACK, SB_NOTHING, SB_MOVE, SB_SCOUT, SB_HEALT, SB_TOTAL_ACTIONS
};

enum SOLDIER_STATES
{
	SB_WAIT, SB_ATTACKING, SB_TOTAL_STATES
};


struct IsArmy {
	bool operator()(const sc2::Unit& unit) {
		switch (unit.unit_type.ToType()) {
		case sc2::UNIT_TYPEID::ZERG_OVERLORD: return false;
		case sc2::UNIT_TYPEID::PROTOSS_PROBE: return false;
		case sc2::UNIT_TYPEID::ZERG_DRONE: return false;
		case sc2::UNIT_TYPEID::TERRAN_SCV: return false;
		case sc2::UNIT_TYPEID::ZERG_QUEEN: return false;
		case sc2::UNIT_TYPEID::ZERG_LARVA: return false;
		case sc2::UNIT_TYPEID::ZERG_EGG: return false;
		case sc2::UNIT_TYPEID::TERRAN_MULE: return false;
		case sc2::UNIT_TYPEID::TERRAN_NUKE: return false;
		default: return true;
		}
	}
};

struct Sight {
	int getSight(const sc2::Unit* unit) {
		switch (unit->unit_type.ToType()) {
		case sc2::UNIT_TYPEID::PROTOSS_ZEALOT: return 9;
		case sc2::UNIT_TYPEID::PROTOSS_VOIDRAY: return 10;
		default: return -1;
		}
	}
};

class SoldadoBot :	public BaseAgent
{
public:
	sc2::Point2D pos_init;
	float thr_min_dist;
	int sight_;
	sc2::Point2D poisition;
	SoldadoBot();
	~SoldadoBot();
	SoldadoBot(const sc2::ObservationInterface* enviroment, MessageClass* message, sc2::ActionInterface* action_, sc2::QueryInterface* query, const sc2::Unit* unit, std::vector<sc2::Point3D> expantion_points);
	void init_state_machine();

	virtual Message Function_Perception(Message msg) override;
	bool LowShield();
	bool IdleUnit();
	virtual void Function_Action(Message m_action) override;

	void HealtFunction();

	void UnitAttack(sc2::Point2D target);
};

