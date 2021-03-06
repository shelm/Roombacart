
/****************************************************************** Includes */


/******************************************************************* Defines */
/*! Notes */
#define tone_heigth		12
#define NOTE_D			74
#define NOTE_D_SHARP 	75
#define NOTE_E			76
#define NOTE_F			77
#define NOTE_F_SHARP	78
#define NOTE_G			79
#define NOTE_G_SHARP	80
#define NOTE_A			81
#define NOTE_A_SHARP	82
#define NOTE_B			83
#define NOTE_C			84
#define NOTE_C_SHARP	85
#define PAUSE			0

/****************************************************************** Typedefs */
/*! struct for representing music note
*/
typedef struct  {
	uint8_t note;
	uint8_t duration;
} roomba_note_t;
/************************************************************** Global const */

/*! melodies that consist of sequences of notes
*/
extern roomba_note_t indiana_jones_theme[];
extern roomba_note_t indiana_jones_theme_part_2[];
extern roomba_note_t indiana_jones_theme_part_3[];
extern roomba_note_t item_detected_sound[];

extern roomba_note_t mortal_kombat[];
extern roomba_note_t mortal_kombat_part_2[];

extern roomba_note_t final_lap[];
extern roomba_note_t final_lap_part_2[];

extern roomba_note_t damage_sound[];

extern roomba_note_t starman_theme[];
extern roomba_note_t starman_theme_part_2[];

/********************************************************** Global variables */


/************************************************ Global function prototypes */
/*! transmits melody to the roomba
*/
void transmit_song(roomba_note_t *music_sheet, uint8_t song_length, uint8_t song_number);

/*! plays the melody on the roomba
*/
void play_song(uint8_t song_number);

/*! checks whether the song is playing
*/
uint8_t check_for_playing_song();

/*************************************************** Global inline functions */


/******************************************************************** Macros */
