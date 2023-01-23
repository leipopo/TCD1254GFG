#include "main.h"
#define sclk htim3
#define basetick_period 500 // 500ns
#define t1 5000 // 500ns 
#define t2 500 // 500ns
#define t3 t1
#define t4 20 

#define scanstart HAL_GPIO_WritePin(sh_iogroup, sh_io, GPIO_PIN_RESET); \
    HAL_TIM_Base_Start_IT(&sclk)

#define scanstop HAL_TIM_Base_Stop_IT(&sclk)



#define sh_iogroup GPIOA
#define sh_io GPIO_PIN_5
#define icg_iogroup GPIOA
#define icg_io GPIO_PIN_7

void pwmgenerate();
extern int32_t
