#ifndef MD_BMS_QUEUE_TASK_H_
#define MD_BMS_QUEUE_TASK_H_

#include "board_config.h"

#if(boardBMS_EN)
#include "queue_task.h"

#if(boardUSE_OS)
#include "freertos.h"
#include "task.h"
#endif

extern Task_T *tpBmsTask;  //队列任务指针

bool bBms_QueueInit(void);

//队列函数
void v_bms_queue_task_main(Task_T *tp_task);

#if(boardUPDATA)
void v_bms_queue_task_updata(Task_T *tp_task);
#endif  //boardUPDATA

#endif  //boardBMS_EN

#endif  //MD_BMS_QUEUE_FUNC_H_

