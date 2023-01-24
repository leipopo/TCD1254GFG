#ifndef _TCD_DRIVER_H_
#define _TCD_DRIVER_H_

#include "tim.h"
#include "adc.h"
#define sclk htim3
#define mastertick_period 500 // 500ns
#define t1 15000 // 15000ns 
#define t2 500 // 500ns
#define t3 5000 // 5000ns
#define t4 20 // 20ns

#define sh_iogroup GPIOA
#define sh_io GPIO_PIN_5
#define icg_iogroup GPIOA
#define icg_io GPIO_PIN_7

typedef struct tcd_data {
    int32_t sh_tick;
    int32_t master_tick;   
    float voltage[2500];
} tcddata;

void scanstart(void);
void TCD_RW(tcddata *t,float os_voltage_vrefint_proportion);
float init_vrefint_reciprocal(void);
static float get_os_signal(float voltage_vrefint_proportion);
static uint16_t adcx_get_chx_value(ADC_HandleTypeDef *ADCx, uint32_t ch);
#endif 

