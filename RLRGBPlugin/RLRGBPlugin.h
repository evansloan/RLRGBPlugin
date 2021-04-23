#pragma once

#include <future>

#include <cpr/cpr.h>
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "nlohmann/json.hpp"

#pragma comment(lib, "pluginsdk.lib")

class RLRGBPlugin : public BakkesMod::Plugin::BakkesModPlugin {
private:
    std::shared_ptr<ServerWrapper> sw;
    std::shared_ptr<TeamWrapper> myTeam;
    std::shared_ptr<TeamWrapper> otherTeam;

    int myTeamScore;
    int otherTeamScore;
    LinearColor myTeamColor;
    LinearColor otherTeamColor;

    std::shared_ptr<ServerWrapper> GetCurrentGame();
    void SendRGBEffect(std::string effect, LinearColor color, float speed, int dur);

public:
    virtual void onLoad();
    virtual void onUnload();

    void Log(std::string msg);

    void OnMatchStarted(std::string name);
    void OnGoalScored(std::string name);
    void OnMatchEnded(std::string name);

};

