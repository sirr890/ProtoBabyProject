#pragma once
#include"FSM.h"
#include "sc2api/sc2_api.h"
#include "MessageClass.h"
#include <Windows.h>

class BaseAgent
{
public:
	FSM* fsm;
	MessageClass* message;
	const sc2::ObservationInterface* enviroment;
	sc2::ActionInterface* action_;
	sc2::QueryInterface* query;
	const sc2::Unit* unit;
	std::vector<sc2::Point3D> expantion_points;

	float current_shield;
	float current_health;

	BaseAgent();
	BaseAgent(const sc2::ObservationInterface* enviroment, MessageClass* blackboard, sc2::ActionInterface* sc2action,
		sc2::QueryInterface* sc2query, const sc2::Unit* sc2unit, std::vector < sc2::Point3D > expantion_points);
	~BaseAgent();
	virtual Message Function_Perception(Message msg) = 0;
	virtual void Function_Action(Message action_) = 0;
	void execute();
};

