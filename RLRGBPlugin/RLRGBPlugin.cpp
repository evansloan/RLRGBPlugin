#include "pch.h"
#include "RLRGBPlugin.h"

BAKKESMOD_PLUGIN(RLRGBPlugin, "Control RGB lights based on Rocket League Events", "1.0", PERMISSION_ALL)

void RLRGBPlugin::onLoad() {
	std::stringstream ss;
	ss << "Stopping " << exports.pluginName << " version: " << exports.pluginVersion;
	Log(ss.str());

	cvarManager->registerCvar("rlrgb_api_url", "", "RGB API URL");

	gameWrapper->HookEvent("Function TAGame.GameEvent_TA.EventMatchStarted", std::bind(&RLRGBPlugin::OnMatchStarted, this, std::placeholders::_1));
	gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.EventGoalScored", std::bind(&RLRGBPlugin::OnGoalScored, this, std::placeholders::_1));
	gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.EventMatchEnded", std::bind(&RLRGBPlugin::OnMatchEnded, this, std::placeholders::_1));
}

void RLRGBPlugin::onUnload() {
	std::stringstream ss;
	ss << "Stopping " << exports.pluginName << " version: " << exports.pluginVersion;
	Log(ss.str());

	gameWrapper->UnhookEvent("Function TAGame.GameEvent_TA.EventMatchStarted");
	gameWrapper->UnhookEvent("Function TAGame.Ball_TA.Explode");
	gameWrapper->UnhookEvent("Function TAGame.GameEvent_Soccar_TA.OnMatchEnded");
}

void RLRGBPlugin::Log(std::string msg) {
	cvarManager->log(msg);
}

void RLRGBPlugin::OnMatchStarted(std::string name) {
	myTeamScore = 0;
	otherTeamScore = 0;
}

void RLRGBPlugin::OnGoalScored(std::string name) {
	ServerWrapper sw = GetCurrentGame();
	if (sw.IsNull()) {
		Log(name + ": Unable to retrieve current game");
		return;
	}

	PriWrapper localPlayer = sw.GetLocalPrimaryPlayer().GetPRI();
	ArrayWrapper<TeamWrapper> teams = sw.GetTeams();

	TeamWrapper myTeam = teams.Get(localPlayer.GetTeamNum());
	TeamWrapper otherTeam = teams.Get(!localPlayer.GetTeamNum());

	LinearColor color;
	std::string effect;
	float speed;

	if (myTeam.GetScore() > myTeamScore) {
		myTeamScore++;
		color = myTeam.GetPrimaryColor();
		effect = "flash";
		speed = 0.2;
	} else {
		otherTeamScore++;
		color = otherTeam.GetPrimaryColor();
		effect = "fade_out";
		speed = 0.0001;
	}
	
	std::async(&RLRGBPlugin::SendRGBEffect, this, effect, color, speed, 2);
}

void RLRGBPlugin::OnMatchEnded(std::string name) {
	ServerWrapper sw = GetCurrentGame();
	if (sw.IsNull()) {
		Log(name + ": Unable to retrieve ServerWrapper");
		return;
	}

	PriWrapper localPlayer = sw.GetLocalPrimaryPlayer().GetPRI();
	ArrayWrapper<TeamWrapper> teams = sw.GetTeams();

	TeamWrapper myTeam = teams.Get(localPlayer.GetTeamNum());
	TeamWrapper otherTeam = teams.Get(!localPlayer.GetTeamNum());

	LinearColor color;
	std::string effect;
	float speed;

	if (myTeam.GetScore() > otherTeam.GetScore()) {
		color = myTeam.GetPrimaryColor();
		effect = "flash";
		speed = 0.2;
	} else {
		color = otherTeam.GetPrimaryColor();
		effect = "fade_out";
		speed = 0.0001;
	}

	std::async(&RLRGBPlugin::SendRGBEffect, this, effect, color, speed, 2);
}

ServerWrapper RLRGBPlugin::GetCurrentGame() {
	if (gameWrapper->IsInOnlineGame()) {
		return gameWrapper->GetOnlineGame();
	} else if (gameWrapper->IsInReplay()) {
		return NULL;
	} else {
		return gameWrapper->GetGameEventAsServer();
	}
}

void RLRGBPlugin::SendRGBEffect(std::string effect, LinearColor color, float speed, int dur) {
	nlohmann::json body = {
		{"effect", effect},
		{"color", {color.R * 255, color.G * 255, color.B * 255}},
		{"speed", speed},
		{"duration", dur}
	};

	cpr::Response res = cpr::Post(
		cpr::Url{cvarManager->getCvar("rlrgb_api_url").getStringValue() + "/effect"},
		cpr::Body{body.dump()},
		cpr::Header{{"Content-Type", "application/json"}}
	);
}