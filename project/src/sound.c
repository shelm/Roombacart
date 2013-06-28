/****************************************************************** Includes */
#include <roomba.h>
#include <uart.h>
#include <sound.h>

/******************************************************************* Defines */


/************************************************************** Global const */

roomba_note_t mortal_kombat[] = {
	{NOTE_G, 16},
	{NOTE_G, 16},
	{NOTE_B, 16},
	{NOTE_G, 16},
	
	{NOTE_C, 16},
	{NOTE_G, 16},
	{NOTE_D+tone_heigth, 16},
	{NOTE_C, 16},
	
	{NOTE_B, 16},
	{NOTE_B, 16},
	{NOTE_D+tone_heigth, 16},
	{NOTE_B, 16},
	
	{NOTE_F+tone_heigth, 16},
	{NOTE_B, 16},
	{NOTE_D+tone_heigth, 16},
	{NOTE_B, 16}
	
};

roomba_note_t mortal_kombat_part_2[] = {
	{NOTE_F, 16},
	{NOTE_F, 16},
	{NOTE_A, 16},
	{NOTE_F, 16},
	
	{NOTE_B, 16},
	{NOTE_F, 16},
	{NOTE_C, 16},
	{NOTE_B, 16},
	
	{NOTE_E, 16},
	{NOTE_E, 16},
	{NOTE_G, 16},
	{NOTE_B, 16},
	
	{NOTE_G, 16},
	{NOTE_E, 16},
	{NOTE_B, 16},
	{NOTE_A, 16}
};

roomba_note_t indiana_jones_theme[] = {
	{NOTE_A_SHARP, 24},
	{NOTE_C, 8},
	{NOTE_D+tone_heigth, 32},
	{NOTE_G+tone_heigth, 64},
	
	{NOTE_A, 24},
	{NOTE_A_SHARP, 8},
	{NOTE_C, 96},
	{NOTE_D+tone_heigth, 24},
	
	{NOTE_E+tone_heigth, 8},
	{NOTE_F_SHARP+tone_heigth, 32},
	{NOTE_C+tone_heigth, 64},
	{NOTE_E+tone_heigth, 24},
	
	{NOTE_F_SHARP+tone_heigth, 8},
	{NOTE_G+tone_heigth, 32},
	{NOTE_A+tone_heigth, 32},
	{NOTE_B+tone_heigth, 32}
};

roomba_note_t indiana_jones_theme_part_2[] = {
	{NOTE_A_SHARP, 24},
	{NOTE_C, 8},
	{NOTE_D+tone_heigth, 32},
	{NOTE_G+tone_heigth, 64},
	
	{NOTE_A+tone_heigth, 24},
	{NOTE_B+tone_heigth, 8},
	{NOTE_C+tone_heigth, 96},
	{NOTE_C, 24},
	
	{NOTE_C, 8},
	{NOTE_B+tone_heigth, 32},
	{NOTE_A+tone_heigth, 24},
	{NOTE_D+tone_heigth, 8},
	
	{NOTE_B+tone_heigth, 32},
	{NOTE_A+tone_heigth, 24},
	{NOTE_D+tone_heigth, 8},
	{NOTE_B+tone_heigth, 32}
};

roomba_note_t indiana_jones_theme_part_3[] = {
	{NOTE_A+tone_heigth, 24},
	{NOTE_D+tone_heigth, 8},
	{NOTE_C+tone_heigth, 32},
	{NOTE_B+tone_heigth, 24},
	
	{NOTE_A+tone_heigth, 8},
	{NOTE_G+tone_heigth, 64},
	{NOTE_B, 24},
	{NOTE_D+tone_heigth, 8},
	
	{NOTE_C, 32},
	{NOTE_A, 24},
	{NOTE_C, 8},
	{NOTE_B, 16},
	
	{NOTE_D+tone_heigth, 16},
	{NOTE_G+tone_heigth, 80},
};

roomba_note_t item_detected_sound[] = {
    {NOTE_G, 16},
    {NOTE_A, 16},
    {NOTE_B, 16},
    {NOTE_C, 16},
};

roomba_note_t final_lap[] = {
	{NOTE_G, 8},
	{NOTE_D+tone_heigth, 8},
	{PAUSE, 4},
	{NOTE_D+tone_heigth, 8},
	
	{NOTE_D+tone_heigth, 8},
	{PAUSE, 4},
	{NOTE_G_SHARP, 8},
	{NOTE_D_SHARP+tone_heigth, 8},
	
	{PAUSE, 4},
	{NOTE_D_SHARP+tone_heigth, 8},
	{NOTE_D_SHARP+tone_heigth, 8},
	{PAUSE, 4},
	
	{NOTE_E+tone_heigth, 12},
	{NOTE_F+tone_heigth, 56},
	{PAUSE, 8},
	{NOTE_F+tone_heigth, 8},
};

roomba_note_t final_lap_part_2[] = {
	{PAUSE, 8},
	{NOTE_E+tone_heigth, 56},
};

roomba_note_t damage_sound[] = {
	{NOTE_F+tone_heigth, 8},
	{NOTE_C, 8},
	{NOTE_F, 8},
	{PAUSE, 8},
	
	{NOTE_F+tone_heigth, 8},
	{NOTE_C, 8},
	{NOTE_F, 8},
	{PAUSE, 8},
	
	{NOTE_F+tone_heigth, 8},
	{NOTE_C, 8},
	{NOTE_F, 8},
	{PAUSE, 8},
};

roomba_note_t starman_theme[] = {
	{NOTE_C, 16},
	{NOTE_C, 16},
	{NOTE_C, 16},
	{NOTE_D, 8},
	
	{NOTE_C, 16},
	{PAUSE, 4},
	{NOTE_C, 16},
	{NOTE_D, 8},

	{NOTE_C, 8},
	{NOTE_D, 8},
	{NOTE_C, 16},
	{NOTE_B, 16},
	
	{NOTE_B, 16},
	{NOTE_B, 16},
	{NOTE_C-tone_heigth, 8},
	{NOTE_B, 8},
};

roomba_note_t starman_theme_part_2[] = {
	{PAUSE, 4},
	{NOTE_B, 16},
	{NOTE_C-tone_heigth, 8},
	{NOTE_B, 8},
	
	{NOTE_C-tone_heigth, 8},
	{NOTE_B, 16},
};

/********************************************************** Global variables */


/*************************************************************** Local const */


/*********************************************************** Local variables */


/******************************************************************** Macros */


/********************************************************** Global functions */
void transmit_song(roomba_note_t *music_sheet, uint8_t song_length, uint8_t song_number) {
	uint8_t i ;
	uart_write_byte(CMD_SONG);
	uart_write_byte(song_number);
	uart_write_byte(song_length);
	for(i = 0; i < song_length; i++) {
		uart_write_byte(music_sheet[i].note);
		uart_write_byte(music_sheet[i].duration);
	}
}

void play_song(uint8_t song_number) {
	uart_write_byte(CMD_PLAY);
	uart_write_byte(song_number);
}

uint8_t check_for_playing_song() {
	uart_write_byte(CMD_SENSORS);
	uart_write_byte(SENSORS_SONG);
	return uart_read_byte();
}
