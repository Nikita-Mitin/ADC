#include"stm32f0xx.h"

void GPIO_Intit(void);
void ADC_Init(void);
uint16_t ADCGetResult(uint16_t, uint16_t);
uint32_t TemperatureGetData(uint16_t);

int main(void){

	ADC_Init();
	ADC->CCR |= ADC_CCR_TSEN;

	uint16_t ADCresult[2];
	uint32_t temp;

	while(1){
		ADCresult[0] = ADCGetResult(2, 0);
		ADCresult[1] = ADCGetResult(2, 1);
		temp = TemperatureGetData(ADCresult[1]);
	}
}


void GPIO_Intit(void){
	RCC->AHBENR |= RCC_AHBENR_GPIOCEN;
	GPIOC->MODER |= GPIO_MODER_MODER9_0
				|GPIO_MODER_MODER8_0;
}

void ADC_Init(void){

	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;  					// тактирование на порт GPIOA
	GPIOA->MODER |= GPIO_MODER_MODER1;  					// аналог вход;
	RCC->APB2ENR |= RCC_APB2ENR_ADC1EN; 					// тактирование на ADC

	ADC1->CFGR2 |= ADC_CFGR2_CKMODE_1;  					// Делитель на 4 (48/4 = 12) ADC <= 14 мГц

	/*____Калибровка_____*/
	if ((ADC1->CR & ADC_CR_ADEN) != 0)
	{
	 ADC1->CR |= ADC_CR_ADDIS;
	}
	while ((ADC1->CR & ADC_CR_ADEN) != 0){}
	ADC1->CFGR1 &= ~ADC_CFGR1_DMAEN;
	ADC1->CR |= ADC_CR_ADCAL;
	while ((ADC1->CR & ADC_CR_ADCAL) != 0){}
	/*__Конец колибровки_*/

	ADC1->CR |= ADC_CR_ADEN;

	ADC1->SMPR |= ADC_SMPR1_SMPR_0
				 |ADC_SMPR1_SMPR_1
			     |ADC_SMPR1_SMPR_2; 						//0x111 239.5 cycles
	ADC1->CHSELR |= ADC_CHSELR_CHSEL16|ADC_CHSELR_CHSEL1;						//IN1

}

uint16_t ADCGetResult(uint16_t quantity, uint16_t count){
	ADC1->CR |= ADC_CR_ADSTART;
	uint16_t ADCResult[count];
	for(uint16_t i = 0; i < quantity; i++){
		while ((ADC1->ISR & ADC_ISR_EOC) == 0);
		ADCResult[i] = ADC1->DR;
	}
	return ADCResult[count];
}

uint32_t TemperatureGetData(uint16_t ADCResult){
	#define TEMP110_CAL_ADDR ((uint16_t*) ((uint32_t) 0x1FFFF7C2))
	#define TEMP30_CAL_ADDR ((uint16_t*) ((uint32_t) 0x1FFFF7B8))
	#define VDD_CALIB ((uint16_t) (330))
	#define VDD_APPLI ((uint16_t) (300))
	int32_t temperature; /* will contain the temperature in degrees Celsius */
	temperature = (((int32_t) ADCResult * VDD_APPLI / VDD_CALIB) - (int32_t) *TEMP30_CAL_ADDR );
	temperature = temperature * (int32_t)(110 - 30);
	temperature = temperature / (int32_t)(*TEMP110_CAL_ADDR - *TEMP30_CAL_ADDR);
    return temperature + 30;

}
