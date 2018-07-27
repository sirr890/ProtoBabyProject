#pragma once

#include "sc2api/sc2_interfaces.h"
#include "sc2api/sc2_agent.h"
#include "sc2api/sc2_map_info.h"
#include "sc2lib/sc2_lib.h"
#include "ProbeAgent.h"
#include "SoldadoBot.h"
#include "WarGeneral.h"
#include <limits>
#include "CoordinatorAgent.h"
#undef min

using namespace sc2;


class ProtoBot : public sc2::Agent
{
public:
	bool nexus_created = false;
	MessageClass* msg;
	CoordinatorAgent* coordinador;

	std::vector<SoldadoBot*> soldier;

	bool flag;
	bool flags;
	bool flagzealot;
	std::map<sc2::Tag, int> mapofprobes;
	std::map<sc2::Tag, int> mapofsoldier;

	ProtoBot();
	virtual void OnGameStart() override;
	virtual void OnStep() override;
	virtual void OnGameEnd() override;
	virtual void OnUnitDestroyed(const sc2::Unit *unit) override;
    virtual void OnUnitCreated(const sc2::Unit *unit) override;
};

