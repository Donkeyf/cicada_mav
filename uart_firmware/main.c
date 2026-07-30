/*
Author: Dilon Hewamanna
Date: 31/02/26

basic firmware code

TODO:
  Double check that H7 has 150 additional peripheral handlers
  Learn how to properly define memory sections in linker file
*/
#include "stm32h743xx.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define BIT(x) (1UL << (x))


static volatile uint32_t s_ticks; // volatile is important!!
void SysTick_Handler(void) {
  s_ticks++;
}


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

}


bool timer_expired(uint32_t *t, uint32_t prd, uint32_t now) {
  if (now + prd < *t) *t = 0;                    // Time wrapped? Reset timer
  if (*t == 0) *t = now + prd;                   // First poll? Set expiration
  if (*t > now) return false;                    // Not expired yet, return
  *t = (now - *t) > prd ? now + prd : *t + prd;  // Next expiration time
  return true;                                   // Expired, return true
}




int main(void){

  uint16_t debug_tx = 12; // UART5 on PB12 and PB13
  uint16_t debug_rx = 13; 
  RCC->APB1LENR |= BIT(20);
  RCC->AHB4ENR |= BIT(1);

  GPIOB->MODER &= ~(3UL << (12 * 2)); // set GPIOB mode to alt function
  GPIOB->MODER |=  (2UL << (12 * 2));
  GPIOB->MODER &= ~(3UL << (13 * 2));
  GPIOB->MODER |=  (2UL << (13 * 2));

  GPIOB->AFR[1] &= ~(15UL << ((12 - 8) * 4)); // zero pin 12 (bit 16)
  GPIOB->AFR[1] &= ~(15UL << ((13 - 8) * 4)); // zero pin 13 (bit 20)
  GPIOB->AFR[1] |= (14UL << ((12 - 8) * 4));  // set alternate function AF14
  GPIOB->AFR[1] |= (14UL << ((13 - 8) * 4));
  RCC->D2CCIP2R &= ~(7UL << 0); // clear bits 2:0
  RCC->D2CCIP2R |=  (3UL << 0); // set HSI to time UART5

  UART5->CR1 = 0;
  UART5->BRR = 64000000 / 115200;  // set baudrate (CHANGE IF NEEDED) freq/baud
  UART5->CR1 |= BIT(2) | BIT(3);
  UART5->CR1 |= BIT(0);

  systick_init(64000000 / 1000);
  uint32_t timer = 0, period = 500; 
  for(;;) {
    if (timer_expired(&timer, period, s_ticks)){
      uart_write_buf(UART5, "hi\r\n", 4);
    }
  }
  return 0;
}

