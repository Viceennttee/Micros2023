#include "stm32f0xx.h"
#include "PWM2.h"
void PWM2 (void)
{
	//Pb3 TIM1_CH3
		RCC->AHBENR |= (1<<18);			// I/O port B clock enabled
		GPIOB->MODER |= (2<<6);		//PORTb3 : Alternate mode
		//seleccionar
		GPIOB->AFR[0]|= (2<<12);		//Alternate Function 02 que es canal del TIM2

		RCC->APB1ENR |= (0<<1);			// TIM2 clock enabled
		TIM2->PSC = 7; 					//Set prescaler to 7, so APBCLK/8 i.e 1MHz
		TIM2->ARR = 1000-1;				//Set ARR = 999, as timer clock is 1MHz the period is 1000 us */
				//Set CCRx = duty_cycle*10
		TIM2->CCMR1 |= (7<<4)| TIM_CCMR1_OC1PE;
										// Select PWM mode 3 on OC1 (OC1M = 111),enable preload register on OC1 (OC1PE = 1)
		TIM2->CCER |= TIM_CCER_CC1P | TIM_CCER_CC1E; 	//Select active high polarity on OC1 (CC1P = 0, reset value),
										//enable the output on OC1 (CC1E = 1)*/
		TIM2->BDTR |= TIM_BDTR_MOE;  	//Enable output (MOE = 1)
		TIM2->CR1 |= TIM_CR1_CEN; 	 	//Enable counter (CEN = 1)	select edge aligned mode (CMS = 00, reset value)
										//select direction as upcounter (DIR = 0, reset value)
		TIM2->EGR |= TIM_EGR_UG; 		//Force update generation (UG = 1) */

}
