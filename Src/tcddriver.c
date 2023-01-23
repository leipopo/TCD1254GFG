#include "tcddriver.h"

/*
此驱动将tcd需要的三路时钟通过定时器中断计数并反转io形成PWM脉冲实现三路PWM所要求的百纳秒级timing
*/

/*
此函数通过空指令循环来延时ns纳秒
ARM CortexM3 标称指令速度1.25MIPS/MHz
此代码mcu主频通过HAL_RCC_GetSysClockFreq()获得，为72Mhz
一秒执行1.25*72*10^6条指令
一条指令执行时间为1x10^9/(1.25*72*10^6)ns
所以延时ns纳秒需要执行ns/(10^9/(1.25*72*10^6))条指令
然后向下取整
*/
void delay_ns(int16_t ns)
{
    int32_t mcufreq = HAL_RCC_GetSysClockFreq();
    ns              = (int32_t)((float)ns * (float)mcufreq) * 1.25f / (1000000000.f);
    for (int32_t i = 0; i < ns; i++) {
        __NOP();
    }
}


int32_t basetick;
void pwmgenerate()
{
    if (basetick == 0) {
        delay_ns(basetick_period - t4);//手册要求的t4 timing
        HAL_GPIO_WritePin(icg_iogroup, icg_io, GPIO_PIN_SET);//拉高icg_io开始准备读取数据
    } else if ((basetick * basetick_period % t3 == 0) && (basetick <= 2547))//一个读取周期里，计数M_io的pwm周期次数来反转sh_io实现同步
     {
        HAL_GPIO_TogglePin(sh_iogroup, sh_io);
    } else if (basetick > 2547) //此时sh_io已经被拉低1000ns后被拉高，可以在此时拉低icg_io
    {
        HAL_GPIO_WritePin(icg_iogroup, icg_io, GPIO_PIN_RESET);
        scanstop;
    }
    basetick++;
}
