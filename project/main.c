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


void update_remote_control_sensors(void);

void update_cliff_sensors(void);

void drive_offroad(void);

void drive_lane(void);

void drive_curve_left(void);

void drive_curve_right(void);

void drive_back_to_lane(void);

void correct_curse_to_left(void);

void correct_curse_to_right(void);


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

bool_t curve_detected = false;
bool_t item_detected_mode = false;

int offroad_counter = 0;
int course_correction_counter = 0;

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
	//play_song(1);
	while(check_for_playing_song()){
		my_msleep(15);
	}
	//play_song(2);

    button_wait(1);
    drive_lane();

    while(true){

        if(button_pressed(0)){
            roomba_stop();
            break;
        }

        update_remote_control_sensors();


        // drive to left because user wants to
		while(INFRARED_OMNI == HARMONY_SIGNAL_TURNLEFT || INFRARED_RIGHT == HARMONY_SIGNAL_TURNLEFT || INFRARED_LEFT == HARMONY_SIGNAL_TURNLEFT) {
			roomba_drive(velocity, 300);
            update_remote_control_sensors();
			my_msleep(50);
		}

        // drive to right because user wants to
		while(INFRARED_OMNI == HARMONY_SIGNAL_TURNRIGHT || INFRARED_RIGHT == HARMONY_SIGNAL_TURNRIGHT || INFRARED_LEFT == HARMONY_SIGNAL_TURNRIGHT) {
			roomba_drive(velocity, -300);
            update_remote_control_sensors();
			my_msleep(50);
		}

        // use item
		if(INFRARED_OMNI == HARMONY_SIGNAL_SPOT || INFRARED_RIGHT == HARMONY_SIGNAL_SPOT || INFRARED_LEFT == HARMONY_SIGNAL_SPOT) {
			roomba_use_item();
		}

        // update sensors
		update_cliff_sensors();

        switch(current_state){
            case DRIVE_LANE:
                if(CLIFF_FRONT_LEFT_SIG < 1200 && CLIFF_FRONT_RIGHT_SIG < 1200){
                    if(direction){
                        drive_curve_left();
                    } else {
                        drive_curve_right();
                    }
                } else if (CLIFF_FRONT_LEFT_SIG < 1200) {
                    correct_curse_to_right();
                } else if (CLIFF_FRONT_RIGHT_SIG < 1200) {
                    correct_curse_to_left();
                }
                break;
            case DRIVE_CURVE_LEFT:
                if(CLIFF_FRONT_LEFT_SIG > 1200) {
                    drive_back_to_lane();
                } else if (CLIFF_LEFT_SIG < 1200) {
                    if(offroad_counter > 5) {
                        drive_offroad();
                        offroad_counter = 0;
                    } else {
                        offroad_counter++;
                    }
                }
                break;
            case DRIVE_CURVE_RIGHT:
                if(CLIFF_FRONT_RIGHT_SIG > 1200) {
                    drive_back_to_lane();
                } else if (CLIFF_RIGHT_SIG < 1200) {
                    if(offroad_counter > 5) {
                        drive_offroad();
                        offroad_counter = 0;
                    } else {
                        offroad_counter++;
                    }
                }
                break;
            case DRIVE_BACK_TO_LANE:

                if (CLIFF_FRONT_LEFT_SIG < 1200 && CLIFF_FRONT_RIGHT_SIG > 1200){
                    direction = false;
                } else if (CLIFF_FRONT_LEFT_SIG > 1200 && CLIFF_FRONT_RIGHT_SIG < 1200){
                    direction = true;
                }

                if(direction){
                    if(CLIFF_FRONT_LEFT_SIG > 1200 && CLIFF_FRONT_RIGHT_SIG > 1200){
                        correct_curse_to_right();
                    } else if (CLIFF_FRONT_LEFT_SIG < 1200 && CLIFF_FRONT_RIGHT_SIG < 1200) {
                        drive_curve_left();
                    }
                } else {
                    if(CLIFF_FRONT_LEFT_SIG > 1200 && CLIFF_FRONT_RIGHT_SIG > 1200){
                        correct_curse_to_left();
                    } else if (CLIFF_FRONT_LEFT_SIG < 1200 && CLIFF_FRONT_RIGHT_SIG < 1200) {
                        drive_curve_right();
                    }
                }
                break;
            case CORRECT_CURSE_TO_LEFT:
                if (CLIFF_FRONT_LEFT_SIG < 1200 && CLIFF_FRONT_RIGHT_SIG > 1200){
                    direction = false;
                } else if (CLIFF_FRONT_LEFT_SIG > 1200 && CLIFF_FRONT_RIGHT_SIG < 1200){
                    direction = true;
                }
                if (CLIFF_FRONT_LEFT_SIG < 1200 && CLIFF_FRONT_RIGHT_SIG < 1200){
                    if(direction == false){
                        drive_curve_right();
                    } else {
                        drive_curve_left();
                    }
                }
                if (course_correction_counter > 5){
                    drive_lane();
                    course_correction_counter = 0;
                } else {
                    course_correction_counter++;
                }
                break;
            case CORRECT_CURSE_TO_RIGHT:
                if (CLIFF_FRONT_LEFT_SIG < 1200 && CLIFF_FRONT_RIGHT_SIG > 1200){
                    direction = false;
                } else if (CLIFF_FRONT_LEFT_SIG > 1200 && CLIFF_FRONT_RIGHT_SIG < 1200){
                    direction = true;
                }
                if (CLIFF_FRONT_LEFT_SIG < 1200 && CLIFF_FRONT_RIGHT_SIG < 1200){
                    if(direction == false){
                        drive_curve_right();
                    } else {
                        drive_curve_left();
                    }
                }
                if (course_correction_counter > 5){
                    drive_lane();
                    course_correction_counter = 0;
                } else {
                    course_correction_counter++;
                }
                break;
            case DRIVE_OFFROAD:
                if (CLIFF_FRONT_LEFT_SIG > 1200){
                    correct_curse_to_right();
                } else if (CLIFF_FRONT_RIGHT_SIG > 1200){
                    correct_curse_to_left();
                } else if (CLIFF_LEFT_SIG > 1200 || CLIFF_RIGHT_SIG > 1200){
                    drive_back_to_lane();
                }
                break;
            default:
                break;

        }

        if(roomba_check_for_item(CLIFF_LEFT_SIG, CLIFF_RIGHT_SIG)) {
            if(!item_detected_mode) {
                roomba_pick_up_item();
                item_detected_mode = true;
            }
        }
        else {
            item_detected_mode = false;
        }

        my_msleep(25);

    }

    return 0;
}


void update_remote_control_sensors(void){
    roomba_update_sensor(&infrared_omni, false);
    roomba_update_sensor(&infrared_right, false);
    roomba_update_sensor(&infrared_left, false);
}

void update_cliff_sensors(void){
    roomba_update_sensor(&cliff_front_left_signal, false);
    roomba_update_sensor(&cliff_front_right_signal, false);
    roomba_update_sensor(&cliff_left_signal, false);
    roomba_update_sensor(&cliff_right_signal, false);
}

void drive_offroad(void){
    current_state = DRIVE_OFFROAD;
    // set velocity to offroad velocity (100) if roomba has no boost
    if(velocity != VELOCITY_BOOST){
        velocity = VELOCITY_OFFROAD;
    }
    roomba_drive(velocity, radius);
}

void drive_lane(void){
    current_state = DRIVE_LANE;
    // set offroad counter to 0, because we drive on lane
    offroad_counter = 0;
    roomba_drive(velocity, radius);
}

void drive_curve_left(void){
    course_correction_counter = 0;
    current_state = DRIVE_CURVE_LEFT;
    // set offroad counter to 0, because we drive on lane
    offroad_counter = 0;
    roomba_drive(velocity, 200);
}

void drive_curve_right(void){
    course_correction_counter = 0;
    current_state = DRIVE_CURVE_RIGHT;
    // set offroad counter to 0, because we drive on lane
    offroad_counter = 0;
    roomba_drive(velocity, -200);
}

void drive_back_to_lane(void){
    current_state = DRIVE_BACK_TO_LANE;
    // set velocity to standard velocity (200) if roomba has no boost
    if(velocity != VELOCITY_BOOST){
        velocity = VELOCITY;
    }
    // correct curse of roomba on lane
    roomba_drive(velocity, radius);
}

void correct_curse_to_left(void){
    current_state = CORRECT_CURSE_TO_LEFT;
    // set velocity to standard velocity (200) if roomba has no boost
    if(velocity != VELOCITY_BOOST){
        velocity = VELOCITY;
    }
    // correct curse of roomba on lane
    roomba_drive(velocity, 1000);
}

void correct_curse_to_right(void){
    current_state = CORRECT_CURSE_TO_RIGHT;
    // set velocity to standard velocity (200) if roomba has no boost
    if(velocity != VELOCITY_BOOST){
        velocity = VELOCITY;
    }
    // correct curse of roomba on lane
    roomba_drive(velocity, -1000);
}
