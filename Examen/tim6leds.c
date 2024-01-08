#include "stm32f0xx.h"
#include "tim6leds.h"

extern volatile unsigned short joy1_PWM, joy2_PWM;
extern volatile unsigned short volt_B;
extern int dato_leido;

void TIM6_DAC_IRQHandler(void)
{
	TIM6->SR = 0; //: No update occurred, se baja la bandera
	ADC1->CR|= (1<<2); // ADC start conversion command po
	I2C2->CR2 |= I2C_CR2_START;

	TIM2->CCR1 = joy1_PWM*10+dato_leido/100;
	TIM3->CCR1 = joy2_PWM*10+dato_leido/100;


	//prende los leds dependiendo del nivel de batería
	if(volt_B>=39){
		GPIOC->ODR &= (1<<8); //apagar los demás menos led de freno
		GPIOC->ODR |= (1<<10);//prender verde

	}
	else if(volt_B<39&&volt_B>=36){
		GPIOC->ODR &= (1<<8); //apagar los demás menos led de freno
		GPIOC->ODR |= (1<<11);//prender amarillo

	}
	else if(volt_B<37){
		GPIOC->ODR &= (1<<8); //apagar los demás menos led de freno
		GPIOC->ODR |= (1<<12);//prender rojo
	}
}

void tim6leds(void)
{
	RCC -> APB1ENR |= (1<<4);	//Enable clocks TIM6
	TIM6 -> PSC |= 8-1;			//DIVISOR/8 8MHz/8 = 1MHz, según
	TIM6 -> ARR = 50000 - 1;	//50,000 CUENTAS DE 1us=0.05s, según

	TIM6->CR1|=1; //Counter enable
	TIM6->DIER|=1; //Module Interrupt enable
	NVIC->ISER[0]=(1<<17); //NVIC Interrupt set enable register

	//leds de la batería y ya
	RCC->AHBENR |= (1<<19);			//Bit 19 (1): I/O port C clock enabled
	GPIOC->MODER |= (1<<20)|(1<<22)|(1<<24);		//PORTC9 : Output mode


}
