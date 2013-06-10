/**
 * $Id: irq.c 629 2012-10-12 12:04:18Z klugeflo $
 */

/******************************************************************************

File: irq.c

Project: RTOS Training OSEK implementation

Description: IRQ handling

Author: Florian Kluge <kluge@informatik.uni-augsburg.de>
        Universität Augsburg

Created: 17.09.2010

Last changed: 06.06.2013

*******************************************************************************

Modification history:
---------------------
17.09.2010 (FAK) Created from old RTOS training project
06.06.2013 (MG) Edited for ES training

*/


/****************************************************************** Includes */

#include "irq.h"

/******************************************************************* Defines */

#ifndef STATIC
#define STATIC static
#endif

#ifndef INLINE
#define INLINE inline
#endif

/******************************************************* Function prototypes */

STATIC int32_t pic_irq(void);
STATIC INLINE bool_t is_irq_enabled(uint32_t irq);


/************************************************************** Global const */


/********************************************************** Global variables */

//! state of IEE/TEE in SR
//STATIC
bool_t irq_state = true;

/*************************************************************** Local const */


/*********************************************************** Local variables */

STATIC irq_handler_t irq_list[N_IRQS];


/******************************************************************** Macros */


STATIC INLINE bool_t is_irq_enabled(uint32_t irq) {
    return (__rdctl_ienable() & (1<<irq)) != 0;
}

/********************************************************** Global functions */


void init_irq(void) {
    uint32_t i;
    for (i=0; i<N_IRQS; ++i) {
        irq_list[i] = NULL;
    }
}


void enable_all_interrupts() {
    __wrctl_ienable(0xffffffff);
    __wrctl_status(__rdctl_status() | 0x1); // Set PIE bit to 0x1 in status register
    irq_state = true;
}


void disable_all_interrupts() {
    __wrctl_ienable(0x0);
    __wrctl_status(__rdctl_status() & 0xfffe); // Set PIE bit to 0x0 in status register
    irq_state = false;
}


status_t irq_enable(uint32_t irq) {
    status_t err;
    if (irq >= N_IRQS)
        err = E_OS_LIMIT;
    else if (irq_list[irq] == NULL) {
        err = E_OS_NOFUNC;
    }
    else {
        __wrctl_ienable(__rdctl_ienable() | (1<<irq));
        err = E_OK;
    __wrctl_status(__rdctl_status() | 0x1); // Set PIE bit to 0x1 in status register
    }
    return err;
}


status_t irq_disable(uint32_t irq) {
    status_t err;
    if (irq >= N_IRQS)
        err = E_OS_LIMIT;
    else {
        __wrctl_ienable(__rdctl_ienable() & ~(1<<irq));
        err = E_OK;
    }
    return err;
}


status_t irq_request(uint32_t irq, irq_handler_t handler) {
    status_t err;
    if (irq >= N_IRQS)
        err = E_OS_LIMIT;
    else if (handler == NULL)
        err = E_OS_NOFUNC;
    else {
        irq_list[irq] = handler;
        err = E_OK;
    }
    return err;
}


status_t irq_free(uint32_t irq) {
    status_t err;
    if (irq >= N_IRQS)
        err = E_OS_LIMIT;
    else if ((irq>1) && is_irq_enabled(irq))
        err = E_OS_NOFUNC;
    else {
        irq_list[irq] = NULL;
        err = E_OK;
    }
    return err;
}


bool_t irq_enabled(uint32_t irq) {
    return is_irq_enabled(irq);
}



/******************************************************* Handler for XCP vec */

void do_irq() {

    int32_t irq;
        
    while ((irq = pic_irq()) >= 0) {
        if (irq_list[irq] != NULL) {
            irq_list[irq](irq);
        }
        else {
            /* disable pending but unhandled interrupts
            * to avoid endless loops */
            __wrctl_ienable(__rdctl_ienable() & ~(1 << irq));
        }
        __wrctl_ipending(__rdctl_ipending() & ~(1 << irq));
    }
}


/*********************************************************** Local functions */


//! plucked from linux-2.6.24
int32_t pic_irq(void) {
    uint32_t irq;
    uint32_t mask;

    /* interrupt 0 is the JTAG UART interrupt */
    uint32_t pend = __rdctl_ipending() & 0xfffffffe;

    
    if (pend & 0x0000ffff) {
        if (pend & 0x000000ff) {
            if (pend & 0x0000000f) {
                mask = 0x00000001;
                irq = 0;
            } else {
                mask = 0x00000010;
                irq = 4;
            }
        } else {
            if (pend & 0x00000f00) {
                mask = 0x00000100;
                irq = 8;
            } else {
                mask = 0x00001000;
                irq = 12;
            }
        }
    } else if(pend & 0xffff0000) {
        if (pend & 0x00ff0000) {
            if (pend & 0x000f0000) {
                mask = 0x00010000;
                irq = 16;
            } else {
                mask = 0x00100000;
                irq = 20;
            }
        } else {
            if (pend & 0x0f000000) {
                mask = 0x01000000;
                irq = 24;
            } else {
                mask = 0x10000000;
                irq = 28;
            }
        }
    } else {
        return -1;
    }
    
    while (! (mask & pend)) {
        mask <<=1;
        irq++;
    }
    return irq;
}
