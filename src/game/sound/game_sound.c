#include <allegro/sound.h>
#include "game_sound.h"

const char* gameMusicFilenames [GAME_MUSIC_COUNT] = {
		"assets/music/menu.s3m"
};

const char* gameSoundFilenames [GAME_SOUNDS_COUNT] = {
		"assets/sound/click.wav",
        "assets/sound/click2.wav",
        "assets/sound/click2.wav",
        "assets/sound/click.wav",
        "assets/sound/die.wav",
};

void game_snd_load_sounds() {
    snd_init_sounds(GAME_SOUNDS_COUNT, gameSoundFilenames);
}

void game_snd_play_music(GameMusic gameMusic) {
    snd_play_music(gameMusicFilenames[gameMusic]);
}

void game_snd_play_sound(GameSound gameSound) {
    snd_play_sound(gameSound, 255, 128, 1000);
}
