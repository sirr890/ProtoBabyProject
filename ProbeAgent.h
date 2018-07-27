#pragma once
#include "BaseAgent.h"
#include <limits>
#undef max

struct IsResource {
	bool operator()(const sc2::Unit& unit) {
		switch (unit.unit_type.ToType()) {
		case sc2::UNIT_TYPEID::NEUTRAL_BATTLESTATIONMINERALFIELD: return true;
		case sc2::UNIT_TYPEID::NEUTRAL_BATTLESTATIONMINERALFIELD750: return true;
		case sc2::UNIT_TYPEID::NEUTRAL_LABMINERALFIELD: return true;
		case sc2::UNIT_TYPEID::NEUTRAL_LABMINERALFIELD750: return true;
		case sc2::UNIT_TYPEID::NEUTRAL_MINERALFIELD: return true;
		case sc2::UNIT_TYPEID::NEUTRAL_MINERALFIELD750: return true;
		case sc2::UNIT_TYPEID::NEUTRAL_PURIFIERMINERALFIELD: return true;
		case sc2::UNIT_TYPEID::NEUTRAL_PURIFIERMINERALFIELD750: return true;
		case sc2::UNIT_TYPEID::NEUTRAL_PURIFIERRICHMINERALFIELD: return true;
		case sc2::UNIT_TYPEID::NEUTRAL_PURIFIERRICHMINERALFIELD750: return true;
		case sc2::UNIT_TYPEID::NEUTRAL_RICHMINERALFIELD: return true;
		case sc2::UNIT_TYPEID::NEUTRAL_RICHMINERALFIELD750: return true;
		case sc2::UNIT_TYPEID::NEUTRAL_VESPENEGEYSER: return true;
		case sc2::UNIT_TYPEID::NEUTRAL_SPACEPLATFORMGEYSER: return true;
		case sc2::UNIT_TYPEID::NEUTRAL_PROTOSSVESPENEGEYSER: return true;
		case sc2::UNIT_TYPEID::NEUTRAL_PURIFIERVESPENEGEYSER: return true;
		case sc2::UNIT_TYPEID::NEUTRAL_RICHVESPENEGEYSER: return true;
		case sc2::UNIT_TYPEID::NEUTRAL_SHAKURASVESPENEGEYSER: return true;
		default: return false;
		}
	}
};

struct IsTownHall {
	bool operator()(const sc2::Unit& unit) {
		switch (unit.unit_type.ToType()) {
		case sc2::UNIT_TYPEID::PROTOSS_NEXUS: return true;
		default: return false;
		}
	}
};

struct IsVespeneGeyser {
	bool operator()(const sc2::Unit& unit) {
		switch (unit.unit_type.ToType()) {
		case sc2::UNIT_TYPEID::NEUTRAL_VESPENEGEYSER: return true;
		case sc2::UNIT_TYPEID::NEUTRAL_SPACEPLATFORMGEYSER: return true;
		case sc2::UNIT_TYPEID::NEUTRAL_PROTOSSVESPENEGEYSER: return true;
		case sc2::UNIT_TYPEID::NEUTRAL_PURIFIERVESPENEGEYSER: return true;
		case sc2::UNIT_TYPEID::NEUTRAL_RICHVESPENEGEYSER: return true;
		case sc2::UNIT_TYPEID::NEUTRAL_SHAKURASVESPENEGEYSER: return true;
		default: return false;
		}
	}
};

struct IsMinieral {
	bool operator()(const sc2::Unit& unit) {
		switch (unit.unit_type.ToType()) {
		case sc2::UNIT_TYPEID::NEUTRAL_BATTLESTATIONMINERALFIELD: return true;
		case sc2::UNIT_TYPEID::NEUTRAL_BATTLESTATIONMINERALFIELD750: return true;
		case sc2::UNIT_TYPEID::NEUTRAL_LABMINERALFIELD: return true;
		case sc2::UNIT_TYPEID::NEUTRAL_LABMINERALFIELD750: return true;
		case sc2::UNIT_TYPEID::NEUTRAL_MINERALFIELD: return true;
		case sc2::UNIT_TYPEID::NEUTRAL_MINERALFIELD750: return true;
		case sc2::UNIT_TYPEID::NEUTRAL_PURIFIERMINERALFIELD: return true;
		case sc2::UNIT_TYPEID::NEUTRAL_PURIFIERMINERALFIELD750: return true;
		case sc2::UNIT_TYPEID::NEUTRAL_PURIFIERRICHMINERALFIELD: return true;
		case sc2::UNIT_TYPEID::NEUTRAL_PURIFIERRICHMINERALFIELD750: return true;
		case sc2::UNIT_TYPEID::NEUTRAL_RICHMINERALFIELD: return true;
		case sc2::UNIT_TYPEID::NEUTRAL_RICHMINERALFIELD750: return true;
		default: return false;
		}
	}
};

enum PROBE_PERCEPTIONS
{
ATTACKED, BUILD_ORDER, NORMAL,SCOUT_ORDER,IDLE,VESPENE_ORDER, TOTAL_PERCEPTIONS
};

enum PROBE_ACTIONS {
	GATHER, BUILD, MOVE, NOTHING, TOTAL_ACTIONS
};

enum PROBE_STATE {
	WORKING, BUILD_COMPLETE, BUILDING, SCOUTING, RUNNING, TOTAL_STATES
};

class ProbeAgent : public BaseAgent
{
public:
	bool vespene;
	sc2::Tag target_tag;
	bool building_assimilator;
	sc2::Point2D position;
	
	ProbeAgent();
	~ProbeAgent();
	ProbeAgent(const sc2::ObservationInterface* enviroment,MessageClass* message,sc2::ActionInterface* action_, sc2::QueryInterface* query,const sc2::Unit* unit, std::vector<sc2::Point3D> expantion_points);
	void init_state_machine();
	
	const sc2::Unit * FindNearestMineralPatch(sc2::Point2D start, const sc2::Unit* &target_structure);
	const sc2::Unit * FindNearestNexus(sc2::Point2D start);
	void GatherFunction(float target_x, float target_y);

	const sc2::Unit * FindNearestEnemy(sc2::Point2D start);

	void Build_Structure(sc2::ABILITY_ID id, float target_x, float target_y);

	bool TryBuildAssimilator(float target_x, float target_y);

	bool TryBuildGas(sc2::AbilityID build_ability, sc2::UnitTypeID worker_type, sc2::Point2D base_location);

	sc2::Tag get_geyser(sc2::AbilityID build_ability, sc2::Point2D base_location);

	bool TryBuildStructure(sc2::AbilityID ability_type_for_structure, sc2::UnitTypeID unit_type, sc2::Point2D location, const sc2::Unit* target);

	bool TryBuildPylon(float target_x, float target_y);

	bool TryBuildNexus(float target_x, float target_y);

	bool TryExpand(sc2::AbilityID build_ability, sc2::UnitTypeID worker_type);

	bool TryBuildStructureNearPylon(sc2::AbilityID ability_type_for_structure, float target_x, float target_y);
	
	virtual Message Function_Perception(Message msg) override;
	virtual void Function_Action(Message m_action) override;
	sc2::Point2D Running_Function(sc2::Point2D point);
};

