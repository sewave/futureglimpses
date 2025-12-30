#include <allegro/sound.h>
#include "game_sound.h"
#include <stdio.h>

// TODO add music
const char* gameMusicFilenames [GAME_MUSIC_COUNT] = {
		"assets/music/none.mid",
        "assets/music/none.mid",
        "assets/music/none.mid",
        "assets/music/none.mid",
        "assets/music/map1.mid",
        "assets/music/none.mid",
        "assets/music/none.mid",
};

// TODO add sounds
const char* gameSoundFilenames [GAME_SOUNDS_COUNT] = {
		"assets/sound/click.wav",
        "assets/sound/click.wav",
        "assets/sound/ironhit.wav",
        "assets/sound/click.wav",
        "assets/sound/click.wav",
        "assets/sound/arrowthr.wav",
        "assets/sound/click.wav",
        "assets/sound/click.wav",
        "assets/sound/click.wav",
        "assets/sound/click.wav",
        "assets/sound/die.wav",
        "assets/sound/click.wav",
        "assets/sound/click.wav",
};

void game_snd_load_sounds() {
    printf("Loading game sounds [");
    snd_init_sounds(GAME_SOUNDS_COUNT, gameSoundFilenames);
    printOKSteps();
}

void game_snd_play_music(GameMusic gameMusic) {
    snd_play_music(gameMusicFilenames[gameMusic]);
}

void game_snd_play_sound(GameSound gameSound) {
    snd_play_sound(gameSound, 255, 128, 1000);
}
