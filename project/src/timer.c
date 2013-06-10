/**
 * $Id: timer.c 639 2012-12-06 14:36:15Z klugeflo $
 */

/******************************************************************************

File: timer.c

Project: RTOS Training OSEK Implementation

Description: Handling of tick timer

Author: Florian Kluge <kluge@informatik.uni-augsburg.de>
        Universität Augsburg

Created: 30.11.2010

Last changed: 06.06.2013

*******************************************************************************

Modification history:
---------------------
06.06.2013 (MG) Edited for ES training

*/


/****************************************************************** Includes */

#include "timer.h"

/******************************************************************* Defines */


/******************************************************* Function prototypes */

void _tt_disable(void);

/************************************************************** Global const */


/********************************************************** Global variables */


/*************************************************************** Local const */


/*********************************************************** Local variables */


/******************************************************************** Macros */


/********************************************************** Global functions */


status_t tt_periodic(uint32_t period) {
    status_t err = 0;

    if (period == 0) {
        _tt_disable();
    } else {
    	IOWR32(A_TIMER, TIMER_CONTROL, TIMER_CONTROL_ITO | TIMER_CONTROL_CONT | TIMER_CONTROL_STOP);
    	IOWR32(A_TIMER, TIMER_STATUS, 0);
    	
	IOWR32(A_TIMER, TIMER_PERIODL, period & 0xffff);
	IOWR32(A_TIMER, TIMER_PERIODH, period >> 16);

	IOWR32(A_TIMER, TIMER_CONTROL, TIMER_CONTROL_ITO | TIMER_CONTROL_CONT | TIMER_CONTROL_START);
    }

    return err;
}

status_t tt_single(uint32_t period) {
    status_t err = 0;

    if (period == 0) {
        _tt_disable();
    } else {
	IOWR32(A_TIMER, TIMER_PERIODL, period & 0xffff);
	IOWR32(A_TIMER, TIMER_PERIODH, period >> 16);
	IOWR32(A_TIMER, TIMER_CONTROL, TIMER_CONTROL_ITO | TIMER_CONTROL_START);
    }

    return err;
}


void tt_stop() {
	_tt_disable();
}

void tt_reset() {

	IOWR32(A_TIMER, TIMER_STATUS, 0);
}


/*********************************************************** Local functions */

void _tt_disable(void) {
	IOWR32(A_TIMER, TIMER_CONTROL, TIMER_CONTROL_STOP);
}
