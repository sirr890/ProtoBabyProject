#include "CoordinatorAgent.h"



CoordinatorAgent::CoordinatorAgent()
{
}

CoordinatorAgent::CoordinatorAgent(const sc2::ObservationInterface * enviroment_, MessageClass * message_,
	sc2::ActionInterface * action_, sc2::QueryInterface * query_, const sc2::Unit * unit_,
	std::vector<sc2::Point3D> expantion_points_, std::vector<sc2::Point3D> enemy_points_)
{
	this->enviroment = enviroment_;
	this->message = message_;
	this->action = action_;
	this->query = query_;
	this->unit = unit_;
	this->expantion_points = expantion_points_;
	this->enemy_points = enemy_points_;
	flags = true;
}


CoordinatorAgent::~CoordinatorAgent()
{
}

size_t CoordinatorAgent::CountUnitType(const sc2::ObservationInterface* observation, sc2::UnitTypeID unit_type) {
	return observation->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnit(unit_type)).size();
}
bool CoordinatorAgent::GetRandomUnit(const sc2::Unit*& unit_out, const sc2::ObservationInterface* observation, sc2::UnitTypeID unit_type) {
	sc2::Units my_units = observation->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnit(unit_type));
	if (!my_units.empty()) {
		unit_out = GetRandomEntry(my_units);
		return true;
	}
	return false;
}
bool CoordinatorAgent::TryBuildUnit(sc2::AbilityID ability_type_for_unit, sc2::UnitTypeID unit_type) {
	const sc2::ObservationInterface* observation = enviroment;

	//If we are at supply cap, don't build anymore units, unless its an overlord.
	if (observation->GetFoodUsed() >= observation->GetFoodCap() && ability_type_for_unit != sc2::ABILITY_ID::TRAIN_OVERLORD) {
		return false;
	}
	const sc2::Unit* unit = nullptr;
	if (!GetRandomUnit(unit, observation, unit_type)) {
		return false;
	}
	if (!unit->orders.empty()) {
		return false;
	}

	if (unit->build_progress != 1) {
		return false;
	}

	action->UnitCommand(unit, ability_type_for_unit);
	return true;
}


void CoordinatorAgent::ManageUpgrades() {
	const sc2::ObservationInterface* observation = enviroment;
	auto upgrades = observation->GetUpgrades();
	size_t base_count = observation->GetUnits(sc2::Unit::Alliance::Self, IsTownHall()).size();
	for (const auto& upgrade : upgrades) {
		if (true) {
			if (upgrade == sc2::UPGRADE_ID::PROTOSSAIRWEAPONSLEVEL1 && base_count > 2) {
				TryBuildUnit(sc2::ABILITY_ID::RESEARCH_PROTOSSAIRWEAPONS, sc2::UNIT_TYPEID::PROTOSS_CYBERNETICSCORE);
			}
			else if (upgrade == sc2::UPGRADE_ID::PROTOSSAIRARMORSLEVEL1 && base_count > 2) {
				TryBuildUnit(sc2::ABILITY_ID::RESEARCH_PROTOSSAIRARMOR, sc2::UNIT_TYPEID::PROTOSS_CYBERNETICSCORE);
			}
			else if (upgrade == sc2::UPGRADE_ID::PROTOSSSHIELDSLEVEL1 && base_count > 2) {
				TryBuildUnit(sc2::ABILITY_ID::RESEARCH_PROTOSSSHIELDS, sc2::UNIT_TYPEID::PROTOSS_FORGE);
			}
			else if (upgrade == sc2::UPGRADE_ID::PROTOSSAIRARMORSLEVEL2 && base_count > 3) {
				TryBuildUnit(sc2::ABILITY_ID::RESEARCH_PROTOSSAIRARMOR, sc2::UNIT_TYPEID::PROTOSS_CYBERNETICSCORE);
			}
			else if (upgrade == sc2::UPGRADE_ID::PROTOSSAIRWEAPONSLEVEL2 && base_count > 2) {
				TryBuildUnit(sc2::ABILITY_ID::RESEARCH_PROTOSSAIRWEAPONS, sc2::UNIT_TYPEID::PROTOSS_CYBERNETICSCORE);
			}
			else if (upgrade == sc2::UPGRADE_ID::PROTOSSSHIELDSLEVEL2 && base_count > 2) {
				TryBuildUnit(sc2::ABILITY_ID::RESEARCH_PROTOSSSHIELDS, sc2::UNIT_TYPEID::PROTOSS_FORGE);
			}
		}
		if (upgrade == sc2::UPGRADE_ID::PROTOSSGROUNDWEAPONSLEVEL1 && base_count > 2) {
			TryBuildUnit(sc2::ABILITY_ID::RESEARCH_PROTOSSGROUNDWEAPONS, sc2::UNIT_TYPEID::PROTOSS_FORGE);
		}
		else if (upgrade == sc2::UPGRADE_ID::PROTOSSGROUNDARMORSLEVEL1 && base_count > 2) {
			TryBuildUnit(sc2::ABILITY_ID::RESEARCH_PROTOSSGROUNDARMOR, sc2::UNIT_TYPEID::PROTOSS_FORGE);
		}
		else if (upgrade == sc2::UPGRADE_ID::PROTOSSGROUNDWEAPONSLEVEL2 && base_count > 3) {
			TryBuildUnit(sc2::ABILITY_ID::RESEARCH_PROTOSSGROUNDWEAPONS, sc2::UNIT_TYPEID::PROTOSS_FORGE);
		}
		else if (upgrade == sc2::UPGRADE_ID::PROTOSSGROUNDARMORSLEVEL2 && base_count > 3) {
			TryBuildUnit(sc2::ABILITY_ID::RESEARCH_PROTOSSGROUNDARMOR, sc2::UNIT_TYPEID::PROTOSS_FORGE);
		}
		else {

			TryBuildUnit(sc2::ABILITY_ID::RESEARCH_PROTOSSAIRARMOR, sc2::UNIT_TYPEID::PROTOSS_CYBERNETICSCORE);
			TryBuildUnit(sc2::ABILITY_ID::RESEARCH_PROTOSSGROUNDARMOR, sc2::UNIT_TYPEID::PROTOSS_FORGE);
			TryBuildUnit(sc2::ABILITY_ID::RESEARCH_PROTOSSAIRWEAPONS, sc2::UNIT_TYPEID::PROTOSS_CYBERNETICSCORE);
			TryBuildUnit(sc2::ABILITY_ID::RESEARCH_PROTOSSGROUNDWEAPONS, sc2::UNIT_TYPEID::PROTOSS_FORGE);
			TryBuildUnit(sc2::ABILITY_ID::RESEARCH_CHARGE, sc2::UNIT_TYPEID::PROTOSS_TWILIGHTCOUNCIL);
			TryBuildUnit(sc2::ABILITY_ID::RESEARCH_PROTOSSSHIELDS, sc2::UNIT_TYPEID::PROTOSS_FORGE);
			TryBuildUnit(sc2::ABILITY_ID::RESEARCH_INTERCEPTORGRAVITONCATAPULT, sc2::UNIT_TYPEID::PROTOSS_FLEETBEACON);
			TryBuildUnit(sc2::ABILITY_ID::RESEARCH_EXTENDEDTHERMALLANCE, sc2::UNIT_TYPEID::PROTOSS_ROBOTICSBAY);
			TryBuildUnit(sc2::ABILITY_ID::RESEARCH_BLINK, sc2::UNIT_TYPEID::PROTOSS_TWILIGHTCOUNCIL);
		}
	}
}

void CoordinatorAgent::BuildOrder() {
	const sc2::ObservationInterface* observation = enviroment;
	size_t gateway_count = CountUnitType(observation, sc2::UNIT_TYPEID::PROTOSS_GATEWAY) + CountUnitType(observation, sc2::UNIT_TYPEID::PROTOSS_WARPGATE);
	size_t cybernetics_count = CountUnitType(observation, sc2::UNIT_TYPEID::PROTOSS_CYBERNETICSCORE);
	size_t forge_count = CountUnitType(observation, sc2::UNIT_TYPEID::PROTOSS_FORGE);
	size_t twilight_council_count = CountUnitType(observation, sc2::UNIT_TYPEID::PROTOSS_TWILIGHTCOUNCIL);
	size_t templar_archive_count = CountUnitType(observation, sc2::UNIT_TYPEID::PROTOSS_TEMPLARARCHIVE);
	size_t base_count = CountUnitType(observation, sc2::UNIT_TYPEID::PROTOSS_NEXUS);
	size_t robotics_facility_count = CountUnitType(observation, sc2::UNIT_TYPEID::PROTOSS_ROBOTICSFACILITY);
	size_t robotics_bay_count = CountUnitType(observation, sc2::UNIT_TYPEID::PROTOSS_ROBOTICSBAY);
	size_t stargate_count = CountUnitType(observation, sc2::UNIT_TYPEID::PROTOSS_STARGATE);
	size_t fleet_beacon_count = CountUnitType(observation, sc2::UNIT_TYPEID::PROTOSS_FLEETBEACON);

	// 3 Gateway per expansion
	if (gateway_count < std::min<size_t>(2 * base_count, 7)) {

		//If we have 1 gateway, prioritize building CyberCore
		if (cybernetics_count < 1 && gateway_count > 0) {
			message->PushMessage(
				{
					{ "type","request" },
				{ "content",{ { "type_request","build" },{ "structure",sc2::ABILITY_ID::BUILD_CYBERNETICSCORE },{ "target_x",-1.0 },{ "target_y",-1.0 } } },
				{ "receiver",AGENT_TAG::MINER }
				}
			);
			return;
		}
		else {

			if (observation->GetFoodWorkers() >= 15 && observation->GetMinerals() > 150 + (100 * gateway_count)) {
				message->PushMessage(
					{
						{ "type","request" },
					{ "content",{ { "type_request","build" },{ "structure",sc2::ABILITY_ID::BUILD_GATEWAY },{ "target_x",-1.0 },{ "target_y",-1.0 } } },
					{ "receiver",AGENT_TAG::MINER }
					}
				);
			}
		}
	}

	if (cybernetics_count > 0 && forge_count < 1)
		message->PushMessage(
			{
				{ "type","request" },
			{ "content",{ { "type_request","build" },{ "structure",sc2::ABILITY_ID::BUILD_FORGE },{ "target_x",-1.0 },{ "target_y",-1.0 } } },
			{ "receiver",AGENT_TAG::MINER }
			}
	);
	{
		if (gateway_count > 1 && cybernetics_count > 0) {
			if (fleet_beacon_count < 1 && stargate_count > 0) {
				if (observation->GetMinerals() > 300 && observation->GetVespene() > 200) {
					message->PushMessage(
						{
							{ "type","request" },
						{ "content",{ { "type_request","build" },{ "structure",sc2::ABILITY_ID::BUILD_FLEETBEACON },{ "target_x",-1.0 },{ "target_y",-1.0 } } },
						{ "receiver",AGENT_TAG::MINER }
						}
					);
				}
			}
			else
				if (stargate_count < 2 * std::min<size_t>(base_count, 5)) {
					if (observation->GetMinerals() > 150 && observation->GetVespene() > 150) {
						message->PushMessage(
							{
								{ "type","request" },
							{ "content",{ { "type_request","build" },{ "structure",sc2::ABILITY_ID::BUILD_STARGATE },{ "target_x",-1.0 },{ "target_y",-1.0 } } },
							{ "receiver",AGENT_TAG::MINER }
							}
						);
					}
				}
		}
	}
	{
		if (gateway_count > 2 && cybernetics_count > 0) {
			if (robotics_facility_count < 1) {//std::min<size_t>(base_count, 4)) {
				if (observation->GetMinerals() > 200 && observation->GetVespene() > 100) {
					message->PushMessage(
						{
							{ "type","request" },
						{ "content",{ { "type_request","build" },{ "structure",sc2::ABILITY_ID::BUILD_ROBOTICSFACILITY },{ "target_x",-1.0 },{ "target_y",-1.0 } } },
						{ "receiver",AGENT_TAG::MINER }
						}
					);
				}
			}
		}
	}

	/*if (forge_count > 0 && twilight_council_count < 1 && base_count > 1) {
		message->PushMessage(
			{
				{ "type","request" },
			{ "content",{ { "type_request","build" },{ "structure",sc2::ABILITY_ID::BUILD_TWILIGHTCOUNCIL },{ "target_x",-1.0 },{ "target_y",-1.0 } } },
			{ "receiver",AGENT_TAG::MINER }
			}
		);
	}*/

	/*	if (twilight_council_count > 0 && templar_archive_count < 1 && base_count > 1) {
			message->PushMessage(
				{
					{ "type","request" },
				{ "content",{ { "type_request","build" },{ "structure",sc2::ABILITY_ID::BUILD_TEMPLARARCHIVE },{ "target_x",-1.0 },{ "target_y",-1.0 } } },
				{ "receiver",AGENT_TAG::MINER }
				}
			);
		}*/

}

void CoordinatorAgent::execute()
{
	if (enviroment->GetGameLoop() > 100 && flags)
	{
		message->PushMessage(
			{
				{ "type","request" },
			{ "content",{ { "type_request","scout" },{ "target_x", enemy_points[0].x },
			{ "target_y",enemy_points[0].y } } },
			{ "receiver",MINER }
			}
		);
		flags = false;
	}
	sc2::Units nexus = enviroment->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnit(sc2::UNIT_TYPEID::PROTOSS_NEXUS));
	sc2::Units bateries = enviroment->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnit(sc2::UNIT_TYPEID::PROTOSS_SHIELDBATTERY));
	sc2::Units cc = enviroment->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnit(sc2::UNIT_TYPEID::PROTOSS_CYBERNETICSCORE));
	sc2::Units phc = enviroment->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnit(sc2::UNIT_TYPEID::PROTOSS_PHOTONCANNON));

	static bool flagcc = true;
	if (cc.size() >= 1 && cc[0]->build_progress == 1 && enviroment->GetMinerals() > 50 && enviroment->GetVespene() > 50 && flagcc)
	{
		flagcc = false;
		action->UnitCommand(cc[0], sc2::ABILITY_ID::RESEARCH_WARPGATE);
	}
	static int coldownbuild = 300;
	if (--coldownbuild < 1 && enviroment->GetMinerals() > 500)
	{
		coldownbuild = 300;
		BuildOrder();
		sc2::Units assm = enviroment->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnit(sc2::UNIT_TYPEID::PROTOSS_ASSIMILATOR));
		if (assm.size() < 2 * nexus.size())
			message->PushMessage(
				{
					{ "type","request" },
				{ "content",{ { "type_request","build" },{ "structure",sc2::ABILITY_ID::BUILD_ASSIMILATOR },{ "target_x",-1.0 },{ "target_y",-1.0 } } },
				{ "receiver",AGENT_TAG::MINER }
				}
		);
		if (phc.size() < 2 * nexus.size())
			message->PushMessage(
				{
					{ "type","request" },
				{ "content",{ { "type_request","build" },{ "structure",sc2::ABILITY_ID::BUILD_PHOTONCANNON },{ "target_x",-1.0 },{ "target_y",-1.0 } } },
				{ "receiver",AGENT_TAG::MINER }
				}
		);

		if (bateries.size() < 1 && cc.size() > 0)
			message->PushMessage(
				{
					{ "type","request" },
				{ "content",{ { "type_request","build" },{ "structure",sc2::ABILITY_ID::BUILD_SHIELDBATTERY },{ "target_x",-1.0 },{ "target_y",-1.0 } } },
				{ "receiver",AGENT_TAG::MINER }
				}
		);
		static int coldownupgrade = 500;
		if (--coldownupgrade < 1 && enviroment->GetMinerals() > 500)
		{
			coldownupgrade = 500;
			ManageUpgrades();
		}
	}
}
