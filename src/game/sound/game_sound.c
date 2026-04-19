#include <allegro/sound.h>
#include "game/sound/game_sound.h"
#include <stdio.h>

const char* gameMusicFilenames [GAME_MUSIC_COUNT] = {
        [GAME_MUSIC_INTRO] = "assets/music/intro.mid",
        [GAME_MUSIC_MENUS] = "assets/music/menus.mid",
        [GAME_MUSIC_VICTORY] = "assets/music/victory.mid",
        [GAME_MUSIC_DEFEAT] = "assets/music/defeat.mid",
        [GAME_MUSIC_MAP_1] = "assets/music/map1.mid",
        [GAME_MUSIC_MAP_2] = "assets/music/map2.mid",
        [GAME_MUSIC_MAP_3] = "assets/music/map3.mid",
};

const char *gameSoundFilenames[GAME_SOUNDS_COUNT] = {
		[GAME_SOUND_CLICK] = "assets/sound/click.wav",
		[GAME_SOUND_IRON_HIT] = "assets/sound/ironhit.wav",
		[GAME_SOUND_CHOP] = "assets/sound/chop.wav",
		[GAME_SOUND_WORK] = "assets/sound/work.wav",
		[GAME_SOUND_GOLD_HIT] = "assets/sound/goldhit.wav",
		[GAME_SOUND_THROW_ARROW] = "assets/sound/arrowthr.wav",
		[GAME_SOUND_ARROW_HIT] = "assets/sound/arrowhit.wav",
		[GAME_SOUND_FIREBALL_LAUNCH] = "assets/sound/fblaunch.wav",
		[GAME_SOUND_FIREBALL_EXPLOSION] = "assets/sound/fbexplo.wav",
		[GAME_SOUND_BUILDING_BUILD] = "assets/sound/work.wav",
		[GAME_SOUND_BUILDING_CRUMBLE] = "assets/sound/crumble.wav",
		[GAME_SOUND_NOT_VALID] = "assets/sound/notvalid.wav",
		[GAME_SOUND_DIE] = "assets/sound/die.wav",
		[GAME_SOUND_AJUM] = "assets/sound/ajum.wav",
		[GAME_SOUND_JA] = "assets/sound/ja.wav",
		[GAME_SOUND_INCOMING_ATTACK] = "assets/sound/attack.wav",
};

void game_snd_load_sounds() {
    printf("Loading game sounds [");
    snd_init_sounds(GAME_SOUNDS_COUNT, gameSoundFilenames);
	common_print_ok_steps();
}

void game_snd_play_music(GameMusic gameMusic) {
    snd_play_music(gameMusicFilenames[gameMusic]);
}

void game_snd_play_sound(GameSound gameSound) {
    snd_play_sound(gameSound, 255, 128, 1000);
}
