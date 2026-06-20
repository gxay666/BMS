#include "App_display.h"

/** @brief 电池组总电压 (V) */
extern float pack_voltage_v;
/** @brief 电池温度 (°C) */
extern int8_t temp_c;
/** @brief 充放电电流 (A)，正=放电，负=充电 */
extern float current_a;
/** @brief 电池 SOC 百分比 (0~100%) */
extern float bat_soc;

/**
 * @brief 显示 OLED 标题栏 (中文 "智能电池管理系统")
 * @note  5 个中文字符，每个 16×16 像素，从 (24, 8) 开始排列
 */
static void App_display_showTitle()
{
    for (size_t i = 0; i < 5; i++)
    {
        Inf_OLED_ShowChinese(24 + i*16, 8, i, 16, 1);
    }

}

/**
 * @brief 格式化并显示电池信息 (电压/温度/电流/SOC)
 * @note  上半行显示电压和温度，下半行显示电流和 SOC
 */
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

/**
 * @brief OLED 显示初始化
 * @note  依次初始化 I2C2 外设、OLED 控制器、清屏
 */
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

/**
 * @brief OLED 显示电池信息
 * @note  清屏 → 显示标题 → 显示电压/温度/电流/SOC → 刷新 OLED GRAM
 */
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
