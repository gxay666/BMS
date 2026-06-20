#ifndef APP_MAIN_H
#define APP_MAIN_H

#include "FreeRTOS.h"
#include "task.h"
#include "stdio.h"
#include "App_display.h"
#include "App_BMS.h"

/**
 * @brief 应用程序主入口
 * @note  创建 BMS 主任务并启动 FreeRTOS 调度器
 */
void App_main(void);


#endif //
