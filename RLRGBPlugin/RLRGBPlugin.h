#pragma once

#include <future>

#include <cpr/cpr.h>
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "nlohmann/json.hpp"

#pragma comment(lib, "pluginsdk.lib")

class RLRGBPlugin : public BakkesMod::Plugin::BakkesModPlugin {
public:
	int myTeamScore = 0;
	int otherTeamScore = 0;

	virtual void onLoad();
	virtual void onUnload();

	void Log(std::string msg);

	void OnMatchStarted(std::string name);
	void OnGoalScored(std::string name);
	void OnMatchEnded(std::string name);

	ServerWrapper GetCurrentGame();


	void SendRGBEffect(std::string effect, LinearColor color, float speed, int dur);
};

