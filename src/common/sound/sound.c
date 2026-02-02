#include <stdio.h>
#include "common/sound.h"
#include <allegro/sound.h>
#include <jgmod.h>

static JGMOD *currentModMusic = NULL;
static MIDI *currentMidiMusic = NULL;
static MusicType currentMusicType = MUSIC_TYPE_MIDI;

static SAMPLE **sounds = NULL;
static int totalSounds = 0;
static uint8_t soundInitialized = FALSE;

/* PRIVATE AREA */
void _snd_destroy_sound(int soundIndex) {
	if (sounds[soundIndex] != NULL) {
		SAMPLE *sample = sounds[soundIndex];
		stop_sample(sample);
		destroy_sample(sample);
		sounds[soundIndex] = NULL;
	}
}

/* PUBLIC AREA */

/*
    Initializes the sound system with the specified number of total voices and music voices.
    The musicType parameter specifies whether to use MOD or MIDI music.
    Returns INITIALIZATION_OK on success, or INITIALIZATION_ERROR on failure.
*/
InitializationStatusEnum snd_init_system(int totalVoices, int musicVoices, MusicType musicType) {
	printf("Initializing sound system...");
	remove_sound();
	currentMusicType = musicType;
	if (currentMusicType == MUSIC_TYPE_MIDI) {
		reserve_voices(totalVoices, musicVoices);
	} else {
		reserve_voices(totalVoices, -1);
	}
	if (install_sound(DIGI_AUTODETECT, MIDI_AUTODETECT, NULL) < ALLEGRO_INIT_OK) return INITIALIZATION_ERROR;
	if (currentMusicType == MUSIC_TYPE_MOD) {
		if (install_mod(musicVoices) < ALLEGRO_INIT_OK) return INITIALIZATION_ERROR;
	}
	common_print_ok();
	soundInitialized = TRUE;
	return INITIALIZATION_OK;
}

void snd_play_music(const char *filename) {
	if(!soundInitialized) return;
	snd_stop_music();
	if (currentMusicType == MUSIC_TYPE_MIDI) {
		currentMidiMusic = load_midi(filename);
		play_midi(currentMidiMusic, TRUE);
	} else {
		currentModMusic = load_mod((char *) filename);
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

void snd_init_sounds(int numSounds, const char **soundFilenames) {
	if(!soundInitialized) return;
	totalSounds = numSounds;
	sounds = (SAMPLE **) calloc(totalSounds, sizeof(SAMPLE *));
	for (int i = 0; i < totalSounds; i++) {
		_snd_destroy_sound(i);
		sounds[i] = load_sample(soundFilenames[i]);
		common_print_init_step();
	}
}

void snd_play_sound(int soundIndex, int volume, int pan, int freq) {
	if(!soundInitialized) return;
	if (soundIndex < 0 || soundIndex >= totalSounds) return;
	if (sounds[soundIndex] == NULL) return;
	play_sample(sounds[soundIndex], volume, pan, freq, FALSE);
}

void snd_destroy_sounds() {
	if (sounds != NULL) {
		for (int i = 0; i < totalSounds; i++) _snd_destroy_sound(i);
		free(sounds);
		sounds = NULL;
		totalSounds = 0;
	}
}
