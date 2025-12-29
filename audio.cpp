// audio.cpp
#include <windows.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

#include <iostream>
#include <fstream>
#include <string>

extern int volumeLevel;
extern bool musicLoaded;

void setMusicVolume()
{
    if (!musicLoaded) return;

    int mciVol = volumeLevel * 10; // 0..1000
    std::string cmd = "setaudio music volume to " + std::to_string(mciVol);
    mciSendStringA(cmd.c_str(), NULL, 0, NULL);
}

void playMusic()
{
    std::string path = "assets/sounds/brad.mp3";

    std::ifstream f(path);
    if (!f.good())
    {
        std::cout << "[error] audio file not found: " << path << std::endl;
        return;
    }
    f.close();

    mciSendStringA("close music", NULL, 0, NULL);

    std::string cmdOpen = "open \"" + path + "\" type mpegvideo alias music";
    if (mciSendStringA(cmdOpen.c_str(), NULL, 0, NULL) != 0)
    {
        std::cout << "[error] cannot open audio file" << std::endl;
        return;
    }

    musicLoaded = true;
    setMusicVolume();
    mciSendStringA("play music repeat", NULL, 0, NULL);
}

#include <algorithm>

extern int volumeLevel;
extern bool musicLoaded;
void setMusicVolumeAttenuated(float dist)
{
    if (!musicLoaded) return;

    // exact formula ta originalã
    float factor = 1.0f / (1.0f + dist * 0.30f);
    int attenuated = int(volumeLevel * factor);

    if (attenuated < 0) attenuated = 0;
    if (attenuated > 100) attenuated = 100;

    int mciVol = attenuated * 10;
    std::string cmd = "setaudio music volume to " + std::to_string(mciVol);
    mciSendStringA(cmd.c_str(), NULL, 0, NULL);
}