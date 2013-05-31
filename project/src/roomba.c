/**
 * $Id: roomba.c 478 2011-04-19 08:06:54Z bradatch $
 */


/****************************************************************** Includes */

#include <roomba.h>
#include <board.h>
#include <tools.h>
#include <uart.h>
#include <string.h>
#include <asm/io.h>



#include <button.h>
#include <ju.h>
#include <led.h>
#include <pio.h>
#include <s7ctl.h>
#include <stddef.h>


/******************************************************************* Defines */

/******************************************************* Function prototypes */

/************************************************************** Global const */

/*! initializing of the five sensors from typ roomba_sensor_type_t with the constant values
*/
const roomba_sensor_type_t cliff_left_sensor =
{
    SENSORS_CLIFF_LEFT,
    0x01,
    false
};

const roomba_sensor_type_t cliff_left_front_sensor =
{
    SENSORS_CLIFF_FRONT_LEFT,
    0x01,
    false
};

const roomba_sensor_type_t cliff_right_front_sensor =
{
    SENSORS_CLIFF_FRONT_RIGHT,
    0x01,
    false
};

const roomba_sensor_type_t cliff_right_sensor =
{
    SENSORS_CLIFF_RIGHT,
    0x01,
    false
};

const roomba_sensor_type_t cliff_left_signal_sensor =
{
    SENSORS_CLIFF_LEFT_SIGNAL,
    0x02,
    false
};

const roomba_sensor_type_t cliff_front_front_left_signal_sensor =
{
    SENSORS_CLIFF_FRONT_LEFT_SIGNAL,
    0x02,
    false
};

const roomba_sensor_type_t cliff_front_front_right_signal_sensor =
{
    SENSORS_CLIFF_FRONT_RIGHT_SIGNAL,
    0x02,
    false
};

const roomba_sensor_type_t cliff_right_signal_sensor =
{
    SENSORS_CLIFF_RIGHT_SIGNAL,
    0x02,
    false
};

const int8_t sensor_number = 1;

const int16_t radius = 32767;                      //  mm
const int16_t radius_counter_clockwise = 1;        //  mm

/********************************************************** Global variables */
roomba_sensor_t sensor_array_cliff_left[1];
roomba_sensor_t sensor_array_cliff_front_left[1];
roomba_sensor_t sensor_array_cliff_front_right[1];
roomba_sensor_t sensor_array_cliff_right[1];

roomba_sensor_t sensor_array_cliff_left_signal[1];
roomba_sensor_t sensor_array_cliff_front_left_signal[1];
roomba_sensor_t sensor_array_cliff_right_signal[1];
roomba_sensor_t sensor_array_cliff_front_right_signal[1];

int16_t velocity = 200;            // mm/s

//char array for output
char str[5];
/*************************************************************** Local const */

const int32_t NUM_DISPLAYS = 4;

/*********************************************************** Local variables */


/******************************************************************** Macros */


/********************************************************** Global functions */


/*********************************************************** Local functions */

void init_roomba()
{

    uart_write_byte(CMD_START);
    my_msleep(ROOMBA_DELAY_MODECHANGE_MS);

    uart_write_byte(CMD_FULL);
    my_msleep(ROOMBA_DELAY_MODECHANGE_MS);
}

void init_cliff_sensors()
{
	roomba_init_sensor(sensor_array_cliff_left, &cliff_left_sensor);
	roomba_init_sensor(sensor_array_cliff_front_left, &cliff_front_left_sensor);
	roomba_init_sensor(sensor_array_cliff_right, &cliff_right_sensor);
	roomba_init_sensor(sensor_array_cliff_front_right, &cliff_front_right_sensor);
}

void init_cliff_signal()
{
	roomba_init_sensor(sensor_array_cliff_left_signal, &cliff_left_sensor_signal);
	roomba_init_sensor(sensor_array_cliff_front_left_signal, &cliff_front_left_sensor_signal;
	roomba_init_sensor(sensor_array_cliff_right_signal, &cliff_right_sensor_signal);
	roomba_init_sensor(sensor_array_cliff_front_right_signal, &cliff_front_right_sensor_signal);
}

void roomba_set_led_on(uint8_t led_bits, uint8_t clean_power_color, uint8_t clean_power_intensity)
{
    uart_write_byte(CMD_LEDS);
    uart_write_byte(led_bits);
    uart_write_byte(clean_power_color);
    uart_write_byte(clean_power_intensity);
}

void roomba_init_sensor(roomba_sensor_t *sensor, const roomba_sensor_type_t* type)
{

    sensor->type = type; // the same as (*sensor).type = type;
    sensor->value = 0;
}

void roomba_request_sensor(roomba_sensor_t *sensor)
{
    uart_write_byte(sensor->type->package_id);
}

void roomba_request_sensors(roomba_sensor_t *sensor_array, int8_t sensor_number)
{
    uart_write_byte(CMD_QUERY_LIST);
    uart_write_byte(sensor_number);

    int8_t i;
    for(i = 0; i < sensor_number; i++)
    {
        roomba_request_sensor(sensor_array + i);
    }
}

void roomba_read_sensor(roomba_sensor_t *sensor)
{

    if (sensor->type->bytes_number == 1)
    {
        if (sensor->type->is_signed)
        {

            sensor->value = uart_read_byte();  // uart_read_byte() returns int32_t (signed)
        }
        else
        {

            sensor->value = uart_read_byte() & 0xFF; // usigned value
        }
    }
    else   // sensor->type->bytes_number == 2
    {

        IOWR32(A_PIO_LBLUE, PIO_DATA, ledb_vals[2]);

        if (sensor->type->is_signed)
        {

            sensor->value = (uart_read_byte() << 8) | (uart_read_byte() & 0xFF); // sign of the highest byte remains
        }
        else
        {
            sensor->value = ((uart_read_byte() & 0xFF) << 8) | (uart_read_byte() & 0xFF);
        }
    }
}

void roomba_read_sensors(roomba_sensor_t *sensor_array, int8_t sensor_number)
{
    int8_t i;
    for(i = 0; i < sensor_number; i++)
    {
        roomba_read_sensor(sensor_array + i);
    }
}

int32_t roomba_return_current_value(roomba_sensor_t *sensor_array, int8_t sensor_number, int32_t measured_value)
{
    roomba_request_sensors(sensor_array, sensor_number);
    roomba_read_sensors(sensor_array, sensor_number);

    //check value;
    measured_value += sensor_array[0].value;
    return measured_value;
}

void roomba_set_letters_string(const char* str, int8_t length)
{
    int i;
    int32_t zero_padding_len = NUM_DISPLAYS - length;

    uart_write_byte(CMD_7SEG_ASCII);
    for (i = 0; i < zero_padding_len; i++)
    {
        uart_write_byte('0');
    }

    for (i = length; i > 0; i--)
    {
        uart_write_byte(str[i-1]);
    }
}

int8_t itos(int number, char* result)
{

    int8_t length = 0;
    int8_t i = 0;
    int32_t j;

    int32_t modulo = 10;
    int32_t divisor = 1;

    if (number == 0)
    {
        result[0] = '0';
        result[1] = '\0';
        return (length + 1);
    }

    else
    {
        while(number > 0)
        {

            j = (number % modulo);
            number -= j;
            result[i] = j/divisor + 48;

            i++;
            modulo *= 10;
            divisor *= 10;

        }
        length = (i);
        result[i] = '\0';
        return length;
    }
}

void show_number_on_display(int32_t number, char* str) //const char* str
{
    if(number < 0)
    {

        roomba_set_led_on(LED_SPOT_GREEN, COLOR_RED, FULL_INTENSITY);

    }
    int8_t length = itos(ABS_T(number), str);
    roomba_set_letters_string(str, length);
    my_msleep(500);
}

void roomba_drive(int16_t velocity, int16_t radius)
{
    //two-bytes velocity and radius value must be share in two one-bytes values
    int8_t velocity_high_byte = velocity >> 8;
    int8_t velocity_low_byte = velocity;
    int8_t radius_high_byte = radius >> 8;
    int8_t radius_low_byte = radius;

    uart_write_byte(CMD_DRIVE);
    uart_write_byte(velocity_high_byte);
    uart_write_byte(velocity_low_byte);
    uart_write_byte(radius_high_byte);
    uart_write_byte(radius_low_byte);

}

void roomba_stop()
{

    roomba_drive(0, 0);

}

//Umschreiben, damit man auf die ausseren Sachen reagieren koennte
int32_t roomba_turn_angle(int32_t angle_to_turn, int16_t velocity, int16_t radius, int32_t faktor)
{
    int32_t roombas_angle = ((faktor * (angle_to_turn << 3)) >> 3);
    int32_t roombas_turned_angle = 0;

    roomba_drive(velocity, radius);

    do
    {
        my_msleep(500);

        roomba_request_sensors(sensor_array_to_turn, sensor_number);
        roomba_read_sensors(sensor_array_to_turn, sensor_number);

        roombas_turned_angle += sensor_array_to_turn[0].value;

        my_msleep(50);

    }
    while(ABS_T(roombas_turned_angle) < ABS_T(roombas_angle));

    return  (((roombas_turned_angle << 3) / faktor) >> 3);
}

uint8_t read_button() {
	uart_write_byte(CMD_SENSORS);
	uart_write_byte(BUT_PACK_ID);
	return uart_read_byte();
}


