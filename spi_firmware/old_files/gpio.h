#ifndef GPIO_H
#define GPIO_H

#include "stm32h743xx.h"
#include <stdint.h>
#include <stdbool.h>
#include "hal.h"

static inline void gpio_set_mode(GPIO_TypeDef *gpio, uint8_t pin, uint8_t mode);
static inline void gpio_set_afr(GPIO_TypeDef *gpio, uint8_t pin, uint8_t afr);
static inline void gpio_write(GPIO_TypeDef *gpio, uint8_t pin, bool val);


#endif