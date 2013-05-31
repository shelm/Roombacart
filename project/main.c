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

    init_cliff_sensors();
    init_cliff_signal();
    //initialize the existing sensors (for example, bump_sensor) with value 0
    //for example
    //roomba_init_sensor(sensor_array_to_drive, &distance_sensor);

    return 0;
}
