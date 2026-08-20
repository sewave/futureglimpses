#include "game/map/lazy_workers.h"
#include "game/video/game_video.h"
#include "game/video/gfx.h"
#include "game/unit/unit.h"
#include "game/map/selection.h"
#include "game/sound/game_sound.h"

#define LAZY_WORKER_TIME SEC_TO_FRAMES(45)
#define IDLE_COUNTER_INCREMENT 15
#define NON_WORKING_COUNTER_INCREMENT 1
#define LAZY_WORKERS_BUTTON_X 72
#define LAZY_WORKERS_BUTTON_Y 12
#define LAZY_WORKERS_BUTTON_Z_ORDER UI_Z_ORDER + 702
#define LAZY_WORKERS_BUTTON_BORDER_Z_ORDER UI_Z_ORDER + 703
#define LAZY_WORKERS_BUTTON_TEXT_Z_ORDER UI_Z_ORDER + 704
#define LAZY_WORKERS_TEXT_X_OFFSET 26
#define LAZY_WORKERS_TEXT_Y_OFFSET 6

static const char* MAX_WORKERS_MESSAGE = "9+";
static char lazyWorkerQuantityString[8];

static uint8_t is_mouse_in_lazy_workers_button(GameContext *context) {
    BITMAP *lazyWorkersButton = game_gfx_get_lazy_workers_button();
    return context->mouseStatus.x >= LAZY_WORKERS_BUTTON_X &&
           context->mouseStatus.x <= LAZY_WORKERS_BUTTON_X + lazyWorkersButton->w &&
           context->mouseStatus.y >= LAZY_WORKERS_BUTTON_Y &&
           context->mouseStatus.y <= LAZY_WORKERS_BUTTON_Y + lazyWorkersButton->h;
}

static uint8_t is_mouse_pressed_in_lazy_workers_button(GameContext *context) {
    return context->mouseStatus.isLeftPressed && is_mouse_in_lazy_workers_button(context);
}

void lazy_workers_update(GameContext *context) {
    context->lazyWorkersCount = 0;

    // Iterate over active list of units with pointers
    GameUnit **activeList = context->activeUnits;
	for (int i = 0; i < context->activeUnitCount; i++, activeList++) {
        GameUnit *unit = *activeList;
        if (unit && unit->controller == UNIT_CONTROLLER_PLAYER && unit->type == UNIT_TYPE_WORKER) {
            WorkerData *workerData = &unit->typed.workerData;
            
            switch(unit->state) {
                case UNIT_STATE_IDLE:
                    workerData->idleCounter += IDLE_COUNTER_INCREMENT;
                    break;
                case UNIT_STATE_DIE:
                case UNIT_STATE_WORK:
                    workerData->idleCounter = 0;
                    break;
                default:
                    workerData->idleCounter += NON_WORKING_COUNTER_INCREMENT;
                    break;
            }
            // Add worker to lazy list if time exceeds threshold and we have less than max
            if (workerData->idleCounter > LAZY_WORKER_TIME && context->lazyWorkersCount < MAX_LAZY_WORKERS) {
                context->lazyWorkers[context->lazyWorkersCount++] = unit;
            }
        }
    }
}

uint8_t lazy_workers_handle_input(GameContext *context) {
    if(context->lazyWorkersCount == 0) return FALSE;
    if(keyboard_is_key_pressed(KEY_F1) || is_mouse_pressed_in_lazy_workers_button(context)) {
        game_snd_play_sound(GAME_SOUND_CLICK);
        GameUnit *selected = NULL;
        if(context->selectedUnitCount == 1) {
            selected = game_unit_get_by_id(context, context->selectedUnits[0]);
            // If the selected unit is not a lazy worker in the list, we will deselect it
            if(selected) {
                int8_t lazyWorkerIndex = -1;
                for(int i = 0; i < context->lazyWorkersCount; i++) {
                    if(context->lazyWorkers[i] == selected) {
                        lazyWorkerIndex = i;
                        break;
                    }
                }
                if(lazyWorkerIndex >= 0) {
                    selected = context->lazyWorkers[(lazyWorkerIndex + 1) % context->lazyWorkersCount];
                }
                else {
                    selected = NULL;
                }
            }
        } 
        game_selection_clear(context);
        if(selected == NULL) {
            game_selection_add_unit(context, context->lazyWorkers[0]);
        }
        else {
            game_selection_add_unit(context, selected);
        }
        game_selection_center_camera_on_selection(context);
        return TRUE;
    }
    return FALSE;
}

void lazy_workers_render(GameContext *context, RenderQueue *renderQueue) {
    if (context->lazyWorkersCount == 0) return;
    // For now we will render the number of lazy workers on the screen, you can customize this to render specific UI elements
    if(context->lazyWorkersCount == MAX_LAZY_WORKERS) {
        strcpy(lazyWorkerQuantityString, MAX_WORKERS_MESSAGE);
    }
    else {
        itoa(context->lazyWorkersCount, lazyWorkerQuantityString, 10);
    }
    BITMAP *lazyWorkersButton = game_gfx_get_lazy_workers_button();
    if (lazyWorkersButton) {
        render_queue_submit_solid(renderQueue, LAZY_WORKERS_BUTTON_Z_ORDER,
            lazyWorkersButton, LAZY_WORKERS_BUTTON_X, LAZY_WORKERS_BUTTON_Y);
        // If the mouse is hovering, draw a yellow rectagle around
        if (is_mouse_in_lazy_workers_button(context) && !context->mouseStatus.isLeftDown) {
            render_queue_submit_rect(renderQueue, LAZY_WORKERS_BUTTON_BORDER_Z_ORDER, LAZY_WORKERS_BUTTON_X, LAZY_WORKERS_BUTTON_Y,
                            LAZY_WORKERS_BUTTON_X + lazyWorkersButton->w - 1,
                            LAZY_WORKERS_BUTTON_Y + lazyWorkersButton->h - 1, PAL_COLOR_YELLOW);
        }
        // Here you would add code to render lazyWorkerQuantityString to the screen using your rendering system, for example:
        render_queue_submit_text(renderQueue, LAZY_WORKERS_BUTTON_TEXT_Z_ORDER, context->gameFont, lazyWorkerQuantityString,
                                            LAZY_WORKERS_BUTTON_X + LAZY_WORKERS_TEXT_X_OFFSET,
                                            LAZY_WORKERS_BUTTON_Y + LAZY_WORKERS_TEXT_Y_OFFSET,
                                            PAL_COLOR_BLACK, TRANSPARENT_INDEX);
    }

}
