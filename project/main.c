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

#include <ir.h>

/******************************************************************* Defines */


/******************************************************* Function prototypes */


void update_remote_control_sensors(void);

void update_cliff_sensors(void);

void user_drives_to_left(void);

void user_drives_to_right(void);

void drive_offroad(void);

void drive_lane(void);

void drive_curve_left(void);

void drive_curve_right(void);

void drive_back_to_lane(void);

void correct_curse_to_left(void);

void correct_curse_to_right(void);

void refresh_current_state(void);

void roomba_got_hit(void);


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
bool_t finish_mark_detected_mode = false;

int offroad_counter = 0;
int course_correction_counter = 0;
int round_counter = 0;

int loop_counter_for_round = 0;
int loop_counter_for_item = 0;

/******************************************************************** Macros */

#define CLIFF_LEFT_SIG         cliff_left_signal.value
#define CLIFF_RIGHT_SIG        cliff_right_signal.value
#define CLIFF_FRONT_LEFT_SIG   cliff_front_left_signal.value
#define CLIFF_FRONT_RIGHT_SIG  cliff_front_right_signal.value


#define INFRARED_OMNI  infrared_omni.value
#define INFRARED_RIGHT infrared_right.value
#define INFRARED_LEFT  infrared_left.value

#define LEFT 0
#define RIGHT 1

/********************************************************** Global functions */


/*********************************************************** Local functions */


int main(int argc, char **argv)
{
	ir_sender_setup();
	ir_sender_on();

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

	transmit_song(mortal_kombat, 16, 0);
	my_msleep(15);
	transmit_song(mortal_kombat_part_2, 16, 1);
	my_msleep(15);
	transmit_song(indiana_jones_theme_part_3, 14, 2);
	my_msleep(15);
	transmit_song(item_detected_sound, 4, 3);
	/*
	play_song(0);
	while(check_for_playing_song()){
		my_msleep(15);
	}
	play_song(1);
	while(check_for_playing_song()){
		my_msleep(15);
	}

	play_song(0);
	while(check_for_playing_song()){
		my_msleep(15);
	}
	play_song(1);
	while(check_for_playing_song()){
		my_msleep(15);
	}*/

	//play_song(2);

	//Dominik Infrared-test
	//ir_sender_setup();
	//IOWR32(A_IR_SENDER, IR_SENDER_DATA, 0xA0A0A0A0);

	//ir_sender_set_item(ITEM_ID_SHELL);
	//ir_sender_on();
	/*
	while(1) {
		update_remote_control_sensors();
		show_number_on_display((INFRARED_LEFT|INFRARED_RIGHT|INFRARED_OMNI));
		my_msleep(100);

	}*/


    button_wait(1);
    drive_lane();
    roomba_start_timer();

    while(true){

        if(button_pressed(0)){
            roomba_stop();
            break;
        }

		current_item = shell;

        //show_number_on_display(offroad_counter, str);
        update_remote_control_sensors();


        // drive to left because user wants to
		if(INFRARED_OMNI == HARMONY_SIGNAL_TURNLEFT || INFRARED_RIGHT == HARMONY_SIGNAL_TURNLEFT || INFRARED_LEFT == HARMONY_SIGNAL_TURNLEFT) {
		    user_drives_to_left();
		}

        // drive to right because user wants to
		if(INFRARED_OMNI == HARMONY_SIGNAL_TURNRIGHT || INFRARED_RIGHT == HARMONY_SIGNAL_TURNRIGHT || INFRARED_LEFT == HARMONY_SIGNAL_TURNRIGHT) {
		    user_drives_to_right();
		}

        // use item
		if(INFRARED_OMNI == HARMONY_SIGNAL_SPOT || INFRARED_RIGHT == HARMONY_SIGNAL_SPOT || INFRARED_LEFT == HARMONY_SIGNAL_SPOT) {
			roomba_use_item();
		}
		if(ir_get_sender_type_from_data(INFRARED_LEFT|INFRARED_OMNI|INFRARED_RIGHT) == IR_SENDER_ROOMBA && ir_get_roomba_id_from_data(INFRARED_LEFT|INFRARED_OMNI|INFRARED_RIGHT) != ROOMBA_ID)
		switch(ir_get_item_id_from_data(INFRARED_LEFT|INFRARED_RIGHT|INFRARED_OMNI)) {
			case ITEM_ID_SHELL:
				roomba_got_hit_by_item(shell);
				break;
			default:
				break;
		}



        if(should_refresh_state)
            refresh_current_state();

        // update sensors
		update_cliff_sensors();


        switch(current_state){
            case USER_DRIVES_TO_LEFT:
                if(CLIFF_FRONT_LEFT_SIG < 1200 && CLIFF_FRONT_RIGHT_SIG < 1200 && CLIFF_LEFT_SIG < 1200 && CLIFF_RIGHT_SIG){
                    if(offroad_counter > 5) {
                        drive_offroad();
                    } else {
                        offroad_counter++;
                    }
                }
                if(!(INFRARED_OMNI == HARMONY_SIGNAL_TURNLEFT || INFRARED_RIGHT == HARMONY_SIGNAL_TURNLEFT || INFRARED_LEFT == HARMONY_SIGNAL_TURNLEFT)){
                    if(CLIFF_FRONT_LEFT_SIG > 1200 || CLIFF_FRONT_RIGHT_SIG > 1200){
                        drive_lane();
                    } else {
                        drive_offroad();
                    }
                }
                break;
            case USER_DRIVES_TO_RIGHT:
                if(CLIFF_FRONT_LEFT_SIG < 1200 && CLIFF_FRONT_RIGHT_SIG < 1200 && CLIFF_LEFT_SIG < 1200 && CLIFF_RIGHT_SIG){
                    if(offroad_counter > 5) {
                        drive_offroad();
                    } else {
                        offroad_counter++;
                    }
                }
                if(!(INFRARED_OMNI == HARMONY_SIGNAL_TURNRIGHT || INFRARED_RIGHT == HARMONY_SIGNAL_TURNRIGHT || INFRARED_LEFT == HARMONY_SIGNAL_TURNRIGHT)){
                    if(CLIFF_FRONT_LEFT_SIG > 1200 || CLIFF_FRONT_RIGHT_SIG > 1200){
                        drive_lane();
                    } else {
                        drive_offroad();
                    }
                }
                break;
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
                    offroad_counter = 0;
                } else if (CLIFF_LEFT_SIG < 1200) {
                    if(offroad_counter > 15) {
                        drive_offroad();
                    } else {
                        offroad_counter++;
                    }
                } else {
                    offroad_counter = 0;
                }
                break;
            case DRIVE_CURVE_RIGHT:
                if(CLIFF_FRONT_RIGHT_SIG > 1200) {
                    drive_back_to_lane();
                    offroad_counter = 0;
                } else if (CLIFF_RIGHT_SIG < 1200) {
                    if(offroad_counter > 15) {
                        drive_offroad();
                    } else {
                        offroad_counter++;
                    }
                } else {
                    offroad_counter = 0;
                }
                break;
            case DRIVE_BACK_TO_LANE:

                if (CLIFF_FRONT_LEFT_SIG < 1200 && CLIFF_FRONT_RIGHT_SIG > 1200){
                    direction = LEFT;
                } else if (CLIFF_FRONT_LEFT_SIG > 1200 && CLIFF_FRONT_RIGHT_SIG < 1200){
                    direction = RIGHT;
                }

                if(direction == RIGHT){
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
                    direction = LEFT;
                } else if (CLIFF_FRONT_LEFT_SIG > 1200 && CLIFF_FRONT_RIGHT_SIG < 1200){
                    direction = RIGHT;
                }
                if (CLIFF_FRONT_LEFT_SIG < 1200 && CLIFF_FRONT_RIGHT_SIG < 1200){
                    if(direction == LEFT){
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
                    direction = LEFT;
                } else if (CLIFF_FRONT_LEFT_SIG > 1200 && CLIFF_FRONT_RIGHT_SIG < 1200){
                    direction = RIGHT;
                }
                if (CLIFF_FRONT_LEFT_SIG < 1200 && CLIFF_FRONT_RIGHT_SIG < 1200){
                    if(direction == LEFT){
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
                item_detected_mode = true;
                show_number_on_display(item_detected_mode, str);
                roomba_pick_up_item();
                loop_counter_for_item = 0;
            }
            //necessary to avoid repeated recognize of the item at the same round
            //main idea:
            //item can be recongnized only if item_detected_mode = false
            //is the item rekognized, is item_detected_mode = true
            //and only if loop_counter_for_item > 5 can loop_counter_for_item be reseted
            else
            {
                if(loop_counter_for_item > 7)
                {
                    item_detected_mode = false;
                    //show_number_on_display(loop_counter_for_item, str);
                }
                else
                {
                    loop_counter_for_item++;
                    //show_number_on_display(loop_counter_for_item, str);
                }
            }
        }
        else
        {
            if(loop_counter_for_item > 7)
            {
                item_detected_mode = false;
                //show_number_on_display(loop_counter_for_item, str);
            }
            else
            {
                loop_counter_for_item++;
                //show_number_on_display(loop_counter_for_item, str);
            }
        }

        if(roomba_check_for_finish_mark(CLIFF_FRONT_LEFT_SIG, CLIFF_FRONT_RIGHT_SIG))
        {

            //necessary to avoid repeated recognize of the finish mark at the same round
            if(round_counter < 3)
            {
                if(!finish_mark_detected_mode)
                {
                    round_counter++;
                    show_number_on_display(round_counter, str);
                    finish_mark_detected_mode = true;
                    loop_counter_for_round = 0;
                }
                else
                {
                    if(loop_counter_for_round > 7)
                    {
                        finish_mark_detected_mode = false;
                        //show_number_on_display(loop_counter_for_round, str);
                    }
                    else
                    {
                        loop_counter_for_round++;
                        //show_number_on_display(loop_counter_for_round, str);
                    }
                }
            }
            else
            {
                roomba_stop();
                show_number_on_display(round_counter, str);
            }
        }
        else
        {
            /*if(loop_counter_for_item > 7)
            {
                item_detected_mode = false;
                //show_number_on_display(loop_counter_for_item, str);
            }
            else
            {
                loop_counter_for_item++;
                //show_number_on_display(loop_counter_for_item, str);
            }*/

            if(loop_counter_for_round > 7)
            {
                finish_mark_detected_mode = false;
                //show_number_on_display(loop_counter_for_round, str);
            }
            else
            {
                loop_counter_for_round++;
                //show_number_on_display(loop_counter_for_round, str);
            }
        }
    }

    return 0;
}


void update_remote_control_sensors(void){
    disable_all_interrupts();
    roomba_update_sensor(&infrared_omni, false);
    roomba_update_sensor(&infrared_right, false);
    roomba_update_sensor(&infrared_left, false);
    enable_all_interrupts();
}

void update_cliff_sensors(void){
    disable_all_interrupts();
    roomba_update_sensor(&cliff_front_left_signal, false);
    roomba_update_sensor(&cliff_front_right_signal, false);
    roomba_update_sensor(&cliff_left_signal, false);
    roomba_update_sensor(&cliff_right_signal, false);
    enable_all_interrupts();
}

void user_drives_to_left(void){
    current_state = USER_DRIVES_TO_LEFT;
    roomba_drive(velocity, 300);
    direction = LEFT;
}

void user_drives_to_right(void){
    current_state = USER_DRIVES_TO_RIGHT;
    roomba_drive(velocity, -300);
    direction = RIGHT;
}

void drive_offroad(void){
    current_state = DRIVE_OFFROAD;
    // set velocity to offroad velocity (100) if roomba has no boost
    if(velocity != VELOCITY_BOOST){
        velocity = VELOCITY_OFFROAD;
    }
    offroad_counter = 0;
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
    //offroad_counter = 0;
    roomba_drive(velocity, velocity == VELOCITY ? 200 : 350);
}

void drive_curve_right(void){
    course_correction_counter = 0;
    current_state = DRIVE_CURVE_RIGHT;
    // set offroad counter to 0, because we drive on lane
    //offroad_counter = 0;
    roomba_drive(velocity, velocity == VELOCITY ? -200 : -350);
}

void drive_back_to_lane(void){
    offroad_counter = 0;
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
    roomba_drive(velocity, velocity == VELOCITY ? 1000 : 1750);
}

void correct_curse_to_right(void){
    current_state = CORRECT_CURSE_TO_RIGHT;
    // set velocity to standard velocity (200) if roomba has no boost
    if(velocity != VELOCITY_BOOST){
        velocity = VELOCITY;
    }
    // correct curse of roomba on lane
    roomba_drive(velocity, velocity == VELOCITY ? -1000 : -1750);
}

void roomba_got_hit(void){
    last_state = current_state;
    current_state = ROOMBA_GOT_HIT_BY_ITEM;
    roomba_drive(velocity, 1);
    roomba_got_hit_by_item(shell);
}

void refresh_current_state(void){
     switch(current_state){
        case DRIVE_LANE:
            drive_lane();
            break;
        case DRIVE_CURVE_LEFT:
            drive_curve_left();
            break;
        case DRIVE_CURVE_RIGHT:
            drive_curve_right();
            break;
        case DRIVE_BACK_TO_LANE:
            drive_back_to_lane();
            break;
        case CORRECT_CURSE_TO_LEFT:
            correct_curse_to_left();
            break;
        case CORRECT_CURSE_TO_RIGHT:
            correct_curse_to_right();
            break;
        case DRIVE_OFFROAD:
            drive_offroad();
            break;
        default:
            break;
     }
     should_refresh_state = false;
}


