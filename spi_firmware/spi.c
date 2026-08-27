#include "stm32h743xx.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "hal.h"


void SPI_init(){
    RCC->APB2ENR |= BIT(12);    // enable SPI1
    SPI1->CFG1 &= ~(7UL << 28); // reset baud rate
    SPI1->CFG1 |= (4UL << 28);   // baud rate SPI master clock/32  
    SPI1->CFG1 |= BIT(15);
    SPI1->CFG1 |= BIT(14);  // enable DMA

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
    spi_transfer(spi, cmd);  

    uint16_t prom;  // PROM data is unsigned int 16;
    prom = (uint16_t)spi_transfer(spi, 0x00) << 8;
    prom |= (uint16_t)spi_transfer(spi, 0x00);
    gpio_write(GPIOB, cs, true);
    return prom;
}

void MS5611_init(SPI_TypeDef *spi, uint16_t *prom, uint8_t cs){
    gpio_write(GPIOB, cs, false);
    spi_transfer(spi, 0x1E); 
    gpio_write(GPIOB, cs, true);

    spin(100000);

    // read calibration data
    prom[1] = read_PROM(spi, 0xA2, cs);
    prom[2]= read_PROM(spi, 0xA4, cs);
    prom[3]= read_PROM(spi, 0xA6, cs);
    prom[4] = read_PROM(spi, 0xA8, cs);
    prom[5] = read_PROM(spi, 0xAA, cs);
    prom[6] = read_PROM(spi, 0xAC, cs);
}

// make sure to change cs and data type
void read_MS5611_adc(SPI_TypeDef *spi, uint8_t cs, uint16_t *C, int32_t* data){
    gpio_write(GPIOB, cs, false);
    spi_write_byte(spi, 0x48);   // convert D1 OSR=4096
    gpio_write(GPIOB, cs, true);

    // read pressure adc
    gpio_write(GPIOB, cs, false);
    uint32_t D1 = 0;
    spi_write_byte(spi, 0x00);
    D1 = spi_transfer(spi, 0x00) << 16;
    D1 |= spi_transfer(spi, 0x00) << 8;
    D1 |= spi_transfer(spi, 0x00);
    gpio_write(GPIOB, cs, true);
    

    gpio_write(GPIOB, cs, false);
    spi_write_byte(spi, 0x58);   // convert D2 OSR=4096
    
    uint32_t D2 = 0;
    spi_transfer(spi, 0x00);
    D2 = spi_transfer(spi, 0x00) << 16;
    D2 |= spi_transfer(spi, 0x00) << 8;
    D2 |= spi_transfer(spi, 0x00);
    gpio_write(GPIOB, cs, true);
   

    int32_t dT = (int32_t)D2 - ((int32_t)C[5] << 8);
    int32_t T = (int32_t)((int64_t)2000 + ((int64_t)dT * ((int64_t)C[6] >> 23)));
    int64_t off = ((int64_t)C[2] << 16) + (((int64_t)C[4] * dT) >> 7);
    int64_t sens = ((int64_t)C[1] << 15) + (((int64_t)C[3] * dT) >> 8);
    int32_t P = ((((int64_t)D1 * sens) >> 21) - off) >> 15;
    data[0] = T;    // degrees celsius
    data[1] = P;    // mbar

    // TODO second order conversion
}

void DMA_init(SPI_TypeDef *spi){
    RCC->AHB1ENR |= BIT(0);

    DMAMUX1_Channel0->CCR = 37; // SPI1 RX
    DMAMUX1_Channel1->CCR = 38; // SPI1 TX

    // enable direct mode
    DMA1_Stream0->FCR &= ~BIT(2);
    DMA1_Stream1->FCR &= ~BIT(2);


    DMA1_Stream0->CR &= ~BIT(0);
    DMA1_Stream0->CR &= ~BIT(1);

    DMA1_Stream0->PAR = (uint32_t)&spi->RXDR;
    DMA1_Stream1->PAR = (uint32_t)&spi->TXDR;

    // set peripheral size, memory size, direction (M to P), memory inc off
    DMA1_Stream1->CR &= ~(DMA_SxCR_MSIZE_Msk | DMA_SxCR_PSIZE_Msk | BIT(10) | DMA_SxCR_DIR_Msk);    
    DMA1_Stream1->CR |= BIT(6) | BIT(10);;

    // set peripheral size, memory size, direction (P to M), memory inc off
    DMA1_Stream0->CR &= ~(DMA_SxCR_MSIZE_Msk | DMA_SxCR_PSIZE_Msk | DMA_SxCR_DIR_Msk);    
    DMA1_Stream0->CR |= BIT(4) | BIT(10);

    DMA1_Stream1->CR &= ~BIT(0);
    DMA1_Stream1->CR &= ~BIT(1);

    NVIC_EnableIRQ(DMA1_Stream0_IRQn);
}


void BMI088_init(SPI_TypeDef *spi, uint8_t cs_accel){
    gpio_write(GPIOB, cs_accel, true);
    spin(1000);

    gpio_write(GPIOB, cs_accel, false);
    spi_transfer(spi, 0x7D);
    spi_transfer(spi, 0x04);
    gpio_write(GPIOB, cs_accel, true);
    spin(500000);
}

void BMI_read_test(SPI_TypeDef *spi, uint8_t cs){
    gpio_write(GPIOB, cs, false);
    spi_transfer(spi,  0x12 | (1<<7));
    spi_transfer(spi, 0x00);
    uint8_t spi_test = 8;
    spi_test = spi_transfer(spi, 0x00);
    gpio_write(GPIOB, cs, true);

    printf("wahhh\n");
    printf("spi test = %08lx\n", spi_test);
}


void BMI088_read_data(SPI_TypeDef *spi, uint8_t* rx_buf, uint8_t* tx_buf, uint8_t buf_len, uint8_t cs, uint8_t reg){
    tx_buf[0] = 0x80 | reg;    // read bit + reg address
    tx_buf[1] =  0x00;  // dummy read byte
    for (int i = 2; i < buf_len; i++) tx_buf[i] = 0x00; // dummy clocking bytes

    // Change TSIZE
    spi->CR1 &= ~BIT(0);
    spi->CR2 &= ~SPI_CR2_TSIZE_Msk;
    spi->CR2 |= buf_len;

    printf("CR2=%08lx\n", spi->CR2);

    // SPI1 RX setup
    DMA1_Stream0->CR &= ~DMA_SxCR_EN;
    while (DMA1_Stream0->CR & DMA_SxCR_EN); // wait till enabled
    DMA1_Stream0->M0AR = (uint32_t)rx_buf;
    DMA1_Stream0->NDTR = buf_len;
    DMA1_Stream0->CR |= DMA_SxCR_EN;

    // SPI1 TX setup
    DMA1_Stream1->CR &= ~DMA_SxCR_EN;
    while (DMA1_Stream1->CR & DMA_SxCR_EN); // wait till enabled
    DMA1_Stream1->M0AR = (uint32_t)tx_buf;
    DMA1_Stream1->NDTR = buf_len;
    DMA1_Stream1->CR |= DMA_SxCR_EN;

    gpio_write(GPIOB, cs, false);
    spi->CR1 |= BIT(0); // enable spi
    spi->CR1 |= BIT(9); // start
    
    // printf("AHB1ENR=%08lx\n", RCC->AHB1ENR);
}