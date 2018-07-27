#include "MessageClass.h"

MessageClass::MessageClass()
{
}


MessageClass::~MessageClass()
{
}

void MessageClass::PushMessage(Message msg)
{
	blackboard.push_back(msg);

	if (VERBOSE)
		std::cout << "Push Message " << msg << std::endl;
}

Message MessageClass::PopMessageTo(sc2::Tag receiver) {
	Message msg;

	for (auto& m : blackboard) {
		if (m["receiver"] == receiver) {
			msg = m;
			break;
		}
	}
	blackboard.remove(msg);

	/*if (VERBOSE && !msg.is_null())
		std::cout << "Pop Message " << msg << std::endl;*/
	return msg;
}

void MessageClass::Clear()
{
		blackboard.clear();
	}

