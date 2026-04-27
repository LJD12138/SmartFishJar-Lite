#include "MD_Display/md_display_iface.h"

#if(boardDISPLAY_EN)

#define OLED_WIDTH_PIXELS   128U
#define OLED_HEIGHT_PIXELS   64U
#define OLED_PAGE_COUNT       8U
#define OLED_CMD              0U
#define OLED_DATA             1U
#define OLED_SPI_TIMEOUT  0x0000FFFFUL

static u8 s_oled_gram[OLED_WIDTH_PIXELS][OLED_PAGE_COUNT];

#if(boardDISP_SPI_MODE == boardDISP_SPI_MODE_HARD_DMA)
static u8 s_oled_dma_page_buf[OLED_WIDTH_PIXELS];

static void oled_hw_spi_wait_idle(void)
{
    uint32_t timeout;

    timeout = OLED_SPI_TIMEOUT;
    while((SET == spi_i2s_flag_get(dispOLED_SPI_PERIPH, SPI_FLAG_TRANS)) && (timeout > 0U))
    {
        timeout--;
    }

    timeout = OLED_SPI_TIMEOUT;
    while((RESET == spi_i2s_flag_get(dispOLED_SPI_PERIPH, SPI_FLAG_TBE)) && (timeout > 0U))
    {
        timeout--;
    }
}

static void oled_hw_spi_dma_write(const u8 *data, u16 len)
{
    uint32_t timeout;

    if((data == NULL) || (len == 0U))
        return;

    dma_channel_disable(dispOLED_SPI_DMA_PERIPH, dispOLED_SPI_DMA_TX_CH);
    dma_memory_address_config(dispOLED_SPI_DMA_PERIPH, dispOLED_SPI_DMA_TX_CH, (uint32_t)data);
    dma_transfer_number_config(dispOLED_SPI_DMA_PERIPH, dispOLED_SPI_DMA_TX_CH, len);
    dma_flag_clear(dispOLED_SPI_DMA_PERIPH, dispOLED_SPI_DMA_TX_CH, DMA_FLAG_G);
    dma_channel_enable(dispOLED_SPI_DMA_PERIPH, dispOLED_SPI_DMA_TX_CH);

    timeout = OLED_SPI_TIMEOUT;
    while((RESET == dma_flag_get(dispOLED_SPI_DMA_PERIPH, dispOLED_SPI_DMA_TX_CH, DMA_FLAG_FTF)) && (timeout > 0U))
    {
        timeout--;
    }

    dma_channel_disable(dispOLED_SPI_DMA_PERIPH, dispOLED_SPI_DMA_TX_CH);
    dma_flag_clear(dispOLED_SPI_DMA_PERIPH, dispOLED_SPI_DMA_TX_CH, DMA_FLAG_G);
    oled_hw_spi_wait_idle();
}

static void oled_hw_write_buffer(const u8 *data, u16 len, u8 mode)
{
    if((data == NULL) || (len == 0U))
        return;

    if(mode == OLED_DATA)
        dispOLED_DC_H();
    else
        dispOLED_DC_L();

    dispOLED_NSS_L();
    oled_hw_spi_dma_write(data, len);
    dispOLED_NSS_H();
    dispOLED_DC_H();
}

static void oled_hw_spi_dma_init(void)
{
    spi_parameter_struct spi_init_struct;
    dma_parameter_struct dma_init_struct;

    rcu_periph_clock_enable(RCU_AF);
    rcu_periph_clock_enable(dispOLED_SPI_RCU);
    rcu_periph_clock_enable(dispOLED_SPI_DMA_RCU);

    gpio_init(dispOLED_SCK_PORT, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, dispOLED_SCK_PIN);
    gpio_init(dispOLED_MOSI_PORT, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, dispOLED_MOSI_PIN);

    spi_i2s_deinit(dispOLED_SPI_PERIPH);
    spi_struct_para_init(&spi_init_struct);
    spi_init_struct.device_mode = SPI_MASTER;
    spi_init_struct.trans_mode = SPI_TRANSMODE_BDTRANSMIT;
    spi_init_struct.frame_size = SPI_FRAMESIZE_8BIT;
    spi_init_struct.nss = SPI_NSS_SOFT;
    spi_init_struct.clock_polarity_phase = SPI_CK_PL_HIGH_PH_2EDGE;
    spi_init_struct.prescale = dispOLED_SPI_PRESCALE;
    spi_init_struct.endian = SPI_ENDIAN_MSB;
    spi_init(dispOLED_SPI_PERIPH, &spi_init_struct);
    spi_nss_internal_high(dispOLED_SPI_PERIPH);
    spi_dma_enable(dispOLED_SPI_PERIPH, SPI_DMA_TRANSMIT);
    spi_enable(dispOLED_SPI_PERIPH);

    dma_deinit(dispOLED_SPI_DMA_PERIPH, dispOLED_SPI_DMA_TX_CH);
    dma_struct_para_init(&dma_init_struct);
    dma_init_struct.periph_addr = (uint32_t)(&SPI_DATA(dispOLED_SPI_PERIPH));
    dma_init_struct.memory_addr = (uint32_t)s_oled_dma_page_buf;
    dma_init_struct.direction = DMA_MEMORY_TO_PERIPHERAL;
    dma_init_struct.memory_width = DMA_MEMORY_WIDTH_8BIT;
    dma_init_struct.periph_width = DMA_PERIPHERAL_WIDTH_8BIT;
    dma_init_struct.priority = DMA_PRIORITY_HIGH;
    dma_init_struct.number = 1U;
    dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    dma_init(dispOLED_SPI_DMA_PERIPH, dispOLED_SPI_DMA_TX_CH, &dma_init_struct);
    dma_circulation_disable(dispOLED_SPI_DMA_PERIPH, dispOLED_SPI_DMA_TX_CH);
    dma_memory_to_memory_disable(dispOLED_SPI_DMA_PERIPH, dispOLED_SPI_DMA_TX_CH);
    dma_channel_disable(dispOLED_SPI_DMA_PERIPH, dispOLED_SPI_DMA_TX_CH);
    dma_flag_clear(dispOLED_SPI_DMA_PERIPH, dispOLED_SPI_DMA_TX_CH, DMA_FLAG_G);
}
#endif

static void oled_delay_us(vu16 us)
{
    vu8 j;
    while(us--)
    {
        for(j = 0; j < 20; j++)
        {
        }
    }
}

static void oled_delay_ms(vu16 ms)
{
    while(ms--)
    {
        oled_delay_us(1000);
    }
}

static void oled_write_byte(u8 dat, u8 mode)
{
#if(boardDISP_SPI_MODE == boardDISP_SPI_MODE_HARD_DMA)
    oled_hw_write_buffer(&dat, 1U, mode);
#else
    u8 i;

    if(mode == OLED_DATA)
        dispOLED_DC_H();
    else
        dispOLED_DC_L();

    dispOLED_NSS_L();
    for(i = 0; i < 8U; i++)
    {
        dispOLED_SCK_L();
        if((dat & 0x80U) != 0U)
            dispOLED_MOSI_H();
        else
            dispOLED_MOSI_L();

        dispOLED_SCK_H();
        dat <<= 1;
    }
    dispOLED_NSS_H();
    dispOLED_DC_H();
#endif
}

static void oled_write_cmd(u8 cmd)
{
    oled_write_byte(cmd, OLED_CMD);
}

static void oled_panel_init_sequence(void)
{
    oled_write_cmd(0xAE);
    oled_write_cmd(0x02);
    oled_write_cmd(0x10);
    oled_write_cmd(0x40);
    oled_write_cmd(0xB0);
    oled_write_cmd(0x81);
    oled_write_cmd(0xCF);
    oled_write_cmd(0xA1);
    oled_write_cmd(0xA6);
    oled_write_cmd(0xA8);
    oled_write_cmd(0x3F);
    oled_write_cmd(0xAD);
    oled_write_cmd(0x8B);
    oled_write_cmd(0x33);
    oled_write_cmd(0xC8);
    oled_write_cmd(0xD3);
    oled_write_cmd(0x00);
    oled_write_cmd(0xD5);
    oled_write_cmd(0x80);
    oled_write_cmd(0xD9);
    oled_write_cmd(0x1F);
    oled_write_cmd(0xDA);
    oled_write_cmd(0x12);
    oled_write_cmd(0xDB);
    oled_write_cmd(0x40);
}

static const u8 *oled_font_5x7(char c)
{
    static const u8 f_space[5] = {0x00,0x00,0x00,0x00,0x00};
    static const u8 f_percent[5] = {0x62,0x64,0x08,0x13,0x23};
    static const u8 f_minus[5] = {0x08,0x08,0x08,0x08,0x08};
    static const u8 f_colon[5] = {0x00,0x36,0x36,0x00,0x00};
    static const u8 f_slash[5] = {0x20,0x10,0x08,0x04,0x02};

    static const u8 d0[5] = {0x3E,0x51,0x49,0x45,0x3E};
    static const u8 d1[5] = {0x00,0x42,0x7F,0x40,0x00};
    static const u8 d2[5] = {0x62,0x51,0x49,0x49,0x46};
    static const u8 d3[5] = {0x22,0x41,0x49,0x49,0x36};
    static const u8 d4[5] = {0x18,0x14,0x12,0x7F,0x10};
    static const u8 d5[5] = {0x2F,0x49,0x49,0x49,0x31};
    static const u8 d6[5] = {0x3E,0x49,0x49,0x49,0x32};
    static const u8 d7[5] = {0x01,0x71,0x09,0x05,0x03};
    static const u8 d8[5] = {0x36,0x49,0x49,0x49,0x36};
    static const u8 d9[5] = {0x26,0x49,0x49,0x49,0x3E};

    static const u8 A[5] = {0x7E,0x11,0x11,0x11,0x7E};
    static const u8 B[5] = {0x7F,0x49,0x49,0x49,0x36};
    static const u8 C[5] = {0x3E,0x41,0x41,0x41,0x22};
    static const u8 D[5] = {0x7F,0x41,0x41,0x22,0x1C};
    static const u8 E[5] = {0x7F,0x49,0x49,0x49,0x41};
    static const u8 F[5] = {0x7F,0x09,0x09,0x09,0x01};
    static const u8 G[5] = {0x3E,0x41,0x49,0x49,0x3A};
    static const u8 H[5] = {0x7F,0x08,0x08,0x08,0x7F};
    static const u8 I[5] = {0x00,0x41,0x7F,0x41,0x00};
    static const u8 K[5] = {0x7F,0x08,0x14,0x22,0x41};
    static const u8 L[5] = {0x7F,0x40,0x40,0x40,0x40};
    static const u8 M[5] = {0x7F,0x02,0x0C,0x02,0x7F};
    static const u8 N[5] = {0x7F,0x04,0x08,0x10,0x7F};
    static const u8 O[5] = {0x3E,0x41,0x41,0x41,0x3E};
    static const u8 P[5] = {0x7F,0x09,0x09,0x09,0x06};
    static const u8 R[5] = {0x7F,0x09,0x19,0x29,0x46};
    static const u8 S[5] = {0x46,0x49,0x49,0x49,0x31};
    static const u8 T[5] = {0x01,0x01,0x7F,0x01,0x01};
    static const u8 U[5] = {0x3F,0x40,0x40,0x40,0x3F};
    static const u8 V[5] = {0x1F,0x20,0x40,0x20,0x1F};
    static const u8 W[5] = {0x7F,0x20,0x18,0x20,0x7F};
    static const u8 Y[5] = {0x07,0x08,0x70,0x08,0x07};

    switch(c)
    {
        case '0': return d0;
        case '1': return d1;
        case '2': return d2;
        case '3': return d3;
        case '4': return d4;
        case '5': return d5;
        case '6': return d6;
        case '7': return d7;
        case '8': return d8;
        case '9': return d9;
        case 'A': return A;
        case 'B': return B;
        case 'C': return C;
        case 'D': return D;
        case 'E': return E;
        case 'F': return F;
        case 'G': return G;
        case 'H': return H;
        case 'I': return I;
        case 'K': return K;
        case 'L': return L;
        case 'M': return M;
        case 'N': return N;
        case 'O': return O;
        case 'P': return P;
        case 'R': return R;
        case 'S': return S;
        case 'T': return T;
        case 'U': return U;
        case 'V': return V;
        case 'W': return W;
        case 'Y': return Y;
        case '%': return f_percent;
        case '-': return f_minus;
        case ':': return f_colon;
        case '/': return f_slash;
        case ' ': return f_space;
        default:  return f_space;
    }
}

/***********************************************************************************************************************
-----函数功能    设置单个OLED像素点
-----说明(备注)  根据坐标写入像素状态，越界坐标直接忽略。
-----传入参数    x:列坐标  y:行坐标  on:像素状态
-----输出参数    none
-----返回值      none
************************************************************************************************************************/
void vDisp_OledDrawPixel(u8 x, u8 y, bool on)
{
    u8 page;
    u8 bit;

    if(x >= OLED_WIDTH_PIXELS || y >= OLED_HEIGHT_PIXELS)
        return;

    page = (u8)(y >> 3);
    bit = (u8)(1U << (y & 0x07U));

    if(on)
        s_oled_gram[x][page] |= bit;
    else
        s_oled_gram[x][page] &= (u8)(~bit);
}

/***********************************************************************************************************************
-----函数功能    绘制OLED水平线
-----说明(备注)  从起始坐标开始，连续绘制指定长度的水平线。
-----传入参数    x:起点列坐标  y:行坐标  len:线长  on:像素状态
-----输出参数    none
-----返回值      none
************************************************************************************************************************/
void vDisp_OledDrawHLine(u8 x, u8 y, u8 len, bool on)
{
    u8 i;
    for(i = 0; i < len; i++)
    {
        vDisp_OledDrawPixel((u8)(x + i), y, on);
    }
}

/***********************************************************************************************************************
-----函数功能    绘制OLED竖直线
-----说明(备注)  从起始坐标开始，连续绘制指定长度的竖直线。
-----传入参数    x:列坐标  y:起点行坐标  len:线长  on:像素状态
-----输出参数    none
-----返回值      none
************************************************************************************************************************/
void vDisp_OledDrawVLine(u8 x, u8 y, u8 len, bool on)
{
    u8 i;
    for(i = 0; i < len; i++)
    {
        vDisp_OledDrawPixel(x, (u8)(y + i), on);
    }
}

/***********************************************************************************************************************
-----函数功能    绘制6x8字符
-----说明(备注)  使用内置5x7字模显示单个字符，字符宽度按6列处理，便于字符间隔控制。
-----传入参数    x:起点列坐标  y:起点行坐标  c:待显示字符
-----输出参数    none
-----返回值      none
************************************************************************************************************************/
void vDisp_OledDrawChar6x8(u8 x, u8 y, char c)
{
    u8 col;
    u8 row;
    const u8 *glyph = oled_font_5x7(c);

    for(col = 0; col < 5U; col++)
    {
        u8 bits = glyph[col];
        for(row = 0; row < 7U; row++)
        {
            vDisp_OledDrawPixel((u8)(x + col), (u8)(y + row), ((bits & 0x01U) != 0U));
            bits >>= 1;
        }
    }

    for(row = 0; row < 7U; row++)
    {
        vDisp_OledDrawPixel((u8)(x + 5U), (u8)(y + row), false);
    }
}

/***********************************************************************************************************************
-----函数功能    绘制6x8字符串
-----说明(备注)  逐字符调用字体绘制接口显示字符串，超出显示区域时自动停止。
-----传入参数    x:起点列坐标  y:起点行坐标  str:字符串指针
-----输出参数    none
-----返回值      none
************************************************************************************************************************/
void vDisp_OledDrawString6x8(u8 x, u8 y, const char *str)
{
    while((*str) != '\0')
    {
        vDisp_OledDrawChar6x8(x, y, *str);
        x = (u8)(x + 6U);
        if(x > 122U)
            break;
        str++;
    }
}

/***********************************************************************************************************************
-----函数功能    清空OLED显存缓存
-----说明(备注)  将GRAM缓存全部清零，等待后续刷新到屏幕。
-----传入参数    none
-----输出参数    none
-----返回值      none
************************************************************************************************************************/
void vDisp_OledClearBuffer(void)
{
    u8 x;
    u8 page;
    for(x = 0; x < OLED_WIDTH_PIXELS; x++)
    {
        for(page = 0; page < OLED_PAGE_COUNT; page++)
        {
            s_oled_gram[x][page] = 0x00U;
        }
    }
}

/***********************************************************************************************************************
-----函数功能    填充OLED显存缓存
-----说明(备注)  用指定数据批量填充GRAM缓存，常用于全亮/全灭测试。
-----传入参数    value:填充值
-----输出参数    none
-----返回值      none
************************************************************************************************************************/
void vDisp_OledFillBuffer(u8 value)
{
    u8 x;
    u8 page;
    for(x = 0; x < OLED_WIDTH_PIXELS; x++)
    {
        for(page = 0; page < OLED_PAGE_COUNT; page++)
        {
            s_oled_gram[x][page] = value;
        }
    }
}

/***********************************************************************************************************************
-----函数功能    刷新OLED显示内容
-----说明(备注)  将显存缓存逐页写入OLED面板。
-----传入参数    none
-----输出参数    none
-----返回值      none
************************************************************************************************************************/
void vDisp_OledRefresh(void)
{
    u8 page;
    u8 x;

    for(page = 0; page < OLED_PAGE_COUNT; page++)
    {
        oled_write_cmd((u8)(0xB0U + page));
        oled_write_cmd(0x02);
        oled_write_cmd(0x10);

#if(boardDISP_SPI_MODE == boardDISP_SPI_MODE_HARD_DMA)
        for(x = 0; x < OLED_WIDTH_PIXELS; x++)
        {
            s_oled_dma_page_buf[x] = s_oled_gram[x][page];
        }
    oled_hw_write_buffer(s_oled_dma_page_buf, OLED_WIDTH_PIXELS, OLED_DATA);
#else
        for(x = 0; x < OLED_WIDTH_PIXELS; x++)
        {
            oled_write_byte(s_oled_gram[x][page], OLED_DATA);
        }
#endif
    }
}

/***********************************************************************************************************************
-----函数功能    设置OLED电源状态
-----说明(备注)  通过指令控制OLED显示开关。
-----传入参数    on:true开显示 false关显示
-----输出参数    none
-----返回值      none
************************************************************************************************************************/
void vDisp_OledSetPower(bool on)
{
    if(on)
        oled_write_cmd(0xAF);
    else
        oled_write_cmd(0xAE);
}


/***********************************************************************************************************************
-----函数功能    初始化OLED硬件接口
-----说明(备注)  完成GPIO时钟、IO方向、复位时序以及面板基础配置。
-----传入参数    none
-----输出参数    none
-----返回值      none
************************************************************************************************************************/
void vDisp_OledIfaceInit(void)
{
    rcu_periph_clock_enable(dispOLED_NSS_RCU);
    rcu_periph_clock_enable(dispOLED_RES_RCU);
    rcu_periph_clock_enable(dispOLED_DC_RCU);

#if(boardDISP_SPI_MODE == boardDISP_SPI_MODE_SOFT)
    rcu_periph_clock_enable(dispOLED_SCK_RCU);
    rcu_periph_clock_enable(dispOLED_MOSI_RCU);
#else
    if(dispOLED_SCK_RCU != dispOLED_NSS_RCU)
        rcu_periph_clock_enable(dispOLED_SCK_RCU);
    if((dispOLED_MOSI_RCU != dispOLED_NSS_RCU) && (dispOLED_MOSI_RCU != dispOLED_SCK_RCU))
        rcu_periph_clock_enable(dispOLED_MOSI_RCU);
#endif

    gpio_init(dispOLED_NSS_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, dispOLED_NSS_PIN);
    gpio_init(dispOLED_RES_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, dispOLED_RES_PIN);
    gpio_init(dispOLED_DC_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, dispOLED_DC_PIN);

#if(boardDISP_SPI_MODE == boardDISP_SPI_MODE_SOFT)
    gpio_init(dispOLED_SCK_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, dispOLED_SCK_PIN);
    gpio_init(dispOLED_MOSI_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, dispOLED_MOSI_PIN);
#else
    oled_hw_spi_dma_init();
#endif

    dispOLED_NSS_H();
    dispOLED_DC_H();

#if(boardDISP_SPI_MODE == boardDISP_SPI_MODE_SOFT)
    dispOLED_SCK_H();
    dispOLED_MOSI_H();
#endif

    dispOLED_RES_L();
    oled_delay_ms(20);
    dispOLED_RES_H();
    oled_delay_ms(200);

    oled_panel_init_sequence();
    vDisp_OledClearBuffer();
    vDisp_OledRefresh();
    vDisp_OledSetPower(true);
}

/***********************************************************************************************************************
-----函数功能    重新初始化OLED显示面板
-----说明(备注)  用于工作中恢复显示时，重新下发面板初始化序列并刷新缓存。
-----传入参数    none
-----输出参数    none
-----返回值      none
************************************************************************************************************************/
void vDisp_OledReInit(void)
{
    oled_panel_init_sequence();
    vDisp_OledClearBuffer();
    vDisp_OledRefresh();
    vDisp_OledSetPower(true);
}

#endif  // boardDISPLAY_EN
