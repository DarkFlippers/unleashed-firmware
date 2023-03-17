#ifndef sound_h
#define sound_h
#include <furi.h>
#include <furi_hal.h>
#include <stdint.h>
#include "doom_music_player_worker.h"

//static const char dspistol[] = "AnyConv:d=,o=,b=120:408,40p,40p,40p,40p,405,40p,40p,40p,405,30p.,30p.,30p.,13p";
static const char dsintro[] =
    "Doom:d=32,o=4,b=56:f,f,f5,f,f,d#5,f,f,c#5,f,f,b,f,f,c5,c#5,f,f,f5,f,f,d#5,f,f,c#5,f,f,8b.,f,f,f5,f,f,d#5,f,f,c#5,f,f,b,f,f,c5,c#5,f,f,f5,f,f,d#5,f,f,c#5,f,f,8b.,a#,a#,a#5,a#,a#,g#5,a#,a#,f#5,a#,a#,e5,a#,a#,f5,f#5,a#,a#,a#5,a#,a#,g#5,a#,a#,f#5,a#,a#,8e5";
//static const char dsgetpow[] = "dsgetpow:d=,o=,b=120:407,40p,30.6,407,40p,406,40p,407,40p,40p,407,30p.,407";
//static const char dsnoway[] = "dsnoway:d=,o=,b=120:407,30.4";

#define MUSIC_PLAYER_SEMITONE_HISTORY_SIZE 4
static const float MUSIC_PLAYER_VOLUMES[] = {0, .25, .5, .75, 1};

typedef struct {
    uint8_t semitone_history[MUSIC_PLAYER_SEMITONE_HISTORY_SIZE];
    uint8_t duration_history[MUSIC_PLAYER_SEMITONE_HISTORY_SIZE];

    uint8_t volume;
    uint8_t semitone;
    uint8_t dots;
    uint8_t duration;
    float position;
} MusicPlayerModel;

typedef struct {
    MusicPlayerModel* model;
    MusicPlayerWorker* worker;
    FuriMutex** model_mutex;
} MusicPlayer;

#endif