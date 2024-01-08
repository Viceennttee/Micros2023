#include "stm32f0xx.h"
#include "captcom.h"
volatile unsigned long counter0, counter1, Counter, tiempo_ms;
int factor, rpm, speed_cm7s;
unsigned char gap=0;
unsigned short nV=0;
extern char mensaje[];
void TIM2_IRQHandler(void)
{
	if ((TIM2->SR & TIM_SR_CC2IF) != 0)
	{
		GPIOC->ODR^=(1<<7);
		TIM2->SR &= ~(TIM_SR_CC2IF);
		if ((TIM2->SR & TIM_SR_CC2OF) != 0) // Check the overflow
			{
				// Overflow error management
				//gap = 0; // Reinitialize the laps computing
				nV++;
				TIM2->SR &= ~(TIM_SR_CC2OF | TIM_SR_CC2IF); // Clear the flags
				return;
			}

		if (gap == 0) /* Test if it is the first rising edge */
			{
				counter0 = TIM2->CCR2; /* Read the capture counter which clears the CC1ICF */
				gap = 1; /* Indicate that the first rising edge has yet been detected */
			}
		else
			{   gap=0;
				counter1 = TIM2->CCR2; /* Read the capture counter which clears the	CC1ICF */
				/*if (counter1 > counter0) // Check capture counter overflow
					{
						Counter = counter1 - counter0;
					}
				else
					{
						//Counter = counter1 + 0xFFFF - counter0 + 1;

					}

				 */
				if (counter1>counter0) Counter=counter1-counter0+nV*(2^32 - 1);
				else Counter=counter1-counter0+(nV-1)*(2^32 - 1);

				nV=0;
				counter0 = counter1;
				tiempo_ms=(Counter*factor)/1000000; /*se multiplican los conteos
				por el tiempo de conteo y se divide entre 1 millón para tener el tiempo
				por vuelta en milisegundos
				 */
				rpm=60*1000/(tiempo_ms); //revoluciones por 60,000ms
				speed_cm7s=53*rpm/60;//53 de circunferencia del círcullo cm/s

				mensaje[6] = (rpm%10)+0x30;
				rpm /= 10;
				mensaje[5] = (rpm%10)+0x30;
				mensaje[4] = (rpm/10)+0x30;
				NVIC->ISER[0]|=(1<<27); //se habilita NVIC de USART
				USART1 -> CR1 |= (1<<3)+(1<<2)+(1<<0)+(1<<6);	//HABILITAMOS EL Tx, Rx, EL UART Y
				//la interrupción por  Transmission complete interrupt enable

			}
	}
	else
	{
		/* Unexpected Interrupt */
		/* Manage an error for robust application */
	}
} //TIM3_ISR
void captcom (void)
{	//PA1 TIM2_CH2 Alternate Function 2
	factor=(166*15); //factor, es el tiempo que toma cada conteo del encoder
	RCC->AHBENR |= (1<<17);			//Bit 17 (1): I/O port A clock enabled
	GPIOA->MODER |= (2<<2);			//PORTA1 : Alternate mode
	GPIOA->AFR[0]|= (2<<4);			//Alternate Function 01

	RCC->APB1ENR |= (1<<0);			//Bit 0 (1): TIM2 clock enabled
	TIM2->PSC = 8-1; 					//Set prescaler to 7, so APBCLK/8 i.e 1MHz

	TIM2->CCMR1 |= TIM_CCMR1_CC2S_0	| TIM_CCMR1_IC2F_0 | TIM_CCMR1_IC2F_1;
			//Select the active input TI1 (CC1S = 01),	program the input filter for 8 clock cycles (IC1F = 0011),
			//select the rising edge on CC1 (CC1P = 0, reset value)	and prescaler at each valid transition (IC1PS = 00, reset value)
	TIM2->CCER |= TIM_CCER_CC2E; 		// Enable capture by setting CC1E (00 Rising edge, default)
	TIM2->DIER |= TIM_DIER_CC2IE; 		// Enable interrupt on Capture/Compare
	TIM2->CR1 |= TIM_CR1_CEN; 			// Enable TIM2 counter

	TIM2->EGR |= TIM_EGR_UG; 		//Force update generation (UG = 1)

	NVIC->ISER[0]=(1<<15);
}
