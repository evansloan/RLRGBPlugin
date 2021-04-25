#pragma once

#include <future>

#include <cpr/cpr.h>
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "nlohmann/json.hpp"

#pragma comment(lib, "pluginsdk.lib")

struct Effect {
    std::shared_ptr<std::string> effectName;
    std::shared_ptr<int> effectDuration;
    std::shared_ptr<float> effectSpeed;
};

class RLRGBPlugin : public BakkesMod::Plugin::BakkesModPlugin {
private:
    std::shared_ptr<ServerWrapper> sw;
    std::shared_ptr<TeamWrapper> myTeam;
    std::shared_ptr<TeamWrapper> otherTeam;
    int myTeamScore;
    int otherTeamScore;

    Effect goalScoredEffect;
    Effect goalAgainstEffect;
    Effect matchWinEffect;
    Effect matchLossEffect;

    std::shared_ptr<ServerWrapper> GetCurrentGame();
    void SendRGBEffect(Effect effect, LinearColor color);

public:
    virtual void onLoad();
    virtual void onUnload();

    void LoadConfig();
    void HookEvents();
    void UnHookEvents();
    void Log(std::string msg);

    void OnMatchStarted(std::string name);
    void OnGoalScored(std::string name);
    void OnMatchEnded(std::string name);

};

