#pragma once
#include "BaseAgent.h"
#include "SoldadoBot.h"
#include <limits>
#undef max

struct IsArmyStructure {
	bool operator()(const sc2::Unit& unit) {
		switch (unit.unit_type.ToType()) {
		case sc2::UNIT_TYPEID::PROTOSS_GATEWAY: return true;
		case sc2::UNIT_TYPEID::PROTOSS_WARPGATE: return true;
		case sc2::UNIT_TYPEID::PROTOSS_STARGATE: return true;
		case sc2::UNIT_TYPEID::PROTOSS_ROBOTICSFACILITY: return true;
		default: return false;
		}
	}
};

struct IsAirStructure {
	bool operator()(const sc2::Unit& unit) {
		switch (unit.unit_type.ToType()) {
		case sc2::UNIT_TYPEID::PROTOSS_CARRIER: return true;
		case sc2::UNIT_TYPEID::PROTOSS_VOIDRAY: return true;
		case sc2::UNIT_TYPEID::PROTOSS_MOTHERSHIP: return true;
		case sc2::UNIT_TYPEID::PROTOSS_MOTHERSHIPCORE: return true;
		default: return false;
		}
	}
};

struct IsNexusPylon {
	bool operator()(const sc2::Unit& unit) {
		switch (unit.unit_type.ToType()) {
		case sc2::UNIT_TYPEID::PROTOSS_NEXUS: return true;
		case sc2::UNIT_TYPEID::PROTOSS_PYLON: return true;
		default: return false;
		}
	}
};

struct IsEnemyBase {
	bool operator()(const sc2::Unit& unit) {
		switch (unit.unit_type.ToType()) {
		case sc2::UNIT_TYPEID::ZERG_HATCHERY: return true;
		case sc2::UNIT_TYPEID::ZERG_LAIR: return true;
		case sc2::UNIT_TYPEID::ZERG_HIVE: return true;
		case sc2::UNIT_TYPEID::TERRAN_COMMANDCENTER: return true;
		case sc2::UNIT_TYPEID::TERRAN_ORBITALCOMMAND: return true;
		case sc2::UNIT_TYPEID::TERRAN_ORBITALCOMMANDFLYING: return true;
		case sc2::UNIT_TYPEID::TERRAN_PLANETARYFORTRESS: return true;
		case sc2::UNIT_TYPEID::PROTOSS_NEXUS: return true;
		default: return false;
		}
	}
};

enum GENERAL_PERCEPTIONS {
	GP_CLOSED_GATEWAY, GP_NEED_ARMY, GP_IDLE_ARMY, GP_LARGE_ARMY, GP_BASE_ATTACKED, GP_SCOUT_ORDER,
	GP_NORMAL, GP_NONE_ENEMY_VISIBLE, GP_BASEOK_LARGEARMY, GP_BASEOK_SMALLARMY,GP_ENEMYBASE, GP_TOTAL_PERCEPTIONS
};

enum GENERAL_ACTIONS
{
	GP_OPEN_GATEWAY, GP_CREATE_SOLDIER, GP_ATTACK, GP_DEFEND_BASE, GP_GROUP_ARMY, GP_SCOUT, GP_NOTHING, GP_UPDATE_ENEMYBASE, GP_TOTAL_ACTIONS
};

enum GENERAL_STATES
{
	GP_PREPARING_ARMY, GP_ATTACKING, GP_DEFENDING, GP_TOTAL_STATES
};

class WarGeneral :	public BaseAgent
{
public:
	sc2::Point3D staging_location_;
	std::map<sc2::UpgradeID, bool> upgrades;
	int target_worker_count_;
	bool air_build_;
	int max_colossus_count_;
	int max_sentry_count_;
	int max_stalker_count_;
	std::vector<SoldadoBot*> soldier;
	float thr_dist_enemy;
	float thr_min_dist;
	float thr_largearmy;
	std::vector<sc2::Point3D> enemy_points;

	WarGeneral();
	WarGeneral(const sc2::ObservationInterface* enviroment, MessageClass* message, sc2::ActionInterface* action_,
		sc2::QueryInterface* query, const sc2::Unit* unit, std::vector<sc2::Point3D> expantion_points, std::vector<sc2::Point3D> enemy_points);
	~WarGeneral();
	void init_state_machine();
	virtual Message Function_Perception(Message msg) override;
	bool CloseGateway();
	void OpenGateway();
	bool GetRandomUnit(const sc2::Unit *& unit_out, sc2::UnitTypeID unit_type);
	bool TryBuildUnit(sc2::AbilityID ability_type_for_unit, sc2::UnitTypeID unit_type);
	size_t CountUnitTypeTotal(sc2::UNIT_TYPEID unit_type, sc2::UNIT_TYPEID production, sc2::ABILITY_ID ability);
	size_t CountUnitTypeBuilding(sc2::UNIT_TYPEID production_building, sc2::ABILITY_ID ability);
	bool TryBuildArmy();
	sc2::PowerSource* NearPower(std::vector<sc2::PowerSource> &power_sources);
	bool TryWarpInUnit(sc2::ABILITY_ID ability_type_for_unit);
	int CountUnitType(sc2::UnitTypeID unit_type);
	bool NeedArmy();
	bool BaseAgentAttacked(const sc2::Unit* &n);
	void AttackFunction(SoldadoBot* sold);
	void DefendBase(sc2::Point2D point);
	bool LargeArmy();
	void AttackOrden(bool all);
	virtual void Function_Action(Message m_action) override;
	bool NoneEnemy();
	bool EnemyBase(const sc2::Unit* &result);
	bool IdleArmy();
};

