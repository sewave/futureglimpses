#ifndef CONFIG_H
#define CONFIG_H
#include "../game_lib.h"

void game_config_load_settings(Config* config);
void game_config_save_settings(Config* config);

uint8_t game_config_get_audio_max_volume(const GameContext *context);
uint8_t game_config_get_audio_min_volume(const GameContext *context);
void game_config_set_audio_music_volume(GameContext *context, uint8_t value);
uint8_t game_config_get_audio_music_volume(const GameContext *context);
uint8_t game_config_get_audio_sfx_volume(const GameContext *context);
void game_config_set_audio_sfx_volume(GameContext *context, uint8_t value);
uint8_t game_config_get_gameplay_life_bars(const GameContext *context);
void game_config_set_gameplay_life_bars(GameContext *context, uint8_t value);
uint8_t game_config_get_language(const GameContext *context);
void game_config_set_language(GameContext *context, uint8_t value);

#endif /* CONFIG_H */
