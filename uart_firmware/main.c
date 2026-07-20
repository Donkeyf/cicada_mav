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

static inline int uart_read_ready(USART_TypeDef *uart) {
  return uart->ISR & BIT(5); //if RXNE bit is set, data is ready
}

static inline uint8_t uart_read_byte(USART_TypeDef *uart) {
  return (uint8_t) (uart->RDR & 255);
}

static inline void uart_write_byte(USART_TypeDef *uart, uint8_t byte) {
  uart->TDR = byte;
  while ((uart->ISR & BIT(7)) == 0) spin(1);
}

static inline void uart_write_buf(USART_TypeDef *uart, char *buf, size_t len) {
  while (len-- > 0) uart_write_byte(uart, *(uint8_t *) buf++);
}


int main(void){

  uint16_t led_red = 0;
  uint16_t led_blue = 1;
  RCC->AHB4ENR |= BIT(2);    // enable GPIOC
  GPIOC->MODER &= (3U << (led_blue * 2));   // clear existing setting
  GPIOC->MODER |= (1U << (led_blue * 2));   // set output
  GPIOC->BSRR = (1U << led_blue) << 0;   // set high    

  uint16_t debug_tx = 12; // UART5 on PB12 and PB13
  uint16_t debug_rx = 13; 
  RCC->APB1LENR != BIT(20);
  GPIOB->AFRH &= ~(15UL << ((12 - 8) * 4)); // zero pin 12 (bit 16)
  GPIOB->AFRH &= ~(15UL << ((13 - 8) * 4)); // zero pin 13 (bit 20)
  GPIOB->AFRH |= (14UL << ((12 - 8) * 4));  // set alternate function AF14
  GPIOB->AFRH |= (14UL << ((13 - 8) * 4));
  UART5->CR1 = 0;
  UART5->BRR = 480000000 / 115200;  // set baudrate (CHANGE IF NEEDED) freq/baud
   UART5->CR1 |= BIT(0) | BIT(2) | BIT(3);


  systick_init(480000000 / 1000);

  uint32_t timer, period = 500;          // Declare timer and 500ms period
  static bool on; 
  for (;;) {
    if (timer_expired(&timer, period, s_ticks)) {
            // This block is executed
      GPIOC->BSRR = (1U << led_blue) << (on ? 0 : 16);  // Every `period` milliseconds
      on = !on;             // Toggle LED state
    }
}

