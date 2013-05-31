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

#include <asm/io.h>
#include <harmony.h>

/******************************************************************* Defines */


/******************************************************* Function prototypes */

/************************************************************** Global const */


/********************************************************** Global variables */


/*************************************************************** Local const */


/*********************************************************** Local variables */

char greeting[4] = {'I', 'H', 'I', 'H'};

/******************************************************************** Macros */

#define CLIFF_LEFT_SIG roomba_return_current_value(sensor_array_cliff_left_signal, 0)
#define CLIFF_FRONT_LEFT_SIG roomba_return_current_value(sensor_array_cliff_front_left_signal, 0)
#define CLIFF_RIGHT_SIG roomba_return_current_value(sensor_array_cliff_right_signal, 0)
#define CLIFF_FRONT_RIGHT_SIG roomba_return_current_value(sensor_array_cliff_front_right_signal, 0)

#define INFRARED_OMNI sensor_array_infrared_omni[0].value//roomba_return_current_value(sensor_array_infrared_omni, 0)
#define INFRARED_RIGHT  sensor_array_infrared_right[0].value//roomba_return_current_value(sensor_array_infrared_right, 0)
#define INFRARED_LEFT  sensor_array_infrared_left[0].value//roomba_return_current_value(sensor_array_infrared_left, 0)

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
    button_wait(1);

    while(true){

        if(button_pressed(0)){
            roomba_stop();
            break;
        }

		roomba_return_current_value(sensor_array_infrared_omni, 0);
		roomba_return_current_value(sensor_array_infrared_right, 0);
		roomba_return_current_value(sensor_array_infrared_left, 0);
		
		if(INFRARED_OMNI == HARMONY_SIGNAL_PAUSE || INFRARED_RIGHT == HARMONY_SIGNAL_PAUSE || INFRARED_LEFT == HARMONY_SIGNAL_PAUSE) {
			roomba_set_led_on(BTN_CLEAN, 100, 100);
		}
		
		while(INFRARED_OMNI == HARMONY_SIGNAL_TURNLEFT || INFRARED_RIGHT == HARMONY_SIGNAL_TURNLEFT || INFRARED_LEFT == HARMONY_SIGNAL_TURNLEFT) {
			roomba_drive(velocity, 300);
			roomba_return_current_value(sensor_array_infrared_omni, 0);
			roomba_return_current_value(sensor_array_infrared_right, 0);
			roomba_return_current_value(sensor_array_infrared_left, 0);
			my_msleep(50);
		}
		
		while(INFRARED_OMNI == HARMONY_SIGNAL_TURNRIGHT || INFRARED_RIGHT == HARMONY_SIGNAL_TURNRIGHT || INFRARED_LEFT == HARMONY_SIGNAL_TURNRIGHT) {
			roomba_drive(velocity, -300);
			roomba_return_current_value(sensor_array_infrared_omni, 0);
			roomba_return_current_value(sensor_array_infrared_right, 0);
			roomba_return_current_value(sensor_array_infrared_left, 0);
			my_msleep(50);
		}
		if(INFRARED_OMNI == HARMONY_SIGNAL_SPOT || INFRARED_RIGHT == HARMONY_SIGNAL_SPOT || INFRARED_LEFT == HARMONY_SIGNAL_SPOT) {
			velocity = (velocity == 400 ?  200 : 400);
		}

        if(CLIFF_FRONT_LEFT_SIG > CLIFF_FRONT_RIGHT_SIG + 500){
            roomba_drive(velocity, 200);
        } else if(CLIFF_FRONT_LEFT_SIG + 500 < CLIFF_FRONT_RIGHT_SIG){
            roomba_drive(velocity, -200);
        } else {
            roomba_drive(velocity, radius);
        }


        my_msleep(50);

    }

    return 0;
}
