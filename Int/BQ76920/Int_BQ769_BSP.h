#ifndef __INT_BQ769_BSP_H__
#define __INT_BQ769_BSP_H__

#include <stdint.h>

/** @brief 单体过压保护阈值 (V) */
#define TLB_OV_PROTECT 4.25
/** @brief 单体过压恢复阈值 (V) */
#define TLB_OV_RELIEVE 4.10
/** @brief 单体欠压保护阈值 (V) */
#define TLB_UV_PROTECT 2.80
/** @brief 单体欠压恢复阈值 (V) */
#define TLB_UV_RELIEVE 3.00
/** @brief 自动关机电压 (V) */
#define TLB_SHUTDOWN_VOLTAGE 2.80

/** @brief 均衡起始电压 (V) */
#define TLB_BALANCE_VOLTAGE 3.30
/** @brief 均衡允许最大电压差 (V) */
#define TLB_BALANCE_DIFF_VOLTAGE 0.05
/** @brief 均衡允许最低温度 (°C) */
#define TLB_BALANCE_MIN_TEMPR 0
/** @brief 均衡允许最高温度 (°C) */
#define TLB_BALANCE_MAX_TEMPR 40

/** @defgroup BQ769_Registers BQ76920 寄存器地址定义
 *  @{
 */
/*----------------------------------
  系统状态与控制寄存器
----------------------------------*/
/** @brief 系统状态寄存器 (故障标志位) */
#define BQ_SYS_STAT 0x00
/** @brief 电池平衡控制寄存器 1 (Cell 1-5) */
#define BQ_CELLBAL1 0x01
/** @brief 电池平衡控制寄存器 2 (Cell 6-10) */
#define BQ_CELLBAL2 0x02
/** @brief 系统控制寄存器 1 (ADC 使能 / 关断) */
#define BQ_SYS_CTRL1 0x04
/** @brief 系统控制寄存器 2 (负载开关 / CC 控制) */
#define BQ_SYS_CTRL2 0x05

/*----------------------------------
  保护配置寄存器
----------------------------------*/
/** @brief 保护使能寄存器 (OV/UV/OCD/SCD 阈值) */
#define BQ_PROTECT1 0x06
/** @brief 二级保护阈值寄存器 */
#define BQ_PROTECT2 0x07
/** @brief 保护延迟时间配置寄存器 */
#define BQ_PROTECT3 0x08
/** @brief 过压阈值寄存器 (16mV/bit) */
#define BQ_OV_TRIP 0x09
/** @brief 欠压阈值寄存器 (16mV/bit) */
#define BQ_UV_TRIP 0x0A
/** @brief 库仑计数器配置寄存器 */
#define BQ_CC_CFG 0x0B

/*----------------------------------
  电池电压检测寄存器（10节）
----------------------------------*/
/** @{ */
/** @brief Cell 1 电压高字节 [11:4] */
#define BQ_VC1_HI 0x0C
/** @brief Cell 1 电压低字节 [3:0] (左对齐) */
#define BQ_VC1_LO 0x0D
#define BQ_VC2_HI 0x0E  /**< Cell 2 电压高字节 */
#define BQ_VC2_LO 0x0F  /**< Cell 2 电压低字节 */
#define BQ_VC3_HI 0x10  /**< Cell 3 电压高字节 */
#define BQ_VC3_LO 0x11  /**< Cell 3 电压低字节 */
#define BQ_VC4_HI 0x12  /**< Cell 4 电压高字节 */
#define BQ_VC4_LO 0x13  /**< Cell 4 电压低字节 */
#define BQ_VC5_HI 0x14  /**< Cell 5 电压高字节 */
#define BQ_VC5_LO 0x15  /**< Cell 5 电压低字节 */

/** Cell 6-10 (仅 BQ76930/40 支持) */
#define BQ_VC6_HI 0x16  /**< Cell 6 电压高字节 */
#define BQ_VC6_LO 0x17  /**< Cell 6 电压低字节 */
#define BQ_VC7_HI 0x18  /**< Cell 7 电压高字节 */
#define BQ_VC7_LO 0x19  /**< Cell 7 电压低字节 */
#define BQ_VC8_HI 0x1A  /**< Cell 8 电压高字节 */
#define BQ_VC8_LO 0x1B  /**< Cell 8 电压低字节 */
#define BQ_VC9_HI 0x1C  /**< Cell 9 电压高字节 */
#define BQ_VC9_LO 0x1D  /**< Cell 9 电压低字节 */
#define BQ_VC10_HI 0x1E /**< Cell 10 电压高字节 */
#define BQ_VC10_LO 0x1F /**< Cell 10 电压低字节 */
/** @} */

/** @brief 电池组总电压高字节 (14-bit ADC, 2.44mV/bit) */
#define BQ_BAT_HI 0x2A
/** @brief 电池组总电压低字节 (低 6 位有效) */
#define BQ_BAT_LO 0x2B

/*----------------------------------
  温度检测寄存器
----------------------------------*/
/** @brief 温度传感器 1 高字节 */
#define BQ_TS1_HI 0x2C
/** @brief 温度传感器 1 低字节 */
#define BQ_TS1_LO 0x2D
/** @brief 温度传感器 2 高字节 (仅 BQ76930/40) */
#define BQ_TS2_HI 0x2E
/** @brief 温度传感器 2 低字节 (仅 BQ76930/40) */
#define BQ_TS2_LO 0x2F
/** @brief 库仑计数器高字节 */
#define BQ_CC_HI 0x32
/** @brief 库仑计数器低字节 */
#define BQ_CC_LO 0x33

/** @brief ADC 增益系数寄存器 1 (高 2-bit) */
#define BQ_ADCGAIN1 0x50
/** @brief ADC 偏移量寄存器 (有符号 8-bit) */
#define BQ_ADCOFFSET 0x51
/** @brief ADC 增益系数寄存器 2 (低 3-bit) */
#define BQ_ADCGAIN2 0x59
/** @} */ /* end of BQ769_Registers */

/**
 * @brief BQ76920 全寄存器镜像结构体
 * @note  每个寄存器以 union { bit-field struct; byte } 形式定义，
 *        支持按位操作 (xxxBit.xxx) 和整体字节操作 (xxxByte)
 */
typedef struct _Register_Group
{
	/** @brief 系统状态寄存器 (0x00)
	 *  @note  OCD/SCD/OV/UV = 故障标志，CC_READY = CC 数据就绪
	 */
	union
	{
		struct
		{
			unsigned char OCD : 1;            /**< 过流故障标志 */
			unsigned char SCD : 1;            /**< 短路故障标志 */
			unsigned char OV : 1;             /**< 过压故障标志 */
			unsigned char UV : 1;             /**< 欠压故障标志 */
			unsigned char OVRD_ALERT : 1;     /**< 过放警报标志 */
			unsigned char DEVICE_XREADY : 1;  /**< 设备准备就绪标志 */
			unsigned char WAKE : 1;           /**< 唤醒状态标志 */
			unsigned char CC_READY : 1;       /**< CC 数据就绪标志 */
		} StatusBit;
		unsigned char StatusByte;
	} SysStatus;

	/** @brief 电池均衡控制寄存器 1 (0x01) — Cell 1-5 */
	union
	{
		struct
		{
			unsigned char CB1 : 1;   /**< Cell 1 均衡使能 */
			unsigned char CB2 : 1;   /**< Cell 2 均衡使能 */
			unsigned char CB3 : 1;   /**< Cell 3 均衡使能 */
			unsigned char CB4 : 1;   /**< Cell 4 均衡使能 */
			unsigned char CB5 : 1;   /**< Cell 5 均衡使能 */
			unsigned char RSVD : 3;  /**< 保留位 */
		} CellBal1Bit;
		unsigned char CellBal1Byte;
	} CellBal1;

	/** @brief 电池均衡控制寄存器 2 (0x02) — Cell 6-10 */
	union
	{
		struct
		{
			unsigned char RSVD : 3;   /**< 保留位 */
			unsigned char CB10 : 1;   /**< Cell 10 均衡使能 */
			unsigned char CB9 : 1;    /**< Cell 9 均衡使能 */
			unsigned char CB8 : 1;    /**< Cell 8 均衡使能 */
			unsigned char CB7 : 1;    /**< Cell 7 均衡使能 */
			unsigned char CB6 : 1;    /**< Cell 6 均衡使能 */
		} CellBal2Bit;
		unsigned char CellBal2Byte;
	} CellBal2;

	/** @brief 电池均衡控制寄存器 3 (0x03) — Cell 11-15 */
	union
	{
		struct
		{
			unsigned char RSVD : 3;   /**< 保留位 */
			unsigned char CB15 : 1;   /**< Cell 15 均衡使能 */
			unsigned char CB14 : 1;   /**< Cell 14 均衡使能 */
			unsigned char CB13 : 1;   /**< Cell 13 均衡使能 */
			unsigned char CB12 : 1;   /**< Cell 12 均衡使能 */
			unsigned char CB11 : 1;   /**< Cell 11 均衡使能 */
		} CellBal3Bit;
		unsigned char CellBal3Byte;
	} CellBal3;

	/** @brief 系统控制寄存器 1 (0x04)
	 *  @note  SHUT_A/SHUT_B 组合控制休眠，ADC_EN 使能 ADC，TEMP_SEL 选择测温方式
	 */
	union
	{
		struct
		{
			unsigned char SHUT_B : 1;        /**< 休眠控制 B (先拉高 B 再 A 进入休眠) */
			unsigned char SHUT_A : 1;        /**< 休眠控制 A */
			unsigned char RSVD1 : 1;         /**< 保留位 */
			unsigned char TEMP_SEL : 1;      /**< 测温选择: 0=外部热敏电阻, 1=内部温度传感器 */
			unsigned char ADC_EN : 1;        /**< ADC 使能: 1=开启 */
			unsigned char RSVD2 : 2;         /**< 保留位 */
			unsigned char LOAD_PRESENT : 1;  /**< 负载存在检测标志 */
		} SysCtrl1Bit;
		unsigned char SysCtrl1Byte;
	} SysCtrl1;

	/** @brief 系统控制寄存器 2 (0x05)
	 *  @note  控制充放电 MOSFET、CC 模式、唤醒使能
	 */
	union
	{
		struct
		{
			unsigned char CHG_ON : 1;      /**< 充电 MOSFET: 1=开启 */
			unsigned char DSG_ON : 1;      /**< 放电 MOSFET: 1=开启 */
			unsigned char WAKE_T : 2;      /**< 唤醒定时器配置 (2-bit) */
			unsigned char WAKE_EN : 1;     /**< 唤醒使能 */
			unsigned char CC_ONESHOT : 1;  /**< CC 单次采样: 1=触发一次 */
			unsigned char CC_EN : 1;       /**< CC 连续采样: 1=使能 */
			unsigned char DELAY_DIS : 1;   /**< 延迟禁用: 0=使能保护延迟 */
		} SysCtrl2Bit;
		unsigned char SysCtrl2Byte;
	} SysCtrl2;

	/** @brief 保护配置寄存器 1 (0x06) — 短路保护参数 */
	union
	{
		struct
		{
			unsigned char SCD_THRESH : 3;  /**< 短路检测阈值 (mV) */
			unsigned char SCD_DELAY : 2;   /**< 短路检测延迟时间 */
			unsigned char RSVD : 2;        /**< 保留位 */
			unsigned char RSNS : 1;        /**< 电流检测阈值翻倍: 0=默认, 1=翻倍 (R_sense ≥ 5mΩ) */
		} Protect1Bit;
		unsigned char Protect1Byte;
	} Protect1;

	/** @brief 保护配置寄存器 2 (0x07) — 过流保护参数 */
	union
	{
		struct
		{
			unsigned char OCD_THRESH : 4;  /**< 过流检测阈值 (mV) */
			unsigned char OCD_DELAY : 3;   /**< 过流检测延迟时间 */
			unsigned char RSVD : 1;        /**< 保留位 */
		} Protect2Bit;
		unsigned char Protect2Byte;
	} Protect2;

	/** @brief 保护配置寄存器 3 (0x08) — 过压/欠压延迟 */
	union
	{
		struct
		{
			unsigned char RSVD : 4;       /**< 保留位 */
			unsigned char OV_DELAY : 2;   /**< 过压检测延迟时间 */
			unsigned char UV_DELAY : 2;   /**< 欠压检测延迟时间 */
		} Protect3Bit;
		unsigned char Protect3Byte;
	} Protect3;

	unsigned char OVTrip;  /**< 过压阈值寄存器镜像 (0x09) */
	unsigned char UVTrip;  /**< 欠压阈值寄存器镜像 (0x0A) */
	unsigned char CCCfg;   /**< CC 配置寄存器镜像 (0x0B, 推荐值 0x19) */

	/** @brief Cell 1 电压寄存器 (0x0C/0x0D) */
	union { struct { unsigned char VC1_HI; unsigned char VC1_LO; } VCell1Byte; unsigned short VCell1Word; } VCell1;
	/** @brief Cell 2 电压寄存器 (0x0E/0x0F) */
	union { struct { unsigned char VC2_HI; unsigned char VC2_LO; } VCell2Byte; unsigned short VCell2Word; } VCell2;
	/** @brief Cell 3 电压寄存器 (0x10/0x11) */
	union { struct { unsigned char VC3_HI; unsigned char VC3_LO; } VCell3Byte; unsigned short VCell3Word; } VCell3;
	/** @brief Cell 4 电压寄存器 (0x12/0x13) */
	union { struct { unsigned char VC4_HI; unsigned char VC4_LO; } VCell4Byte; unsigned short VCell4Word; } VCell4;
	/** @brief Cell 5 电压寄存器 (0x14/0x15) */
	union { struct { unsigned char VC5_HI; unsigned char VC5_LO; } VCell5Byte; unsigned short VCell5Word; } VCell5;
	/** @brief Cell 6 电压寄存器 (0x16/0x17) */
	union { struct { unsigned char VC6_HI; unsigned char VC6_LO; } VCell6Byte; unsigned short VCell6Word; } VCell6;
	/** @brief Cell 7 电压寄存器 (0x18/0x19) */
	union { struct { unsigned char VC7_HI; unsigned char VC7_LO; } VCell7Byte; unsigned short VCell7Word; } VCell7;
	/** @brief Cell 8 电压寄存器 (0x1A/0x1B) */
	union { struct { unsigned char VC8_HI; unsigned char VC8_LO; } VCell8Byte; unsigned short VCell8Word; } VCell8;
	/** @brief Cell 9 电压寄存器 (0x1C/0x1D) */
	union { struct { unsigned char VC9_HI; unsigned char VC9_LO; } VCell9Byte; unsigned short VCell9Word; } VCell9;
	/** @brief Cell 10 电压寄存器 (0x1E/0x1F) */
	union { struct { unsigned char VC10_HI; unsigned char VC10_LO; } VCell10Byte; unsigned short VCell10Word; } VCell10;
	/** @brief Cell 11 电压寄存器 */
	union { struct { unsigned char VC11_HI; unsigned char VC11_LO; } VCell11Byte; unsigned short VCell11Word; } VCell11;
	/** @brief Cell 12 电压寄存器 */
	union { struct { unsigned char VC12_HI; unsigned char VC12_LO; } VCell12Byte; unsigned short VCell12Word; } VCell12;
	/** @brief Cell 13 电压寄存器 */
	union { struct { unsigned char VC13_HI; unsigned char VC13_LO; } VCell13Byte; unsigned short VCell13Word; } VCell13;
	/** @brief Cell 14 电压寄存器 */
	union { struct { unsigned char VC14_HI; unsigned char VC14_LO; } VCell14Byte; unsigned short VCell14Word; } VCell14;
	/** @brief Cell 15 电压寄存器 */
	union { struct { unsigned char VC15_HI; unsigned char VC15_LO; } VCell15Byte; unsigned short VCell15Word; } VCell15;

	/** @brief 电池组总电压寄存器 (0x2A/0x2B) */
	union { struct { unsigned char BAT_HI; unsigned char BAT_LO; } VBatByte; unsigned short VBatWord; } VBat;

	/** @brief 温度传感器 1 寄存器 (0x2C/0x2D) */
	union { struct { unsigned char TS1_HI; unsigned char TS1_LO; } TS1Byte; unsigned short TS1Word; } TS1;
	/** @brief 温度传感器 2 寄存器 (0x2E/0x2F) */
	union { struct { unsigned char TS2_HI; unsigned char TS2_LO; } TS2Byte; unsigned short TS2Word; } TS2;
	/** @brief 温度传感器 3 寄存器 (仅 BQ76940) */
	union { struct { unsigned char TS3_HI; unsigned char TS3_LO; } TS3Byte; unsigned short TS3Word; } TS3;

	/** @brief 库仑计数器寄存器 (0x32/0x33, 16-bit 2's complement) */
	union { struct { unsigned char CC_HI; unsigned char CC_LO; } CCByte; unsigned short CCWord; } CC;

	/** @brief ADC 增益寄存器 1 (0x50) — 高 2-bit */
	union
	{
		struct
		{
			unsigned char RSVD1 : 2;        /**< 保留位 */
			unsigned char ADCGAIN_4_3 : 2;  /**< ADC 增益 bit[4:3] */
			unsigned char RSVD2 : 4;        /**< 保留位 */
		} ADCGain1Bit;
		unsigned char ADCGain1Byte;
	} ADCGain1;

	unsigned char ADCOffset; /**< ADC 偏移量寄存器 (0x51, 有符号 8-bit) */

	/** @brief ADC 增益寄存器 2 (0x59) — 低 3-bit */
	union
	{
		struct
		{
			unsigned char RSVD : 5;         /**< 保留位 */
			unsigned char ADCGAIN_2_0 : 3;  /**< ADC 增益 bit[2:0] */
		} ADCGain2Bit;
		unsigned char ADCGain2Byte;
	} ADCGain2;

} RegisterGroup;

/** @brief BQ76920 短路保护延迟时间选项 */
typedef enum
{
	BMS_SCD_DELAY_50us = 0x00,  /**< 短路延迟 50µs */
	BMS_SCD_DELAY_100us = 0x01, /**< 短路延迟 100µs */
	BMS_SCD_DELAY_200us = 0x02, /**< 短路延迟 200µs */
	BMS_SCD_DELAY_400us = 0x03, /**< 短路延迟 400µs */
} BMS_SCDDelayTypedef;

/** @brief BQ76920 过流保护延迟时间选项 */
typedef enum
{
	BMS_OCD_DEALY_10ms = 0x00,   /**< 过流延迟 10ms */
	BMS_OCD_DELAY_20ms = 0x01,   /**< 过流延迟 20ms */
	BMS_OCD_DELAY_40ms = 0x02,   /**< 过流延迟 40ms */
	BMS_OCD_DELAY_80ms = 0x03,   /**< 过流延迟 80ms */
	BMS_OCD_DELAY_160ms = 0x04,  /**< 过流延迟 160ms */
	BMS_OCD_DELAY_320ms = 0x05,  /**< 过流延迟 320ms */
	BMS_OCD_DELAY_640ms = 0x06,  /**< 过流延迟 640ms */
	BMS_OCD_DELAY_1280ms = 0x07, /**< 过流延迟 1280ms */
} BMS_OCDDelayTypedef;

/** @brief BQ76920 过压保护延迟时间选项 */
typedef enum
{
	BMS_OV_DELAY_1s = 0x00, /**< 过压延迟 1s */
	BMS_OV_DELAY_2s = 0x01, /**< 过压延迟 2s */
	BMS_OV_DELAY_4s = 0x02, /**< 过压延迟 4s */
	BMS_OV_DELAY_8s = 0x03, /**< 过压延迟 8s */
} BMS_OVDelayTypedef;

/** @brief BQ76920 欠压保护延迟时间选项 */
typedef enum
{
	BMS_UV_DELAY_1s = 0x00,  /**< 欠压延迟 1s */
	BMS_UV_DELAY_4s = 0x01,  /**< 欠压延迟 4s */
	BMS_UV_DELAY_8s = 0x02,  /**< 欠压延迟 8s */
	BMS_UV_DELAY_16s = 0x03, /**< 欠压延迟 16s */
} BMS_UVDelayTypedef;

/** @brief BQ76920 电芯索引位掩码 (用于均衡/保护等位操作) */
typedef enum
{
	BQ_CELL_NONE = 0x0000,  /**< 无电芯选中 */
	BQ_CELL_INDEX1 = 0x0001,  /**< Cell 1  位掩码 */
	BQ_CELL_INDEX2 = 0x0002,  /**< Cell 2  位掩码 */
	BQ_CELL_INDEX3 = 0x0004,  /**< Cell 3  位掩码 */
	BQ_CELL_INDEX4 = 0x0008,  /**< Cell 4  位掩码 */
	BQ_CELL_INDEX5 = 0x0010,  /**< Cell 5  位掩码 */
	BQ_CELL_INDEX6 = 0x0020,  /**< Cell 6  位掩码 */
	BQ_CELL_INDEX7 = 0x0040,  /**< Cell 7  位掩码 */
	BQ_CELL_INDEX8 = 0x0080,  /**< Cell 8  位掩码 */
	BQ_CELL_INDEX9 = 0x0100,  /**< Cell 9  位掩码 */
	BQ_CELL_INDEX10 = 0x0200, /**< Cell 10 位掩码 */
	BQ_CELL_INDEX11 = 0x0400, /**< Cell 11 位掩码 */
	BQ_CELL_INDEX12 = 0x0800, /**< Cell 12 位掩码 */
	BQ_CELL_INDEX13 = 0x1000, /**< Cell 13 位掩码 */
	BQ_CELL_INDEX14 = 0x2000, /**< Cell 14 位掩码 */
	BQ_CELL_INDEX15 = 0x4000, /**< Cell 15 位掩码 */
	BQ_CELL_ALL = 0x3FFF,     /**< 全部电芯 (bit 0-13) */
} BQ_CellIndexTypedef;

#endif
