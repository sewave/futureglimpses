#ifndef GAME_SOUND_H
#define GAME_SOUND_H
#include "../../common/sound/sound.h"

#define MOD_VOICES 4
#define GAME_VOICES 8

typedef enum {
	GAME_MUSIC_TITLE,
    GAME_MUSIC_COUNT
} GameMusic;

typedef enum {
	GAME_SOUND_SEA_WAVES,
	GAME_SOUND_CLICK,
	GAME_SOUND_HUMAN_STEP,
	GAME_SOUND_HIT,
	GAME_SOUND_DIE,
	GAME_SOUNDS_COUNT
} GameSound;

void game_snd_play_music(GameMusic gameMusic);
void game_snd_play_sound(GameSound gameSound);
void game_snd_load_sounds();

#endif /* GAME_SOUND_H */
