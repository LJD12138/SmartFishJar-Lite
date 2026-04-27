#include "MD_Light/md_light_iface.h"

#if(boardLIGHT_EN)
#include "MD_Light/md_light_task.h"

/*****************************************************************************************************************
-----函数功能    RGBW照明GPIO+定时器初始化（TIMER3 PB6/PB7/PB8/PB9）
******************************************************************************************************************/
void vLight_IfaceInit(void)
{
    timer_oc_parameter_struct timer_ocinitpara;
    timer_parameter_struct timer_initpara;

    rcu_periph_clock_enable(lightW_GPIO_RCU);  // GPIOB
    /* PB6=CH0暖白, PB7=CH1蓝, PB8=CH2绿, PB9=CH3红 */
    gpio_init(lightW_GPIO_PORT, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, lightW_PIN);
    gpio_init(lightB_GPIO_PORT, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, lightB_PIN);
    gpio_init(lightG_GPIO_PORT, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, lightG_PIN);
    gpio_init(lightR_GPIO_PORT, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, lightR_PIN);

    rcu_periph_clock_enable(lightTIMER_RCU);
    timer_deinit(lightTIMER);

    timer_struct_para_init(&timer_initpara);
    timer_initpara.prescaler         = lightPWM_PSC - 1;
    timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;
    timer_initpara.counterdirection  = TIMER_COUNTER_UP;
    timer_initpara.period            = lightPWM_MAX_VALUE - 1;
    timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;
    timer_initpara.repetitioncounter = 0;
    timer_init(lightTIMER, &timer_initpara);

    timer_ocinitpara.outputstate  = TIMER_CCX_ENABLE;
    timer_ocinitpara.ocpolarity   = TIMER_OC_POLARITY_HIGH;
    timer_ocinitpara.ocidlestate  = TIMER_OC_IDLE_STATE_LOW;

    /* CH0 - 暖白光 */
    timer_channel_output_config(lightTIMER, lightW_TIMER_CH, &timer_ocinitpara);
    timer_channel_output_pulse_value_config(lightTIMER, lightW_TIMER_CH, 0);
    timer_channel_output_mode_config(lightTIMER, lightW_TIMER_CH, TIMER_OC_MODE_PWM0);
    timer_channel_output_shadow_config(lightTIMER, lightW_TIMER_CH, TIMER_OC_SHADOW_DISABLE);

    /* CH1 - 蓝光 */
    timer_channel_output_config(lightTIMER, lightB_TIMER_CH, &timer_ocinitpara);
    timer_channel_output_pulse_value_config(lightTIMER, lightB_TIMER_CH, 0);
    timer_channel_output_mode_config(lightTIMER, lightB_TIMER_CH, TIMER_OC_MODE_PWM0);
    timer_channel_output_shadow_config(lightTIMER, lightB_TIMER_CH, TIMER_OC_SHADOW_DISABLE);

    /* CH2 - 绿光 */
    timer_channel_output_config(lightTIMER, lightG_TIMER_CH, &timer_ocinitpara);
    timer_channel_output_pulse_value_config(lightTIMER, lightG_TIMER_CH, 0);
    timer_channel_output_mode_config(lightTIMER, lightG_TIMER_CH, TIMER_OC_MODE_PWM0);
    timer_channel_output_shadow_config(lightTIMER, lightG_TIMER_CH, TIMER_OC_SHADOW_DISABLE);

    /* CH3 - 红光 */
    timer_channel_output_config(lightTIMER, lightR_TIMER_CH, &timer_ocinitpara);
    timer_channel_output_pulse_value_config(lightTIMER, lightR_TIMER_CH, 0);
    timer_channel_output_mode_config(lightTIMER, lightR_TIMER_CH, TIMER_OC_MODE_PWM0);
    timer_channel_output_shadow_config(lightTIMER, lightR_TIMER_CH, TIMER_OC_SHADOW_DISABLE);

    timer_auto_reload_shadow_enable(lightTIMER);
    timer_enable(lightTIMER);

    /* 默认全关 */
    lightW_PWM_SET(0);
    lightB_PWM_SET(0);
    lightG_PWM_SET(0);
    lightR_PWM_SET(0);

    tLight.usWarm = 0;
    tLight.usBlue = 0;
    tLight.usGreen = 0;
    tLight.usRed = 0;
}

#endif  //boardLIGHT_EN
