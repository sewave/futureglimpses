#include <string.h>
#include <stdlib.h>
#include "render_queue.h"
// --- Helper Functions ---

// Comparison function for qsort (sorts by zOrder ascending)
static int compare_commands(const void *a, const void *b) {
	return ((RenderCommand *) a)->zOrder - ((RenderCommand *) b)->zOrder;
}

static RenderCommand *render_queue_get_next_command(RenderQueue *queue, int z, RenderCommandType type) {
	if (queue->count >= MAX_COMMANDS) return NULL;
	RenderCommand *cmd = &queue->commands[queue->count++];
	cmd->type = type;
	cmd->zOrder = z;
	return cmd;
}

static void render_sprite(BITMAP *target, RenderSpriteCommand *spriteCmd) {
	switch (spriteCmd->flags) {
		case RND_FLAG_NORMAL:
			draw_sprite(target, spriteCmd->bitmap, spriteCmd->x, spriteCmd->y);
			break;
		case RND_FLAG_H_FLIP:
			draw_sprite_h_flip(target, spriteCmd->bitmap, spriteCmd->x, spriteCmd->y);
			break;
		case RND_FLAG_V_FLIP:
			draw_sprite_v_flip(target, spriteCmd->bitmap, spriteCmd->x, spriteCmd->y);
			break;
		case RND_FLAG_HV_FLIP:
			draw_sprite_vh_flip(target, spriteCmd->bitmap, spriteCmd->x, spriteCmd->y);
			break;
	}
}

// --- Manager Functions ---
void render_queue_init(RenderQueue *queue) {
	queue->count = 0;
	memset(queue->commands, 0, sizeof(RenderCommand) * MAX_COMMANDS);
}

void render_queue_clear(RenderQueue *queue) {
	queue->count = 0;
}

// --- Submission Functions ---
void render_queue_submit_clear(RenderQueue *queue, int z, int color) {
	RenderCommand *cmd = render_queue_get_next_command(queue, z, RND_CMD_CLEAR);
	if (cmd) {
		cmd->data.clear.color = color;
	}
}

void render_queue_submit_solid(RenderQueue *queue, int z, BITMAP *bmp, int x, int y) {
	RenderCommand *cmd = render_queue_get_next_command(queue, z, RND_CMD_SOLID);
	if (cmd) {
		cmd->data.solid.bitmap = bmp;
		cmd->data.solid.x = x;
		cmd->data.solid.y = y;
	}
}

void render_queue_submit_solid_partial(RenderQueue *queue,
									   int z,
									   BITMAP *bmp,
									   int originX,
									   int originY,
									   int destX,
									   int destY,
									   int width,
									   int height) {
	RenderCommand *cmd = render_queue_get_next_command(queue, z, RND_CMD_SOLID_PARTIAL);
	if (cmd) {
		cmd->data.solidPartial.bitmap = bmp;
		cmd->data.solidPartial.originX = originX;
		cmd->data.solidPartial.originY = originY;
		cmd->data.solidPartial.destX = destX;
		cmd->data.solidPartial.destY = destY;
		cmd->data.solidPartial.height = height;
		cmd->data.solidPartial.width = width;
	}
}

void render_queue_submit_sprite(RenderQueue *queue, int z, BITMAP *bmp, int x, int y, int flags) {
	RenderCommand *cmd = render_queue_get_next_command(queue, z, RND_CMD_SPRITE);
	if (cmd) {
		cmd->data.sprite.bitmap = bmp;
		cmd->data.sprite.x = x;
		cmd->data.sprite.y = y;
		cmd->data.sprite.flags = flags;
	}
}

void render_queue_submit_masked_partial(RenderQueue *queue,
										int z,
										BITMAP *bmp,
										int originX,
										int originY,
										int destX,
										int destY,
										int width,
										int height) {
	RenderCommand *cmd = render_queue_get_next_command(queue, z, RND_CMD_MASKED_PARTIAL);
	if (cmd) {
		cmd->data.maskedPartial.bitmap = bmp;
		cmd->data.maskedPartial.originX = originX;
		cmd->data.maskedPartial.originY = originY;
		cmd->data.maskedPartial.destX = destX;
		cmd->data.maskedPartial.destY = destY;
		cmd->data.maskedPartial.height = height;
		cmd->data.maskedPartial.width = width;
	}
}

void render_queue_submit_rect_fill(RenderQueue *queue, int z, int x1, int y1, int x2, int y2, int color) {
	RenderCommand *cmd = render_queue_get_next_command(queue, z, RND_CMD_RECT_FILL);
	if (cmd) {
		cmd->data.rectFill.x1 = x1;
		cmd->data.rectFill.y1 = y1;
		cmd->data.rectFill.x2 = x2;
		cmd->data.rectFill.y2 = y2;
		cmd->data.rectFill.color = color;
	}
}

void render_queue_submit_rect(RenderQueue *queue, int z, int x1, int y1, int x2, int y2, int color) {
	RenderCommand *cmd = render_queue_get_next_command(queue, z, RND_CMD_RECT);
	if (cmd) {
		cmd->data.rect.x1 = x1;
		cmd->data.rect.y1 = y1;
		cmd->data.rect.x2 = x2;
		cmd->data.rect.y2 = y2;
		cmd->data.rect.color = color;
	}
}

void render_queue_submit_text(RenderQueue *queue, int z, FONT *font, const char *text, int x, int y, int color, int background) {
	RenderCommand *cmd = render_queue_get_next_command(queue, z, RND_CMD_TEXT);
	if (cmd) {
		cmd->data.text.font = font;
		cmd->data.text.text = text;
		cmd->data.text.x = x;
		cmd->data.text.y = y;
		cmd->data.text.color = color;
		cmd->data.text.background = background;
	}
}

// --- Execution Function ---
void render_queue_execute(RenderQueue *queue, BITMAP *target) {
	if (queue->count == 0) return;

	qsort(queue->commands, queue->count, sizeof(RenderCommand), compare_commands);

	RenderCommand *cmd = &queue->commands[0];
	for (int i = 0; i < queue->count; ++i, ++cmd) {
		acquire_bitmap(target);

		switch (cmd->type) {
			case RND_CMD_CLEAR:
				clear_to_color(target, cmd->data.clear.color);
				break;
			case RND_CMD_SOLID:
				blit(cmd->data.solid.bitmap,
					 target,
					 0, 0,
					 cmd->data.solid.x,
					 cmd->data.solid.y,
					 cmd->data.solid.bitmap->w,
					 cmd->data.solid.bitmap->h);
				break;
			case RND_CMD_SOLID_PARTIAL:
				blit(cmd->data.solidPartial.bitmap,
					 target,
					 cmd->data.solidPartial.originX,
					 cmd->data.solidPartial.originY,
					 cmd->data.solidPartial.destX,
					 cmd->data.solidPartial.destY,
					 cmd->data.solidPartial.width,
					 cmd->data.solidPartial.height);
			case RND_CMD_SPRITE:
				render_sprite(target, &cmd->data.sprite);
				break;
			case RND_CMD_MASKED_PARTIAL:
				masked_blit(cmd->data.maskedPartial.bitmap,
							target,
							cmd->data.maskedPartial.originX,
							cmd->data.maskedPartial.originY,
							cmd->data.maskedPartial.destX,
							cmd->data.maskedPartial.destY,
							cmd->data.maskedPartial.width,
							cmd->data.maskedPartial.height);
				break;
			case RND_CMD_RECT:
				rect(target,
						 cmd->data.rect.x1, cmd->data.rect.y1,
						 cmd->data.rect.x2, cmd->data.rect.y2,
						 cmd->data.rect.color);
				break;	
			case RND_CMD_RECT_FILL:
				rectfill(target,
						 cmd->data.rectFill.x1, cmd->data.rectFill.y1,
						 cmd->data.rectFill.x2, cmd->data.rectFill.y2,
						 cmd->data.rectFill.color);
				break;
			case RND_CMD_TEXT:
				textout_ex(target,
						   cmd->data.text.font,
						   cmd->data.text.text,
						   cmd->data.text.x,
						   cmd->data.text.y,
						   cmd->data.text.color,
						   cmd->data.text.background);
				break;
		}
		release_bitmap(target);
	}

	render_queue_clear(queue);
}
