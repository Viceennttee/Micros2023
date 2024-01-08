#include "stm32f0xx.h"
#include "PWM1.h"


void PWM1 (void)
{
	//PC6 TIM3_CH1
		RCC->AHBENR |= (1<<19);			//Bit 19 (1): I/O port C clock enabled
		GPIOC->MODER |= (2<<12);		//PORTC6 : Alternate mode, solo hay una funcion alterna

		RCC->APB1ENR |= (1<<1);			//Bit 1 (1): TIM3 clock enabled
		TIM3->PSC = 7; 					//Set prescaler to 7, so APBCLK/8 i.e 1MHz
		TIM3->ARR = 1000-1;				//Set ARR = 999, as timer clock is 1MHz the period is 1000 us */
		TIM3->CCMR1 |= (7<<4)| TIM_CCMR1_OC1PE;
										// Select PWM mode 3 on OC1 (OC1M = 111),enable preload register on OC1 (OC1PE = 1)
		TIM3->CCER |= TIM_CCER_CC1P | TIM_CCER_CC1E; 	//Select active high polarity on OC1 (CC1P = 0, reset value),
										//enable the output on OC1 (CC1E = 1)*/
		TIM3->BDTR |= TIM_BDTR_MOE;  	//Enable output (MOE = 1)
		TIM3->CR1 |= TIM_CR1_CEN; 	 	//Enable counter (CEN = 1)	select edge aligned mode (CMS = 00, reset value)
										//select direction as upcounter (DIR = 0, reset value)
		TIM3->EGR |= TIM_EGR_UG; 		//Force update generation (UG = 1) */
}
