// audio.cpp
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

#include <iostream>
#include <string>
#include <cmath>

using std::string;
using std::cout;
using std::endl;

// =======================
// VARIABILE EXTERNE
// =======================
extern int volumeLevel;   // 0..100
extern bool musicLoaded;
extern bool radioOn;

// =======================
// PLAYLIST
// =======================
const char* songs[] =
{
    "assets/sounds/audio1.mp3",
    "assets/sounds/audio2.mp3",
    "assets/sounds/audio3.mp3"
};

int songCount = 3;
int currentSong = 0;

// =======================
// MINIAUDIO STATE
// =======================
static ma_engine engine;
static ma_sound sound;
static bool engineInitialized = false;

// =======================
// INIT ENGINE (ONCE)
// =======================
static void initAudio()
{
    if (engineInitialized)
        return;

    if (ma_engine_init(NULL, &engine) != MA_SUCCESS)
    {
        cout << "[audio] failed to init engine" << endl;
        return;
    }

    engineInitialized = true;
}


void stopMusic()
{
    if (!musicLoaded) return;

    ma_sound_stop(&sound);
}

void toggleRadio()
{
    if (!engineInitialized)
        return;

    radioOn = !radioOn;

    if (radioOn)
    {
        ma_sound_start(&sound);
    }
    else
    {
        ma_sound_stop(&sound);
    }
}


// =======================
// SET VOLUME (SAFE)
// =======================
void setMusicVolume()
{
    if (!musicLoaded) return;

    float v = volumeLevel / 100.0f;
    ma_sound_set_volume(&sound, v);
}

// =======================
// PLAY MUSIC
// =======================

void playMusic()
{
    initAudio();

    if (musicLoaded)
        ma_sound_uninit(&sound);

    const char* path = songs[currentSong];

    if (ma_sound_init_from_file(
        &engine,
        path,
        MA_SOUND_FLAG_STREAM,
        NULL,
        NULL,
        &sound) != MA_SUCCESS)
    {
        cout << "[audio] failed to load: " << path << endl;
        musicLoaded = false;
        return;
    }

    musicLoaded = true;

    ma_sound_set_looping(&sound, MA_TRUE);
    setMusicVolume();

    if (radioOn)               // 👈 IMPORTANT
        ma_sound_start(&sound);
}


// =======================
// NEXT / PREV
// =======================
void nextSong()
{
    currentSong = (currentSong + 1) % songCount;
    playMusic();
}

void prevSong()
{
    currentSong--;
    if (currentSong < 0)
        currentSong = songCount - 1;

    playMusic();
}

// =======================
// DISTANCE ATTENUATION (REAL, NO DISTORTION)
// =======================
void setMusicVolumeAttenuated(float dist)
{
    if (!musicLoaded) return;

    float factor = 1.0f / (1.0f + dist * 0.30f);
    float v = (volumeLevel / 100.0f) * factor;

    if (v < 0.0f) v = 0.0f;
    if (v > 1.0f) v = 1.0f;

    ma_sound_set_volume(&sound, v);
}
