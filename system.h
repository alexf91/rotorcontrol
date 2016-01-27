#ifndef _SYSTEM_H_
#define _SYSTEM_H_

#include <stdint.h>
#include "config.h"

#define OCR_VALUE ((F_CPU / (64*F_SYSTICK)) - 1)

/**
 * @brief Reset the controller
 */
extern void system_reset(void);

/**
 * @brief Initialize the systick timer
 */
extern void systick_init(void);

/**
 * @brief return the 32 bit systick counter value
 */
extern uint32_t systick_counter(void);

#endif /* _SYSTEM_H_ */
