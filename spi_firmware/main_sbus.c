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
#define SBUS_RX 0

#define BIT(x) (1UL << (x))


static volatile uint32_t s_ticks; // volatile is important!!
void SysTick_Handler(void) {
  s_ticks++;
}

static uint8_t imu_type = 0;  // 0 = accel, 1 = gyro
int16_t acc_x;
int16_t acc_y;
int16_t acc_z;
int16_t gyro_x;
int16_t gyro_y;
int16_t gyro_z;
uint8_t accel_rx_buf[8];
uint8_t accel_tx_buf[8];
uint8_t gyro_rx_buf[7];
uint8_t gyro_tx_buf[7];

void HardFault_Handler(void) {
  printf("CFSR=%08lx, HFSR=%08lx, BFAR=%08lx\n", SCB->CFSR, SCB->CFSR, SCB->BFAR);
}

void DMA1_Stream0_IRQHandler(void){
  if (DMA1->LISR & DMA_LISR_TCIF0){
		DMA1->LIFCR = DMA_LIFCR_CTCIF0 | DMA_LIFCR_CTCIF1 | DMA_LIFCR_CHTIF0 | DMA_LIFCR_CHTIF1
            | DMA_LIFCR_CTEIF0 | DMA_LIFCR_CTEIF1;	// clear DMA transfer complete flag

		while (!(SPI1->SR & BIT(3))); // wait for EOT to confirm transaction truly finished
    SPI1->IFCR = 0xFFFFFFFF; // clear every latched SPI status flag;

		// TODO do accel and gyro
    if (imu_type == 0){
      gpio_write(GPIOB, ACCEL_CSB, true);
      acc_x = (int16_t)((accel_rx_buf[3] << 8) | accel_rx_buf[2]);
      acc_y = (int16_t)((accel_rx_buf[5] << 8) | accel_rx_buf[4]);
      acc_z = (int16_t)((accel_rx_buf[7] << 8) | accel_rx_buf[6]);
      imu_type = 1;

      printf("accx=%04lx, accy=%04lx, accz=%04lx\r\n", acc_x, acc_y, acc_z);
      

      BMI088_read_data(SPI1, gyro_rx_buf, gyro_tx_buf, 7, GYRO_CSB, 0x02);  // send for gyroscope reading
      spin(10000);
      printf("post-CSTART NDTR0=%lu NDTR1=%lu SR=%08lx\n", DMA1_Stream0->NDTR, DMA1_Stream1->NDTR, SPI1->SR);

    } else {
      gpio_write(GPIOB, GYRO_CSB, true);
      gyro_x = (int16_t)((gyro_rx_buf[2] << 8) | gyro_rx_buf[1]);
      gyro_y = (int16_t)((gyro_rx_buf[4] << 8) | gyro_rx_buf[3]);
      gyro_z = (int16_t)((gyro_rx_buf[6] << 8) | gyro_rx_buf[5]);
      imu_type = 0;

      printf("gyrox=%04lx, gyroy=%04lx, gyroz=%04lx\r\n", gyro_x, gyro_y, gyro_z);
    }
	}
}

void UART8_IRQHandler(void){
  
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

__attribute__((section(".vectors"))) void (*const tab[16 + 150])(void) = {
  [0]  = _estack,
  [1]  = _reset,
  [3] = HardFault_Handler,
  [15] = SysTick_Handler,
  [16 + DMA1_Stream0_IRQn] = DMA1_Stream0_IRQHandler,
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
  RCC->AHB4ENR |= BIT(4);

  gpio_set_mode(GPIOE, SBUS_RX, 2); // set sbus alternate function
  gpio_set_afr(GPIOE, SBUS_RX, 8);  // set uart8 rx
  RCC->D2CCIP2R &= ~(7UL << 0);
  RCC->APB1LENR |= BIT(31); // enable UART8 clock

  cpu_max_init();
  uart_init(UART8, 120000000 / 100000); // non standard baud rate for sbus (100k)



}


int main(void){
  RCC->D1CCIPR &= ~(3UL << 0);
  RCC->D1CCIPR |= (0UL << 0); // 00 = hsi_ck selected as per_ck
  RCC->D2CCIP1R &= ~(7UL << 12); // SPI123SEL is typically bits [14:12] on H743 — verify against your header, select pll1q

  RCC->AHB4ENR |= BIT(1) | BIT(0); // enable GPIOB clock
  gpio_set_mode(GPIOB, BARO_CSB, 1);  // output
  gpio_set_mode(GPIOB, ACCEL_CSB, 1);
  gpio_set_mode(GPIOB, GYRO_CSB, 1);


  gpio_set_mode(GPIOA, 5, 2);  // alternate function
  gpio_set_mode(GPIOA, 6, 2);  // alternate function
  gpio_set_mode(GPIOA, 7, 2);  // alternate function

  gpio_set_afr(GPIOA, 5, 5);  // AF5 spi
  gpio_set_afr(GPIOA, 6, 5);  // AF5 spi
  gpio_set_afr(GPIOA, 7, 5);  // AF5 spi

  gpio_set_mode(GPIOB, UART_RX, 2);
  gpio_set_mode(GPIOB, UART_TX, 2);
  gpio_set_afr(GPIOB, UART_RX, 14);
  gpio_set_afr(GPIOB, UART_TX, 14);
  RCC->D2CCIP2R &= ~(7UL << 0); // clear bits 2:0, pll1
  RCC->APB1LENR |= BIT(20); //  enable uart clock

  gpio_write(GPIOB, BARO_CSB, true);
  gpio_write(GPIOB, ACCEL_CSB, true);
  gpio_write(GPIOB, GYRO_CSB, true);

  cpu_max_init();

  uart_init(UART5, 120000000 / 115200);
  printf("yoooo\r\n");

  SPI_init();
  
  uint16_t calib[7] = {0};
  MS5611_init(SPI1, calib, BARO_CSB);  // init barometer

  int32_t* data[2];
  read_MS5611_adc(SPI1, BARO_CSB, calib, data); 
  printf("temp=%08lx, pressure=%08lx\n", data[0], data[1]);

  DMA_init(SPI1);
  BMI088_init(SPI1, ACCEL_CSB);
  imu_type = 0;

  // BMI_read_test(SPI1, ACCEL_CSB);
  BMI088_read_data(SPI1, accel_rx_buf, accel_tx_buf, 8, ACCEL_CSB, 0x12);

  systick_init(480000000 / 1000);
  uint32_t timer = 0, period = 500; 
  for(;;) {
    if (timer_expired(&timer, period, s_ticks)){
      //temperature = read_MS5611_adc(SPI1, BARO_CSB, calib);
    }
  }
  
      return 0;
}

