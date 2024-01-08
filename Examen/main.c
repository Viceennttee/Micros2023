
#include "stm32f0xx.h"

#include "ADC_examen.h"
#include "tim6leds.h"
#include "PWM1.h"
#include "PWM2.h"
#include "i2c_examen.h"

int main(void)
{
	ADC_examen();
	tim6leds();
	PWM1();
	PWM2();
	i2c_examen();
    while(1)
    {
    }
}
