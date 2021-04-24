# RLRGBPlugin
Control ws2812b LED strip effects using Bakkesmod, Rocket League, and [rpi-rgb-api](https://github.com/evansloan/rpi-rgb-api)

## Usage
1. Set up a [rpi-rgb-api](https://github.com/evansloan/rpi-rgb-api) server
2. Download and extract the latest [release](https://github.com/evansloan/RLRGBPlugin/releases/), or build with Visual Studio
3. Copy RLRGBPlugin.dll into `%USERPROFILE%\AppData\Roaming\bakkesmod\bakkesmod\plugins`
4. Copy rlrgbplugin.set into `%USERPROFILE%\AppData\Roaming\bakkesmod\bakkesmod\plugins\settings`
5. In the Bakkesmod settings console, under RLRGBPlugin, set `RGB API URL` to the rpi-rgb-api URL
