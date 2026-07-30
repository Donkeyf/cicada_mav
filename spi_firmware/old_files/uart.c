#include "stm32h743xx.h"
#include <stddef.h>
#include <stdint.h>
#include "uart.h"
#include "hal.h"

// TODO DO RCC->D2CCIP2R to chosen clock source

static inline void uart_init(USART_TypeDef *uart, uint32_t baud) {
  uart->CR1 = 0;  // clear control register
  uart->BRR = baud; // set baudrate
  uart->CR1 |= BIT(2) | BIT(3); // set txne, rxne
  uart->CR1 |= BIT(0);  // enablue uart
}

static inline int uart_read_ready(USART_TypeDef *uart) {
  return uart->ISR & BIT(5); //if RXNE bit is set, data is ready
}

static inline uint8_t uart_read_byte(USART_TypeDef *uart) {
  return (uint8_t) (uart->RDR & 255);
}

static inline void uart_write_byte(USART_TypeDef *uart, uint8_t byte) {
  while ((uart->ISR & BIT(7)) == 0)
    spin(1);

  uart->TDR = byte;
}

static inline void uart_write_buf(USART_TypeDef *uart, char *buf, size_t len) {
  while (len-- > 0) uart_write_byte(uart, *(uint8_t *) buf++);
}