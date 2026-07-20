/*
Author: Dilon Hewamanna
Date: 31/02/26

basic firmware code

TODO:
  Double check that H7 has 150 additional peripheral handlers
  Learn how to properly define memory sections in linker file
*/
#include "stm32h743xx.h"
#include "stm32h7xx.h"
#include <stdint.h>
#include <stdbool.h>

#define BIT(x) (1UL << (x))




///////              RESET HANDLER (MAKE SURE TO CHECK JUST THIS ON BOARD)
// Startup code
__attribute__((naked, noreturn)) void _reset(void) {
  // memset .bss to zero, and copy .data section to RAM region
  extern long _sbss, _ebss, _sdata, _edata, _sidata;
  for (long *dst = &_sbss; dst < &_ebss; dst++) *dst = 0;
  for (long *dst = &_sdata, *src = &_sidata; dst < &_edata;) *dst++ = *src++;

  main();             // Call main()
  for (;;) (void) 0;  // Infinite loop in the case if main() returns
}

extern void _estack(void);  // Defined in link.ld

// 16 standard and 150 STM32-specific handlers
__attribute__((section(".vectors"))) void (*const tab[16 + 150])(void) = {
  _estack, _reset, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, SysTick_Handler
};


static inline void spin(volatile uint32_t count) {
  while (count--) (void) 0;
}


static inline void systick_init(uint32_t ticks) {
  if ((ticks - 1) > 0xffffff) return;  // systick timer is 24 bit
  SysTick->LOAD = ticks - 1;
  SysTick->VAL = 0;
  SysTick->CTRL = BIT(0) | BIT(1) | BIT(2);
  RCC->APB4ENR = BIT(1);

}


static volatile uint32_t s_ticks; // volatile is important!!
void SysTick_Handler(void) {
  s_ticks++;
}


bool timer_expired(uint32_t *t, uint32_t prd, uint32_t now) {
  if (now + prd < *t) *t = 0;                    // Time wrapped? Reset timer
  if (*t == 0) *t = now + prd;                   // First poll? Set expiration
  if (*t > now) return false;                    // Not expired yet, return
  *t = (now - *t) > prd ? now + prd : *t + prd;  // Next expiration time
  return true;                                   // Expired, return true
}





int main(void){

  uint16_t led_red = 0;
  uint16_t led_blue = 1;

  RCC->AHB4ENR |= BIT(2);    // enable GPIOC
  GPIOC->MODER &= (3U << (led_blue * 2));   // clear existing setting
  GPIOC->MODER |= (1U << (led_blue * 2));   // set output
  GPIOC->BSRR = (1U << led_blue) << 0;   // set high    

  systick_init(480000000 / 1000);

  uint32_t timer, period = 500;          // Declare timer and 500ms period
  for (;;) {
    if (timer_expired(&timer, period, s_ticks)) {
      static bool on;       // This block is executed
      GPIOC->BSRR = (1U << led_blue) << 16;  // Every `period` milliseconds
      on = !on;             // Toggle LED state
    }
}

