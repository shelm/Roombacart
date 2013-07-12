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

#include <timer.h> // needed for timer
#include <irq.h>   // needed for timer

#include <ir.h>

/******************************************************************* Defines */

#define ONE_SECOND_TIMER 75000000

/******************************************************* Function prototypes */

/************************************************************** Global const */

/*! initializing of the five sensors from typ roomba_sensor_type_t with the constant values
*/
const roomba_sensor_type_t cliff_left_signal_sensor =
{
    SENSORS_CLIFF_LEFT_SIGNAL,
    0x02,
    false
};

const roomba_sensor_type_t cliff_front_left_signal_sensor =
{
    SENSORS_CLIFF_FRONT_LEFT_SIGNAL,
    0x02,
    false
};

const roomba_sensor_type_t cliff_front_right_signal_sensor =
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

const roomba_sensor_type_t infrared_omni_sensor =
{
    SENSORS_INFRARED_CHARACTER_OMNI,
    0x01,
    false
};


const roomba_sensor_type_t infrared_right_sensor =
{
    SENSORS_INFRARED_CHARACTER_RIGHT,
    0x01,
    false
};

const roomba_sensor_type_t infrared_left_sensor =
{
    SENSORS_INFRARED_CHARACTER_LEFT,
    0x01,
    false
};

//const int8_t sensor_number = 1;

const int16_t radius = 32767;                      //  mm
const int16_t radius_counter_clockwise = 1;        //  mm

volatile enum item_t current_item = empty;
volatile enum item_t last_item_used = empty;
volatile enum state_t current_state = -1;
volatile enum state_t last_state = -1;
volatile bool_t should_refresh_state = false;

volatile int16_t timer_array[4] = {5, 5, 3, 0};
volatile int16_t timer_active_array[4] = {0, 0, 0, 0};

volatile uint16_t random_item = 1;

/********************************************************** Global variables */

roomba_sensor_t cliff_left_signal;
roomba_sensor_t cliff_front_left_signal;
roomba_sensor_t cliff_right_signal;
roomba_sensor_t cliff_front_right_signal;

roomba_sensor_t infrared_omni;
roomba_sensor_t infrared_right;
roomba_sensor_t infrared_left;

volatile int16_t velocity = VELOCITY;    // mm/s

volatile uint16_t random_seed = 0;
//char array for output
char str[5];
char speed_str[4] = {' ', 'D', 'P', 'S'};
char shell_str[4] = {' ', 'L', 'H', 'S'};

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

void init_cliff_signal()
{
	roomba_init_sensor(&cliff_left_signal, &cliff_left_signal_sensor);
	roomba_init_sensor(&cliff_front_left_signal, &cliff_front_left_signal_sensor);
	roomba_init_sensor(&cliff_right_signal, &cliff_right_signal_sensor);
	roomba_init_sensor(&cliff_front_right_signal, &cliff_front_right_signal_sensor);
}

void init_infrared()
{
	roomba_init_sensor(&infrared_omni, &infrared_omni_sensor);
	roomba_init_sensor(&infrared_right, &infrared_right_sensor);
	roomba_init_sensor(&infrared_left, &infrared_left_sensor);
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
    uart_write_byte(CMD_QUERY_LIST);
    uart_write_byte(1);
	uart_write_byte(sensor->type->package_id);
}

void roomba_read_sensor(roomba_sensor_t *sensor, bool_t accumulate_value)
{
    int32_t current_value;

    if (sensor->type->bytes_number == 1)
    {
        if (sensor->type->is_signed)
        {
            current_value = uart_read_byte();  // uart_read_byte() returns int32_t (signed)
        }
        else
        {
            current_value = uart_read_byte() & 0xFF; // usigned value
        }
    }
    else   // sensor->type->bytes_number == 2
    {
        if (sensor->type->is_signed)
        {
            current_value = (uart_read_byte() << 8) | (uart_read_byte() & 0xFF); // sign of the highest byte remains
        }
        else
        {
            current_value = ((uart_read_byte() & 0xFF) << 8) | (uart_read_byte() & 0xFF);
        }
    }

	if(accumulate_value)
	{
	    sensor->value += current_value;
	}
	else
	{
	   sensor->value = current_value;
	}
}

int32_t roomba_update_sensor(roomba_sensor_t *sensor, bool_t accumulate_value)
{
  roomba_request_sensor(sensor);
  roomba_read_sensor(sensor, accumulate_value);

  return sensor->value;
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
//    int32_t roombas_angle = ((faktor * (angle_to_turn << 3)) >> 3);
//    int32_t roombas_turned_angle = 0;
//
//    roomba_drive(velocity, radius);
//
//    do
//    {
//        my_msleep(500);
//
//        roomba_request_sensors(sensor_array_to_turn, sensor_number);
//        roomba_read_sensors(sensor_array_to_turn, sensor_number);
//
//        roombas_turned_angle += sensor_array_to_turn[0].value;
//
//        my_msleep(50);
//
//    }
//    while(ABS_T(roombas_turned_angle) < ABS_T(roombas_angle));
//
//    return  (((roombas_turned_angle << 3) / faktor) >> 3);
return 0;
}

uint8_t read_button() {
	uart_write_byte(CMD_SENSORS);
	uart_write_byte(SENSORS_BUTTONS);
	return uart_read_byte();
}

bool_t roomba_check_for_item(int32_t cliff_left_signal_val, int32_t cliff_right_signal_val) {
    //show_number_on_display(cliff_left_signal_val, str);
    if(cliff_left_signal_val > 2900 || cliff_right_signal_val > 2900)
        return true;
    return false;
}

bool_t roomba_check_for_finish_mark(int32_t cliff_front_left_signal_val, int32_t cliff_front_right_signal_val) {
    //show_number_on_display(cliff_left_signal_val, str);
     if((cliff_front_left_signal_val > 2900 && cliff_front_right_signal_val > 2900) ||
       (cliff_front_left_signal_val > 2900 && cliff_front_right_signal_val < 1200) ||
       (cliff_front_left_signal_val < 1200 && cliff_front_right_signal_val > 2900))
        return true;
    return false;
}

void roomba_pick_up_item() {
	play_song(3);
    current_item = roomba_generate_rand_item();
}

enum item_t roomba_generate_rand_item() {
    /*int8_t my_number = my_rand();
    show_number_on_display(my_number, str);
    my_msleep(500);
    return my_number;*/
    return (my_rand()%(last-1))+1;
}

void roomba_use_item() {
    should_refresh_state = true;
    ir_sender_on();
    switch(current_item) {
        case speed:
            velocity = VELOCITY_BOOST;
            timer_active_array[0] = 1;
            break;
        case shell:
			ir_sender_set(ITEM_ID_SHELL, ITEM_ID_SHELL, ITEM_ID_SHELL, ITEM_ID_SHELL);
			ir_sender_on();
			timer_active_array[1] = 1;
            break;
        default:
            break;
    }
    last_item_used = current_item;
    current_item = empty;
    roomba_set_led_on(0, 230,255);
}

void roomba_item_effect_ends() {

    switch(last_item_used) {
        case speed:
            velocity = VELOCITY;
            roomba_set_led_on(0, 70,255);
            break;
        case shell:
			ir_sender_off();
			break;
        default:
            break;
    }
    should_refresh_state = true;
    last_item_used = empty;
}

void roomba_got_hit_by_item(uint8_t item_id) {
	 switch(item_id) {
        case shell:
            timer_active_array[2] = 1;
            break;
        default:
            break;
    }
}

void roomba_got_hit_effect_ends(){
    current_state = last_state;
    should_refresh_state = true;
}

void roomba_start_timer(void){
    irq_request(IRQ_TIMER_N, roomba_check_timer_array);
    tt_periodic(ONE_SECOND_TIMER);
    irq_enable(IRQ_TIMER_N);
}

uint32_t roomba_check_timer_array(){

    if(timer_array[0] > 0 && timer_active_array[0] == 1){
		if(!check_for_playing_song()) {
			play_song(1);
		}
        timer_array[0]--;
    } else if (timer_array[0] == 0 && timer_active_array[0] == 1){
        timer_active_array[0] = 0;
        timer_array[0] = 5;
        roomba_item_effect_ends();
    }
    if(timer_array[1] > 0 && timer_active_array[1] == 1){
        timer_array[1]--;
    } else if (timer_array[1] == 0 && timer_active_array[1] == 1){
        timer_active_array[1] = 0;
        timer_array[1] = 5;
        roomba_item_effect_ends();
    }
    if(timer_array[2] > 0 && timer_active_array[2] == 1){
        play_song(0);
        timer_array[2]--;
    } else if (timer_array[2] == 0 && timer_active_array[2] == 1){
        timer_active_array[2] = 0;
        timer_array[2] = 3;
        roomba_got_hit_effect_ends();
    }

    if(timer_array[3] < 100){
         timer_array[3]++;
    }
    else{
        timer_array[3] = 0;
    }

    tt_reset();
    irq_disable(IRQ_TIMER_N);
    return 1;
}

uint16_t my_rand(void){

  return timer_array[3];

}

void roomba_show_item(){
     switch(current_item) {
        case speed:
            roomba_set_letters_string(speed_str, 4);
            break;
        case shell:
			roomba_set_letters_string(shell_str, 4);
            break;
        default:
            break;
    }
}

void roomba_show_is_focused() {
	roomba_set_led_on(0,128,128);
	my_msleep(100);
	roomba_set_led_on(0,0,0);
}

uint8_t check_for_active_item() {
	return (timer_active_array[0] | timer_active_array[1] | timer_active_array[2]);
};
