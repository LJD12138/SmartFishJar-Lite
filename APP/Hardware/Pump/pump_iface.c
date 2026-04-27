#include "Pump/pump_iface.h"

#if(boardWATER_PUMP_EN)
#include "Pump/pump_task.h"

/*****************************************************************************************************************
-----函数功能    水泵 GPIO + TIMER0 初始化（PA8 = TIMER0_CH0）
******************************************************************************************************************/
void vPump_IfaceInit(void)
{
    timer_oc_parameter_struct timer_ocinitpara;
    timer_parameter_struct timer_initpara;

    rcu_periph_clock_enable(pumpPWM_GPIO_RCU);
    gpio_init(pumpPWM_GPIO_PORT, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, pumpPWM_PIN);

    rcu_periph_clock_enable(pumpTIMER_RCU);
    timer_deinit(pumpTIMER);

    timer_struct_para_init(&timer_initpara);
    timer_initpara.prescaler         = pumpPWM_PSC - 1;
    timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;
    timer_initpara.counterdirection  = TIMER_COUNTER_UP;
    timer_initpara.period            = pumpPWM_MAX_VALUE - 1;
    timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;
    timer_initpara.repetitioncounter = 0;
    timer_init(pumpTIMER, &timer_initpara);

    timer_ocinitpara.outputstate  = TIMER_CCX_ENABLE;
    timer_ocinitpara.ocpolarity   = TIMER_OC_POLARITY_HIGH;
    timer_ocinitpara.ocidlestate  = TIMER_OC_IDLE_STATE_LOW;
    timer_ocinitpara.outputnstate = TIMER_CCXN_DISABLE;
    timer_ocinitpara.ocnpolarity  = TIMER_OCN_POLARITY_HIGH;
    timer_ocinitpara.ocnidlestate = TIMER_OCN_IDLE_STATE_LOW;

    timer_channel_output_config(pumpTIMER, pumpTIMER_CH, &timer_ocinitpara);
    timer_channel_output_pulse_value_config(pumpTIMER, pumpTIMER_CH, 0);
    timer_channel_output_mode_config(pumpTIMER, pumpTIMER_CH, TIMER_OC_MODE_PWM0);
    timer_channel_output_shadow_config(pumpTIMER, pumpTIMER_CH, TIMER_OC_SHADOW_DISABLE);

    timer_auto_reload_shadow_enable(pumpTIMER);
    timer_primary_output_config(pumpTIMER, ENABLE);  // TIMER0为高级定时器，需使能主输出
    timer_enable(pumpTIMER);

    pumpPWM_SET(0);
}

#if(boardLOW_POWER)
void vPump_IoEnterLowPower(void)
{
    rcu_periph_clock_enable(pumpPWM_GPIO_RCU);
    gpio_init(pumpPWM_GPIO_PORT, GPIO_MODE_AIN, GPIO_OSPEED_2MHZ, pumpPWM_PIN);
    rcu_periph_clock_disable(pumpTIMER_RCU);
    timer_disable(pumpTIMER);
}
#endif

#endif  //boardWATER_PUMP_EN
