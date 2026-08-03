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
#include "hal.h"

#define BARO_CSB 0
#define GYRO_CSB 1
#define ACCEL_CSB 2
#define UART_RX 12
#define UART_TX 13
#define BLUE 1

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
  SCB->VTOR = (uint32_t)tab;

  // RCC->APB1LENR |= BIT(20);
  // RCC->AHB4ENR |= BIT(1);    // enable GPIOC
  // gpio_set_mode(GPIOB, UART_TX, 2);
  // gpio_set_mode(GPIOB, UART_RX, 2);
  // gpio_set_afr(GPIOB, UART_TX, 14);
  // gpio_set_afr(GPIOB, UART_RX, 14);
  // RCC->D2CCIP2R &= ~(7UL << 0); // set pclk1 as timing uart
  // uart_init(UART5, 120000000 / 115200);

  RCC->AHB4ENR |= BIT(2);
  gpio_set_mode(GPIOC, BLUE, 1UL);
  
  cpu_max_init();

  //gpio_write(GPIOC, 1, true);
  

  // systick_init(480000000 / 1000);
  // uint32_t timer = 0, period = 1000; 
  // bool on = false;
  // for(;;) {
  //   if (timer_expired(&timer, period, s_ticks)){
  //     on = !on;
  //     gpio_write(GPIOC, BLUE, on);
  //   }
  // }
  // return 0;
}

