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

/******************************************************************* Defines */


/******************************************************* Function prototypes */

/************************************************************** Global const */


/********************************************************** Global variables */


/*************************************************************** Local const */


/*********************************************************** Local variables */

char greeting[4] = {'I', 'H', 'I', 'H'};

/******************************************************************** Macros */


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

    button_wait(1);

    while(true){

        if(button_pressed(0)){
            roomba_stop();
            break;
        }

        int32_t cliff_left_signal = roomba_return_current_value(sensor_array_cliff_left_signal, 0);
        int32_t cliff_front_left_signal = roomba_return_current_value(sensor_array_cliff_front_left_signal, 0);
        int32_t cliff_right_signal = roomba_return_current_value(sensor_array_cliff_front_right_signal, 0);
        int32_t cliff_front_right_signal = roomba_return_current_value(sensor_array_cliff_front_right_signal, 0);

        if(cliff_front_left_signal > cliff_front_right_signal + 500){
            roomba_drive(velocity, 200);
        } else if(cliff_front_left_signal + 500 < cliff_front_right_signal){
            roomba_drive(velocity, -200);
        } else {
            roomba_drive(velocity, radius);
        }


        my_msleep(50);

    }

    return 0;
}
