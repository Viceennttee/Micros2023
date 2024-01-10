#include "stm32f0xx.h"

volatile uint16_t i;
//secuencia del motor
unsigned char secuencia[]={12,6,3,9};
unsigned char paso=0;
unsigned char reload;

void TIM6_DAC_IRQHandler (void)
{
	TIM6->SR=0;
	if (i--) ;
	else{
		if (GPIOA->IDR &(1<<0)){
			GPIOC->ODR = secuencia[paso++%4] ;
		}
		else {GPIOC->ODR=secuencia[paso--%4];
		}
		i= reload;
	}



}

void TIM6_delay_ms_init(uint16_t t)
{
	i=t;
	reload=t;

	TIM6->CR1|=1;					//Timer on

	TIM6->DIER|=1;					//Module Interrupt enable
	NVIC->ISER[0]=(1<<17);			//NVIC Interrupt set enable register
	//__EnableInterrupts();
}

int main(void)
{
	RCC->AHBENR |= (1<<17);		//Bit 17 (1): I/O port A clock enabled and the input bottom
	RCC->AHBENR |= (1<<19);			//Bit 19 (1): I/O port C clock enabled
	GPIOC->MODER |= 0x055;		//PORTC1-3 y 9: Output mode

	RCC->APB1ENR |= (1<<4);			//Bit 4 (1): TIM6
	TIM6->ARR=8000-1;				//Clk = 8 MHz. 8000 cuentas hacen 1 ms
	TIM6->PSC=0;					//preescaler=valor+1;

	TIM6_delay_ms_init(10);


	/* Loop forever */
	for(;;)
	{
	}
}
