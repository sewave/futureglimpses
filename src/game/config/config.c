#include "config.h"
#include <allegro/config.h>
#include <stdio.h>

#define CONFIG_FILE_PATH "config.cfg"
#define CONFIG_SECTION_AUDIO "audio"
#define CONFIG_SECTION_GAMEPLAY "gameplay"
#define CONFIG_SECTION_UI "ui"
#define CONFIG_KEY_MUSIC_VOLUME "music_volume"
#define CONFIG_KEY_SFX_VOLUME "sfx_volume"
#define CONFIG_KEY_LIFE_BAR "life_bar"
#define CONFIG_KEY_LANGUAGE "language"

void game_config_load_settings(Config* config) {
    printf("Loading configuration...");
    set_config_file(CONFIG_FILE_PATH);
    config->musicVolume = (uint8_t) get_config_int(CONFIG_SECTION_AUDIO, CONFIG_KEY_MUSIC_VOLUME, DEFAULT_MUSIC_VOLUME);
    config->sfxVolume = (uint8_t) get_config_int(CONFIG_SECTION_AUDIO, CONFIG_KEY_SFX_VOLUME, DEFAULT_SFX_VOLUME);
    config->lifeBar = (LifeBarEnum) get_config_int(CONFIG_SECTION_GAMEPLAY, CONFIG_KEY_LIFE_BAR, DEFAULT_LIFE_BAR);
    config->language = (GameLanguageEnum) get_config_int(CONFIG_SECTION_UI, CONFIG_KEY_LANGUAGE, DEFAULT_LANGUAGE);
    printOK(); 
}

void game_config_save_settings(Config* config) {
    set_config_file(CONFIG_FILE_PATH);
    set_config_int(CONFIG_SECTION_AUDIO, CONFIG_KEY_MUSIC_VOLUME, config->musicVolume);
    set_config_int(CONFIG_SECTION_AUDIO, CONFIG_KEY_SFX_VOLUME, config->sfxVolume);
    set_config_int(CONFIG_SECTION_GAMEPLAY, CONFIG_KEY_LIFE_BAR, config->lifeBar);
    set_config_int(CONFIG_SECTION_UI, CONFIG_KEY_LANGUAGE, config->language);
    flush_config_file();
}

uint8_t game_config_get_audio_max_volume(const GameContext *context) {
	return 255;
}

uint8_t game_config_get_audio_min_volume(const GameContext *context) {
	return 0;
}

void game_config_set_audio_music_volume(GameContext *context, uint8_t value) {
	context->config.musicVolume = value;
	set_volume(context->config.sfxVolume, context->config.musicVolume);
}

uint8_t game_config_get_audio_music_volume(const GameContext *context) {
	return context->config.musicVolume;
}

uint8_t game_config_get_audio_sfx_volume(const GameContext *context) {
	return context->config.sfxVolume;
}

void game_config_set_audio_sfx_volume(GameContext *context, uint8_t value) {
	context->config.sfxVolume = value;
	set_volume(context->config.sfxVolume, context->config.musicVolume);
}

uint8_t game_config_get_gameplay_life_bars(const GameContext *context) {
	return context->config.lifeBar;
}

void game_config_set_gameplay_life_bars(GameContext *context, uint8_t value) {
	context->config.lifeBar = (LifeBarEnum) value;
}

uint8_t game_config_get_language(const GameContext *context) {
	return context->config.language;
}

void game_config_set_language(GameContext *context, uint8_t value) {
	context->config.language = (GameLanguageEnum) value;
    game_text_set_language(context->config.language);
}
