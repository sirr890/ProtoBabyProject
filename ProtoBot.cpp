#include <iostream>
#include "ProtoBot.h"
#include "MinerClass.h"
#include "sc2api/sc2_api.h"
#include <map>

using namespace sc2;
ProtoBot::ProtoBot()
{
	msg = new MessageClass();
}

std::vector<Point3D> expantion_points;
std::vector<sc2::Point3D> enemy_points;

MinerClass* miner;

WarGeneral* general;

void ProtoBot::OnGameStart()
{
	const ObservationInterface* observation = Observation();
	flag = true;
	flagzealot = true;
	flags = true;
	msg = new MessageClass();

	expantion_points = search::CalculateExpansionLocations(Observation(), Query());
	if (!observation->GetGameInfo().enemy_start_locations.empty())
	{
		sc2::Point3D init;
		init.x= observation->GetGameInfo().enemy_start_locations[0].x;
		init.y = observation->GetGameInfo().enemy_start_locations[0].y;
		enemy_points.push_back(init);
	}
	sc2::Unit* unit = new Unit();
	unit->tag = AGENT_TAG::MINER;
	miner = new MinerClass(Observation(), msg, Actions(), Query(), unit, expantion_points);
	unit = new Unit();
	unit->tag = AGENT_TAG::WARGENERAL;
	general = new WarGeneral(Observation(), msg, Actions(), Query(), unit, expantion_points, enemy_points);

	unit = new Unit();
	unit->tag = AGENT_TAG::COORDINADOR;
	coordinador = new CoordinatorAgent(Observation(), msg, Actions(), Query(), unit, expantion_points, enemy_points);
	Units probes = Observation()->GetUnits(Unit::Alliance::Self, IsUnit(sc2::UNIT_TYPEID::PROTOSS_PROBE));
	if (probes.size() > 0)
	{
		for (auto&probe : probes)
		{
			OnUnitCreated(probe);
		}
	}
}

void ProtoBot::OnStep() {
	coordinador->execute();
		miner->execute();
		for (auto& probe : miner->probes)
			probe->execute();
		
		general->execute();
		for (auto& soldier : general->soldier)
			soldier->execute();
}

void ProtoBot::OnUnitDestroyed(const sc2::Unit *unit)
{
	auto value = mapofprobes.find(unit->tag);
	if (value != mapofprobes.end())
	{
		for (int i = 0; i < miner->probes.size();i++)
		{
			if (miner->probes[i]->unit->tag == unit->tag)
			{
				miner->probes.erase(miner->probes.begin() + i);
				mapofprobes.erase(value);
				return;
			}
		}
	}

	value = mapofsoldier.find(unit->tag);
	if (value != mapofsoldier.end())
	{
		for (int i = 0; i < general->soldier.size();i++)
		{
			if (general->soldier[i]->unit->tag == unit->tag)
			{
				general->soldier.erase(general->soldier.begin() + i);
				mapofsoldier.erase(value);
				return;
			}
		}
	}
}

void ProtoBot::OnGameEnd() {
	miner->probes.clear();
	general->soldier.clear();
	
	delete miner;
	delete general;
	delete coordinador;
	msg->Clear();
	mapofprobes.clear();
	mapofsoldier.clear();
};

void ProtoBot::OnUnitCreated(const sc2::Unit *unit)
{
	switch (unit->unit_type.ToType())
	{
	case sc2::UNIT_TYPEID::PROTOSS_PROBE:
	{
		if (mapofprobes.find(unit->tag) == mapofprobes.end()) {
			ProbeAgent* probe = new ProbeAgent(Observation(), msg, Actions(), Query(), unit, expantion_points);
			miner->probes.push_back(probe);
			mapofprobes[unit->tag] = (int)mapofprobes.size();
		}
	}
	break;
	case sc2::UNIT_TYPEID::PROTOSS_ZEALOT:
	case sc2::UNIT_TYPEID::PROTOSS_VOIDRAY:
	case sc2::UNIT_TYPEID::PROTOSS_ADEPT:
	case sc2::UNIT_TYPEID::PROTOSS_MOTHERSHIPCORE:
	case sc2::UNIT_TYPEID::PROTOSS_COLOSSUS:
	case sc2::UNIT_TYPEID::PROTOSS_CARRIER:
	case sc2::UNIT_TYPEID::PROTOSS_HIGHTEMPLAR:
	case sc2::UNIT_TYPEID::PROTOSS_ORACLE:
	case sc2::UNIT_TYPEID::PROTOSS_IMMORTAL:
	case sc2::UNIT_TYPEID::PROTOSS_STALKER:
	case sc2::UNIT_TYPEID::PROTOSS_MOTHERSHIP:
	{
		SoldadoBot* s = new SoldadoBot(Observation(), msg, Actions(), Query(), unit, expantion_points);
		general->soldier.push_back(s);
		mapofsoldier[unit->tag] = (int)mapofsoldier.size();

	}
	break;
	default:
		break;
	}
}