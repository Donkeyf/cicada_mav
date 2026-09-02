#include "stm32h743xx.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "hal.h"


void sbus_init(USART_TypeDef *uart){
    DMAMUX1_Channel2->CCR = 81;

    // enable direct mode
    DMA1_Stream2->FCR = BIT(2);

    DMA1_Stream2->CR &= ~BIT(0);
    DMA1_Stream2->CR &= ~BIT(1);

    DMA1_Stream2->PAR = uart->RDR;  // read register
    DMA1_Stream2->CR &= ~(DMA_SxCR_MSIZE_Msk | DMA_SxCR_PSIZE_Msk | BIT(10) | DMA_SxCR_DIR_Msk);
    DMA1_Stream2->CR |= BIT(4) | BIT(10);   // transfer complete flag and MINC mode

    NVIC_EnableIRQ(DMA1_Stream2_IRQn);
    NVIC_EnableIRQ(UART8_IRQn); // enable idle interrupt and DMA

}



