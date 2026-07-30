#include "stm32h743xx.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "hal.h"

void cpu_max_init(){
    // enable HSE
    RCC->CR |= RCC_CR_HSEON;
    while (!(RCC->CR & (1 << 17))); // wait till ready

    PWR->D3CR &= ~PWR_D3CR_VOS_Msk;
    PWR->D3CR |= 3UL << 14; // set VOS 1
    while (!(PWR->D3CR & PWR_D3CR_VOSRDY));

    RCC->APB4ENR |= RCC_APB4ENR_SYSCFGEN;   // enable SYSCFG
    SYSCFG->PWRCR |= SYSCFG_PWRCR_ODEN; // enable VOS0
    while (!(PWR->D3CR & PWR_D3CR_VOSRDY));

    // set latency and frequency
    FLASH->ACR &= ~FLASH_ACR_LATENCY_Msk;
    FLASH->ACR |= FLASH_ACR_LATENCY_1WS;
    FLASH->ACR |= (1UL << 4);

    // set domain prescalers (HPRE) all else are /1
    RCC->D1CFGR &= ~RCC_D1CFGR_HPRE_Msk;
    RCC->D1CFGR |= RCC_D1CFGR_HPRE_DIV4;
    RCC->D2CFGR &= RCC_D2CFGR_D2PPRE1_Msk;
    RCC->D2CFGR &= RCC_D2CFGR_D2PPRE2_Msk;

    // set PLL dividers
    RCC->PLLCKSELR &= ~RCC_PLLCKSELR_PLLSRC_Msk;
    RCC->PLLCKSELR |= RCC_PLLCKSELR_PLLSRC_HSE;
    RCC->PLLCKSELR &= ~RCC_PLLCKSELR_DIVM1_Msk;
    RCC->PLLCKSELR |= 1UL << 4;

    // DIVN1 and Q1 mostly for SPI
    RCC->PLL1DIVR &= ~RCC_PLL1DIVR_N1_Msk;
    RCC->PLL1DIVR |= 59UL << 0;
    RCC->PLL1DIVR &= ~RCC_PLL1DIVR_Q1_Msk;
    RCC->PLL1DIVR |= 14UL << 16;
    RCC->PLL1DIVR &= ~RCC_PLL1DIVR_P1_Msk;
    RCC->PLL1DIVR |= 1UL << 9;

    // Enable P1 and Q1, PLL1 using 16mhz
    RCC->PLLCFGR |= 3UL << 2;
    RCC->PLLCFGR |= BIT(16);
    RCC->PLLCFGR |= BIT(17);
    

    // enable PLL
    RCC->CR |= BIT(24);
    while (!(RCC->CR & BIT(25)));

    // select PLL1 for sysclock
    RCC->CFGR &= ~RCC_CFGR_SW_Msk;
    RCC->CFGR |= RCC_CFGR_SW_PLL1;
    while ((RCC->CFGR & RCC_CFGR_SWS_Msk) != RCC_CFGR_SWS_PLL1);
}

void cpu_max_init(){
    // set latency and frequency
    FLASH->ACR &= ~FLASH_ACR_LATENCY_Msk;
    FLASH->ACR |= FLASH_ACR_LATENCY_1WS;
    FLASH->ACR |= (1UL << 4);
    while(!(FLASH->ACR & FLASH_ACR_LATENCY_1WS));

    PWR->CR3 &= ~(PWR_CR3_BYPASS | PWR_CR3_SCUEN | PWR_CR3_LDOEN);
    PWR->CR3 |= PWR_CR3_LDOEN;


}