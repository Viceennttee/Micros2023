#include "stm32f0xx.h"
#include "uart_3de11.h"
unsigned char mensaje[] = {"r/m ###\r\n"};
unsigned char i = 0;

void USART1_IRQHandler (void)
{
	USART1->TDR = mensaje[i++]; //Contains the data character to be transmitted
	if (mensaje[i]==0) {
		i = 0;//si si la cadena esta vacía comenzar de nuevo, creo
	}
}
void uart_3de11(void)
{
	RCC->AHBENR|=(1<<17);	//CLK PORTA
	RCC->APB2ENR|=(1<<14);	//USART 1

	GPIOA->MODER|=(2<<20)+(2<<18);	//PORTA9 & PORTA10 Función alterna
	GPIOA->AFR[1]|=(1<<4)+(1<<8);	//PORTA9 & PORTA10 USART1_TX, USART1_RX

	USART1->BRR|=5000;				//CLK UART / Baud Rate -> 8MHz / 9600, se supone
	//para que funcione en la terminal del celular debe estar a 5000

	USART1->CR1|=(1<<0)+(1<<3)+(1<<2)+(1<<6);	/*TE=RE=USART_Enable=1,
	 Bit 6 hab interrupción transmisión
	 */
}
