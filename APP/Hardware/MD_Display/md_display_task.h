#ifndef MD_DISPLAY_TASK_H_
#define MD_DISPLAY_TASK_H_

#include "board_config.h"

#if(boardDISPLAY_EN)
#include "queue_task.h"

#if(boardKEY_EN)
#include "Key/key_task.h"
#endif

#include "u8g2.h"

#if(boardENG_MODE_EN)
#include "MD_Display/md_display_eng_mode.h"
#endif  //boardENG_MODE_EN

#if(boardUSE_OS)
#include "freertos.h"
#include "task.h"
#endif  //boardUSE_OS


#define     	dispUSE_U8G2								1  //1:使用U8g2渲染 0:使用本地OLED显存渲染


//*********************************显示任务ID**********************************
//显示队列任务ID, 由队列管理函数装载对应任务函数
typedef enum
{
    DTI_NULL = 0,       //空任务
    DTI_INIT,           //初始化显示
    DTI_CLOSING,        //关闭中显示
    DTI_SHUT_DOWN,      //关闭完成显示
    DTI_ERR,            //错误显示
    DTI_BOOTING,        //启动中显示
    DTI_WORK,           //工作显示
    DTI_UPDATA,         //升级显示
    DTI_ENG,            //工程模式显示
}DispTaskId_E;

//*********************************任务对象**********************************
//显示任务运行参数, 由显示任务、定时节拍和亮灭控制共用
typedef struct
{
    bool             	bLight;           	//1:打开   0:关闭
	bool             	bSleepShow;       	//1:打开   0:关闭
	DevState_E    		eDevState;          //设备状态
	vu16             	usAutoOffTime;     //息屏时间
	vu16             	usAutoOffCnt;      //息屏倒计时
	#if(boardENG_MODE_EN)
	DispTypeSet_E    	eLightSetType;     //亮度设置
	#endif
}Disp_T;  
extern Disp_T tDisp; 

//显示页面ID, vDisp_RenderUi根据该ID选择绘制函数
typedef enum
{
	DPI_NONE = 0,
    DPI_INIT,           //初始化显示
	DPI_BOOTING,
	DPI_HOME,
	DPI_LIGHT,
	DPI_HEAT,
	DPI_WPUMP,
	DPI_O2PUMP,
	DPI_SETTING,
	DPI_ADC,
	DPI_ENV,
	DPI_ACT,
	DPI_ALARM,
	DPI_QUICK,
	DPI_UPGRADE,
	DPI_CLOSING,
	DPI_SLEEP,
	DPI_ERROR,
}DispPageId_E;

//页面叠加层, 用于临时状态显示
typedef enum
{
	DOI_NONE = 0,
	DOI_QUICK,
	DOI_ERROR,
}DispOverlayId_E;

//页面焦点ID, 卡片焦点对应当前页可选项目
typedef enum
{
	DFI_NONE = 0,
	DFI_TAB,
	DFI_CARD_1,
	DFI_CARD_2,
	DFI_CARD_3,
	DFI_CARD_4,
	DFI_SOFTKEY_LEFT,
	DFI_SOFTKEY_ENTER,
	DFI_SOFTKEY_RIGHT,
}DispFocusId_E;

//局部刷新脏标志, 页面或布局变化时使用DDM_FULL
typedef enum
{
	DDM_NONE = 0x0000,
	DDM_LAYOUT = 0x0001,
	DDM_HEADER = 0x0002,
	DDM_CONTENT = 0x0004,
	DDM_SOFTKEY = 0x0008,
	DDM_FULL = 0xFFFF,
}DispDirtyMask_E;

//设置页项目顺序, 需要和设置页字段布局保持一致
typedef enum
{
	DSI_SLEEP = 0,
	DSI_BUZZER,
	DSI_HIGH_LIGHT,
	DSI_LOW_LIGHT,
	DSI_RESET,
	DSI_INVAILD,
}DispSettingItem_E;

//快捷控制项目, 用于切换对应执行器模式
typedef enum
{
	DQI_LIGHT = 0,
	DQI_O2PUMP,
	DQI_WPUMP,
	DQI_HEAT,
	DQI_INVAILD,
}DispQuickItem_E;

//主页标签顺序, 绘制和按键导航共用
typedef enum
{
	DHT_LIGHT = 0,
	DHT_HEAT,
	DHT_WPUMP,
	DHT_O2PUMP,
	DHT_SETTING,
	DHT_ADC,
	DHT_COUNT,
}DispHomeTabId_E;

//主页面分组, 左右键在主页/设置/ADC之间切换
typedef enum
{
	DMP_HOME = 0,
	DMP_SETTING,
	DMP_ADC,
	DMP_COUNT,
}DispMainPage_E;

//主页总览模块, 同时用于映射详情页
typedef enum
{
	DHM_LIGHT = 0,
	DHM_HEAT,
	DHM_WPUMP,
	DHM_O2PUMP,
	DHM_COUNT,
}DispHomeModule_E;

//主页交互模式, 总览选模块, 标签激活后选字段
typedef enum
{
	DHM_OVERVIEW = 0,
	DHM_TAB_ACTIVE,
}DispHomeMode_E;

//底部三段软键文本
typedef struct
{
	const char *pcLeft;
	const char *pcCenter;
	const char *pcRight;
}DispSoftKey_T;

//设置页编辑缓存, 保存成功后再写入记忆参数
typedef struct
{
	vu16 usSleepTime;
	bool bBuzOff;
	u8 ucHighLightValue;
	u8 ucLowLightValue;
	bool bRestoreDefault;
	bool bCacheValid;
	bool bDirty;
}DispSettingCache_T;

//窄卡片长文本滚动状态
typedef struct
{
	u8 ucFieldKey;
	u8 ucOffset;
	u8 ucHoldCnt;
	u8 ucMaxOffset;
	u16 usTextHash;
}DispTextScroll_T;

//错误页进入前的工作页恢复点
typedef struct
{
	DispPageId_E ePageId;
	DispMainPage_E eMainPage;
	DispHomeMode_E eHomeMode;
	DispHomeTabId_E eHomeTab;
	DispHomeModule_E eHomeModule;
	u8 ucTabVisibleStart;
	u8 ucTabItemIndex;
	u8 ucFieldIndex;
	u8 ucAdcGroupIndex;
	bool bEditing;
}DispHomeRestore_T;

//页面上下文, 保存当前页、焦点、刷新区域、编辑缓存和提示文本
typedef struct
{
	DispPageId_E ePageId;
	DispPageId_E ePrevPageId;
	DispOverlayId_E eOverlayId;
	DispFocusId_E eFocusId;
	DispFocusId_E ePrevFocusId;
	DispMainPage_E eMainPage;
	DispHomeMode_E eHomeMode;
	DispHomeTabId_E eHomeTab;
	DispHomeModule_E eHomeModule;
	vu16 usDirtyMask;
	u8 ucTabVisibleStart;
	u8 ucTabItemIndex;
	u8 ucFieldIndex;
	u8 ucAdcGroupIndex;
	bool bEditing;
	bool bOverlayLock;
	DispSettingCache_T tSettingCache;
	DispTextScroll_T tTextScroll;
	DispHomeRestore_T tHomeRestore;
	DispQuickItem_E eQuickItem;
	char acHint[20];
	vu16 usHintCnt;
}DispPageCtx_T;

extern DispPageCtx_T tDispPageCtx;

extern u8g2_t u8g2;             // 显示器初始化结构体
extern Task_T *tpDispTask;      //队列任务对象
extern bool g_bDispPageDirty;   //页面脏标志

#if(boardUSE_OS)
extern TaskHandle_t tDispTaskHandler;
#endif

//*********************************记忆参数**********************************
//显示模块记忆参数, 存入APP参数区
#pragma pack(1)
typedef struct
{
	u8           ucHighLightValue;
	u8           ucLowLightValue;
	vu16         usAutoOffTime;      //存储息屏的时间,大于0存在有息屏,0为常亮
}DispMemParam_T;
#pragma pack()


//显示任务对外接口
bool bDisp_TaskInit(void);
bool bDisp_SetDevState(DevState_E state);
bool bDisp_Switch(SwitchType_E type, bool fore_en);
void vDisp_TickTimer(void);
bool bDisp_MemParamInit(DispMemParam_T* p_disp_mem);
u16 usDisp_ErrCodeDisplay(void);
void vDisp_PageInitContext(void);
void vDisp_PageSyncByState(DevState_E state);
void vDisp_RequestRefresh(void);
bool bDisp_PageSwitch(DispPageId_E page_id, bool force_refresh);
#if(boardKEY_EN)
bool bDisp_PageHandleKey(KeyTriEvent_e key_event);
#endif

#if(!boardUSE_OS)
void vDisp_Task(void *pvParameters);
#endif  //boardUSE_OS

#if(boardLOW_POWER)
void vLcd_EnterLowPower(void);
void vLcd_ExitLowPower(void);
#endif  //boardLOW_POWER

#endif  //boardDISPLAY_EN

#endif  //MD_DISPLAY_TASK_H_
