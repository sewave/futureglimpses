#ifndef PLAYER_ATTACKED_H
#define PLAYER_ATTACKED_H
#include "game/game_lib.h"

void player_attacked_update(PlayerAttackedData *playerAttackedData);
void player_attacked_render(PlayerAttackedData *playerAttackedData, RenderQueue *renderQueue);
void player_attacked_register_attack(PlayerAttackedData *playerAttackedData, uint16_t xPosition, uint16_t yPosition);
void player_attacked_init(PlayerAttackedData *playerAttackedData);

#endif /* PLAYER_ATTACKED_H */
