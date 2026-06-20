#include "App_main.h"

/**
 * @brief BMS 主任务入口 (FreeRTOS Task)
 * @param pvParameters 任务参数 (未使用)
 * @note  初始化显示和 BMS 芯片后，每 2 秒轮询一次电池信息并刷新 OLED
 */
void App_BMS_task(void *pvParameters)
{
    App_display_init();

    App_BMS_Init();

    while(1)
    {
       App_BMS_LoadBatInfo();
       App_display_show_message();
       vTaskDelay(pdMS_TO_TICKS(2000)); // Delay for 2000 milliseconds
    }
}

/**
 * @brief 应用程序主入口
 * @note  创建 BMS 主任务并启动 FreeRTOS 调度器
 */
void App_main(void)
{
    //创建主任务
    xTaskCreate(App_BMS_task, "App_BMS_task", 512, NULL, 5, NULL);

    //启动调度器
    vTaskStartScheduler();
}
