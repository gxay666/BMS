#ifndef _APP_BMS_H_

#include "Int_BQ769.h"

/**
 * @brief BMS 应用层初始化
 * @note  依次执行：休眠唤醒 → 加载 ADC 增益/偏移 → 配置保护寄存器
 */
void App_BMS_Init(void);

/**
 * @brief 加载电池信息 (电压/电流/温度/SOC) 并执行均衡
 * @note  每次调用读取全部电芯数据并尝试被动均衡，建议周期性调用
 */
void App_BMS_LoadBatInfo(void);


#define _APP_BMS_H_

#endif // !_APP_BMS_H_


