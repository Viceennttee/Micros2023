#include "stm32f0xx.h"
#include "ADC_examen.h"

volatile unsigned short ADC_result, adc_voltaje_mV;
volatile unsigned short joy1_PWM, joy2_PWM, volt_B;

void ADC1_COMP_IRQHandler(void)
{
	if (ADC1->CHSELR & (1<<1)) {
	    // Canal 1 joystick,
		ADC_result = ADC1->DR; //lectura del ADC en PA1
		adc_voltaje_mV = ADC_result*3300/4095; //pasar el ADC a mV
		joy1_PWM=ADC_result-2048; //variable a enviar al PWM menos el 1.6



		ADC1->CHSELR = (1<<2); // Habilitar el canal 2(PA1) amperaje

	} else if (ADC1->CHSELR & (1<<2)) {
	            // Canal 2 joystick2
		ADC_result= ADC1->DR; //lectura del ADC en PA2
		adc_voltaje_mV = ADC_result*3300/4095; //pasar el ADC a mV
		joy2_PWM=ADC_result-2048;  //variable a enviar al PWM menos el 1.6




		ADC1->CHSELR = (1<<3); // Habilitar el canal 3 (PA3) voltaje de batería

	} else if (ADC1->CHSELR & (1<<3)) {
        // Canal 2 Voltimetro
        ADC_result= ADC1->DR; //lectura del ADC en PA3
    	adc_voltaje_mV = ADC_result*3300/4095;
		volt_B=adc_voltaje_mV*(1200+15000)/12000; //Voltaje de la batería en V, divisor

        ADC1->CHSELR = (1<<1); // Habilitar el joystick1 de nuevo

	}
	NVIC->ISER[0]|=(1<<27); //se habilita NVIC de USART
	USART1 -> CR1 |= (1<<3)+(1<<2)+(1<<0)+(1<<6);	//HABILITAMOS EL Tx, Rx, EL UART Y
	//la interrupción por  Transmission complete interrupt enable

}

void ADC_examen()
{
	RCC->AHBENR|= (1<<17); // PORTA Clock joystick, sensor de corriente y voltaje
	RCC->AHBENR|= (1<<19); // PORTC Clock temperatura
	RCC->APB2ENR |= (1<<9); // enable clocks for ADC1 clock

	GPIOA->MODER|=(3<<2); // PA1 joystick 1
	GPIOA->MODER|=(3<<4);  ///PA2 joystick 2
	GPIOA->MODER|=(3<<6); // PA3 voltaje de la batería

	ADC1->CFGR1|=(1<<26)+(1<<27); // ADC_IN1 MUX selected, single conv, SW trigger//Right alignment, 12 bits
	ADC1->CHSELR|=(1<<1);// se empieza ADC en el canal 1 (PA1)joystick1

	ADC1->IER|=(1<<2); //habilitación de interrupción en EOC (End of conversion;
	NVIC->ISER[0]=(1<<12); //NVIC del ADC

	ADC1->CR|=1; //habilitación del ADC;
	do{} while ((ADC1->ISR & 1)==0); //while ADC not ready
	/*ADC1->ISR|=1;
	 * Creo que esta línea es inncesesaria en la inicialización
	 */
}
