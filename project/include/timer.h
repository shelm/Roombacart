#ifndef DRIVER_TIMER_H
#define DRIVER_TIMER_H 1

#include <board.h>
#include <stdint.h>
#include <irq.h>
#include <pio.h>
#include <asm/io.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Defines for Timer controlling */
#define TIMER_STATUS		0
#define TIMER_CONTROL		4
#define TIMER_PERIODL		8
#define TIMER_PERIODH		12

#define NIOS2_TIMER_STATUS_TO	(1<<0)
#define NIOS2_TIMER_STATUS_RUN	(1<<1)

#define TIMER_CONTROL_ITO	(1<<0)
#define TIMER_CONTROL_CONT	(1<<1)
#define TIMER_CONTROL_START	(1<<2)
#define TIMER_CONTROL_STOP	(1<<3)


status_t tt_periodic(uint32_t);
status_t tt_single(uint32_t);
void tt_stop(void);
/* Call tt_reset in callback handler to reset the timer */
void tt_reset(void);


#ifdef __cplusplus
}
#endif

#endif /* DRIVER_TIMER_H */
