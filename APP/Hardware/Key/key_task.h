#ifndef KEY_TASK_H_
#define KEY_TASK_H_

#include "board_config.h"

#if(boardKEY_EN)
//#define		//4Tab									//10Tab
#define     	keyTASK_CYCLE_TIME                		10  //按键任务更新时间
#define     	keyNUM                            		5   //按键的数量
#define     	keyGROUP_NUM                      		10  //组合按键种类   
#define     	keySHORT_PRESS_TIME               		2   //短按按键的最小时间*10ms
#define     	keyLONG_PRESS_TIME                		90 //长按按键的最小时间*10ms
#define     	keySUPER_LONG_PRESS_TIME           		250	//超长按按键的最小时间*10ms
#define     	keyNUPRESS_MAX_TIME               		35  //组合按键最大的等待时间*10ms
#define     	keyADD_SPACE_TIME                 		20  //长按累加间隔

// 确认键 PC13
#define     	keyGPIO_ENTER_RCU       				RCU_GPIOC
#define     	keyGPIO_ENTER_PORT      				GPIOC
#define     	keyGPIO_ENTER_PIN       				GPIO_PIN_13

// 左键 PC14
#define     	keyGPIO_LEFT_RCU        				RCU_GPIOC
#define     	keyGPIO_LEFT_PORT       				GPIOC
#define     	keyGPIO_LEFT_PIN        				GPIO_PIN_14

// 上键 PC15
#define     	keyGPIO_UP_RCU          				RCU_GPIOC
#define     	keyGPIO_UP_PORT         				GPIOC
#define     	keyGPIO_UP_PIN          				GPIO_PIN_15

// 下键 PC12
#define     	keyGPIO_DOWN_RCU        				RCU_GPIOC
#define     	keyGPIO_DOWN_PORT       				GPIOC
#define     	keyGPIO_DOWN_PIN        				GPIO_PIN_12

// 右键 PD2
#define     	keyGPIO_RIGHT_RCU       				RCU_GPIOD
#define     	keyGPIO_RIGHT_PORT      				GPIOD
#define     	keyGPIO_RIGHT_PIN       				GPIO_PIN_2

__STATIC_INLINE bool bKey_EnterIsPress(void)
{
    if((GPIO_ISTAT(keyGPIO_ENTER_PORT) & (keyGPIO_ENTER_PIN)) == 0)
        return true;
    else
        return false;
}

__STATIC_INLINE bool bKey_LeftIsPress(void)
{
    if((GPIO_ISTAT(keyGPIO_LEFT_PORT) & (keyGPIO_LEFT_PIN)) == 0)
        return true;
    else
        return false;
}

__STATIC_INLINE bool bKey_UpIsPress(void)
{
    if((GPIO_ISTAT(keyGPIO_UP_PORT) & (keyGPIO_UP_PIN)) == 0)
        return true;
    else
        return false;
}

__STATIC_INLINE bool bKey_DownIsPress(void)
{
    if((GPIO_ISTAT(keyGPIO_DOWN_PORT) & (keyGPIO_DOWN_PIN)) == 0)
        return true;
    else
        return false;
}

__STATIC_INLINE bool bKey_RightIsPress(void)
{
    if((GPIO_ISTAT(keyGPIO_RIGHT_PORT) & (keyGPIO_RIGHT_PIN)) == 0)
        return true;
    else
        return false;
}

// 向后兼容旧函数名（映射到新函数）
#define bKey_PowerIsPress()     bKey_EnterIsPress()
#define bKey_AcIsPress()        bKey_LeftIsPress()
#define bKey_LightIsPress()     bKey_UpIsPress()
#define bKey_UsbIsPress()       bKey_DownIsPress()
#define bKey_DcIsPress()        bKey_RightIsPress()

typedef enum
{
	KTE_FUN_NULL = 0,
	KTE_ENTER_LONG,
	KTE_ENTER_SHORT,
	KTE_ENTER_SUPER_LONG,
	KTE_LEFT_LONG,
	KTE_LEFT_SHORT,
	KTE_LEFT_SUPER_LONG,
	KTE_UP_LONG,
	KTE_UP_SHORT,
	KTE_UP_SUPER_LONG,
	KTE_DOWN_LONG,
	KTE_DOWN_SHORT,
	KTE_DOWN_SUPER_LONG,
	KTE_RIGHT_LONG,
	KTE_RIGHT_SHORT,
	KTE_RIGHT_SUPER_LONG,
}KeyTriEvent_e;  //触发事件


typedef struct
{
    bool    			(*IsPress)(void);        
    vs16    			sOnPressCnt;	  	//按键按下计时  0:表示按键没触发   -1:表示按键功能已经被记录
	bool        		bEnMulitFunKey;     //使能多功能按键
	bool        		bEnLongPressAdd;    //使能长按累加      
}KeyHandler_t;


void vKey_TaskInit(void);
void vKey_PowerIsTri(void);
void vKey_ParamInit(void);

#if(!boardUSE_OS)
void vKey_Task(void *pvParameters);
#endif

#if(boardLOW_POWER)
void vKey_EnterLowPower(void);
void vKey_ExitLowPower(void);
#endif

#endif  //boardKEY_EN

#endif  //KEY_TASK_H_


