#include "AP_SysClock.h"

void timer_init()
{
        //gpiod init
        RCC->AHB1ENR = RCC->AHB1ENR | RCC_AHB1LPENR_GPIODLPEN; //AHB1 GPIOD RCC enable
        
        GPIOD->MODER = GPIOD->MODER &~(GPIO_MODER_MODE13_Msk);
        GPIOD->MODER = GPIOD->MODER | GPIO_MODER_MODE13_1; //10 -> Alternate function mode

        GPIOD->AFRH = GPIOD->AFRH &~(GPIO_AFRH_AFSEL13_Msk);
        GPIOD->AFRH |= GPIO_AFRH_AFSEL13_1; //0010 -> PD13 = AF2(timer4)


        //timer init
        RCC->APB1ENR = RCC->APB1ENR | RCC_APB1ENR_TIM4EN; //APB1 TIM4 enable
        TIM4->CCMR1 = TIM4->CCMR1 &~(TIM_CCMR1_CC2S_Msk); //00- > TIM4 ch2 as output

        TIM4->CCMR1 = TIM4->CCMR1 &~(TIM_CCMR1_OC2M_Msk);
        TIM4->CCMR1 = TIM4->CCMR1 | (TIM_CCMR1_OC2M_1);
        TIM4->CCMR1 = TIM4->CCMR1 | (TIM_CCMR1_OC2M_2);//110 -> PWM mode 1

        TIM4->PSC = TIM4->PSC &~(TIM_PSC_PSC_Msk);
        TIM4->PSC |= 0b1101000111; //839

        TIM4->ARR = TIM4->ARR &~(TIM_ARR_ARR_Msk);
        TIM4->ARR |= 0b1001; //10
        
        TIM4->CCR2 = TIM4->CCR2 &~(TIM_CCR2_CCR2_Msk);
        TIM4->CCR2 |= 0b0101; //5


        TIM4->CCMR1 = TIM4->CCMR1 | (TIM_CCMR1_OC2PE); //1 -> output compare 1 preload enable
        TIM4->CR1 = TIM4->CR1 | (TIM_CR1_ARPE); //1 -> Arr register is buffered

        TIM4->CR1 = TIM4->CR1 &~(TIM_CR1_CMS_Msk); //00 -> Edge-aligned mode
        TIM4->CR1 = TIM4->CR1 &~(TIM_CR1_DIR_Msk); //0 -> upcounter
}

void timer_start()
{
        TIM4->CCER = TIM4->CCER | (TIM_CCER_CC2E); //Capture/compare 2 output enable
        TIM4->CR1 = TIM4->CR1 | TIM_CR1_CEN; //counter enable
}