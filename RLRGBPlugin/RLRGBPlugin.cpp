#include "pch.h"
#include "RLRGBPlugin.h"

BAKKESMOD_PLUGIN(RLRGBPlugin, "RLRGBPlugin", "1.2", PERMISSION_ALL)

void RLRGBPlugin::onLoad() {
    std::stringstream ss;
    ss << "Starting " << exports.pluginName << " version: " << exports.pluginVersion;
    Log(ss.str());

    goalScoredEffect = Effect{
        std::make_shared<std::string>(),
        std::make_shared<int>(),
        std::make_shared<float>()
    };

    goalAgainstEffect = Effect{
        std::make_shared<std::string>(),
        std::make_shared<int>(),
        std::make_shared<float>()
    };

    matchWinEffect = Effect{
        std::make_shared<std::string>(),
        std::make_shared<int>(),
        std::make_shared<float>()
    };

    matchLossEffect = Effect{
        std::make_shared<std::string>(),
        std::make_shared<int>(),
        std::make_shared<float>()
    };

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
    cvarManager->registerCvar("rlrgb_api_url", "", "");
    cvarManager->registerCvar("rlrgb_goal_effect", "flash", "").bindTo(goalScoredEffect.effectName);
    cvarManager->registerCvar("rlrgb_goal_speed", "0.2", "", true, true, 0.0, true, 1.0).bindTo(goalScoredEffect.effectSpeed);
    cvarManager->registerCvar("rlrgb_goal_duration", "2", "", true, true, 0, true, 5).bindTo(goalScoredEffect.effectDuration);
    cvarManager->registerCvar("rlrgb_goal_against_effect", "fade_out", "").bindTo(goalAgainstEffect.effectName);
    cvarManager->registerCvar("rlrgb_goal_against_speed", "0.0001", "", true, true, 0.0, true, 1.0).bindTo(goalAgainstEffect.effectSpeed);
    cvarManager->registerCvar("rlrgb_goal_against_duration", "2", "").bindTo(goalAgainstEffect.effectDuration);
    cvarManager->registerCvar("rlrgb_match_win_effect", "flash", "").bindTo(matchWinEffect.effectName);
    cvarManager->registerCvar("rlrgb_match_win_speed", "0.2", "", true, true, 0.0, true, 1.0).bindTo(matchWinEffect.effectSpeed);
    cvarManager->registerCvar("rlrgb_match_win_duration", "2", "", true, true, 0, true, 5).bindTo(matchWinEffect.effectDuration);
    cvarManager->registerCvar("rlrgb_match_loss_effect", "fade_out", "").bindTo(matchLossEffect.effectName);
    cvarManager->registerCvar("rlrgb_match_loss_speed", "0.0001", "", true, true, 0.0, true, 1.0).bindTo(matchLossEffect.effectSpeed);
    cvarManager->registerCvar("rlrgb_match_loss_duration", "2", "", true, true, 0, true, 5).bindTo(matchLossEffect.effectDuration);
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
        std::async(&RLRGBPlugin::SendRGBEffect, this, goalScoredEffect, myTeam->GetPrimaryColor());
    } else {
        otherTeamScore++;
        std::async(&RLRGBPlugin::SendRGBEffect, this, goalAgainstEffect, otherTeam->GetPrimaryColor());
    }
   
}

void RLRGBPlugin::OnMatchEnded(std::string name) {
    if (sw->IsNull()) {
        Log(name + ": Unable to retrieve current game");
        return;
    }

    if (myTeam->GetScore() > otherTeam->GetScore()) {
        std::async(&RLRGBPlugin::SendRGBEffect, this, matchWinEffect, myTeam->GetPrimaryColor());
    } else {
        std::async(&RLRGBPlugin::SendRGBEffect, this, matchLossEffect, otherTeam->GetPrimaryColor());
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

void RLRGBPlugin::SendRGBEffect(Effect effect, LinearColor color) {
    nlohmann::json body = {
        {"effect", *effect.effectName},
        {"color", {color.R * 255, color.G * 255, color.B * 255}},
        {"speed", *effect.effectSpeed},
        {"duration", *effect.effectDuration}
    };

    cpr::PostAsync(
        cpr::Url{cvarManager->getCvar("rlrgb_api_url").getStringValue() + "/effect"},
        cpr::Body{body.dump()},
        cpr::Header{{"Content-Type", "application/json"}}
    );
}