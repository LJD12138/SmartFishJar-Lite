#include "MD_HeatManage/md_hm_iface.h"
#include "MD_HeatManage/md_hm_task.h"

/*****************************************************************************************************************
-----函数功能    风扇/加热棒 GPIO初始化（TIMER2全重映射 PC6-PC9）
-----说明(备注)  TIMER2 Init 唯一入口，O2Pump仅配置通道不重复初始化定时器
-----传入参数    none
-----输出参数    none
-----返回值      none
******************************************************************************************************************/
static void v_fan_gpio_init(void)
{
    timer_oc_parameter_struct timer_ocinitpara;
    timer_parameter_struct timer_initpara;

    rcu_periph_clock_enable(RCU_AF);
    rcu_periph_clock_enable(hmTIMER_RCU);
    rcu_periph_clock_enable(RCU_GPIOC);

    /* TIMER2 全重映射 -> PC6/PC7/PC8/PC9 */
    gpio_pin_remap_config(GPIO_TIMER2_FULL_REMAP, ENABLE);

    /* 加热棒 PC6 = TIMER2_CH0 */
    gpio_init(heatPWM_GPIO_PORT, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, heatPWM_PIN);
    /* O2泵 PC7 = TIMER2_CH1（引脚只初始化，通道由O2Pump模块配置）*/
    gpio_init(GPIOC, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_7);
    /* 风扇2 PC8 = TIMER2_CH2 */
    gpio_init(fanPWM2_GPIO_PORT, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, fanPWM2_PIN);
    /* 风扇1 PC9 = TIMER2_CH3 */
    gpio_init(fanPWM1_GPIO_PORT, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, fanPWM1_PIN);

    /* TIMER2 初始化（整个TIMER2唯一初始化点） */
    timer_deinit(hmTIMER);
    timer_struct_para_init(&timer_initpara);
    timer_initpara.prescaler         = hmPWM_PSC - 1;
    timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;
    timer_initpara.counterdirection  = TIMER_COUNTER_UP;
    timer_initpara.period            = hmPWM_MAX_VALUE - 1;
    timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;
    timer_initpara.repetitioncounter = 0;
    timer_init(hmTIMER, &timer_initpara);

    /* 配置4个PWM通道 */
    timer_ocinitpara.outputstate  = TIMER_CCX_ENABLE;
    timer_ocinitpara.ocpolarity   = TIMER_OC_POLARITY_HIGH;
    timer_ocinitpara.ocidlestate  = TIMER_OC_IDLE_STATE_LOW;

    /* CH0 - 加热棒 */
    timer_channel_output_config(hmTIMER, TIMER_CH_0, &timer_ocinitpara);
    timer_channel_output_pulse_value_config(hmTIMER, TIMER_CH_0, 0);
    timer_channel_output_mode_config(hmTIMER, TIMER_CH_0, TIMER_OC_MODE_PWM0);
    timer_channel_output_shadow_config(hmTIMER, TIMER_CH_0, TIMER_OC_SHADOW_DISABLE);

    /* CH1 - O2泵（由O2Pump模块使用，此处配置为0） */
    timer_channel_output_config(hmTIMER, TIMER_CH_1, &timer_ocinitpara);
    timer_channel_output_pulse_value_config(hmTIMER, TIMER_CH_1, 0);
    timer_channel_output_mode_config(hmTIMER, TIMER_CH_1, TIMER_OC_MODE_PWM0);
    timer_channel_output_shadow_config(hmTIMER, TIMER_CH_1, TIMER_OC_SHADOW_DISABLE);

    /* CH2 - 风扇2 */
    timer_channel_output_config(hmTIMER, TIMER_CH_2, &timer_ocinitpara);
    timer_channel_output_pulse_value_config(hmTIMER, TIMER_CH_2, 0);
    timer_channel_output_mode_config(hmTIMER, TIMER_CH_2, TIMER_OC_MODE_PWM0);
    timer_channel_output_shadow_config(hmTIMER, TIMER_CH_2, TIMER_OC_SHADOW_DISABLE);

    /* CH3 - 风扇1 */
    timer_channel_output_config(hmTIMER, TIMER_CH_3, &timer_ocinitpara);
    timer_channel_output_pulse_value_config(hmTIMER, TIMER_CH_3, 0);
    timer_channel_output_mode_config(hmTIMER, TIMER_CH_3, TIMER_OC_MODE_PWM0);
    timer_channel_output_shadow_config(hmTIMER, TIMER_CH_3, TIMER_OC_SHADOW_DISABLE);

    timer_auto_reload_shadow_enable(hmTIMER);
    timer_enable(hmTIMER);

    heatPWM_SET(0);
    fanPWM1_SET(0);
    fanPWM2_SET(0);
}

/*****************************************************************************************************************
-----函数功能    风扇/加热棒 PWM初始化入口
-----说明(备注)  none
******************************************************************************************************************/
void vFan_PwmInit(void)
{
    v_fan_gpio_init();
}

#if(boardLOW_POWER)
void vFan_IoEnterLowPower(void)
{
    rcu_periph_clock_enable(RCU_GPIOC);
    gpio_init(GPIOC, GPIO_MODE_AIN, GPIO_OSPEED_2MHZ, heatPWM_PIN);
    gpio_init(GPIOC, GPIO_MODE_AIN, GPIO_OSPEED_2MHZ, GPIO_PIN_7);
    gpio_init(GPIOC, GPIO_MODE_AIN, GPIO_OSPEED_2MHZ, fanPWM2_PIN);
    gpio_init(GPIOC, GPIO_MODE_AIN, GPIO_OSPEED_2MHZ, fanPWM1_PIN);
    rcu_periph_clock_disable(hmTIMER_RCU);
    timer_disable(hmTIMER);
}
#endif


