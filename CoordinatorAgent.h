#pragma once
#include "BaseAgent.h"
#include "sc2api/sc2_api.h"
#include "MinerClass.h"

enum AGENT_TAG{COORDINADOR, MINER, WARGENERAL};

class CoordinatorAgent
{
public:
	const sc2::ObservationInterface* enviroment;
	MessageClass* message;
	sc2::ActionInterface* action;
	sc2::QueryInterface* query;
	const sc2::Unit* unit;
	bool flags;
	std::vector<sc2::Point3D> expantion_points;
	std::vector<sc2::Point3D> enemy_points;

	CoordinatorAgent();
	CoordinatorAgent(const sc2::ObservationInterface* enviroment_, MessageClass* message_, sc2::ActionInterface* action_,
		sc2::QueryInterface* query_, const sc2::Unit* unit_, std::vector<sc2::Point3D> expantion_points_,
		std::vector<sc2::Point3D> enemy_points_);
	~CoordinatorAgent();
	size_t CountUnitType(const sc2::ObservationInterface * observation, sc2::UnitTypeID unit_type);
	bool GetRandomUnit(const sc2::Unit *& unit_out, const sc2::ObservationInterface * observation, sc2::UnitTypeID unit_type);
	bool TryBuildUnit(sc2::AbilityID ability_type_for_unit, sc2::UnitTypeID unit_type);
	void ManageUpgrades();
	void BuildOrder();
	void execute();
};

