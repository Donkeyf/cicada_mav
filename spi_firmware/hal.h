#ifndef HAL_H
#define HAL_H

#include "stm32h743xx.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define BIT(x) (1UL << (x))

static inline void spin(volatile uint32_t count) {
  while (count--) (void) 0;
}

////////////////////    GPIO FUNCTIONS   ////////////////////
static inline void gpio_set_mode(GPIO_TypeDef *gpio, uint8_t pin, uint8_t mode) {
    gpio->MODER &= ~(3U << (pin * 2));        // Clear existing setting
    gpio->MODER |= (mode & 3) << (pin * 2);   // Set new mode
}

static inline void gpio_set_afr(GPIO_TypeDef *gpio, uint8_t pin, uint8_t afr){
    if(pin <= 7){
        gpio->AFR[0] &= ~(15UL << ((pin) * 4));
        gpio->AFR[0] |= (afr << ((pin) * 4));
    } else{
        gpio->AFR[1] &= ~(15UL << ((pin - 8) * 4));
        gpio->AFR[1] |= (afr << ((pin - 8) * 4));  
    }
}

static inline void gpio_write(GPIO_TypeDef *gpio, uint8_t pin, bool val){
    gpio->BSRR = (1U << pin) << (val ? 0 : 16);
}



////////////////////    UART FUNCTIONS   ////////////////////
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



////////////////////    SPI FUNCTIONS   ////////////////////
static inline int spi_read_ready(SPI_TypeDef *spi){
    return spi->SR & BIT(0);    //  is spi ready to read
}

static inline uint8_t spi_read_byte(SPI_TypeDef *spi){
    return (uint8_t) (spi->RXDR & 255);
}

static inline void spi_write_byte(SPI_TypeDef *spi, uint8_t byte){
    spi->CR1 |= BIT(9);
    while (!(spi->SR & BIT(1)));
    spi->TXDR = byte;
    //(void)spi->RXDR; // discard received byte
}

// static inline uint8_t spi_transfer(SPI_TypeDef *spi, uint8_t byte){
//     spi_write_byte(spi, byte);
//     uart_write_buf(UART5, "A\r\n", 3);
//     while(!(spi_read_ready(spi)));
//     uart_write_buf(UART5, "B\r\n", 3);
//     return spi_read_byte(spi);
// }

static inline uint8_t spi_transfer(SPI_TypeDef *spi, uint8_t byte, uint8_t cs){
    while (!(spi->SR & BIT(1))); // TXP
    *((volatile uint8_t *)&spi->TXDR) = byte;
    spi->CR1 |= BIT(9); // start

    while (!(spi->SR & BIT(0))); // RXP
    uint8_t read = *((volatile uint8_t *)&spi->RXDR);

    while (!(spi->SR & BIT(3))); // wait for EOT to confirm transaction truly finished
    spi->IFCR = BIT(4) | BIT(3); // clear EOT (bit9) and TXTF (bit3)

    return read;
}

uint16_t read_PROM(SPI_TypeDef *spi, uint8_t cmd, uint8_t cs);
void SPI_init();
void MS5611_init(SPI_TypeDef *spi, uint16_t *prom, uint8_t cs);
int32_t read_MS5611_adc(SPI_TypeDef *spi, uint8_t cs, uint16_t *C);



#endif