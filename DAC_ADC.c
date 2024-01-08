#include "stm32f0xx.h"

volatile unsigned short ADC_result;
volatile unsigned short duty_cycle;
volatile unsigned long dato;
volatile unsigned char hay_dato_nuevo=0;
//inicializaciones

void UARTRX_int(void)
{
	RCC->AHBENR=(1<<17);		//CLK PORTA, ahí están los pines rx y tx
	RCC->APB2ENR=(1<<14);		//USART 1

	GPIOA->MODER|=(2<<20)+(2<<18);  //PORTA9 y PORTA10 funcion alterna
	GPIOA->AFR[1]|=(1<<4)+(1<<8);	//PORTA9 y PORTA10 UART1_Tx, USART1_Rx

	USART1->BRR=833;				//clk UART/baud rate -> 8 MHz/9600

	USART1->CR1=(1<<0)+(1<<3)+(1<<2)+ (1<<5);  //TE=RE=UEN=1, Bit 6: Hab interrupcion de recepcion
	NVIC->ISER[0]=(1<<27); //interrupción del NVIC
}
void PWM_init (void)
{
	//PC6 TIM3_CH1
		RCC->AHBENR |= (1<<19);			//I/O port C clock enabled para el PC6
		GPIOC->MODER |= (2<<12);		//PORTC6 : Alternate mode, solo hay una funcion alterna, PWM

		RCC->APB1ENR |= (1<<1);			//TIM3 clock enabled, se puede buscar en el manual
		TIM3->PSC = 8-1; 				//Set prescaler to 7, so APBCLK/8 i.e 1MHz
		TIM3->ARR = 1000-1;				//Set ARR = 999, as timer clock is 1MHz the period is 1000 us */
		TIM3->CCMR1 |= (7<<4)| TIM_CCMR1_OC1PE;
										// Select PWM mode 2 on OC1 (OC1M = 111),enable preload register on OC1 (OC1PE = 1)
		TIM3->CCER |= TIM_CCER_CC1P | TIM_CCER_CC1E; 	//Select active high polarity on OC1 (CC1P = 0, reset value),
										//enable the output on OC1 (CC1E = 1)*/
		TIM3->BDTR |= TIM_BDTR_MOE;  	//Enable output (MOE = 1)
		TIM3->CR1 |= TIM_CR1_CEN; 	 	//Enable counter (CEN = 1)	select edge aligned mode (CMS = 00, reset value)
										//select direction as upcounter (DIR = 0, reset value)
		TIM3->EGR |= TIM_EGR_UG; 		//Force update generation (UG = 1) */

		TIM3->DIER|=(1<<1);			//Falta ver UIE, update interrupt register
		NVIC->ISER[0]=(1<<16);			//Interrupción del NVIC
}

void adc_in_it(void)
{
	RCC->AHBENR|= (1<<17); // PORTA Clock del PA1
	RCC->APB2ENR |= (1<<9); // enable clocks for ADC1 clock

	GPIOA->MODER|=(3<<2); // Analog input PA1

	ADC1->CFGR1|=(1<<26); // ADC_IN1 MUX selected, single conv(nosotros inciamos las conversiones), SW trigger//Right alignment, 12 bits
	ADC1->CHSELR|=(1<<1); // ADC_IN1, selección del canal 1
	ADC1->IER|=(1<<2); //ADC_Intr_enable=1, cuanda haya un EOC;
	NVIC->ISER[0]=(1<<12); //NVIC del ADC, manual de referencia

	ADC1->CR|=1; //Enable del ADC
	do{} while ((ADC1->ISR & 1)==0); //while ADC not ready, esperar
	ADC1->ISR|=1; //Clear flag ADRDY
}



//interrupciones
void ADC1_COMP_IRQHandler(void)
{
	ADC_result = ADC1->DR; //se le asigna el valor a la variable
	duty_cycle=ADC_result*1000/4095;
}

void TIM3_IRQHandler(void){
	TIM3->SR &= ~TIM_SR_UIF;; //se limpia la bandera de la interrupción del tim3
	TIM3->CCR1 = duty_cycle;		//Set CCRx = duty_cycle*10, es un factor de conversión
	ADC1->CR|= (1<<2); //empezar las conversiones del ADC
}
void USART1_IRQHandler (void)
{
	unsigned char temp;
	temp=USART1->RDR;
	if (temp!=13) dato=(dato*10)+(temp-0x30);
	//if ((temp>='0') && (temp<='9'))
	else
	{
		dato=0;
		hay_dato_nuevo=1;
	}
}




int main (void)
{
	adc_in_it(); //configuración ADC
	PWM_init(); //configuración del TIM6 que será usado en el PWM
	UARTRX_int();
	for(;;);
}
