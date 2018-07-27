#pragma once
#include <list>
#include "json.hpp"
#include "sc2api/sc2_api.h"
#include "sc2lib/sc2_lib.h"

#include "sc2utils/sc2_manage_process.h"

#include <iostream>
#define VERBOSE false
typedef nlohmann::json Message;

class MessageClass
{
public:
    std::list<Message> blackboard;
	MessageClass();
	~MessageClass();
    void PushMessage(Message msg);
    Message PopMessageTo(sc2::Tag receiver);
	void Clear();
};

