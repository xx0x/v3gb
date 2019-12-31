/********************************************************************
 * V3, a VVVVVV GameBoy remake                                      *
 *  by                                                              *
 * Vaclav Mach		                                                *
 ********************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <rand.h>
#include <types.h>
#include <gb/console.h>
#include <gb/gb.h>
#include <gb/drawing.h>

// bitmaps
#include "img/level_tiles.c"
#include "img/player_img.c"
#include "img/checkpoint.c"
#include "img/grid.c"
#include "img/mirrorline.c"

// game definitions
#include "defs/player.h"
#include "defs/object.h"
#include "defs/level.h"
#include "defs/player_checkpoint.h"

// library files
#include "inc/sound.c"

// levels
#include "levels/level_1.c"
#include "levels/level_2.c"
#include "levels/level_3.c"
#include "levels/level_4.c"
#include "levels/level_5.c"
#include "levels/level_6.c"
#include "levels/level_7.c"
#include "levels/test_level.c"

/* ************************************************************ */
// NEW TYPEDEFS
#define BYTE_IS_UNSIGNED 1

#define START_LEVEL level_1

#define LEVEL_TILE_SIZE 8

/* level size (160x144) */
#define LEVEL_WIDTH_PX SCREENWIDTH
#define LEVEL_HEIGHT_PX SCREENHEIGHT

#define LEVEL_WIDTH 20
#define LEVEL_HEIGHT 18

/* player size */
#define PLAYER_WIDTH_PX 8
#define PLAYER_HEIGHT_PX 16

/* player */
#define MIN_PX 0
#define MIN_PY 0

#define MAX_PX LEVEL_WIDTH_PX - PLAYER_WIDTH_PX
#define MAX_PY LEVEL_HEIGHT_PX - PLAYER_HEIGHT_PX

#define PLAYER_SPRITE 0
#define MESSAGE_SPIRTE 10
#define OBJECTS_SPRITES_START 4

#define SPRITE_SIZE_TILES 2

#define PLAYER_FLY_SPEED 4
#define PLAYER_SPEED 2

#define BG_TILES_COUNT 15

const unsigned char *const msg_died = "YOU DIED";
const unsigned char *const msg_start = " START! ";
const unsigned char *const msg_empty = "        ";

INT16 player_x, player_y;
BOOLEAN player_flipped, player_flipping_disabled, player_flying_disabled;
INT16 player_last_move;
BOOLEAN player_dead;
PLAYER_CHECKPOINT player_checkpoint;

//UINT8 gridx;

LEVEL *current_level;

UWORD pw;

BOOLEAN flip_key_pressed;

void show_start()
{
}

void show_died()
{
}

void hide_msg()
{
	HIDE_WIN;
	SHOW_BKG;
}

void init_screen()
{
	set_bkg_data(0, BG_TILES_COUNT, level_tiles); // copy 11 tiles to memory
	SHOW_BKG;
}

void init_level()
{
	set_bkg_tiles(0, 0, LEVEL_WIDTH, LEVEL_HEIGHT, current_level->map);
}

void init_player()
{
	player_x = current_level->start_x;
	player_y = current_level->start_y;
	player_flying_disabled = FALSE;
	player_flipped = FALSE;
	player_flipping_disabled = FALSE;
	player_dead = FALSE;
	player_last_move = 0;

	SHOW_SPRITES;
}

void init_objects()
{
	// counters etc
	UINT8 i, j, k;
	UINT8 sprite_start;
	OBJECT_TYPE type;

	// object values
	UINT8 x, y, width, height;
	unsigned char *img;

	if (current_level->objects_count == 0)
	{
		return;
	}

	sprite_start = OBJECTS_SPRITES_START;

	// each object
	for (i = 0; i < (current_level->objects_count); i++)
	{

		// get values
		x = (UINT8)(current_level->objects[i].x);
		y = (UINT8)(current_level->objects[i].y);
		width = (UINT8)(current_level->objects[i].width);
		height = (UINT8)(current_level->objects[i].height);
		type = (UINT8)(current_level->objects[i].type);

		// find image
		switch (type)
		{
		case OBJECT_GRID:
			img = object_grid_img;
			break;
		case OBJECT_MIRRORLINE:
			img = object_mirrorline_img;
			break;
		case OBJECT_CHECKPOINT:

			// is it active?
			if (i == player_checkpoint.id && current_level == player_checkpoint.level)
			{
				img = object_checkpoint_active_img;
			}
			else
			{
				img = object_checkpoint_img;
			}
			break;
		}

		// for each sprite, set it and move to the position
		for (j = 0; j < width; j++)
		{
			for (k = 0; k < height; k = k + 2)
			{
				set_sprite_data(sprite_start, SPRITE_SIZE_TILES, img);
				set_sprite_tile(sprite_start, sprite_start);
				move_sprite(sprite_start, x + j * LEVEL_TILE_SIZE + 8, y + 16 + k * LEVEL_TILE_SIZE);
				sprite_start += SPRITE_SIZE_TILES;
			}
		}
	}
}

void destroy_objects()
{
	UINT8 i, j, k, sprite_start;

	// if no level, dont destroy
	if (current_level == NULL)
		return;

	sprite_start = OBJECTS_SPRITES_START;

	// hide each object
	for (i = 0; i < (current_level->objects_count); i++)
	{
		for (j = 0; j < current_level->objects[i].width; j++)
		{
			for (k = 0; k < current_level->objects[i].height; k = k + 2)
			{
				move_sprite(sprite_start, 0, 0);
				sprite_start += SPRITE_SIZE_TILES;
			}
		}
	}
}

void load_level(LEVEL *level)
{
	destroy_objects();
	current_level = level;
	init_level();
	init_objects();
}

void move_objects()
{
	// counters etc
	UINT8 i, j;
	UINT8 sprite_start;
	OBJECT_TYPE type;

	// object values
	UINT8 x, y, width, height, max_x, min_x, direction_x;

	if (current_level->objects_count == 0)
	{
		return;
	}

	sprite_start = OBJECTS_SPRITES_START;

	// each object
	for (i = 0; i < (current_level->objects_count); i++)
	{

		// get values
		type = current_level->objects[i].type;
		x = (UINT8)(current_level->objects[i].x);
		y = (UINT8)(current_level->objects[i].y);
		width = (UINT8)(current_level->objects[i].width);
		height = (UINT8)(current_level->objects[i].height);

		// find what to do?
		switch (type)
		{
		case OBJECT_GRID:

			max_x = (UINT8)(current_level->objects[i].max_x);
			min_x = (UINT8)(current_level->objects[i].min_x);
			direction_x = current_level->objects[i].direction_x;

			// change direction?
			if (x >= max_x || x <= min_x)
			{
				current_level->objects[i].direction_x = current_level->objects[i].direction_x * (-1);
			}
			current_level->objects[i].x += current_level->objects[i].direction_x;

			// for each sprite, move to the position
			for (j = 0; j < width; j++)
			{
				move_sprite(sprite_start, x + j * LEVEL_TILE_SIZE + 8, y + 16);
				sprite_start += 2;
			}
			break;
		}
	}
}

/* returns tile on given coordinates */
UINT8 current_level_get_tile(INT16 x, INT16 y)
{
	INT16 level_x = x / LEVEL_TILE_SIZE;
	INT16 level_y = y / LEVEL_TILE_SIZE;
	return current_level->map[level_x + LEVEL_WIDTH * level_y];
}

void player_paint()
{
	INT16 x = player_x + PLAYER_WIDTH_PX;
	INT16 y = player_y + PLAYER_HEIGHT_PX;

	SPRITES_8x16;
	if (player_last_move < 0)
	{
		if (!player_flipped)
		{
			set_sprite_data(PLAYER_SPRITE, 2, player_img_left);
		}
		else
		{
			set_sprite_data(PLAYER_SPRITE, 2, player_img_flipped_left);
		}
	}
	else
	{
		if (!player_flipped)
		{
			set_sprite_data(PLAYER_SPRITE, 2, player_img_right);
		}
		else
		{
			set_sprite_data(PLAYER_SPRITE, 2, player_img_flipped_right);
		}
	}

	move_sprite(PLAYER_SPRITE, (UINT8)x, (UINT8)y);
}

void player_die()
{
	UINT8 i;

	sound_die();

	player_dead = TRUE;
	player_flying_disabled = TRUE;
	player_paint();
	show_died();

	for (i = 0; i < 10; i++)
	{
		move_sprite(PLAYER_SPRITE, 0, 0);
		delay(50);
		move_sprite(PLAYER_SPRITE, player_x + PLAYER_WIDTH_PX, player_y + PLAYER_HEIGHT_PX);
		delay(50);
	}

	hide_msg();
	init_player();

	if (player_checkpoint.level != NULL)
	{
		load_level(player_checkpoint.level);
		player_x = player_checkpoint.x;
		player_y = player_checkpoint.y;
	}
}

void player_set_checkpoint(UINT8 checkpoint_id)
{

	UINT8 i, sprite_start;

	if (checkpoint_id != player_checkpoint.id || player_checkpoint.level != current_level)
	{
		// play sound
		sound_check();

		// set it!
		player_checkpoint.id = checkpoint_id;
		player_checkpoint.x = current_level->objects[checkpoint_id].x;
		player_checkpoint.y = current_level->objects[checkpoint_id].y - 4;
		player_checkpoint.level = current_level;

		// change all other images to default (self to active)
		sprite_start = OBJECTS_SPRITES_START;
		for (i = 0; i < (current_level->objects_count); i++)
		{

			if (current_level->objects[i].type == OBJECT_CHECKPOINT)
			{
				if (i == checkpoint_id)
				{
					// change image
					set_sprite_data(sprite_start, SPRITE_SIZE_TILES, object_checkpoint_active_img);
				}
				else
				{
					// if other checkpoint, set to default image
					set_sprite_data(sprite_start, SPRITE_SIZE_TILES, object_checkpoint_img);
				}
			}

			sprite_start += (SPRITE_SIZE_TILES * current_level->objects[i].width) * ((current_level->objects[i].height + 1) / 2);
		}
	}
}

void object_found(UINT8 obj_id, OBJECT_TYPE type)
{
	UINT8 x;

	switch (type)
	{
	case OBJECT_GRID:

		player_flying_disabled = TRUE;
		player_flipping_disabled = FALSE;

		if (current_level->objects[obj_id].direction_x > 0)
		{
			player_move_right(1);
		}
		else
		{
			player_move_left(1);
		}
		break;

	case OBJECT_CHECKPOINT:

		player_set_checkpoint(obj_id);
		break;

	case OBJECT_MIRRORLINE:

		x = current_level->objects[obj_id].x + 5;

		if (player_x - x <= 2 || x - player_x <= 2)
		{
			if (!object_mirrorline_locked)
			{
				sound_flip();
				player_flipped = !player_flipped;
				object_mirrorline_locked = TRUE;
			}
		}
		else
		{
			object_mirrorline_locked = FALSE;
		}

		break;
	}
}

void test_objects_for_player()
{
	// counters etc
	UINT8 i;

	// object values
	UINT8 x, y, w, h, px, py, ph, pw;

	// set flying to enabled
	player_flying_disabled = FALSE;

	if (current_level->objects_count == 0)
	{
		return;
	}

	px = player_x;
	py = player_y;
	pw = PLAYER_WIDTH_PX;
	ph = PLAYER_HEIGHT_PX;

	// each object
	for (i = 0; i < (current_level->objects_count); i++)
	{

		x = (UINT8)(current_level->objects[i].x);
		y = (UINT8)(current_level->objects[i].y);
		w = (UINT8)(current_level->objects[i].width * LEVEL_TILE_SIZE);
		h = (UINT8)(current_level->objects[i].height * LEVEL_TILE_SIZE);

		// match found?
		if (py <= y + h && py + ph >= y && px <= x + w && px + pw >= x)
		{
			object_found(i, current_level->objects[i].type);
		}
	}
}

BOOLEAN player_test_for_position(INT16 x, INT16 y)
{
	UINT8 tile;

	if (x <= 0 || x > LEVEL_WIDTH_PX || y < 0 || y > LEVEL_HEIGHT_PX)
	{
		return TRUE;
	}

	player_dead = FALSE;

	tile = current_level_get_tile(x, y);

	// EMPTY SPACE
	if (tile == 0)
	{
		return TRUE;
	}

	// VVVVVVVs
	if (tile >= 2 && tile <= 5)
	{
		player_dead = TRUE;
		return TRUE;
	}

	return FALSE;
}

BOOLEAN player_fly()
{
	BOOLEAN player_dead_1;
	BOOLEAN player_dead_2;
	INT16 testx;

	if (player_flying_disabled)
	{
		return TRUE;
	}

	if (!player_flipped)
	{
		testx = player_x;
		if (player_test_for_position(player_x, player_y + PLAYER_HEIGHT_PX))
		{
			testx = player_x + PLAYER_WIDTH_PX;
			player_dead_1 = player_dead;
			if (player_test_for_position(testx, player_y + PLAYER_HEIGHT_PX))
			{
				player_dead_2 = player_dead;
				player_y++;
				player_flipping_disabled = TRUE;
				if (player_dead_1 || player_dead_2)
				{
					player_die();
					return FALSE;
				}

				// load another map?
				if (player_y > MAX_PY)
				{
					player_y = MIN_PY;

					if (current_level->south != NULL)
					{
						load_level(current_level->south);
					}
				}
				return TRUE;
			}
		}
	}
	else
	{
		testx = player_x;
		if (player_test_for_position(testx, player_y - 1))
		{
			player_dead_1 = player_dead;
			testx = player_x + PLAYER_WIDTH_PX;
			if (player_test_for_position(testx, player_y - 1))
			{
				player_dead_2 = player_dead;
				player_y--;
				player_flipping_disabled = TRUE;
				if (player_dead_1 || player_dead_2)
				{
					player_die();
					return FALSE;
				}

				// load another map?
				if (player_y <= MIN_PY - (PLAYER_HEIGHT_PX / 2))
				{
					player_y = MAX_PY - (PLAYER_HEIGHT_PX / 2) - 2;

					if (current_level->north != NULL)
					{
						load_level(current_level->north);
					}
				}

				return TRUE;
			}
		}
	}

	player_flipping_disabled = FALSE;
	return FALSE;
}

void player_move_left(UINT8 speed)
{
	BOOLEAN player_dead_1;
	BOOLEAN player_dead_2;

	if (player_test_for_position(player_x - speed, player_y))
	{
		player_dead_1 = player_dead;

		if (player_test_for_position(player_x - speed, player_y + PLAYER_HEIGHT_PX - 1))
		{
			player_dead_2 = player_dead;
			player_x -= speed;
			player_last_move = -1;

			if (player_dead_1 || player_dead_2)
			{
				player_die();
			}
		}
	}

	// left side of the map
	if (player_x <= (PLAYER_WIDTH_PX / 2))
	{
		player_x = MAX_PX + (PLAYER_WIDTH_PX / 2);
		if (current_level->west != NULL)
		{
			load_level(current_level->west);
		}
	}
}

void player_move_right(UINT8 speed)
{
	BOOLEAN player_dead_1;
	BOOLEAN player_dead_2;
	INT16 testx;
	INT16 testy;

	// normal test
	testy = player_y;
	testx = player_x + speed + PLAYER_WIDTH_PX;

	if (player_test_for_position(testx, testy))
	{
		player_dead_1 = player_dead;
		testy = player_y + PLAYER_HEIGHT_PX - 1;

		if (player_test_for_position(testx, testy))
		{
			player_dead_2 = player_dead;
			player_x += speed;
			player_last_move = 1;

			if (player_dead_1 || player_dead_2)
			{
				player_die();
			}
		}
	}

	// testing right side of the map
	if (player_x >= MAX_PX)
	{
		player_x = -(PLAYER_WIDTH_PX / 2) + 2;
		if (current_level->east != NULL)
		{
			load_level(current_level->east);
		}
	}
}

/* player */
void check_input()
{
	UINT8 key;

	key = joypad();

	/* move */
	if (!player_dead && (key & J_LEFT))
	{

		player_move_left(PLAYER_SPEED);
	}
	else if (!player_dead && (key & J_RIGHT))
	{

		player_move_right(PLAYER_SPEED);
	}

	// DO FLIP and disable flipping till the player releases the key
	if ((key & J_A))
	{
		if (!player_flipping_disabled && !flip_key_pressed)
		{
			sound_flip();
			player_flipped = !player_flipped;
			player_flipping_disabled = TRUE;
			player_flying_disabled = FALSE;
		}
		flip_key_pressed = TRUE;
	}
	else
	{
		flip_key_pressed = FALSE;
	}
}

void paint()
{
	player_paint();
}

/*--------------------------------------------------------------------------*
 | main program                                                             |
 *--------------------------------------------------------------------------*/
void main()
{
	UINT8 i;

	flip_key_pressed = FALSE;

	sound_init();
	init_screen();
	current_level = &START_LEVEL;

	init_level();
	init_objects();

	init_player();

	pw = 6;
	while (1)
	{
		delay(pw);

		for (i = 0; i < PLAYER_FLY_SPEED; i++)
		{
			if (!player_fly())
			{
				break;
			}
		}

		move_objects();
		test_objects_for_player();
		check_input();
		paint();
	}
}
