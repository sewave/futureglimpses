#ifndef SOUND_H
#define SOUND_H
#include "../common.h"
#include <allegro/sound.h>

typedef enum {
    MUSIC_TYPE_MIDI,
    MUSIC_TYPE_MOD
} MusicType;

InitializationStatusEnum snd_init_system(int totalVoices, int musicVoices, MusicType musicType);
void snd_play_music(const char* filename);
void snd_pause_music(void);
void snd_resume_music(void);
void snd_stop_music(void);
void snd_init_sounds(int numSounds);
void snd_load_sound(int soundIndex, SAMPLE* sample);
void snd_play_sound(int soundIndex, int volume, int pan, int freq);
void snd_destroy_sounds(void);

#endif /* SOUND_H */
