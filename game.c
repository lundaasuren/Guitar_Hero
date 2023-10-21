/*
 * game.c
 *
 * Functionality related to the game state and features.
 *
 * Author: Jarrod Bennett, Cody Burnett
 */ 

#include "game.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "display.h"
#include "ledmatrix.h"
#include "terminalio.h"
#include <stdbool.h>

static const uint8_t track[TRACK_LENGTH] = {0x00,
	0x00, 0x00, 0x08, 0x08, 0x08, 0x80, 0x04, 0x02,
	0x04, 0x40, 0x08, 0x80, 0x00, 0x00, 0x04, 0x02,
	0x04, 0x40, 0x08, 0x04, 0x40, 0x02, 0x20, 0x01,
	0x10, 0x10, 0x10, 0x10, 0x00, 0x00, 0x02, 0x20,
	0x04, 0x40, 0x08, 0x80, 0x04, 0x40, 0x02, 0x20,
	0x04, 0x40, 0x08, 0x04, 0x40, 0x40, 0x02, 0x20,
	0x04, 0x40, 0x08, 0x04, 0x40, 0x02, 0x20, 0x01,
	0x10, 0x10, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x08, 0x08, 0x08, 0x80, 0x04, 0x02,
	0x04, 0x40, 0x02, 0x08, 0x80, 0x00, 0x02, 0x01,
	0x04, 0x40, 0x08, 0x80, 0x04, 0x02, 0x20, 0x01,
	0x10, 0x10, 0x12, 0x20, 0x00, 0x00, 0x02, 0x20,
	0x04, 0x40, 0x08, 0x04, 0x40, 0x40, 0x02, 0x20,
	0x04, 0x40, 0x08, 0x04, 0x40, 0x40, 0x02, 0x20,
	0x04, 0x40, 0x08, 0x04, 0x40, 0x40, 0x02, 0x20,
	0x01, 0x10, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00};
	
static bool green_note[TRACK_LENGTH];
static bool red_note[TRACK_LENGTH];


uint16_t beat;
int game_score = 0;

// Initialise the game by resetting the grid and beat
void initialise_game(void)
{
	// initialise the display we are using.
	default_grid();
	beat = 0;
}

// Play a note in the given lane
void play_note(uint8_t lane)
{
	// YOUR CODE HERE
	// possible steps:
	// a) check if there is a note in the scoring area of the given lane -
	//    look at advance_note below for a hint to the logic
	// b) if there is, immediately turn those notes green
	// c) set up a variable to remember if notes should be green,
	//    and trigger it simultaneously with b)
	// d) in advance_note below, poll that variable, and choose COLOUR_GREEN
	//    instead of COLOUR_RED for ledmatrix_update_pixel if required
	// e) depending on your implementation, clear the variable in
	//    advance_note when a note disappears from the screen
	
	for (uint8_t col=0; col<MATRIX_NUM_COLUMNS; col++)
	{
		// col counts from one end, future from the other
		uint8_t future = MATRIX_NUM_COLUMNS-1-col;
		// notes are only drawn every five columns
		if ((future+beat)%5)
		{
			continue;
		}
		
		// index of which note in the track to play
		uint8_t index = (future+beat)/5;
		// if the index is beyond the end of the track,
		// no note can be drawn
		if (index >= TRACK_LENGTH)
		{
			continue;
		}
		
		// check if there's a note in the specific path
		if (track[index] & (1<<lane))
		{
			if (col == 11 || col == 15)
			{
				green_note[index] = true;
				game_score++;
				print_game_score(game_score);
				
				ledmatrix_update_pixel(col, 2*lane, COLOUR_GREEN);
				ledmatrix_update_pixel(col, 2*lane+1, COLOUR_GREEN);
			}
			else if (col == 12 || col == 14)
			{
				green_note[index] = true;
				game_score += 2;
				print_game_score(game_score);
				
				ledmatrix_update_pixel(col, 2*lane, COLOUR_GREEN);
				ledmatrix_update_pixel(col, 2*lane+1, COLOUR_GREEN);
			}
			else if (col == 13)
			{
				green_note[index] = true;
				game_score += 3;
				print_game_score(game_score);

				ledmatrix_update_pixel(col, 2*lane, COLOUR_GREEN);
				ledmatrix_update_pixel(col, 2*lane+1, COLOUR_GREEN);
			}
		}
		else
		{
			game_score--;
			print_game_score(game_score);
		}
	}
}

// Advance the notes one row down the display
void advance_note(void)
{
	// remove all the current notes; reverse of below
	for (uint8_t col=0; col<MATRIX_NUM_COLUMNS; col++)
	{
		uint8_t future = MATRIX_NUM_COLUMNS - 1 - col;
		uint8_t index = (future + beat) / 5;
		if (index >= TRACK_LENGTH)
		{
			break;
		}
		if ((future+beat) % 5)
		{
			continue;
		}
		for (uint8_t lane = 0; lane < 4; lane++)
		{
			if (track[index] & (1<<lane))
			{
				PixelColour colour;
				// yellows in the scoring area
				if (col==11 || col == 15)
				{
					colour = COLOUR_QUART_YELLOW;
				}
				else if (col==12 || col == 14)
				{
					colour = COLOUR_HALF_YELLOW;
				}
				else if (col==13)
				{
					colour = COLOUR_YELLOW;
				}
				else
				{
					colour = COLOUR_BLACK;
				}
				
				ledmatrix_update_pixel(col, 2*lane, colour);
				ledmatrix_update_pixel(col, 2*lane+1, colour);
			}
		}
	}
	
	// increment the beat
	beat++;
	
	// Ghost note implementation below:

	// Clearing the top row
	ledmatrix_update_column(-1, COLOUR_BLACK);
	// index of which note in the track to play
	uint8_t index = (MATRIX_NUM_COLUMNS+beat)/5;
	// if the index is beyond the end of the track,
	// no note can be drawn
	if (!(index >= TRACK_LENGTH))
	{
		// Drawing the ghost note
		uint8_t next_note = find_next_valid_note(index);

		for (uint8_t lane=0; lane<4; lane++)
		{	
			if (next_note)
			{
				if (track[next_note] & (1<<lane))
				{
					ledmatrix_update_pixel(0, 2*lane, COLOUR_HALF_RED);
					ledmatrix_update_pixel(0, 2*lane+1, COLOUR_HALF_RED);
				}
			}
		}
	}
	
	// draw the new notes
	for (uint8_t col=0; col<MATRIX_NUM_COLUMNS; col++)
	{
		// col counts from one end, future from the other
		uint8_t future = MATRIX_NUM_COLUMNS-1-col;
		// notes are only drawn every five columns
		if ((future+beat)%5)
		{
			continue;
		}
		
		// index of which note in the track to play
		uint8_t index = (future+beat)/5;
		// if the index is beyond the end of the track,
		// no note can be drawn
		if (index >= TRACK_LENGTH)
		{
			continue;
		}

		// iterate over the four paths
		for (uint8_t lane=0; lane<4; lane++)
		{	
			// check if there's a note in the specific path
			if (track[index] & (1<<lane))
			{
				PixelColour colour;
				if (green_note[index])
				{
					colour = COLOUR_GREEN;
				}
				else
				{
					red_note[index] = true;
					colour = COLOUR_RED;
				}
				// if so, colour the two pixels red
				ledmatrix_update_pixel(col, 2*lane, colour);
				ledmatrix_update_pixel(col, 2*lane+1, colour);
			}
		}
		
		// detecting slid off unplayed note
		if (col == MATRIX_NUM_COLUMNS -1)
		{
			if (red_note[index])
			{
				game_score--;
				print_game_score(game_score);
			}
			
			if (index == TRACK_LENGTH - 1)
			{
				game_over = true;
			}
			else
			{
				game_over = false;
			}
		}
	}
}

// Returns 1 if the game is over, 0 otherwise.
uint8_t is_game_over(void)
{
	// YOUR CODE HERE
	// Detect if the game is over i.e. if a player has won.
	if (game_over)
	{
		return 1;
	}
	return 0;
}

// Returns the index of next valid note, 0 otherwise.
uint8_t find_next_valid_note(uint8_t index)
{
	for (uint8_t next_note = index+1; next_note < TRACK_LENGTH; next_note++)
	{
		if (track[next_note] & 0x0F)
		{
			return next_note;
		}
	}
	return 0;
}

void print_game_score(int score)
{
	move_terminal_cursor(10, 4);
	printf("Game Score: %4d", score);
}