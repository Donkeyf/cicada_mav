#include "stm32h743xx.h"

#define BIT(x) (1UL << (x))

static inline void clock_init() {
    RCC->CR |= BIT(16); // turn HSE on
    // wait till HSE ready
    while(!(RCC->CR & BIT(17)));

    PWR->D3CR |= 3UL << 14; // voltage scaling 3
    while(!(PWR->D3CR & BIT(13)));    

}