#include "BaseAgent.h"

BaseAgent::BaseAgent()
{
}

BaseAgent::BaseAgent(const sc2::ObservationInterface * enviroment, MessageClass * blackboard, sc2::ActionInterface * sc2action, sc2::QueryInterface* sc2query, const sc2::Unit * sc2unit, std::vector<sc2::Point3D> expantion_points)
{
	this->enviroment = enviroment;
	this->message = blackboard;
	this->action_ = sc2action;
	this->query = sc2query;
	this->unit = sc2unit;
	current_shield = this->unit->shield;
	current_health = this->unit->health;
	this->expantion_points = expantion_points;
}


BaseAgent::~BaseAgent()
{
}

void BaseAgent::execute()
{
		Message msg= message->PopMessageTo(unit->tag);
		Message perceived = Function_Perception(msg);
		perception perception_ = (perception)perceived["perception_id"];
		action action_ = fsm->execute(perception_);
		perceived["action_id"] = action_;
		Function_Action(perceived);
		fsm->change_state();
}

