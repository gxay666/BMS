#include "App_BMS.h"

/**
 * @brief BMS 应用层初始化
 * @note  依次执行：休眠唤醒 → 加载 ADC 增益/偏移 → 配置保护寄存器
 */
void App_BMS_Init(void)
{
    Int_BQ769_Ship();
    vTaskDelay(300);
    Int_BQ769_WakeUp();

    Int_BQ769_LoadGain();
    Int_BQ769_LoadOffset();
    Int_BQ769_ConfigReg();
}

/**
 * @brief 加载电池信息 (电压/电流/温度/SOC) 并执行均衡
 * @note  每次调用读取全部电芯数据并尝试被动均衡，建议周期性调用
 */
void App_BMS_LoadBatInfo(void)
{
    Int_BQ769_LoadCellVoltage();
    Int_BQ769_LoadPackVoltage();
    Int_BQ769_LoadCurrent();
    Int_BQ769_LoadTemp();
    Int_BQ769_LoadBatSOC();

    Int_BQ769_BatBalance();
}
