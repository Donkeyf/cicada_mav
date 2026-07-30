#ifndef UART_H
#define UART_H

#include "stm32h743xx.h"
#include <stddef.h>
#include "hal.h"

static inline int uart_read_ready(USART_TypeDef *uart);
static inline uint8_t uart_read_byte(USART_TypeDef *uart);
static inline void uart_write_byte(USART_TypeDef *uart, uint8_t byte);
static inline void uart_write_buf(USART_TypeDef *uart, char *buf, size_t len);

#endif