#include "App_display.h"

static void App_display_showTitle()
{
    for (size_t i = 0; i < 5; i++)
    {
        Inf_OLED_ShowChinese(24 + i*16, 8, i, 16, 1);
    }
    
}
void App_display_init(void) 
{
    //底层I2C初始化
    MX_I2C2_Init();
    //中间件OLED显示初始化
    Inf_OLED_Init();
    //显示数据初始化
    Inf_OLED_Clear();
    printf("Display initialized\r\n");
    
}

void App_display_show_message(const char *message) 
{
    //显示标题
    App_display_showTitle();
    //显示消息
    Inf_OLED_ShowString(24, 24, (uint8_t *)message, 12, 1);
    //刷新显示
    Inf_OLED_Refresh();
}