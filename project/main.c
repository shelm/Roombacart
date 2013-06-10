/****************************************************************** Includes */

#include <board.h>
#include <button.h>
#include <ju.h>
#include <led.h>
#include <pio.h>
#include <roomba.h>
#include <s7ctl.h>
#include <stddef.h>
#include <tools.h>
#include <uart.h>

#include <sound.h>
#include <asm/io.h>
#include <harmony.h>


/******************************************************************* Defines */


/******************************************************* Function prototypes */

/************************************************************** Global const */


/********************************************************** Global variables */


/*************************************************************** Local const */


/*********************************************************** Local variables */

char greeting[4] = {'I', 'H', 'I', 'H'};

/*
 *  false if left sensor left line first
 *  true if right sensor left line first
 */
bool_t direction;

bool_t offroad = false;

bool_t item_detected_mode = false;

int count = 0;

/******************************************************************** Macros */

#define CLIFF_LEFT_SIG         cliff_left_signal.value
#define CLIFF_RIGHT_SIG        cliff_right_signal.value
#define CLIFF_FRONT_LEFT_SIG   cliff_front_left_signal.value
#define CLIFF_FRONT_RIGHT_SIG  cliff_front_right_signal.value


#define INFRARED_OMNI  infrared_omni.value
#define INFRARED_RIGHT infrared_right.value
#define INFRARED_LEFT  infrared_left.value

/********************************************************** Global functions */


/*********************************************************** Local functions */
int main(int argc, char **argv)
{
    button_wait(0);
    my_msleep(200);

    //initialize the roomba
    init_roomba();
    roomba_stop();
    my_msleep(500);

    IOWR32(A_PIO_LBLUE, PIO_DATA, ledb_vals[0]);
    roomba_set_letters_string(greeting, 4);

    init_cliff_signal();
	init_infrared();

	transmit_song(indiana_jones_theme, 16, 0);
	my_msleep(15);
	transmit_song(indiana_jones_theme_part_2, 16, 1);
	my_msleep(15);
	transmit_song(indiana_jones_theme_part_3, 14, 2);
	my_msleep(15);
	transmit_song(item_detected_sound, 4, 3);
	play_song(0);
	while(check_for_playing_song()){
		my_msleep(15);
	}
//	play_song(1);
	while(check_for_playing_song()){
		my_msleep(15);
	}
//	play_song(2);

    button_wait(1);

    while(true){
        //disable_all_interrupts();
        if(button_pressed(0)){
            roomba_stop();
            break;
        }
		roomba_update_sensor(&infrared_omni, false);
		roomba_update_sensor(&infrared_right, false);
		roomba_update_sensor(&infrared_left, false);

		if(INFRARED_OMNI == HARMONY_SIGNAL_PAUSE || INFRARED_RIGHT == HARMONY_SIGNAL_PAUSE || INFRARED_LEFT == HARMONY_SIGNAL_PAUSE) {
			roomba_set_led_on(BTN_CLEAN, 100, 100);
		}

		while(INFRARED_OMNI == HARMONY_SIGNAL_TURNLEFT || INFRARED_RIGHT == HARMONY_SIGNAL_TURNLEFT || INFRARED_LEFT == HARMONY_SIGNAL_TURNLEFT) {
			roomba_drive(velocity, 300);
			roomba_update_sensor(&infrared_omni, false);
		    roomba_update_sensor(&infrared_right, false);
		    roomba_update_sensor(&infrared_left, false);
			my_msleep(50);
		}

		while(INFRARED_OMNI == HARMONY_SIGNAL_TURNRIGHT || INFRARED_RIGHT == HARMONY_SIGNAL_TURNRIGHT || INFRARED_LEFT == HARMONY_SIGNAL_TURNRIGHT) {
			roomba_drive(velocity, -300);

			roomba_update_sensor(&infrared_omni, false);
		    roomba_update_sensor(&infrared_right, false);
		    roomba_update_sensor(&infrared_left, false);
			my_msleep(50);
		}
		if(INFRARED_OMNI == HARMONY_SIGNAL_SPOT || INFRARED_RIGHT == HARMONY_SIGNAL_SPOT || INFRARED_LEFT == HARMONY_SIGNAL_SPOT) {
			roomba_use_item();
		}


       //drive along the line
		roomba_update_sensor(&cliff_front_left_signal, false);
		roomba_update_sensor(&cliff_front_right_signal, false);
		roomba_update_sensor(&cliff_left_signal, false);
		roomba_update_sensor(&cliff_right_signal, false);

        if(CLIFF_FRONT_LEFT_SIG > 1200 && CLIFF_FRONT_RIGHT_SIG < 1200){
            direction = true;
            roomba_drive(velocity, 1000);

        } else if(CLIFF_FRONT_LEFT_SIG < 1200 && CLIFF_FRONT_RIGHT_SIG > 1200){
            direction = false;
            roomba_drive(velocity, -1000);
        }


		if (CLIFF_FRONT_LEFT_SIG < 1200 && CLIFF_FRONT_RIGHT_SIG < 1200) {
            if (direction) {
                roomba_drive(velocity, 200);
            } else {
                roomba_drive(velocity, -200);
            }
            offroad = true;
		} else if (CLIFF_FRONT_LEFT_SIG > 1200 && CLIFF_FRONT_RIGHT_SIG > 1200) {
		    if(offroad){
                if (direction) {
                    roomba_drive(velocity, -800);
                } else {
                    roomba_drive(velocity, 800);
                }
                if(count > 10){
                    offroad = false;
                    count = 0;
                } else {
                    count++;
                }
		    }
        } else if (CLIFF_FRONT_LEFT_SIG > 1200 || CLIFF_FRONT_RIGHT_SIG > 1200) {
            roomba_drive(velocity, radius);
        }


//        if(CLIFF_FRONT_LEFT_SIG > CLIFF_FRONT_RIGHT_SIG + 500){
//            roomba_drive(velocity, 200);
//        } else if(CLIFF_FRONT_LEFT_SIG + 500 < CLIFF_FRONT_RIGHT_SIG){
//            roomba_drive(velocity, -200);
//        } else {
//            roomba_drive(velocity, radius);
//        }

        if(roomba_check_for_item(CLIFF_LEFT_SIG, CLIFF_RIGHT_SIG)) {
            if(!item_detected_mode) {
                roomba_pick_up_item();
                item_detected_mode = true;
            }
        }
        else {
            item_detected_mode = false;
        }
        //enable_all_interrupts();
        my_msleep(50);

    }

    return 0;
}
