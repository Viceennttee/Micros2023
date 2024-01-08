#include "stm32f0xx.h"
#include "i2c_examen.h"
volatile int dato_leido;
void i2c_examen() {
    // Habilita el reloj del periférico I2C1
	RCC->APB1ENR|=(1<<22);				//IIC2
	RCC->AHBENR|=(1<<18);				//GPIOB
	GPIOB->MODER|=(2<<22)+(2<<20);		//ALT function GPIOB11-GPIOB10
	GPIOB->AFR[1]=(1<<12)+(1<<8);		//ALTF1

    // Configura la velocidad de I2C (por ejemplo, 100 kHz)
	I2C2->TIMINGR = (uint32_t)0x10220F13;		// PRESC=1 (8 MHz/2),
													// SCLDEL = 2 = 2*250ns=500 (100 ns minimo)
													// SDADEL = 2 = 3*250ns=750 (0 ns minimo)
													// SCLH = 0x0F = 15+1 = 16 * 250ns = 4 us
													// SCLL = 0x13 = 19+1 = 20 * 250ns = 5 us

    // Habilita I2C y el modo de reconocimiento
    I2C1->CR1 = I2C_CR1_PE;

	I2C2->CR2 |= (1 << 16); //1 byte a transmitir, recibir

    // Configura la dirección del esclavo IMU
    I2C1->CR2 = (0x68 << 1); // 0x68 es del IMU

    I2C1->CR1 |= I2C_CR1_TCIE | I2C_CR1_ERRIE; //interrupción cuando se completa
    NVIC_EnableIRQ(I2C1_IRQn);

}

void I2C1_IRQHandler() {
    if (I2C1->ISR & I2C_ISR_TC) {
    	dato_leido=I2C1->RXDR;//asignar variable a valor leído de IMU
        I2C1->ICR = I2C_ICR_ADDRCF; //BANDERA
    }

}




