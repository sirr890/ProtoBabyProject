#pragma once
typedef int action;
typedef int state;
typedef int perception;

struct transition {
	action action_num;
	state state_num;

	void operator=(transition trans) {
		this->action_num = trans.action_num;
		this->state_num = trans.state_num;
	}

	transition() {
		this->action_num = 0;
		this->state_num = 0;
	}

	transition(action a, state s) {
		this->action_num = a;
		this->state_num = s;
	}

	transition(transition &trans) {
		this->action_num = trans.action_num;
		this->state_num = trans.state_num;
	}
};

class FSM
{
	int state_total;
	int observation_total;
	transition ** transition_matrix;
	void delete_transition_matrix();

public:
	state current_state;
	state next_state;

	FSM();
	FSM(state state_num, perception observation_num);
	~FSM();
	void create_transition_matrix(state state_num, perception observation_num);
	void set_transition(state state_num, perception observation_num, transition trans_struct);
	transition get_transition(perception observation_num);
	action execute(perception observation_num);
	void change_state();
};

