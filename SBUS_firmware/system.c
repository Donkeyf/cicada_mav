#include "stm32h743xx.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "hal.h"

// void cpu_max_init(){
//     // enable HSE
//     RCC->CR |= RCC_CR_HSEON;
//     while (!(RCC->CR & (1 << 17))); // wait till ready

//     PWR->D3CR &= ~PWR_D3CR_VOS_Msk;
//     PWR->D3CR |= 3UL << 14; // set VOS 1
//     while (!(PWR->D3CR & PWR_D3CR_VOSRDY));

//     RCC->APB4ENR |= RCC_APB4ENR_SYSCFGEN;   // enable SYSCFG
//     SYSCFG->PWRCR |= SYSCFG_PWRCR_ODEN; // enable VOS0
//     while (!(PWR->D3CR & PWR_D3CR_VOSRDY));

//     // set latency and frequency
//     FLASH->ACR &= ~FLASH_ACR_LATENCY_Msk;
//     FLASH->ACR |= FLASH_ACR_LATENCY_1WS;
//     FLASH->ACR |= (1UL << 4);

//     // set domain prescalers (HPRE) all else are /1
//     RCC->D1CFGR &= ~RCC_D1CFGR_HPRE_Msk;
//     RCC->D1CFGR |= RCC_D1CFGR_HPRE_DIV4;
//     RCC->D2CFGR &= RCC_D2CFGR_D2PPRE1_Msk;
//     RCC->D2CFGR &= RCC_D2CFGR_D2PPRE2_Msk;

//     // set PLL dividers
//     RCC->PLLCKSELR &= ~RCC_PLLCKSELR_PLLSRC_Msk;
//     RCC->PLLCKSELR |= RCC_PLLCKSELR_PLLSRC_HSE;
//     RCC->PLLCKSELR &= ~RCC_PLLCKSELR_DIVM1_Msk;
//     RCC->PLLCKSELR |= 1UL << 4;

//     // DIVN1 and Q1 mostly for SPI
//     RCC->PLL1DIVR &= ~RCC_PLL1DIVR_N1_Msk;
//     RCC->PLL1DIVR |= 59UL << 0;
//     RCC->PLL1DIVR &= ~RCC_PLL1DIVR_Q1_Msk;
//     RCC->PLL1DIVR |= 14UL << 16;
//     RCC->PLL1DIVR &= ~RCC_PLL1DIVR_P1_Msk;
//     RCC->PLL1DIVR |= 1UL << 9;

//     // Enable P1 and Q1, PLL1 using 16mhz
//     RCC->PLLCFGR |= 3UL << 2;
//     RCC->PLLCFGR |= BIT(16);
//     RCC->PLLCFGR |= BIT(17);
    

//     // enable PLL
//     RCC->CR |= BIT(24);
//     while (!(RCC->CR & BIT(25)));

//     // select PLL1 for sysclock
//     RCC->CFGR &= ~RCC_CFGR_SW_Msk;
//     RCC->CFGR |= RCC_CFGR_SW_PLL1;
//     while ((RCC->CFGR & RCC_CFGR_SWS_Msk) != RCC_CFGR_SWS_PLL1);
// }



void cpu_max_init(){
    // set latency and frequency
    FLASH->ACR &= ~FLASH_ACR_LATENCY_Msk;
    FLASH->ACR |= FLASH_ACR_LATENCY_1WS;
    FLASH->ACR |= (1UL << 4U);
    while(!((FLASH->ACR & FLASH_ACR_LATENCY_1WS) == 1));

    PWR->CR3 &= ~(PWR_CR3_BYPASS | PWR_CR3_SCUEN | PWR_CR3_LDOEN);
    PWR->CR3 |= PWR_CR3_LDOEN;

    PWR->D3CR &= ~PWR_D3CR_VOS_Msk;
    PWR->D3CR |= 3UL << 14U; // set VOS 1

    RCC->APB4ENR |= RCC_APB4ENR_SYSCFGEN;   // enable SYSCFG
    SYSCFG->PWRCR |= SYSCFG_PWRCR_ODEN; // enable VOS0
    while ((PWR->D3CR & PWR_D3CR_VOSRDY) == 0);

    // enable HSE
    RCC->CR |= RCC_CR_HSEON;
   while ((RCC->CR & RCC_CR_HSERDY) == 0); // wait till ready

    // set PLL source as HSE
    RCC->PLLCKSELR &= ~RCC_PLLCKSELR_PLLSRC_Msk;
    RCC->PLLCKSELR |= RCC_PLLCKSELR_PLLSRC_HSE;
    
    // enable PLLs
    RCC->PLLCFGR |= RCC_PLLCFGR_DIVP1EN;
    RCC->PLLCFGR |= RCC_PLLCFGR_DIVQ1EN;
    RCC->PLLCFGR |= RCC_PLLCFGR_DIVR1EN;

    // Set VCO input range 8-16mhz and output wide range
    RCC->PLLCFGR &= ~RCC_PLLCFGR_PLL1RGE;
    RCC->PLLCFGR |= 3UL << 2U;
    RCC->PLLCFGR &= ~RCC_PLLCFGR_PLL1VCOSEL;

    // set PLL dividers
    RCC->PLLCKSELR &= ~RCC_PLLCKSELR_DIVM1;
    RCC->PLLCKSELR |= 1UL << RCC_PLLCKSELR_DIVM1_Pos;
    RCC->PLL1DIVR &= ~RCC_PLL1DIVR_N1;
    RCC->PLL1DIVR |= (60UL - 1UL) << RCC_PLL1DIVR_N1_Pos;
    RCC->PLL1DIVR &= ~RCC_PLL1DIVR_P1;
    RCC->PLL1DIVR |= (2UL - 1UL) << RCC_PLL1DIVR_P1_Pos;
    RCC->PLL1DIVR &= ~RCC_PLL1DIVR_Q1;
    RCC->PLL1DIVR |= (15UL - 1UL) << RCC_PLL1DIVR_Q1_Pos;
    RCC->PLL1DIVR &= ~RCC_PLL1DIVR_R1;
    RCC->PLL1DIVR |= (2UL - 1UL) << RCC_PLL1DIVR_R1_Pos;

    // enable PLL1
    RCC->CR |= RCC_CR_PLL1ON;
    while ((RCC->CR & RCC_CR_PLL1RDY_Msk) == 0);

    // intermediate prescaler when target freq higher than 80mhz
    RCC->D1CFGR &= ~RCC_D1CFGR_HPRE;
    RCC->D1CFGR |= RCC_D1CFGR_HPRE_DIV2;
    RCC->CFGR &= ~RCC_CFGR_SW;
    RCC->CFGR |= RCC_CFGR_SW_PLL1;
    while ((RCC->CFGR & RCC_CFGR_SWS_Msk) == 0);

    // set prescalers
    RCC->D1CFGR &= ~RCC_D1CFGR_D1CPRE;
    RCC->D1CFGR &= ~RCC_D1CFGR_HPRE;
    RCC->D1CFGR |= RCC_D1CFGR_HPRE_DIV4;   
    RCC->D2CFGR &= ~RCC_D2CFGR_D2PPRE1;
    RCC->D2CFGR &= ~RCC_D2CFGR_D2PPRE2;
    RCC->D1CFGR &= ~RCC_D1CFGR_D1PPRE;
    RCC->D3CFGR &= ~RCC_D3CFGR_D3PPRE;

}   