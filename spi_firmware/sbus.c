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

void parse_sbus_data(uint8_t *sbus_buf, uint16_t* channel_data, bool* failsafe, bool* frame_lost){
    channel_data[0] = (sbus_buf[1] | sbus_buf[2] << 8) & 0x07FF;
    channel_data[1] = (sbus_buf[2] >> 3 | sbus_buf[3] << 5) & 0x07FF;
    channel_data[2] = (sbus_buf[3] >> 6 | sbus_buf[4] << 2 | sbus_buf[5] << 10) & 0x07FF;
    channel_data[3] = (sbus_buf[5] >> 1 | sbus_buf[6] << 7) & 0x07FF;
    channel_data[4] = (sbus_buf[6] >> 4 | sbus_buf[7] << 4) & 0x07FF;
    channel_data[5] = (sbus_buf[7] >> 7 | sbus_buf[8] << 1 | sbus_buf[9] << 9) & 0x07FF;
    channel_data[6] = (sbus_buf[8] >> 2 | sbus_buf[10] << 6) & 0x07FF;
    channel_data[6]  = (sbus_buf[9]  >> 2 | sbus_buf[10] << 6) & 0x07FF;
    channel_data[7]  = (sbus_buf[10] >> 5 | sbus_buf[11] << 3) & 0x07FF;
    channel_data[8]  = (sbus_buf[12] | sbus_buf[13] << 8) & 0x07FF;
    channel_data[9]  = (sbus_buf[13] >> 3 | sbus_buf[14] << 5) & 0x07FF;
    channel_data[10] = (sbus_buf[14] >> 6 | sbus_buf[15] << 2 | sbus_buf[16] << 10) & 0x07FF;
    channel_data[11] = (sbus_buf[16] >> 1 | sbus_buf[17] << 7) & 0x07FF;
    channel_data[12] = (sbus_buf[17] >> 4 | sbus_buf[18] << 4) & 0x07FF;
    channel_data[13] = (sbus_buf[18] >> 7 | sbus_buf[19] << 1 | sbus_buf[20] << 9)  & 0x07FF;
    channel_data[14] = (sbus_buf[20] >> 2 | sbus_buf[21] << 6) & 0x07FF;
    channel_data[15] = (sbus_buf[21] >> 5 | sbus_buf[22] << 3) & 0x07FF;

    if (sbus_buf[23] & 0x04) *frame_lost = true;
    if (sbus_buf[23] & 0x08) *failsafe = true;

}



