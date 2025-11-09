#ifndef SOUND_H
#define SOUND_H
#include "../common.h"

typedef enum {
    MUSIC_TYPE_MIDI,
    MUSIC_TYPE_MOD
} MusicType;

InitializationStatusEnum snd_init_system(int totalVoices, int musicVoices, MusicType musicType);
void snd_play_music(char* filename);
void snd_pause_music(void);
void snd_resume_music(void);
void snd_stop_music(void);

#endif /* SOUND_H */
