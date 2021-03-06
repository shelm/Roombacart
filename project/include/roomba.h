#ifndef _ROOMBA_H
#define _ROOMBA_H 1

/****************************************************************** Includes */

#ifdef __cplusplus
extern "C" {
#endif

#include "board.h"
#include <irq.h>
#include "tools.h"

/******************************************************************* Typedefs */
#define ABS_T(number) ((number) >= 0? (number) : -(number))

/*! struct with the constant sensor values;
*/
typedef struct
{
    int8_t package_id;
    uint8_t bytes_number;
    bool_t is_signed;
} roomba_sensor_type_t;

/*! struct with the roomba_sensor_t struct and value of this sensor;
*/
typedef struct {
    const roomba_sensor_type_t *type;
    int32_t volatile value;
} roomba_sensor_t;

/*! enum for all available items
*/
enum item_t{
    empty,
    speed,
    shell,
    last
};

/*! enum for all available states
*/
enum state_t{
    DRIVE_OFFROAD,
    DRIVE_LANE,
    DRIVE_CURVE_LEFT,
    DRIVE_CURVE_RIGHT,
    DRIVE_BACK_TO_LANE,
    CORRECT_CURSE_TO_LEFT,
    CORRECT_CURSE_TO_RIGHT,
    USER_DRIVES_TO_LEFT,
    USER_DRIVES_TO_RIGHT,
    ROOMBA_GOT_HIT_BY_ITEM
};

/******************************************************************* Defines */

#define ROOMBA_DELAY_MODECHANGE_MS      20

#define CMD_SOFT_RESET			0x07	//supposedly does a soft reset
#define CMD_START				0x80	//start the ROI; roomba is in passive mode afterwards; use multiple times to reset
#define CMD_BAUD				0x81	//1 data byte; set baud rate in bits per second; data byte is the baud code (0-11)
										//use a code of 11 as a good default (115200bps)
#define CMD_CONTROL				0x82	//enables control of the roomba; send after start; sets roomba to "safe" mode
#define CMD_SAFE				0x83	//return roomba to "safe" mode
#define CMD_FULL				0x84	//put roomba in "full" mode
#define CMD_POWER				0x85	//virtual "power" button press; puts roomba in "sleep" mode
#define CMD_SPOT				0x86	//virtual "spot" button press; starts the "spot" cleaning algorithm
#define CMD_CLEAN				0x87	//virtual "clean" button press; starts "clean" cleaning algorithm
#define CMD_MAX					0x88	//virtual "max" button press; starts "max" cleaning algorithm
#define CMD_DRIVE				0x89	//4 data bytes; controls the drive wheels two bytes for velocity and two for direction;
										//each represents a 16-bit signed value using two's complement
										//velocity is the speed to use from -500 to 500 (0xFE0C to 0x01F4)
										//direction is either the turn radius (-2000 to 2000mm) (0xF830 to 0x07D0) or a special value:
										//	straight: 32768 (0x8000) or 32767 (0x7FFF)
										//	Spin counter-clockwise: 1 (0x0001)
										//	Spin clockwise: -1 (0xFFFF)
										//when going forward a positive radius turns to the right and a negative to the left
#define CMD_MOTORS				0x8A	//1 data byte; turn clean motors on/off;
										//bitfield: main brush=bit 2(0x04), vacuum=bit 1(0x02), side brush=bit 0(0x01)
										//bits 3(0x08) and 4(0x10) control side brush and main brush direction;
										//0=default direction;1=other direction
#define CMD_LEDS		     	0x8B	//3 data bytes;
										//first is a bitfield: status(green)=bit 5(0x20), status(red)=bit 4(0x10),
										//spot(green)=bit 3(0x08), clean(green)=bit 2(0x04), max(green)=bit 1(0x02),
										//dirt detect(blue)=bit 0(0x01)
										//second is the color of the power led: 0=green, 255=red; values inbetween mix the colors
										//third is the intensity of the power led: 0=off, 255=full on;
										//values inbetween are intermediate intensities
#define CMD_SONG			    0x8C	//2+2N data bytes; first byte defines number of song to specify (0-4);
										//the second byte defines the length of the song(1-16); number of specified notes follow
										//a note consists of two bytes: note number(0x1F-0x7F)(G0-G8)
										//and note duration in units of 1/64th of a second
#define CMD_PLAY			    0x8D	//1 data byte; play a song in the slot defined by data byte 1; song does not repeat
#define CMD_SENSORS			    0x8E	//1 data byte; requests sensor data; data byte specifies the requested packet ID;
										//see OI Specification for sensor packet IDs;
										//a value of 100 specifies to retrieve all sensor data
										//values from 0 to 6 and from 101 to 107 specify groups of sensor data
#define CMD_DOCK			    0x8F	//turns on "force seeking dock" algorithm
#define CMD_PWM_MOTORS			0x90	//3 data bytes;control speed of brushes: [main brush pwm][side brush pwm][vacuum pwm];
										//brushes take values from -127-127; vacuum 0-127
#define CMD_DRIVE_DIRECT		0x91	//4 data bytes;first two bytes control the right wheel velocity;
										//the last two bytes the left wheel velocity (mm/s)
										//the first byte of each velocity is the high byte,
										//and the second is the low byte of a 16-bit signed value (twos-complement); see CMD_DRIVE
										//velocity is -500 - 500
#define CMD_DRIVE_PWM			0x92	//4 data bytes;first two bytes control the right wheel PWM;
										//the last two bytes the left wheel PWM
										//the first byte of each PWM is the high byte,
										//and the second is the low byte of a 16-bit signed value
										//PWM is -255 - 255
#define CMD_STREAM			    0x94	//N+1 data bytes; starts a stream of data packets; the data is sent every 15ms
										//first data byte is the number of requested packets
										//each following data byte is an ID of a requested packet
										//return format is: [0x13] [Length] [Packet 1 ID][Packet 1 Data]
										//[Packet 2 ID][Packet 2 Data] ... [Checksum]
										//length is the number of following bytes (not including checksum)
										//the checksum is 1 byte: the 8-bit complement of all bytes between header and checksum
										//if more data is requested than can be sent in the timeslot the results get corrupted
#define CMD_QUERY_LIST			0x95	//N+1 data bytes; first data byte is the number of requested packets
										//each following data byte is an ID of a requested packet
										//the packets are returned in the specified order
#define CMD_PAUSE_RESUME_STREAM	0x96	//1 data byte; data of 0 stops a running stream without clearing the list of requested packets
										//data of 1 starts the stream using the last used packet list
#define CMD_SCHEDULE_LED		0xA2	//2 data bytes; scheduling leds;first byte for the weekday leds;
										//second byte for the scheduling leds; both bitfields
#define CMD_7SEG_RAW			0xA3	//4 data bytes;[digit 3][digit 2][digit 1][digit 0];see roomba oi spec p. 13
#define CMD_7SEG_ASCII			0xA4	//4 data bytes;[digit 3][digit 2][digit 1][digit 0];ascii codes from 32-126 are supported
#define CMD_BUTTONS			    0xA5	//1 data byte;simulates button push with autorelease; bitfield
#define CMD_SCHEDULE			0xA7	//15 data bytes; sends a new schedule;24 hour time format
										//[days] [sun_hour] [sun_minute] [mon_hour] [mon_minute] [tue_hour] [tue_minute]
										//[wed_hour] [wed_minute] [thu_hour] [thu_minute] [fri_hour] [fri_minute]
										//[sat_hour] [sat_minute]
#define CMD_SET_DATE			0xA8	//3 data bytes; [day][hour][minute]; 24 hour time format;
										//day codes from 0(sunday) to 6(saturday)

#define LED_CHECK_ROBOT_RED			0x08
#define LED_DOCK_GREEN				0x04
#define LED_SPOT_GREEN				0x02
#define LED_DIRT_DETECT_BLUE		0x01

#define LED_SCHED_SCHEDULE	0x10
#define LED_SCHED_CLOCK		0x08
#define LED_SCHED_AM		0x04
#define LED_SCHED_PM		0x02
#define LED_SCHED_COLON		0x01

#define BTN_CLOCK		0x00000080
#define BTN_SCHEDULE	0x40
#define BTN_DAY			0x20
#define BTN_HOUR		0x10
#define BTN_MINUTE		0x08
#define BTN_DOCK		0x04
#define BTN_SPOT		0x02
#define BTN_CLEAN		0x01

#define SENSORS_BUTTONS     0x12

#define SENSORS_ALL						  0x00
#define SENSORS_PHYSICAL				  0x01
#define SENSORS_BUTTONS_AND_INTERNAL	  0x02
#define SENSORS_POWER					  0x03

#define SENSORS_BUMPS_WHEEL_DROPS         0x07
#define SENSORS_VIRTUAL_WALL              0x0D
#define SENSORS_INFRARED_CHARACTER_OMNI   0x11
#define SENSORS_INFRARED_CHARACTER_LEFT   0x34
#define SENSORS_INFRARED_CHARACTER_RIGHT  0x35
#define SENSORS_DISTANCE                  0x13
#define SENSORS_ANGLE                     0x14
#define SENSORS_TEMPERATURE               0x18

#define SENSORS_LIGHT_BUMP                0x2D
#define SENSORS_LIGHT_BUMP_LEFT           0x2E
#define SENSORS_LIGHT_BUMP_FRONT_LEFT     0x2F
#define SENSORS_LIGHT_BUMP_CENTER_LEFT    0x30
#define SENSORS_LIGHT_BUMP_CENTER_RIGHT   0x31
#define SENSORS_LIGHT_BUMP_FRONT_RIGHT    0x32
#define SENSORS_LIGHT_BUMP_RIGHT          0x33

#define SENSORS_CLIFF_LEFT          0x09
#define SENSORS_CLIFF_FRONT_LEFT    0x0A
#define SENSORS_CLIFF_FRONT_RIGHT   0x0B
#define SENSORS_CLIFF_RIGHT         0x0C

#define SENSORS_CLIFF_LEFT_SIGNAL        0x1C
#define SENSORS_CLIFF_FRONT_LEFT_SIGNAL  0x1D
#define SENSORS_CLIFF_FRONT_RIGHT_SIGNAL 0x1E
#define SENSORS_CLIFF_RIGHT_SIGNAL       0x1F

#define SENSORS_SONG	0x25

#define FULL_INTENSITY 0xff
#define NO_INTENSITY   0x00
#define COLOR_GREEN    0x00
#define COLOR_RED      0xff

#define VELOCITY         200   //mm/s
#define VELOCITY_OFFROAD 100   //mm/s
#define VELOCITY_BOOST   400   //mm/s

#define ITEM_ID_SHELL 0x01
#define IS_FOCUSED_ID 0x02
/******************************************************* Function prototypes */

/*! initializes the rumba
*/
void init_roomba(void);

/*! initializes the infrared
*/
void init_infrared(void);

/*! initializes the cliff signal sensors
*/
void init_cliff_signal(void);

/*! sets a specific led on;
    assignes clean power led clean_power_color and clean_power_intensity
*/
void roomba_set_led_on(uint8_t led_bits, uint8_t clean_power_color, uint8_t clean_power_intensity);

/*! inits sensor from type roomba_sensor_type_t with value 0
*/
void roomba_init_sensor(roomba_sensor_t *sensor, const roomba_sensor_type_t* type);

/*! requests sensor from type roomba_sensor_type_t 
*/
void roomba_request_sensor(roomba_sensor_t *sensor);

/*! reads the value of the sensor
*/
void roomba_read_sensor(roomba_sensor_t *sensor, bool_t accumulate_value);

/*! updates sensor;
    calls roomba_request_sensor and roomba_read_sensor)
*/
int32_t roomba_update_sensor(roomba_sensor_t *sensor, bool_t accumulate_value);

/*! shows a specific sensor value;
    input value length is length of resultstring (for example for string "113" is length = 3)
*/
void roomba_set_letters_string(const char* str, int8_t length);

/*! sets roomba in motion, passing the velocity and radius value
*/
void roomba_drive(int16_t velocity, int16_t radius);

/*! stops roomba (calling function roomba_drive with the 0-values for velocity and radius)
*/
void roomba_stop();

/*!
 * reads the buttonpackage from the roomba an returns the value
 * /
uint8_t read_button();

/*! function converts the resulting number in the decimal string (char array)
    and saves it into result;
	return value is length of resultstring (for example for string "113" is length = 3)
*/
int8_t itos(int number, char* result);

/*! detect whether the roomba drives over an item or not
*/
bool_t roomba_check_for_item(int32_t cliff_left_signal_val, int32_t cliff_right_signal_val);

/*! detect whether the roomba drives over an finish mark or not
*/
bool_t roomba_check_for_finish_mark(int32_t cliff_front_left_signal_val, int32_t cliff_front_right_signal_val);

/*! tells the roomba what to do if an item has been detected
*/
void roomba_pick_up_item(void);

/*! generates a random item
*/
enum item_t roomba_generate_rand_item(void);

/*! uses the current item
*/
void roomba_use_item(void);

/*! ends the current effect of the last item used
*/
void roomba_item_effect_ends(void);

/*! handles the effect when the roomba got hit by an item from his opponent
 */
void roomba_got_hit_by_item(uint8_t item_id);

/*! ends the got hit by item effect
 */
void roomba_got_hit_effect_ends(void);

/*!starts timer
*/
void roomba_start_timer(void);

/*!checks  for the specific values of the timers
*/
uint32_t roomba_check_timer_array(void);

/*! returns a random value
 */ 
uint16_t my_rand(void);

/*! shows the received item on the display
*/
void roomba_show_item(void);

/*! indicates if the roomba is in aim range of another roomba 
 */
void roomba_show_is_focused();

/*! Indicates whether the roomba is currently using any item
 */
uint8_t check_for_active_item();

/************************************************************** Global const */

/*! initialized sensors with values:
    - package_id,
    - bytes_number (one ore two),
    - is_signed (true, if the value can be negative, false, if cant)
*/
extern const roomba_sensor_type_t cliff_left_signal_sensor;
extern const roomba_sensor_type_t cliff_front_left_signal_sensor;
extern const roomba_sensor_type_t cliff_front_right_signal_sensor;
extern const roomba_sensor_type_t cliff_right_signal_sensor;

extern const roomba_sensor_type_t infrared_omni_sensor;
extern const roomba_sensor_type_t infrared_right_sensor;
extern const roomba_sensor_type_t infrared_left_sensor;


extern const int16_t radius;
extern const int16_t radius_counter_clockwise;

/********************************************************** Global variables */
extern roomba_sensor_t cliff_left_signal;
extern roomba_sensor_t cliff_front_left_signal;
extern roomba_sensor_t cliff_right_signal;
extern roomba_sensor_t cliff_front_right_signal;

extern roomba_sensor_t infrared_omni;
extern roomba_sensor_t infrared_right;
extern roomba_sensor_t infrared_left;

extern volatile int16_t velocity;

/*!char array for text output
*/
extern char str[5];

extern volatile enum item_t current_item;
extern volatile enum item_t last_item_used;
extern volatile enum state_t current_state;
extern volatile enum state_t last_state;
/*! array for timer usage
*   0: boost item
    1: fire shell
    2: got hit by shell
    3: random item id
*/
extern volatile int16_t timer_array[4];

/*! array for activate/deactivate timer usage
*   0: boost item
    1: fire shell
    2: got hit by shell
    3: random item id
*/
extern volatile int16_t timer_active_array[4];

/*! The seed for my_rand()
 */
extern volatile uint16_t random_seed;

extern volatile bool_t should_refresh_state;

extern char speed_str[4];
extern char shell_str[4];

#ifdef __cplusplus
}
#endif

#endif /* !_ROOMBA_H */

/******************************************************************** Macros */

/**
* Checks if a button on the simulator is pressed or not
**/
#define SIM_BUTTON_PRESSED(button) (read_button() == button)
/**
 * Waits for Button input
**/
#define SIM_BUTTON_WAIT(button) while(!SIM_BUTTON_PRESSED(button))my_msleep(50);
