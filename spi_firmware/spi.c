#include "stm32h743xx.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "hal.h"


void SPI_init(){
    printf("yo\r\n");
    RCC->APB2ENR |= BIT(12);    // enable SPI1
    SPI1->CFG1 &= ~(7UL << 28); // reset baud rate
    SPI1->CFG1 |= (4UL << 28);   // baud rate SPI master clock/32  

    SPI1->CFG1 &= ~(31UL); // reset DSIZE
    SPI1->CFG1 |= 7UL;   // DSIZE set to 8 bits


    SPI1->CFG2 &= ~(7UL << 19);   // motorola SP
    SPI1->CFG2 &= ~(3UL << 17);   // full-duplex
    SPI1->CFG2 &= ~(BIT(24) | BIT(25) | BIT(23)); // set CPOL, CPHA, MSB, SSM
    SPI1->CFG2 |=  BIT(26); // set SSM
    SPI1->CR1 |= BIT(12);  // SSI high
    SPI1->CFG2 |= BIT(22);  // set master
    SPI1->CR2 &= ~SPI_CR2_TSIZE_Msk;
    SPI1->CR2 |= BIT(0);
    SPI1->CR1 |= BIT(0);
    
}


uint16_t read_PROM(SPI_TypeDef *spi, uint8_t cmd, uint8_t cs){
    // read calibration values 
    gpio_write(GPIOB, cs, false);
    spi_transfer(spi, cmd, cs);  

    uint16_t prom;  // PROM data is unsigned int 16;
    prom = (uint16_t)spi_transfer(spi, 0x00, cs) << 8;
    prom |= (uint16_t)spi_transfer(spi, 0x00, cs);
    gpio_write(GPIOB, cs, true);
    return prom;
}

void MS5611_init(SPI_TypeDef *spi, uint16_t *prom, uint8_t cs){
    gpio_write(GPIOB, cs, false);
    spi_transfer(spi, 0x1E, cs); 
    gpio_write(GPIOB, cs, true);

    spin(10000);

    // read calibration data
    prom[1] = read_PROM(spi, 0xA2, cs);
    printf("prom1=%04lx\n", prom[1]);
    prom[2]= read_PROM(spi, 0xA4, cs);
    printf("prom2=%04lx\n", prom[2]);
    prom[3]= read_PROM(spi, 0xA6, cs);
    printf("prom3=%04lx\n", prom[3]);
    prom[4] = read_PROM(spi, 0xA8, cs);
    printf("prom4=%04lx\n", prom[4]);
    prom[5] = read_PROM(spi, 0xAA, cs);
    printf("prom5=%04lx\n", prom[5]);
    prom[6] = read_PROM(spi, 0xAC, cs);
    printf("prom6=%04lx\n", prom[6]);

}

// make sure to change cs and data type
int32_t read_MS5611_adc(SPI_TypeDef *spi, uint8_t cs, uint16_t *C){
    gpio_write(GPIOB, cs, false);
    spi_write_byte(spi, 0x48);   // convert D1 OSR=4096
    gpio_write(GPIOB, cs, true);

    // read pressure adc
    gpio_write(GPIOB, cs, false);
    uint32_t D1 = 0;
    spi_write_byte(spi, 0x00);
    gpio_write(GPIOB, cs, true);
    D1 = spi_transfer(spi, 0x00, cs) << 16;
    D1 |= spi_transfer(spi, 0x00, cs) << 8;
    D1 |= spi_transfer(spi, 0x00, cs);
    

    gpio_write(GPIOB, cs, false);
    spi_write_byte(spi, 0x58);   // convert D2 OSR=4096
    
    uint32_t D2 = 0;
    spi_transfer(spi, 0x00, cs);
    D2 = spi_transfer(spi, 0x00, cs) << 16;
    D2 |= spi_transfer(spi, 0x00, cs) << 8;
    D2 |= spi_transfer(spi, 0x00, cs);
    gpio_write(GPIOB, cs, true);
   

    int32_t dT = (int32_t)D2 - ((int32_t)C[5] << 8);
    int32_t T = (int32_t)((int64_t)2000 + ((int64_t)dT * C[6] >> 23));
    //int64_t off = ((int64_t)C[2] << 16) + (((int64_t)C[4] * dT) >> 7);
    //int64_t sens = ((int64_t)C[1] << 15) + (((int64_t)C[3] * dT) >> 8);
    //((((int64_t)D1 * sens) >> 21) - off) >> 15;



    // TODO second order temperature compensation
    return T;
}