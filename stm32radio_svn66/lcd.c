#include "stm32f10x.h"
#include "rtthread.h"
#include "board.h"

#include <rtgui/rtgui.h>
#include <rtgui/driver.h>
#include <rtgui/rtgui_server.h>
#include <rtgui/rtgui_system.h>

void lcd_backlight_init(void);
void brightness_set(unsigned int value);

struct rtgui_graphic_driver _rtgui_lcd_driver;

extern  void   info_init(void);
extern  void   player_init(void);
void radio_rtgui_init(void)
{
    rtgui_rect_t rect;

    rtgui_system_server_init();

    /* register dock panel */
    rect.x1 = 0;
    rect.y1 = 0;
    rect.x2 = 240;
    rect.y2 = 25;
    rtgui_panel_register("info", &rect);
    rtgui_panel_set_nofocused("info");

    /* register main panel */
    rect.x1 = 0;
    rect.y1 = 25;
    rect.x2 = 240;
    rect.y2 = 320;
    rtgui_panel_register("main", &rect);
    rtgui_panel_set_default_focused("main");

    //rt_hw_lcd_init();
    {
#if LCD_VERSION == 1
#include "fmt0371/FMT0371.h"
        _rtgui_lcd_driver.name            = "lcd";
        _rtgui_lcd_driver.byte_per_pixel  = 2;
        _rtgui_lcd_driver.width           = 240;
        _rtgui_lcd_driver.height          = 320;
        _rtgui_lcd_driver.draw_hline      = fmt_lcd_draw_hline;
        _rtgui_lcd_driver.draw_raw_hline  = fmt_lcd_draw_raw_hline;
        _rtgui_lcd_driver.draw_vline      = fmt_lcd_draw_vline;
        _rtgui_lcd_driver.get_pixel       = fmt_lcd_get_pixel;
        _rtgui_lcd_driver.set_pixel       = fmt_lcd_set_pixel;
        _rtgui_lcd_driver.screen_update   = fmt_lcd_update;
        _rtgui_lcd_driver.get_framebuffer = fmt_lcd_get_framebuffer;
        fmt_lcd_init();
#elif LCD_VERSION == 2
#include "ili_lcd_general.h"
        _rtgui_lcd_driver.name            = "lcd";
        _rtgui_lcd_driver.byte_per_pixel  = 2;
        _rtgui_lcd_driver.width           = 240;
        _rtgui_lcd_driver.height          = 320;
        _rtgui_lcd_driver.draw_hline      = rt_hw_lcd_draw_hline;
        _rtgui_lcd_driver.draw_raw_hline  = rt_hw_lcd_draw_raw_hline;
        _rtgui_lcd_driver.draw_vline      = rt_hw_lcd_draw_vline;
        _rtgui_lcd_driver.get_pixel       = rt_hw_lcd_get_pixel;
        _rtgui_lcd_driver.set_pixel       = rt_hw_lcd_set_pixel;
        _rtgui_lcd_driver.screen_update   = rt_hw_lcd_update;
        _rtgui_lcd_driver.get_framebuffer = rt_hw_lcd_get_framebuffer;
        lcd_Initializtion();
#elif LCD_VERSION == 3
#include "ssd1289.h"
        _rtgui_lcd_driver.name            = "lcd";
        _rtgui_lcd_driver.byte_per_pixel  = 2;
        _rtgui_lcd_driver.width           = 240;
        _rtgui_lcd_driver.height          = 320;
        _rtgui_lcd_driver.draw_hline      = ssd1289_lcd_draw_hline;
        _rtgui_lcd_driver.draw_raw_hline  = ssd1289_lcd_draw_raw_hline;
        _rtgui_lcd_driver.draw_vline      = ssd1289_lcd_draw_vline;
        _rtgui_lcd_driver.get_pixel       = ssd1289_lcd_get_pixel;
        _rtgui_lcd_driver.set_pixel       = ssd1289_lcd_set_pixel;
        _rtgui_lcd_driver.screen_update   = ssd1289_lcd_update;
        _rtgui_lcd_driver.get_framebuffer = ssd1289_lcd_get_framebuffer;
        ssd1289_init();
#endif
    }//rt_hw_lcd_init

    /* add lcd driver into graphic driver */
    rtgui_graphic_driver_add(&_rtgui_lcd_driver);

    info_init();
    player_init();

    lcd_backlight_init();
}

#if ( LCD_USE_PWM == 1) || ( LCD_USE_PWM == 2 )
static TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
static TIM_OCInitTypeDef  TIM_OCInitStructure;
#define ARR  12000
#endif

void lcd_backlight_init(void)
{
    /* for old version */
    GPIO_InitTypeDef GPIO_InitStructure;
#if ( LCD_USE_PWM == 0)
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOF,ENABLE);

    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOF,&GPIO_InitStructure);
    GPIO_SetBits(GPIOF,GPIO_Pin_9);
    /* for old version */
#endif

#if ( LCD_USE_PWM == 1)
    /* new version: use pwm in PB9 TIM4_CH4 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
    /* TIM4 clock enable */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

    /* GPIOB Configuration:TIM4 Channel4 as alternate function push-pull */
    GPIO_InitStructure.GPIO_Pin   =  GPIO_Pin_9;//GPIO_Pin_8 |
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /* Time base configuration */
    TIM_TimeBaseStructure.TIM_Period = ARR;//12000
    TIM_TimeBaseStructure.TIM_Prescaler = 2;//Ԥ��Ƶ2,Ƶ��36M
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);

    /* PWM1 Mode configuration: Channel1 */
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    /* PWM1 Mode configuration: Channel4 */
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = (ARR/100)*50; /* ���ó�ʼ�������� */
    TIM_OC4Init(TIM4, &TIM_OCInitStructure);

    TIM_OC4PreloadConfig(TIM4, TIM_OCPreload_Enable);
    TIM_ARRPreloadConfig(TIM4, ENABLE);
    /* TIM4 enable counter */
    TIM_Cmd(TIM4, ENABLE);
#elif ( LCD_USE_PWM == 2 )
    /* new version: use pwm in PB6 TIM4_CH1 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
    /* TIM4 clock enable */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

    /* GPIOB Configuration:TIM4 Channel4 as alternate function push-pull */
    GPIO_InitStructure.GPIO_Pin   =  GPIO_Pin_6;//GPIO_Pin_8 |
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /* Time base configuration */
    TIM_TimeBaseStructure.TIM_Period = ARR;//12000
    TIM_TimeBaseStructure.TIM_Prescaler = 2;//Ԥ��Ƶ2,Ƶ��36M
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);

    /* PWM1 Mode configuration: Channel1 */
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    /* PWM1 Mode configuration: Channel4 */
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = (ARR/100)*50; /* ���ó�ʼ�������� */
    TIM_OC1Init(TIM4, &TIM_OCInitStructure);

    TIM_OC1PreloadConfig(TIM4, TIM_OCPreload_Enable);
    TIM_ARRPreloadConfig(TIM4, ENABLE);
    /* TIM4 enable counter */
    TIM_Cmd(TIM4, ENABLE);
#endif
}

void brightness_set(unsigned int value)
{
#if ( LCD_USE_PWM == 0)
    if(value)
    {
        GPIO_SetBits(GPIOF,GPIO_Pin_9);
    }
    else
    {
        GPIO_ResetBits(GPIOF,GPIO_Pin_9);
    }
#endif

#if ( LCD_USE_PWM == 1 )
    if(value>100)value=50;
    TIM_OCInitStructure.TIM_Pulse = (ARR/100)*value;
    TIM_OC4Init(TIM4, &TIM_OCInitStructure);
#elif ( LCD_USE_PWM == 2 )
    if(value>100)value=50;
    TIM_OCInitStructure.TIM_Pulse = (ARR/100)*value;
    TIM_OC1Init(TIM4, &TIM_OCInitStructure);
#endif
}
