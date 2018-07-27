#include "FSM.h"

//default constructor
FSM::FSM()
{
	this->state_total = 0;
	this->observation_total = 0;
	this->current_state = 0;
	this->next_state = 0;
	this->transition_matrix = nullptr;
}

//custom constructor
FSM::FSM(state state_num, perception observation_num)
{
	this->transition_matrix = nullptr;
	create_transition_matrix(state_num, observation_num);
}

//reserve memory for transition matrix (include delete previously created transition matrix)
void FSM::create_transition_matrix(state state_num, perception observation_num)
{
	if (this->transition_matrix != nullptr)
		delete_transition_matrix();

	this->transition_matrix = new transition*[state_num];
	for (int i = 0;i < state_num; i++)
		this->transition_matrix[i] = new transition[observation_num];

	this->state_total = state_num;
	this->observation_total = observation_num;
	this->current_state = 0;
	this->next_state = 0;
}

//destructor
FSM::~FSM()
{
	delete_transition_matrix();
}

//free transition matrix memory (private method to avoid bug, used by create_transition_matrix)
void FSM::delete_transition_matrix()
{
	for (int i = 0;i < this->state_total; i++)
		delete[] this->transition_matrix[i];
	delete[] this->transition_matrix;
	this->transition_matrix = nullptr;
}

//set transition FSM[state_num][observation_num]=transition
void FSM::set_transition(state state_num, perception observation_num, transition trans_struct)
{
	this->transition_matrix[state_num][observation_num] = trans_struct;
}

//get transition in FSM[current_state][observation_num]
transition FSM::get_transition(perception observation_num)
{
	return this->transition_matrix[this->current_state][observation_num];
}

//set next state and return action in FSM[current_state][observation_num]
action FSM::execute(perception observation_num)
{
	this->next_state = this->transition_matrix[this->current_state][observation_num].state_num;
	return this->transition_matrix[this->current_state][observation_num].action_num;
}

//change the current_state if action performed
void FSM::change_state()
{
	this->current_state = this->next_state;
}

