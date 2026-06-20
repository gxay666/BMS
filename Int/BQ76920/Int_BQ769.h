#ifndef __INT_BQ769_H_
#define __INT_BQ769_H_
#include "gpio.h"
#include "i2c.h"
#include "FreeRTOS.h"
#include "task.h"
#include "stdbool.h"

#include "Int_BQ769_BSP.h"
#include "Com_Bq769.h"

/** @brief BQ76920 I2C 7-bit 设备地址 */
#define BQ769_ADDRESS 0x08
/** @brief BQ76920 I2C 写地址 (7-bit 左移 1 位) */
#define BQ769_I2C_ADDRESS_WRITE (BQ769_ADDRESS << 1)
/** @brief BQ76920 I2C 读地址 (写地址 | 0x01) */
#define BQ769_I2C_ADDRESS_READ ((BQ769_ADDRESS << 1) | 0x01)

/** @brief 单体过压保护阈值 (V) */
#define OV_Trip 4.2
/** @brief 单体欠压保护阈值 (V) */
#define UV_Trip 3.8

/** @brief 串联电芯数量 (5S) */
#define Cell_Num 5

/**
 * @brief 唤醒 BQ76920 芯片
 * @note  拉高 WAKE 引脚 5ms 后释放，等待 3s 芯片启动完成
 */
void Int_BQ769_WakeUp(void);

/**
 * @brief 使 BQ76920 进入 SHIP 休眠模式
 * @note  通过 SYS_CTRL1 寄存器的 SHUT_A/SHUT_B 控制进入低功耗状态
 */
void Int_BQ769_Ship(void);

/**
 * @brief 复位 BQ76920 (先休眠再唤醒)
 */
void Int_BQ769_Reset(void);

/**
 * @brief 向 BQ76920 寄存器写入单个字节数据
 * @param reg 目标寄存器地址
 * @param data 要写入的 8 位数据
 * @note  自动计算并附带 CRC-8 校验字节
 */
void Int_BQ769_WriteReg(uint8_t reg, uint8_t data);

/**
 * @brief 从 BQ76920 寄存器读取多个字节
 * @param reg      起始寄存器地址
 * @param buff     接收缓冲区指针
 * @param read_len 读取字节数
 * @note  每次读取 1 字节数据 + 1 字节 CRC，校验失败时该字节置 0xFF
 */
void Int_BQ769_ReadReg(uint8_t reg, uint8_t *buff, uint8_t read_len);

/**
 * @brief 从 BQ76920 加载 ADC 增益值 (gain_mv)
 * @note  读取 ADCGAIN1 和 ADCGAIN2 寄存器，计算增益系数 (mV/LSB)
 */
void Int_BQ769_LoadGain(void);

/**
 * @brief 从 BQ76920 加载 ADC 偏移值 (offset_mv)
 * @note  读取 ADCOFFSET 寄存器，单位为 mV
 */
void Int_BQ769_LoadOffset(void);

/**
 * @brief 读取各节电芯电压
 * @note  从 VC1_HI 连续读取 10 字节 (5 节电芯 × 2 字节)，计算后存入 cell_voltage_v[]
 */
void Int_BQ769_LoadCellVoltage(void);

/**
 * @brief 读取电池组总电压
 * @note  从 BAT_HI/BAT_LO 读取，结合 gain/offset 计算后存入 pack_voltage_v
 */
void Int_BQ769_LoadPackVoltage(void);

/**
 * @brief 读取温度传感器
 * @note  从 TS1_HI/TS1_LO 读取 ADC 值，转换为热敏电阻阻值后查表得温度，存入 temp_c
 */
void Int_BQ769_LoadTemp(void);

/**
 * @brief 读取库仑计数器 (CC) 电流值
 * @note  CC 读数为 16-bit 2's complement (µV)，除以采样电阻 (5mΩ) 得出电流 (A)，存入 current_a
 */
void Int_BQ769_LoadCurrent(void);

/**
 * @brief 估算电池 SOC (State of Charge)
 * @note  读取各节电芯电压取平均值 (mV)，查表转换为 SOC 百分比，存入 bat_soc
 */
void Int_BQ769_LoadBatSOC(void);

/**
 * @brief 配置 BQ76920 保护寄存器
 * @note  依次配置 SYS_CTRL1/2、PROTECT1/2/3、OV_TRIP、UV_TRIP、CC_CFG 等寄存器
 */
void Int_BQ769_ConfigReg(void);

/**
 * @brief 电池被动均衡控制
 * @note  找出最低电压电芯，对其他电压过高的电芯开启 CB 均衡，
 *        避开相邻电芯同时均衡，并在温度 0~40°C 且电压 > 3.7V 条件下执行
 */
void Int_BQ769_BatBalance(void);

#endif // __INT_BQ769_H_
