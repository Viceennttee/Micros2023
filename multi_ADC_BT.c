#include "stm32f0xx.h"

volatile unsigned short ADC_result, adc_voltaje_mV;
unsigned char i = 0;
unsigned char mensaje[] = {"V #.### \r\n"};


void ADC_init()
{
	RCC->AHBENR|= (1<<17); // PORTA Clock
	RCC->APB2ENR |= (1<<9); // enable clocks for ADC1 clock

	GPIOA->MODER|=(3<<2); // Analog input PA1
	GPIOA->MODER|=(3<<4);  ///PA2
	GPIOA->MODER|=(3<<6); // Analog input PA3
	GPIOA->MODER|=(3<<8);  ///PA4
	//ADC1->CFGR1|=(1<<26)+(1<<27); // ADC_IN1 MUX selected, single conv, SW trigger//Right alignment, 12 bits
	ADC1->CHSELR|=(1<<1);//|(1<<2); // ADC_IN1 y ADC_IN2
	ADC1->IER|=(1<<2); //ADC_Intr_enable=1;
	NVIC->ISER[0]=(1<<12); //NVIC del ADC

	ADC1->CR|=1; //ADEN=1;
	do{} while ((ADC1->ISR & 1)==0); //while ADC not ready
	ADC1->ISR|=1; //Clear flag ADRDY
}

void ADC1_COMP_IRQHandler(void)
{
	if (ADC1->CHSELR & (1<<1)) {
	            // Canal 1
		ADC_result = ADC1->DR;
		ADC1->CHSELR &= ~(1<<1); // Deshabilitar el canal 1
		ADC1->CHSELR |= (1<<2); // Habilitar el canal 2
	} else if (ADC1->CHSELR & (1<<2)) {
	            // Canal 2
		ADC_result= ADC1->DR;
		ADC1->CHSELR &= ~(1<<2); // Deshabilitar el canal 2
		ADC1->CHSELR |= (1<<3); // Habilitar el canal 3
	} else if (ADC1->CHSELR & (1<<3)) {
        // Canal 2
        ADC_result= ADC1->DR;
        ADC1->CHSELR &= ~(1<<3); // Deshabilitar el canal 2
        ADC1->CHSELR |= (1<<4); // Habilitar el canal 3
	} else if (ADC1->CHSELR & (1<<4)) {
        // Canal 2
        ADC_result= ADC1->DR;
        ADC1->CHSELR &= ~(1<<4); // Deshabilitar el canal 2
        ADC1->CHSELR |= (1<<1); // Habilitar el canal 3
	}
	adc_voltaje_mV = ADC_result*3300/4095; //PA1

	itua();

	NVIC->ISER[0]|=(1<<27);
	USART1 -> CR1 |= (1<<3)+(1<<2)+(1<<0)+(1<<6);	//HABILITAMOS EL Tx, Rx, EL UART Y INTRx

}

void itua(void)
{
	mensaje[6] = (adc_voltaje_mV%10)+0x30;
	adc_voltaje_mV /= 10;
	mensaje[5] = (adc_voltaje_mV%10)+0x30;
	adc_voltaje_mV /= 10;
	mensaje[4] = (adc_voltaje_mV%10)+0x30;
	mensaje[2] = (adc_voltaje_mV/10)+0x30;
}

void USART1_IRQHandler (void)
{
	USART1->TDR = mensaje[i++];
	if (mensaje[i]==0) {
		//NVIC->ICER[0]=(1<<27);
		i = 0;
	}
}

void UART_init(void)
{
	RCC->AHBENR|=(1<<17);	//CLK PORTA
	RCC->APB2ENR|=(1<<14);	//USART 1

	GPIOA->MODER|=(2<<20)+(2<<18);	//PORTA9 & PORTA10 Función alterna
	GPIOA->AFR[1]|=(1<<4)+(1<<8);	//PORTA9 & PORTA10 USART1_TX, USART1_RX

	USART1->BRR|=5000;				//CLK UART / Baud Rate -> 8MHz / 9600
	USART1->CR1|=(1<<0)+(1<<3)+(1<<2)+(1<<6);	//TE=RE=UEN=1, Bit 6 hab interrupción transmisión
}

void TIM6_init(void)
{
	//RCC->APB1ENR|= RCC_APB1ENR_TIM6EN;
	RCC -> APB1ENR |= (1<<4);	//Enable clocks TIM6
	TIM6 -> PSC |= 7;			//DIVISOR/8 8MHz/8 = 1MHz
	TIM6 -> ARR = 50000 - 1;	//50,000 CUENTAS DE 1us

	TIM6->CR1|=1; //Timer on
	TIM6->DIER|=1; //Module Interrupt enable
	NVIC->ISER[0]=(1<<17); //NVIC Interrupt set enable register

}

void TIM6_DAC_IRQHandler(void)
{
	//ADC1->CHSELR|=(1<<2);
	TIM6->SR = 0;
	ADC1->CR|= (1<<2);
}


int main(void)
{
	ADC_init();
	UART_init();
	TIM6_init();

	while(1);

}
