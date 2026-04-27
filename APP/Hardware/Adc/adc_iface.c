#include "Adc/adc_iface.h"

#if(boardADC_EN)

u16 adc_value[ADC_CHANNEL_NUM];

/*****************************************************************************************************************
-----函数功能    IO初始化
-----说明(备注)  none
-----传入参数    none
-----输出参数    none
-----返回值      none
******************************************************************************************************************/
static void gpio_config(void)
{
    /* PA0 - VIN电流, PA1 - VIN电压, PA2 - 5V_NTC */
    rcu_periph_clock_enable(RCU_GPIOA);
    gpio_init(GPIOA, GPIO_MODE_AIN, GPIO_OSPEED_10MHZ, GPIO_PIN_0);  // adcVIN_CURR
    gpio_init(GPIOA, GPIO_MODE_AIN, GPIO_OSPEED_10MHZ, GPIO_PIN_1);  // adcVIN_VOLT
    gpio_init(GPIOA, GPIO_MODE_AIN, GPIO_OSPEED_10MHZ, GPIO_PIN_2);  // adc5V_NTC
    gpio_init(GPIOA, GPIO_MODE_AIN, GPIO_OSPEED_10MHZ, GPIO_PIN_6);  // adcWATER_NTC2
    gpio_init(GPIOA, GPIO_MODE_AIN, GPIO_OSPEED_10MHZ, GPIO_PIN_7);  // adcWATER_NTC1

    /* PB0 - 加热棒电流, PB1 - 氧气泵电流 */
    rcu_periph_clock_enable(RCU_GPIOB);
    gpio_init(GPIOB, GPIO_MODE_AIN, GPIO_OSPEED_10MHZ, GPIO_PIN_0);  // adcHEAT_CURR
    gpio_init(GPIOB, GPIO_MODE_AIN, GPIO_OSPEED_10MHZ, GPIO_PIN_1);  // adcO2_CURR

    /* PC0/1/3/4/5 - 灯光电流/12V_NTC/12V电压/光照传感器/水泵电流 */
    rcu_periph_clock_enable(RCU_GPIOC);
    gpio_init(GPIOC, GPIO_MODE_AIN, GPIO_OSPEED_10MHZ, GPIO_PIN_0);  // adcLIGHT_CURR
    gpio_init(GPIOC, GPIO_MODE_AIN, GPIO_OSPEED_10MHZ, GPIO_PIN_1);  // adc12V_NTC
    gpio_init(GPIOC, GPIO_MODE_AIN, GPIO_OSPEED_10MHZ, GPIO_PIN_3);  // adc12V_VOLT
    gpio_init(GPIOC, GPIO_MODE_AIN, GPIO_OSPEED_10MHZ, GPIO_PIN_4);  // adcLIGHT_RES
    gpio_init(GPIOC, GPIO_MODE_AIN, GPIO_OSPEED_10MHZ, GPIO_PIN_5);  // adcPUMP_CURR
}



static void delay_1ms(u16 time)
{    
   vu32 i = 0;
   while(time--)
   {
      i = 24000;  
      while(i--);    
   }
}

/*****************************************************************************************************************
-----函数功能    DMA初始化
-----说明(备注)  none
-----传入参数    none
-----输出参数    none
-----返回值      none
******************************************************************************************************************/
#if(ADC_DMAX)
static void dma_config(void)
{
	/* ADC_DMA_channel configuration */
	dma_parameter_struct dma_data_parameter;

    /* ADC DMA_channel configuration */
    dma_deinit(adcDMA, adcDMA_CH);

	 /* initialize DMA single data mode */
    dma_data_parameter.periph_addr  = (uint32_t)(&ADC_RDATA(ADCX));
    dma_data_parameter.periph_inc   = DMA_PERIPH_INCREASE_DISABLE;
    dma_data_parameter.memory_addr  = (uint32_t)(&adc_value);
    dma_data_parameter.memory_inc   = DMA_MEMORY_INCREASE_ENABLE;
    dma_data_parameter.periph_width = DMA_PERIPHERAL_WIDTH_16BIT;
    dma_data_parameter.memory_width = DMA_MEMORY_WIDTH_16BIT;  
    dma_data_parameter.direction    = DMA_PERIPHERAL_TO_MEMORY;
    dma_data_parameter.number       = ADC_CHANNEL_NUM;
    dma_data_parameter.priority     = DMA_PRIORITY_HIGH;
    dma_init(adcDMA, adcDMA_CH, &dma_data_parameter);

    /* enable DMA circulation mode */
    dma_circulation_enable(adcDMA, adcDMA_CH);

    /* enable DMA channel */
    dma_channel_enable(adcDMA, adcDMA_CH);
}
#endif

/*****************************************************************************************************************
-----函数功能    ADC初始化
-----说明(备注)  none
-----传入参数    none
-----输出参数    none
-----返回值      none
******************************************************************************************************************/
static void adc_config(void)
{
    /* ADC mode config */
	adc_mode_config(ADC_MODE_FREE); 
    /* ADC contineous function disable */
    adc_special_function_config(ADCX, ADC_CONTINUOUS_MODE, ENABLE);
    /* ADC scan mode disable */
    adc_special_function_config(ADCX, ADC_SCAN_MODE, ENABLE);
    /* ADC data alignment config */
    adc_data_alignment_config(ADCX, ADC_DATAALIGN_RIGHT);

    /* ADC channel length config */
    adc_channel_length_config(ADCX, ADC_REGULAR_CHANNEL, ADC_CHANNEL_NUM);

    /* ADC regular channel config - 12路，严格按通道号顺序 */
    adc_regular_channel_config(ADCX, 0,  adcVIN_CURR_CH,    ADC_SAMPLETIME_239POINT5); // PA0  CH0
    adc_regular_channel_config(ADCX, 1,  adcVIN_VOLT_CH,    ADC_SAMPLETIME_239POINT5); // PA1  CH1
    adc_regular_channel_config(ADCX, 2,  adc5V_NTC_CH,      ADC_SAMPLETIME_239POINT5); // PA2  CH2
    adc_regular_channel_config(ADCX, 3,  adcWATER_NTC2_CH,  ADC_SAMPLETIME_239POINT5); // PA6  CH6
    adc_regular_channel_config(ADCX, 4,  adcWATER_NTC1_CH,  ADC_SAMPLETIME_239POINT5); // PA7  CH7
    adc_regular_channel_config(ADCX, 5,  adcHEAT_CURR_CH,   ADC_SAMPLETIME_239POINT5); // PB0  CH8
    adc_regular_channel_config(ADCX, 6,  adcO2_CURR_CH,     ADC_SAMPLETIME_239POINT5); // PB1  CH9
    adc_regular_channel_config(ADCX, 7,  adcLIGHT_CURR_CH,  ADC_SAMPLETIME_239POINT5); // PC0  CH10
    adc_regular_channel_config(ADCX, 8,  adc12V_NTC_CH,     ADC_SAMPLETIME_239POINT5); // PC1  CH11
    adc_regular_channel_config(ADCX, 9,  adc12V_VOLT_CH,    ADC_SAMPLETIME_239POINT5); // PC3  CH13
    adc_regular_channel_config(ADCX, 10, adcLIGHT_RES_CH,   ADC_SAMPLETIME_239POINT5); // PC4  CH14
    adc_regular_channel_config(ADCX, 11, adcPUMP_CURR_CH,   ADC_SAMPLETIME_239POINT5); // PC5  CH15
	
    /* ADC trigger config */
	adc_external_trigger_source_config(ADCX, ADC_REGULAR_CHANNEL, ADC0_1_2_EXTTRIG_REGULAR_NONE);
    adc_external_trigger_config(ADCX, ADC_REGULAR_CHANNEL, ENABLE);

    /* ADC DMA function enable */
    adc_dma_mode_enable(ADCX);

    /* enable ADC interface */
    adc_enable(ADCX);
    /* wait for ADC stability */
    delay_1ms(1);
    /* ADC calibration and reset calibration */
    adc_calibration_enable(ADCX);

    /* enable ADC software trigger */
    adc_software_trigger_enable(ADCX, ADC_REGULAR_CHANNEL);
}


void vAdc_Init(void)
{
	/* enable ADCX clock */
	rcu_periph_clock_enable(ADCX_RCU);
	/* enable adcDMA clock */
	rcu_periph_clock_enable(adcDMA_RCU);  
	 /* config ADC clock */
    rcu_adc_clock_config(RCU_CKADC_CKAPB2_DIV16);
   /*=============================配置ADC=============================*/    
	
    gpio_config();
	/* DMA configuration */
	#if (ADC_DMAX)
    dma_config();
	#endif
    /* ADC configuration */
    adc_config();
}

void vAdc_DeInit(void)
{
	adc_deinit(ADCX);
}

#if(boardLOW_POWER)
void vAdc_IoEnterLowPower(void)
{
	gpio_init(adcMppt_TEMP_EN_GPIO, GPIO_MODE_AIN, GPIO_OSPEED_2MHZ, adcMppt_TEMP_EN_PIN);
	
	adc_disable(ADCX);
	dma_channel_disable(ADCX, adcDMA_CH);
	rcu_periph_clock_disable(ADCX_RCU);
	rcu_periph_clock_disable(adcDMA_RCU); 
}
#endif

#endif  //boardADC_EN




