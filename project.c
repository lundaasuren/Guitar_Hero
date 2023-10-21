/*
 * project.c
 *
 * Main file
 *
 * Authors: Peter Sutton, Luke Kamols, Jarrod Bennett, Cody Burnett
 * Modified by Lundaasuren Munkhbat
 */

#include <stdio.h>
#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdbool.h>

#define F_CPU 8000000UL
#include <util/delay.h>

#include "game.h"
#include "display.h"
#include "ledmatrix.h"
#include "buttons.h"
#include "serialio.h"
#include "terminalio.h"
#include "timer0.h"
#include "timer1.h"
#include "timer2.h"

// Function prototypes - these are defined below (after main()) in the order
// given here
void initialise_hardware(void);
void start_screen(void);
void new_game(void);
void play_game(void);
void handle_game_over(void);

uint16_t game_speed = 1000;
bool manual_mode = false;

/////////////////////////////// main //////////////////////////////////
int main(void)
{
	// Setup hardware and call backs. This will turn on 
	// interrupts.
	initialise_hardware();
	
	// Show the splash screen message. Returns when display
	// is complete.
	
	// Loop forever and continuously play the game.
	while(1)
	{
		start_screen();
		new_game();
		play_game();
		handle_game_over();
	}
}

void initialise_hardware(void)
{
	ledmatrix_setup();
	init_button_interrupts();
	// Setup serial port for 19200 baud communication with no echo
	// of incoming characters
	init_serial_stdio(19200, 0);
	
	init_timer0();
	init_timer1();
	init_timer2();
	
	// Turn on global interrupts
	sei();
}

void start_screen(void)
{
	// Clear terminal screen and output a message
	clear_terminal();
	show_cursor();
	clear_terminal();
	hide_cursor();
	set_display_attribute(FG_WHITE);
	move_terminal_cursor(10,4);
	printf_P(PSTR("  ______   __     __  _______         __    __"));
	move_terminal_cursor(10,5);
	printf_P(PSTR(" /      \\ |  \\   |  \\|       \\       |  \\  |  \\"));
	move_terminal_cursor(10,6);
	printf_P(PSTR("|  $$$$$$\\| $$   | $$| $$$$$$$\\      | $$  | $$  ______    ______    ______"));
	move_terminal_cursor(10,7);
	printf_P(PSTR("| $$__| $$| $$   | $$| $$__| $$      | $$__| $$ /      \\  /      \\  /      \\"));
	move_terminal_cursor(10,8);
	printf_P(PSTR("| $$    $$ \\$$\\ /  $$| $$    $$      | $$    $$|  $$$$$$\\|  $$$$$$\\|  $$$$$$\\"));
	move_terminal_cursor(10,9);
	printf_P(PSTR("| $$$$$$$$  \\$$\\  $$ | $$$$$$$\\      | $$$$$$$$| $$    $$| $$   \\$$| $$  | $$"));
	move_terminal_cursor(10,10);
	printf_P(PSTR("| $$  | $$   \\$$ $$  | $$  | $$      | $$  | $$| $$$$$$$$| $$      | $$__/ $$"));
	move_terminal_cursor(10,11);
	printf_P(PSTR("| $$  | $$    \\$$$   | $$  | $$      | $$  | $$ \\$$     \\| $$       \\$$    $$"));
	move_terminal_cursor(10,12);
	printf_P(PSTR(" \\$$   \\$$     \\$     \\$$   \\$$       \\$$   \\$$  \\$$$$$$$ \\$$        \\$$$$$$"));
	move_terminal_cursor(10,14);
	// change this to your name and student number; remove the chevrons <>
	printf_P(PSTR("CSSE2010/7201 A2 by LUNDAASUREN MUNKHBAT - 47668599"));
	// Displaying game speed
	move_terminal_cursor(10,16);
	if (game_speed == 1000)
	{
		printf_P(PSTR("Game Speed: Normal"));
	}
	else if (game_speed == 500)
	{
		printf_P(PSTR("Game Speed: Fast"));
	}
	else if (game_speed == 250)
	{
		printf_P(PSTR("Game Speed: Extreme"));
	}
	
	// Output the static start screen and wait for a push button 
	// to be pushed or a serial input of 's'
	show_start_screen();

	uint32_t last_screen_update, current_time;
	last_screen_update = get_current_time();
	
	uint8_t frame_number = 0;
	

	// Wait until a button is pressed, or 's' is pressed on the terminal
	while(1)
	{
		// First check for if a 's' is pressed
		// There are two steps to this
		// 1) collect any serial input (if available)
		// 2) check if the input is equal to the character 's'
		char serial_input = -1;
		if (serial_input_available())
		{
			serial_input = fgetc(stdin);
		}
		// If the serial input is 's', then exit the start screen
		if (serial_input == 's' || serial_input == 'S')
		{
			break;
		}
		// If the serial input is 'm', then turn on the manual mode
		if (serial_input == 'm' || serial_input == 'M')
		{
			if (manual_mode)
			{
				manual_mode = false;
				move_terminal_cursor(10,15);
				printf_P(PSTR("Manual mode: OFF"));
			}
			else
			{
				manual_mode = true;
				move_terminal_cursor(10,15);
				printf_P(PSTR("Manual mode: ON "));
			}
		}
		
		if (serial_input == '1' || serial_input == '!')
		{
			game_speed = 1000;
			move_terminal_cursor(10,16);
			printf_P(PSTR("Game Speed: Normal "));
		}
		if (serial_input == '2' || serial_input == '@')
		{
			game_speed = 500;
			move_terminal_cursor(10,16);
			printf_P(PSTR("Game Speed: Fast   "));
		}
		if (serial_input == '3' || serial_input == '#')
		{
			game_speed = 250;
			move_terminal_cursor(10,16);
			printf_P(PSTR("Game Speed: Extreme"));
		}
		
		// Next check for any button presses
		int8_t btn = button_pushed();
		if (btn != NO_BUTTON_PUSHED)
		{
			break;
		}

		// every 200 ms, update the animation
		current_time = get_current_time();
		if (current_time - last_screen_update > game_speed/5)
		{
			update_start_screen(frame_number);
			frame_number = (frame_number + 1) % 32;
			last_screen_update = current_time;
		}
	}
	// Implement seven segment display here!
	uint8_t seven_seg[11] = {63, 6, 91, 79, 102, 109, 125, 7, 127, 111, 128};
	
	// Implement the game CountDown over here!
	uint32_t start_time;
	ledmatrix_clear();
	// Drawing 3
	ledmatrix_update_pixel(5, 1, COLOUR_RED);
	ledmatrix_update_pixel(5, 2, COLOUR_RED);
	ledmatrix_update_pixel(4, 2, COLOUR_RED);
	ledmatrix_update_pixel(4, 3, COLOUR_RED);
	ledmatrix_update_pixel(4, 4, COLOUR_RED);
	ledmatrix_update_pixel(4, 5, COLOUR_RED);
	ledmatrix_update_pixel(5, 5, COLOUR_RED);
	ledmatrix_update_pixel(5, 6, COLOUR_RED);
	ledmatrix_update_pixel(6, 5, COLOUR_RED);
	ledmatrix_update_pixel(6, 6, COLOUR_RED);
	ledmatrix_update_pixel(7, 2, COLOUR_RED);
	ledmatrix_update_pixel(7, 3, COLOUR_RED);
	ledmatrix_update_pixel(7, 4, COLOUR_RED);
	ledmatrix_update_pixel(8, 5, COLOUR_RED);
	ledmatrix_update_pixel(8, 6, COLOUR_RED);
	ledmatrix_update_pixel(9, 2, COLOUR_RED);
	ledmatrix_update_pixel(9, 1, COLOUR_RED);
	ledmatrix_update_pixel(9, 5, COLOUR_RED);
	ledmatrix_update_pixel(9, 6, COLOUR_RED);
	ledmatrix_update_pixel(10, 2, COLOUR_RED);
	ledmatrix_update_pixel(10, 3, COLOUR_RED);
	ledmatrix_update_pixel(10, 4, COLOUR_RED);
	ledmatrix_update_pixel(10, 5, COLOUR_RED);
	
	start_time = get_current_time();
	while (get_current_time() < start_time + game_speed)
	{
		;
	}

	ledmatrix_clear();
	// Drawing 2
	ledmatrix_update_pixel(4, 2, COLOUR_RED);
	ledmatrix_update_pixel(4, 3, COLOUR_RED);
	ledmatrix_update_pixel(4, 4, COLOUR_RED);
	ledmatrix_update_pixel(4, 5, COLOUR_RED);
	ledmatrix_update_pixel(5, 1, COLOUR_RED);
	ledmatrix_update_pixel(5, 2, COLOUR_RED);
	ledmatrix_update_pixel(5, 5, COLOUR_RED);
	ledmatrix_update_pixel(5, 6, COLOUR_RED);
	ledmatrix_update_pixel(6, 5, COLOUR_RED);
	ledmatrix_update_pixel(6, 6, COLOUR_RED);
	ledmatrix_update_pixel(7, 2, COLOUR_RED);
	ledmatrix_update_pixel(7, 3, COLOUR_RED);
	ledmatrix_update_pixel(7, 4, COLOUR_RED);
	ledmatrix_update_pixel(8, 2, COLOUR_RED);
	ledmatrix_update_pixel(8, 3, COLOUR_RED);
	ledmatrix_update_pixel(9, 1, COLOUR_RED);
	ledmatrix_update_pixel(9, 2, COLOUR_RED);
	ledmatrix_update_pixel(10, 1, COLOUR_RED);
	ledmatrix_update_pixel(10, 2, COLOUR_RED);
	ledmatrix_update_pixel(10, 3, COLOUR_RED);
	ledmatrix_update_pixel(10, 4, COLOUR_RED);
	ledmatrix_update_pixel(10, 5, COLOUR_RED);
	ledmatrix_update_pixel(10, 6, COLOUR_RED);
	
	start_time = get_current_time();
	while (get_current_time() < start_time + game_speed)
	{
		;
	}

	ledmatrix_clear();
	// Drawing 1
	ledmatrix_update_pixel(4, 3, COLOUR_RED);
	ledmatrix_update_pixel(4, 4, COLOUR_RED);
	ledmatrix_update_pixel(5, 2, COLOUR_RED);
	ledmatrix_update_pixel(5, 3, COLOUR_RED);
	ledmatrix_update_pixel(5, 4, COLOUR_RED);
	ledmatrix_update_pixel(6, 3, COLOUR_RED);
	ledmatrix_update_pixel(6, 4, COLOUR_RED);
	ledmatrix_update_pixel(7, 3, COLOUR_RED);
	ledmatrix_update_pixel(7, 4, COLOUR_RED);
	ledmatrix_update_pixel(8, 3, COLOUR_RED);
	ledmatrix_update_pixel(8, 4, COLOUR_RED);
	ledmatrix_update_pixel(9, 3, COLOUR_RED);
	ledmatrix_update_pixel(9, 4, COLOUR_RED);
	ledmatrix_update_pixel(10, 3, COLOUR_RED);
	ledmatrix_update_pixel(10, 4, COLOUR_RED);
	ledmatrix_update_pixel(10, 5, COLOUR_RED);
	ledmatrix_update_pixel(10, 2, COLOUR_RED);
	
	start_time = get_current_time();
	while (get_current_time() < start_time + game_speed)
	{
		;
	}

	ledmatrix_clear();
	// Drawing GO
	ledmatrix_update_pixel(5, 2, COLOUR_RED);
	ledmatrix_update_pixel(5, 1, COLOUR_RED);
	ledmatrix_update_pixel(5, 5, COLOUR_RED);
	ledmatrix_update_pixel(5, 6, COLOUR_RED);
	ledmatrix_update_pixel(5, 7, COLOUR_RED);
	ledmatrix_update_pixel(6, 0, COLOUR_RED);
	ledmatrix_update_pixel(6, 5, COLOUR_RED);
	ledmatrix_update_pixel(6, 7, COLOUR_RED);
	ledmatrix_update_pixel(7, 0, COLOUR_RED);
	ledmatrix_update_pixel(7, 2, COLOUR_RED);
	ledmatrix_update_pixel(7, 5, COLOUR_RED);
	ledmatrix_update_pixel(7, 7, COLOUR_RED);
	ledmatrix_update_pixel(8, 0, COLOUR_RED);
	ledmatrix_update_pixel(8, 2, COLOUR_RED);
	ledmatrix_update_pixel(8, 5, COLOUR_RED);
	ledmatrix_update_pixel(8, 7, COLOUR_RED);
	ledmatrix_update_pixel(9, 2, COLOUR_RED);
	ledmatrix_update_pixel(9, 1, COLOUR_RED);
	ledmatrix_update_pixel(9, 5, COLOUR_RED);
	ledmatrix_update_pixel(9, 6, COLOUR_RED);
	ledmatrix_update_pixel(9, 7, COLOUR_RED);

	start_time = get_current_time();
	while (get_current_time() < start_time + game_speed)
	{
		;
	}

	ledmatrix_clear();

}

void new_game(void)
{
	// Clear the serial terminal
	clear_terminal();
	
	// Initialize the game and display
	initialise_game();
	
	// Clear a button push or serial input if any are waiting
	// (The cast to void means the return value is ignored.)
	(void)button_pushed();
	clear_serial_input_buffer();
}

void play_game(void)
{
	print_game_score(0);
	
	move_terminal_cursor(10,16);
	if (game_speed == 1000)
	{
		printf_P(PSTR("Game Speed: Normal"));
	}
	else if (game_speed == 500)
	{
		printf_P(PSTR("Game Speed: Fast"));
	}
	else if (game_speed == 250)
	{
		printf_P(PSTR("Game Speed: Extreme"));
	}

	uint32_t last_advance_time, current_time;
	int8_t btn; // The button pushed
	
	last_advance_time = get_current_time();
	
	// We play the game until it's over
	while (!is_game_over())
	{
		char serial_input = -1;
		if (serial_input_available())
		{
			serial_input = fgetc(stdin);
		}

		// If the serial input is 'm', then turn on the manual mode
		if (serial_input == 'm' || serial_input == 'M')
		{
			if (manual_mode)
			{
				manual_mode = false;
				move_terminal_cursor(10,15);
				printf_P(PSTR("Manual mode: OFF"));
			}
			else
			{
				manual_mode = true;
				move_terminal_cursor(10,15);
				printf_P(PSTR("Manual mode: ON "));
			}
		}
		
		// We need to check if any button has been pushed, this will be
		// NO_BUTTON_PUSHED if no button has been pushed
		// Checkout the function comment in `buttons.h` and the implementation
		// in `buttons.c`.
		btn = button_pushed();
		
		if (btn == BUTTON0_PUSHED || (serial_input == 'f' || serial_input == 'F'))
		{
			// If button 0 play the lowest note (right lane)
			play_note(3);
		}
		else if (btn == BUTTON1_PUSHED || (serial_input == 'd' || serial_input == 'D'))
		{
			// If button 1 play the second lowest note (right lane)
			play_note(2);
		}
		else if (btn == BUTTON2_PUSHED || (serial_input == 's' || serial_input == 'S'))
		{
			// If button 2 play the second lowest note (left lane)
			play_note(1);
		}
		else if (btn == BUTTON3_PUSHED || (serial_input == 'a' || serial_input == 'A'))
		{
			// If button 3 play the lowest note (left lane)
			play_note(0);
		}
		
		current_time = get_current_time();
		if (manual_mode)
		{
			if (serial_input == 'n' || serial_input == 'N')
			{
				advance_note();
				last_advance_time = current_time;
			}
		}
		else
		{
			if (current_time >= last_advance_time + game_speed/5)
			{
				// 200ms (0.2 second) has passed since the last time we advance the
				// notes here, so update the advance the notes
				advance_note();
				
				// Update the most recent time the notes were advance
				last_advance_time = current_time;
			}
		}
		
	}
	// We get here if the game is over.
	if (is_game_over())
	{
		handle_game_over();
	}
}

void handle_game_over()
{
	clear_terminal();
	move_terminal_cursor(10,14);
	printf_P(PSTR("GAME OVER"));
	move_terminal_cursor(10,15);
	printf("Final Score: %d", game_score);
	move_terminal_cursor(10,16);
	if (game_speed == 1000)
	{
		printf_P(PSTR("Game Speed: Normal"));
	}
	else if (game_speed == 500)
	{
		printf_P(PSTR("Game Speed: Fast"));
	}
	else if (game_speed == 250)
	{
		printf_P(PSTR("Game Speed: Extreme"));
	}
	move_terminal_cursor(10,18);
	printf_P(PSTR("Press a button or 's'/'S' to start a new game"));
	
	// Do nothing until a button is pushed. Hint: 's'/'S' should also start a
	// new game
	while (button_pushed() == NO_BUTTON_PUSHED)
	{
		char serial_input = -1;
		if (serial_input_available())
		{
			serial_input = fgetc(stdin);
		}
		// If the serial input is 's', then exit the start screen
		if (serial_input == 's' || serial_input == 'S')
		{
			break;
		}
	}
}
