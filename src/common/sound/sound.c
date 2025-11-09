#include "sound.h"
#include <allegro/sound.h>
#include <jgmod.h>

JGMOD *currentModMusic = NULL;
MIDI *currentMidiMusic = NULL;
MusicType currentMusicType = MUSIC_TYPE_MIDI;

/*
    Initializes the sound system with the specified number of total voices and music voices.
    The musicType parameter specifies whether to use MOD or MIDI music.
    Returns INITIALIZATION_OK on success, or INITIALIZATION_ERROR on failure.
*/
InitializationStatusEnum snd_init_system(int totalVoices, int musicVoices, MusicType musicType) {
	remove_sound();
	if (install_sound(DIGI_AUTODETECT, MIDI_AUTODETECT, NULL) < ALLEGRO_INIT_OK) return INITIALIZATION_ERROR;
	currentMusicType = musicType;
	if (currentMusicType == MUSIC_TYPE_MIDI) {
		reserve_voices(totalVoices, musicVoices);
	} else {
		reserve_voices(totalVoices, -1);
		if (install_mod(musicVoices) < ALLEGRO_INIT_OK) return INITIALIZATION_ERROR;
	}
	return INITIALIZATION_OK;
}

void snd_play_music(char *filename) {
	snd_stop_music();
	if (currentMusicType == MUSIC_TYPE_MIDI) {
		currentMidiMusic = load_midi(filename);
		play_midi(currentMidiMusic, TRUE);
	} else {
		currentModMusic = load_mod(filename);
		play_mod(currentModMusic, TRUE);
	}
}

void snd_pause_music(void) {
	if (currentModMusic != NULL) pause_mod();
	if (currentMidiMusic != NULL) midi_pause();
}

void snd_resume_music(void) {
	if (currentModMusic != NULL) resume_mod();
	if (currentMidiMusic != NULL) midi_resume();
}

void snd_stop_music(void) {
	if (currentModMusic != NULL) {
		stop_mod();
		destroy_mod(currentModMusic);
		currentModMusic = NULL;
	}
	if (currentMidiMusic != NULL) {
		stop_midi();
		destroy_midi(currentMidiMusic);
		currentMidiMusic = NULL;
	}
}
