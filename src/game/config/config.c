#include "config.h"
#include <allegro/config.h>

#define CONFIG_FILE_PATH "config.cfg"
#define CONFIG_SECTION_AUDIO "audio"
#define CONFIG_SECTION_GAMEPLAY "gameplay"
#define CONFIG_KEY_MUSIC_VOLUME "music_volume"
#define CONFIG_KEY_SFX_VOLUME "sfx_volume"
#define CONFIG_KEY_LIFE_BAR "life_bar"

void game_config_load_settings(Config* config) {
    printf("Loading configuration...");
    set_config_file(CONFIG_FILE_PATH);
    config->musicVolume = (uint8_t) get_config_int(CONFIG_SECTION_AUDIO, CONFIG_KEY_MUSIC_VOLUME, DEFAULT_MUSIC_VOLUME);
    config->sfxVolume = (uint8_t) get_config_int(CONFIG_SECTION_AUDIO, CONFIG_KEY_SFX_VOLUME, DEFAULT_SFX_VOLUME);
    config->lifeBar = (LifeBarEnum) get_config_int(CONFIG_SECTION_GAMEPLAY, CONFIG_KEY_LIFE_BAR, DEFAULT_LIFE_BAR);
    printOK(); 
}

void game_config_save_settings(Config* config) {
    set_config_file(CONFIG_FILE_PATH);
    set_config_int(CONFIG_SECTION_AUDIO, CONFIG_KEY_MUSIC_VOLUME, config->musicVolume);
    set_config_int(CONFIG_SECTION_AUDIO, CONFIG_KEY_SFX_VOLUME, config->sfxVolume);
    set_config_int(CONFIG_SECTION_GAMEPLAY, CONFIG_KEY_LIFE_BAR, config->lifeBar);
    flush_config_file();
}
