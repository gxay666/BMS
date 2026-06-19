#include "App_display.h"

//pack voltage
extern float pack_voltage_v;
//temperature
extern int8_t temp_c;
//Current
extern float current_a;
//SOC
extern float bat_soc;


static void App_display_showTitle()
{
    for (size_t i = 0; i < 5; i++)
    {
        Inf_OLED_ShowChinese(24 + i*16, 8, i, 16, 1);
    }

}

void App_display_ShowBatInfo(void)
{
    char info_buff[10] = {0};
    sprintf(info_buff, "V:%3.1fV", pack_voltage_v);
    Inf_OLED_ShowString(8, 24, (uint8_t *)info_buff, 16, 1);
    memset(info_buff, 0, sizeof(info_buff));

    sprintf(info_buff, "T:%dC", temp_c);
    Inf_OLED_ShowString(8 + 8 * 8, 24, (uint8_t *)info_buff, 16, 1);
    memset(info_buff, 0, sizeof(info_buff));

    sprintf(info_buff, "I:%3.1fA", current_a);
    Inf_OLED_ShowString(8, 8 + 16 + 16, (uint8_t *)info_buff, 16, 1);
    memset(info_buff, 0, sizeof(info_buff));

    sprintf(info_buff, "S:%3.1f%%", bat_soc);
    Inf_OLED_ShowString(8 + 8 * 8, 8 + 16 + 16, (uint8_t *)info_buff, 16, 1);
    

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

void App_display_show_message(void) 
{
    //清屏
    Inf_OLED_Clear();
    //显示标题
    App_display_showTitle();
    //显示消息
    App_display_ShowBatInfo();
    //刷新显示
    Inf_OLED_Refresh();
}
