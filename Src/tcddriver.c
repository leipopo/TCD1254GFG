#include "tcddriver.h"

/*
此驱动将发生tcd需要的三路PWM并将OS信号进行AD转换
*/

void scanstart(void)
{
    HAL_GPIO_WritePin(sh_iogroup, sh_io, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(icg_iogroup, icg_io, GPIO_PIN_RESET);
    HAL_TIM_Base_Start_IT(&sclk);
}

static void scanstop(void)
{
    HAL_TIM_Base_Stop_IT(&sclk);
}

/*
此函数通过空指令循环来延时ns纳秒
ARM CortexM3 标称指令速度1.25MIPS/MHz
此代码mcu主频通过HAL_RCC_GetSysClockFreq()获得，为72Mhz
一秒执行1.25*72*10^6条指令
一条指令执行时间为1x10^9/(1.25*72*10^6)ns
所以延时ns纳秒需要执行ns/(10^9/(1.25*72*10^6))条指令
然后向下取整
*/
static void delay_ns(int16_t ns)
{
    int32_t mcufreq = HAL_RCC_GetSysClockFreq();
    ns              = (int32_t)((float)ns * (float)mcufreq) * 1.25f / (1000000000.f);
    for (int32_t i = 0; i < ns; i++) {
        __NOP();
    }
}

/*
此函数通过定时器中断计数并反转io形成PWM脉冲实现三路PWM所要求的百纳秒级timing,并在规定的sh周期读取os信号
*/

void TCD_RW(tcddata *t,float os_vvp)
{
    if (t->sh_tick == 0 && t->master_tick == 0) {
        delay_ns(mastertick_period / 2 - t4);
        HAL_GPIO_WritePin(icg_iogroup, icg_io, GPIO_PIN_SET);//拉高icg开始一个读取周期
    }

    if ((t->master_tick + 10) % 40 == 0) {
        HAL_GPIO_WritePin(sh_iogroup, sh_io, GPIO_PIN_SET);
    } else if (t->master_tick % 40 == 20) {
        HAL_GPIO_WritePin(sh_iogroup, sh_io, GPIO_PIN_RESET);
        t->sh_tick++;
    }//sh周期低电平持续30个master周期，高电平持续10个master周期，故每40个master周期切换一次

    if (t->sh_tick > 32 && t->sh_tick < 2533) // os输出区间为33--2532共2500个sh周期，故在此区间内读取os信号
    {
        t->voltage[t->sh_tick - 32] = get_os_signal(os_vvp);
    }
    t->master_tick++;

    if (t->sh_tick > 2560) // 2547个sh周期时即读取完毕，但此时sh在高电平按手册要求需在sh上升至高电平前100ns--1000ns内拉低icg结束读取，故继续运行至此2547周期结束拉低icg
    {
        t->sh_tick     = 0;
        t->master_tick = 0;
        delay_ns(t2);
        HAL_GPIO_WritePin(icg_iogroup, icg_io, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(sh_iogroup, sh_io, GPIO_PIN_RESET);
        scanstop();
    }
}

/*
此函数为对内部参考电压电压 VREFINT 进行 adc 采样将其作为校准值
*/

float init_vrefint_reciprocal(void)
{
    uint8_t i          = 0;
    uint32_t total_adc = 0;
    for (i = 0; i < 200; i++) {
        total_adc += adcx_get_chx_value(&hadc1, ADC_CHANNEL_VREFINT);
    }
    return 200 * 1.2f / total_adc;
}

/*
此函数为OS信号的AD转换
*/
static float get_os_signal(float voltage_vrefint_proportion)
{
    uint16_t adc = adcx_get_chx_value(&hadc1, ADC_CHANNEL_4);
    return adc * voltage_vrefint_proportion;
}

static uint16_t adcx_get_chx_value(ADC_HandleTypeDef *ADCx, uint32_t ch)
{
    static ADC_ChannelConfTypeDef sConfig = {0};
    sConfig.Channel = ch;
    sConfig.Rank = 1;
    sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;

    if (HAL_ADC_ConfigChannel(ADCx, &sConfig) != HAL_OK)
    {
        Error_Handler();
    }

    HAL_ADC_Start(ADCx);

    HAL_ADC_PollForConversion(ADCx, 10);
    return (uint16_t)HAL_ADC_GetValue(ADCx);

}