#include "sc2api/sc2_api.h"
#include "sc2lib/sc2_lib.h"
#include "sc2utils/sc2_manage_process.h"

#include "bot_examples.h"
#include <iostream>
#include <list>
#include <algorithm>
#include "MessageClass.h"
#include "ProtoBot.h"

int main(int argc, char* argv[]) {
	sc2::Coordinator coordinator;
	if (!coordinator.LoadSettings(argc, argv)) {
		return 1;
	}
	//coordinator.SetRealtime(true);

	ProtoBot bot;
	ProtossMultiplayerBot bot1;

	coordinator.SetParticipants({
		CreateParticipant(sc2::Race::Protoss, &bot),
		CreateComputer(sc2::Race::Protoss, sc2::Difficulty::MediumHard)
		//CreateParticipant(sc2::Race::Protoss, &bot1)
		});

	coordinator.LaunchStarcraft();

	bool do_break = false;
	while (!do_break) {
		coordinator.StartGame("../maps/NeonVioletSquareLE.SC2Map");
		while (coordinator.Update() && !do_break) {
			if (sc2::PollKeyPress()) {
				do_break = true;
			
			}
		}
	}

	return 0;
}
