/*
 * game.h
 *
 * Author: Jarrod Bennett, Cody Burnett
 *
 * Function prototypes for game functions available externally. You may wish
 * to add extra function prototypes here to make other functions available to
 * other files.
 */


#ifndef GAME_H_
#define GAME_H_

#include <stdint.h>
#include <stdbool.h>

#define TRACK_LENGTH 129
int16_t game_score;
uint8_t combo_score;
uint16_t freq;	// Hz
float dutycycle;	// %
uint16_t clockperiod;
uint16_t pulsewidth;
uint16_t beat_count;
uint16_t beat;
int16_t game_score;
uint8_t turn_off_audio;
uint8_t note_played;
uint8_t track_choice;

// Initialise the game by resetting the grid and beat
void initialise_game(void);

// Play a note in the given lane
void play_note(uint8_t lane);

// Advance the notes one row down the display
void advance_note(void);

// Returns 1 if the game is over, 0 otherwise.
uint8_t is_game_over(void);
// Returns the index of next note
uint8_t find_next_valid_note(uint8_t index);

void print_game_score(int score);

int is_long_note_being_played(void);
#endif
