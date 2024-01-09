
#include <stdint.h>
#include <stm32f051x8.h>
// Funcionalidad del programa: Leer el ángulo de inclinación del MPU cada cierto tiempo.

#define SLAVE_ADDRESS 0x68
#define YOUT_LOW_REGISTER 0x46
#define YOUT_HIGH_REGISTER 0x45
#define WHO_AM_I 0x75 //me devolvió un 0x68 como dice el manual
#define I2C_SLV0_DO 0x63 //Data out when slave 0 is set to write debe ser 0 en reset
#define GYRO_SENSITIVITY 131.0

volatile uint8_t lowByte, highByte;
volatile int16_t completeData;
volatile float degreesPerSecond;
volatile float timeInterval;
volatile float angle;
volatile uint8_t switchvar=1;
volatile unsigned char mensaje[] = {"º___\r\n"};
volatile unsigned char i = 1;

void I2C2_init(void){
	RCC->AHBENR |= (1<<18); 	// Enable GPIOB internal clock
	GPIOB->MODER |= (2<<20) + (2<<22);	// Set P10 & P11 as alternate function
			// GPIOB->AFR[1] is not necessary as P10 & P11 default AF is SCL & SDA
			// GPIOB->PUPDR |= (1<<16) + (1<18);	// Set pins as pull-up, as the master can not pull up the line
			// GPIOB->OSPEEDR |= (1<<16) + (1<18); // Set speed as medium
			// Not necessary as it commonly is pulled-up by external resistors (ex 4.7 kΩ)
	GPIOB->AFR[1]=(1<<12)+(1<<8);

	RCC->APB1ENR |= (1<<22); 	// Enable I2C2 internal clock

	I2C2->CR1 = 0;				// Clear control registry (initialization)
	I2C2->CR2 = 0;				// Clear control registry (initialization)

	//I2C2->CR1 &= ~(1<<12);		// Set ANFOFF (analog filter on) (supresses spikes with a pulse width 50ns)
	//I2C1->CR1 |= (0<<8);		// Set DNF (Digital noise filter) (programmable filters)

	I2C2->TIMINGR |= (uint32_t)0x10220F13;
	// PRESC=1 (8 MHz/2),
	// SCLDEL = 2 = 2*250ns=500 (100 ns minimo)
	// SDADEL = 2 = 3*250ns=750 (0 ns minimo)
	// SCLH = 0x0F = 15+1 = 16 * 250ns = 4 us
	// SCLL = 0x13 = 19+1 = 20 * 250ns = 5 us

	I2C2->CR1 |= I2C_CR1_PE; // Initialize I2C PE


}

void TIM6_init(unsigned int ms){
	RCC->APB1ENR |= RCC_APB1ENR_TIM6EN; // Enable TIM6 clock

	TIM6->PSC = 8000-1; 				// Set Preescaler | 8 MHz -> 1 kHz
	TIM6->ARR = (ms);					// Set Autoreload value to ms to wait

	TIM6->CR1 |= TIM_CR1_CEN;			// Enable TIM6

	TIM6->DIER |= (1<<0);				// Update Interrupt enable
	NVIC->ISER[0] = (1<<17);			// Interrupt enable in NVIC

	timeInterval = (float)1.000 / (float)(1000.000/((float)ms));
}

void USART_init(unsigned int baudrate){
	RCC->AHBENR |= (1<<17);				// Clk GPIOA
	RCC->APB2ENR |= (1<<14);			// Clk USART1

	GPIOA->MODER |= (2<<20)+(2<<18);	//PORTA9 & PORTA10 Funcion alterna
	GPIOA->AFR[1] |= (1<<4)+(1<<8);		//PORTA9 & PORTA10 USART1_TX, USART1_RX

	USART1->BRR |= baudrate;			//CLK UART / Baud Rate -> 8MHz / 9600
	USART1->CR1 |= (1<<0)+(1<<3)+(1<<6);	// UE=TE=TCIE=1
}

uint8_t I2C_read(uint8_t register_address){
	uint8_t temp;

	I2C2->CR2 = 0; // Restart CR2


	//un byte a leer y dirección del esclavo
	I2C2->CR2 |= (1 << 16) | (SLAVE_ADDRESS << 1);

	I2C2->CR2 |= I2C_CR2_START; // Start generation


	// Wait until TXE is set
    do{} while ((I2C2->ISR & I2C_ISR_TXE) == 0);


	/* Send the register address
	 * que en este caso es Yout High y Yout low
	 */
	I2C2->TXDR = register_address;

	//Writing 1 to this bit clears the STOPF flag in the I2C_ISR register.
	I2C2->ICR |= I2C_ICR_STOPCF;

	//que se transmita todoo
	//voy a comentar la líne ade abajo para ver si pasa algo
    do{} while ((I2C2->ISR & I2C_ISR_TC) == 0);

	// Start a read operation, ahora que ya se tiene el registro
    // I2C_CR2_RD_WRN 1: Master requests a read transfer.
	I2C2->CR2 |= (1<<16) | (SLAVE_ADDRESS << 1) | I2C_CR2_RD_WRN;
	I2C2->CR2 |= I2C_CR2_START;


	//Receive data register not empty (receivers)
	//tambieén esta la voy a comentar
	do{} while((I2C2->ISR & I2C_ISR_RXNE) == 0);

	temp = I2C2->RXDR; //lectura de Yout HIGH o LOW

	//esperar a que ya esté todoo
	//voya comentar todos los whiles de lectura
    do{} while ((I2C2->ISR & I2C_ISR_TC) == 0);

    //1: Stop generation after current byte transfer
	I2C2->CR2 |= I2C_CR2_STOP;

	//Writing 1 to this bit clears the STOPF flag in the I2C_ISR register.
    I2C2->ICR |= I2C_ICR_STOPCF;

    return temp;
}

void I2C_write(uint8_t registerAddress, uint8_t data){
	I2C2->CR2 = 0; // Restart CR2

	I2C2->CR2 |= (2 << 16) | (SLAVE_ADDRESS << 1); //2 bytes a transmitir
	//dirección del esclavo

	I2C2->CR2 |= I2C_CR2_START; //(1<<13)

	// Wait until TXE is set
	do{} while ((I2C2->ISR & I2C_ISR_TXE) == 0);

	// Send the register address
	I2C2->TXDR = registerAddress;

	// Wait until TXE is set
	do{} while ((I2C2->ISR & I2C_ISR_TXE) == 0);

	// Send the register address
	I2C2->TXDR = data;

	//hacer nada hasta que ya se haya enviado TODO, Address y dato
	do{} while ((I2C2->ISR & I2C_ISR_TC) == 0);


	//Stop generation (master mode)after current byte transfer
	I2C2->CR2 |= I2C_CR2_STOP;

	// STOP detection flag clear
	/*no se muy bien para que es este porque no usa interrupciones de I2c*/
	I2C2->ICR |= I2C_ICR_STOPCF;
}

void TIM6_DAC_IRQHandler(){
	TIM6->SR = 0; //bajar bandera
	switch(switchvar){
		case 1:
			//leer los dos bytes que componen Yout LOW
			lowByte =I2C_read(YOUT_LOW_REGISTER);// I2C_read(YOUT_LOW_REGISTER);
			switchvar=2;
			break;
		case 2:
			//leer los bytes que compenen Yout HIGH
			highByte =I2C_read(YOUT_HIGH_REGISTER);// I2C_read(WHO_AM_I);
			switchvar=1;
			break;
	}
	completeData = lowByte | (highByte<<8); //juntar los bytes y obtener el número

	if(completeData >= 32767) {
		completeData += 32768; //valores negativos es el complemento
	}


	/*
	 * La fórmula de abajo viene de la hoja de datos y se despejó
	 * Y_angular_rate
	 */
	degreesPerSecond = (float)completeData / GYRO_SENSITIVITY;




	angle += degreesPerSecond * 200;
	// Ex angle = 122.134


	// Poner el ángulo en variable mensaje
	uint8_t integerPart = (int)angle;  // 122
	float decimalPart = angle - integerPart; // 0.134

	uint8_t decimalPlaces = 2;
	for(i=0 ; i < decimalPlaces ; i++) {
		decimalPart *= 10;
	}
	decimalPart = (int)decimalPart;
	// decimalPart = 13

	uint8_t a = 6;
	do{
		mensaje[a--] = ((int)decimalPart % 10) + '0';
		decimalPart /= 10;
	} while(decimalPart > 0);

	mensaje[a--] = '.';
	// 'º___.13 \r\n'

	do{
		mensaje[a--] = (integerPart % 10) + '0';
		integerPart /= 10;
	} while(integerPart > 0);
	// 'º122.13 \r\n'

	NVIC->ISER[0] |= (1<<27); //se habilita NVIC de USART
}

void USART1_IRQHandler (void) {
	USART1->TDR = mensaje[i++];

	if (mensaje[i]==0) {
		NVIC->ICER[0]=(1<<27);
		i = 0;
	}
}

void MPU_wakeup(){
	uint8_t registerAddress = 0x6B;
	uint8_t data = 0x00;
	I2C_write(registerAddress, data);
}

int main(void) {
	I2C2_init();
	MPU_wakeup();

	USART_init(833); // 8MHz / 9600 = 833
	TIM6_init(200);

	for(;;);
}

