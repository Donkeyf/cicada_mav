#include "stm32h743xx.h"
#include <stddef.h>
#include <stdint.h>
#include "hal.h"

void SPI_init();

static inline int spi_read_ready(SPI_TypeDef *spi);
static inline uint8_t spi_read_byte(SPI_TypeDef *spi);
static inline void spi_write_byte(SPI_TypeDef *spi, uint8_t byte);
static inline uint8_t spi_transfer(SPI_TypeDef *spi, uint8_t byte);
static inline uint16_t read_PROM(SPI_TypeDef *spi, uint8_t cmd);

void MS5611_init(SPI_TypeDef *spi, uint16_t *prom);
void read_MS5611_adc(SPI_TypeDef *spi, uint32_t cs, uint16_t *C);