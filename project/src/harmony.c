/****************************************************************** Includes */
#include <harmony.h>
#include <roomba.h>


/******************************************************************* Defines */


/******************************************************* Function prototypes */


/************************************************************** Global const */


/********************************************************** Global variables */


/*************************************************************** Local const */


/*********************************************************** Local variables */


/******************************************************************** Macros */


/********************************************************** Global functions */

//void remote_drive_roomba(void){
	
	//uint8_t infraredSignal_omni = 0;
	//uint8_t infraredSignal_left = 0;
	//uint8_t infraredSignal_right = 0;
	
	//while(1){				
		
		//infraredSignal_omni = roomba_get_packet(ROOMBA_PID_INFRARED_OMNI);
		//infraredSignal_left = roomba_get_packet(ROOMBA_PID_INFRARED_OMNI);
		//infraredSignal_right = roomba_get_packet(ROOMBA_PID_INFRARED_OMNI);
		
		///** drive straight forward*/
		//if(infraredSignal_omni == HARMONY_SIGNAL_BACK || infraredSignal_left == HARMONY_SIGNAL_BACK || infraredSignal_right == HARMONY_SIGNAL_BACK){
			
			//drive(ROOMBA_VELOCITY_STANDART, ROOMBA_DRIVE_STRAIGHT);
			//}
			
		///** drive straight backward*/
		//else if(infraredSignal_omni == HARMONY_SIGNAL_DOCK || infraredSignal_left == HARMONY_SIGNAL_BACK || infraredSignal_right == HARMONY_SIGNAL_BACK){
			
			//drive(-ROOMBA_VELOCITY_STANDART, ROOMBA_DRIVE_STRAIGHT);
			//}
		
		///** drive forward left */
		//else if(infraredSignal_omni == HARMONY_SIGNAL_FORWARD || infraredSignal_left == HARMONY_SIGNAL_BACK || infraredSignal_right == HARMONY_SIGNAL_BACK){
			
			//drive(ROOMBA_VELOCITY_STANDART, 300);
			//}
			
		///** drive forward right */
		//else if(infraredSignal_omni == HARMONY_SIGNAL_LARGE || infraredSignal_left == HARMONY_SIGNAL_BACK || infraredSignal_right == HARMONY_SIGNAL_BACK){
			
			//drive(ROOMBA_VELOCITY_STANDART, -300);
			//}
			
		///** use powerup */
		//else if(infraredSignal_omni == HARMONY_SIGNAL_LEFT || infraredSignal_left == HARMONY_SIGNAL_BACK || infraredSignal_right == HARMONY_SIGNAL_BACK){
			
			//play_song();			
			//}
			
		///** stop if no signal is recieved at all */
		//else if (infraredSignal_omni == 0 && infraredSignal_left == 0 && infraredSignal_right == 0){
			//stop();
			//}
			
		//my_msleep(ROOMBA_SENSOR_UPDATE_RATE);
		
		//}
	//}


/*********************************************************** Local functions */
