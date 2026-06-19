#include "App_main.h"

void App_main_task(void *pvParameters)
{
    App_display_init();
    App_display_show_message("BMS is ready");

    App_BMS_Init();
    
    while(1)
    {  
       App_BMS_LoadBatInfo();    
       vTaskDelay(pdMS_TO_TICKS(2000)); // Delay for 2000 milliseconds     
    }
}
void App_main(void) 
{
    //创建任务
    xTaskCreate(App_main_task, "App_main_task", 512, NULL, 5, NULL);
    
    //启动调度器
    vTaskStartScheduler();
}
