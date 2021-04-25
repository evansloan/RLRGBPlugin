#include "pch.h"
#include "RLRGBPlugin.h"

BAKKESMOD_PLUGIN(RLRGBPlugin, "RLRGBPlugin", "1.1", PERMISSION_ALL)

void RLRGBPlugin::onLoad() {
    std::stringstream ss;
    ss << "Starting " << exports.pluginName << " version: " << exports.pluginVersion;
    Log(ss.str());

    LoadConfig();
    HookEvents();
}

void RLRGBPlugin::onUnload() {
    std::stringstream ss;
    ss << "Stopping " << exports.pluginName << " version: " << exports.pluginVersion;
    Log(ss.str());

    UnHookEvents();
}

void RLRGBPlugin::LoadConfig() {
    cvarManager->registerCvar("rlrgb_api_url", "", "RGB API URL");
    goalScoredEffect = std::make_shared<std::string>();
    cvarManager->registerCvar("rlrgb_goal_effect", "flash", "Effect to display on goal scored").bindTo(goalScoredEffect);
    goalAgainstEffect = std::make_shared<std::string>();
    cvarManager->registerCvar("rlrgb_goal_against_effect", "fade_out", "Effect to display on goal against scored").bindTo(goalAgainstEffect);
    matchWinEffect = std::make_shared<std::string>();
    cvarManager->registerCvar("rlrgb_match_win_effect", "flash", "Effect to display on match win").bindTo(matchWinEffect);
    matchLossEffect = std::make_shared<std::string>();
    cvarManager->registerCvar("rlrgb_match_loss_effect", "fade_out", "Effect to display on match loss").bindTo(matchLossEffect);
}

void RLRGBPlugin::HookEvents() {
    gameWrapper->HookEvent("Function TAGame.GameEvent_TA.EventMatchStarted", std::bind(&RLRGBPlugin::OnMatchStarted, this, std::placeholders::_1));
    gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.EventGoalScored", std::bind(&RLRGBPlugin::OnGoalScored, this, std::placeholders::_1));
    gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.EventMatchEnded", std::bind(&RLRGBPlugin::OnMatchEnded, this, std::placeholders::_1));
}

void RLRGBPlugin::UnHookEvents() {
    gameWrapper->UnhookEvent("Function TAGame.GameEvent_TA.EventMatchStarted");
    gameWrapper->UnhookEvent("Function TAGame.GameEvent_Soccar_TA.EventGoalScored");
    gameWrapper->UnhookEvent("Function TAGame.GameEvent_Soccar_TA.OnMatchEnded");
}

void RLRGBPlugin::Log(std::string msg) {
    cvarManager->log(msg);
}

void RLRGBPlugin::OnMatchStarted(std::string name) {
    sw = GetCurrentGame();
    if (sw->IsNull()) {
        Log(name + ": Unable to retrieve current game");
        return;
    }
    
    PriWrapper localPlayer = sw->GetLocalPrimaryPlayer().GetPRI();
    ArrayWrapper<TeamWrapper> teams = sw->GetTeams();

    myTeam = std::make_shared<TeamWrapper>(teams.Get(localPlayer.GetTeamNum()));
    otherTeam = std::make_shared<TeamWrapper>(teams.Get(!localPlayer.GetTeamNum()));

    myTeamScore = 0;
    otherTeamScore = 0;
}

void RLRGBPlugin::OnGoalScored(std::string name) {
    if (sw->IsNull()) {
        Log(name + ": Unable to retrieve current game");
        return;
    }

    if (myTeam->GetScore() > myTeamScore) {
        myTeamScore++;
        std::async(&RLRGBPlugin::SendRGBEffect, this, *goalScoredEffect, myTeam->GetPrimaryColor(), 0.2, 2);
    } else {
        otherTeamScore++;
        std::async(&RLRGBPlugin::SendRGBEffect, this, *goalAgainstEffect, otherTeam->GetPrimaryColor(), 0.0001, 2);
    }
   
}

void RLRGBPlugin::OnMatchEnded(std::string name) {
    if (myTeam->GetScore() > otherTeam->GetScore()) {
        std::async(&RLRGBPlugin::SendRGBEffect, this, *matchWinEffect, myTeam->GetPrimaryColor(), 0.2, 2);
    } else {
        std::async(&RLRGBPlugin::SendRGBEffect, this, *matchLossEffect, otherTeam->GetPrimaryColor(), 0.0001, 2);
    }
}

std::shared_ptr<ServerWrapper> RLRGBPlugin::GetCurrentGame() {
    if (gameWrapper->IsInOnlineGame()) {
        return std::make_shared<ServerWrapper>(gameWrapper->GetOnlineGame());
    } else if (gameWrapper->IsInReplay()) {
        return nullptr;
    } else {
        return std::make_shared<ServerWrapper>(gameWrapper->GetGameEventAsServer());
    }
}

void RLRGBPlugin::SendRGBEffect(std::string effect, LinearColor color, float speed, int dur) {
    nlohmann::json body = {
        {"effect", effect},
        {"color", {color.R * 255, color.G * 255, color.B * 255}},
        {"speed", speed},
        {"duration", dur}
    };

    cpr::PostAsync(
        cpr::Url{cvarManager->getCvar("rlrgb_api_url").getStringValue() + "/effect"},
        cpr::Body{body.dump()},
        cpr::Header{{"Content-Type", "application/json"}}
    );
}