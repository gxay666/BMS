#ifndef _App_display_h
#define _App_display_h

#include "Int_OLED.h"
#include "stdio.h"

/**
 * @brief OLED 显示初始化
 * @note  依次初始化 I2C2 外设、OLED 控制器、清屏
 */
void App_display_init(void);

/**
 * @brief OLED 显示电池信息
 * @note  清屏 → 显示标题 → 显示电压/温度/电流/SOC → 刷新 OLED GRAM
 */
void App_display_show_message(void);

#endif // !_App_display_h
