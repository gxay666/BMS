#include "App_main.h"

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
void App_main(void) 
{
    //创建主任务
    xTaskCreate(App_BMS_task, "App_BMS_task", 512, NULL, 5, NULL);
    
    //启动调度器
    vTaskStartScheduler();
}
