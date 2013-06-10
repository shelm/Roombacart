/**
 * $Id: irq.h 488 2011-10-12 11:59:26Z klugeflo $
 */

/******************************************************************************

File: irq.h

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

#ifndef IRQ_H_
#define IRQ_H_ 1


/****************************************************************** Includes */

#include <stdint.h>
#include <board.h>
#include <asm/nios2intrinsics.h>
#include <tools.h>
#include <stddef.h>
#include <pio.h>

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************* Defines */


#define E_OK                        0
#define E_OS_ACCESS                 1
#define E_OS_CALLEVEL               2
#define E_OS_ID                     3
#define E_OS_LIMIT                  4
#define E_OS_NOFUNC                 5
#define E_OS_RESOURCE               6
#define E_OS_STATE                  7
#define E_OS_VALUE                  8


/****************************************************************** Typedefs */

typedef uint8_t status_t;
typedef uint32_t (*irq_handler_t)(uint32_t);

/************************************************************** Global const */


/********************************************************** Global variables */

extern bool_t irq_state;

/************************************************ Global function prototypes */

/* Enables all interrupts (also sets PIE bit in status register) */
void enable_all_interrupts();

/* Disables all interrupts (also unsets PIE bit in status register) */
void disable_all_interrupts();


// some additional functions for interrupt control
void init_irq(void);

status_t irq_enable(uint32_t irq);

status_t irq_disable(uint32_t irq);

status_t irq_request(uint32_t irq, irq_handler_t handler);

status_t irq_free(uint32_t irq);

bool_t irq_enabled(uint32_t irq);


void do_irq();

/*************************************************** Global inline functions */


/******************************************************************** Macros */

#define _do_enable_all_interrupts(sf)               \
    _sc_enable_all_interrupts(sf)

#define _do_disable_all_interrupts(sf)              \
    _sc_disable_all_interrupts(sf)

#ifdef __cplusplus
}
#endif

#endif /* IRQ_H_ */
