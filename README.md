# 智能电池管理系统 (BMS)

> 基于 STM32F1 + HAL/CubeMX + FreeRTOS 的 5 串锂电池管理系统，通过 BQ76920 前端采集电压/电流/温度/SOC 并实时显示，支持多重保护与被动均衡。

## 硬件平台

| 组件 | 型号 |
|------|------|
| MCU | STM32F103C8T6 |
| 电池前端 (AFE) | BQ76920 (5S, I2C 地址 0x08) |
| OLED 显示 | SSD1306 128×64 (I2C 地址 0x78) |
| 采样电阻 | 5 mΩ |
| 电芯配置 | 5S (标称 18.5V) |

## 软件架构

```
┌─────────────────────────────────────┐
│           APP 应用层                  │
│  App_main / App_BMS / App_display    │
├─────────────────────────────────────┤
│           Com 通信层                  │
│       Com_Bq769 (I2C + CRC-8)       │
├─────────────────────────────────────┤
│           Int 驱动层                  │
│  Int_BQ769 (AFE)  /  Int_OLED (显示) │
├─────────────────────────────────────┤
│     HAL + FreeRTOS + CMSIS           │
└─────────────────────────────────────┘
```

## 功能特性

- **电池监测** — 5 路电芯电压、总电压、充放电电流、温度、SOC 估算
- **保护机制** — 过压 (4.2V)、欠压 (3.8V)、过流、短路保护，含延迟消抖
- **被动均衡** — 基于最低电压电芯的相邻避让均衡策略 (温度/电压双条件判定)
- **OLED 显示** — 中文标题 + 电压/温度/电流/SOC 四参数实时刷新
- **CRC-8 校验** — 所有 I2C 读写附带 CRC 校验，通信异常数据标记 0xFF

## 目录结构

```
BMS/
├── APP/                  # 应用层
│   ├── App_main.c/h      # 主任务入口 (FreeRTOS Task)
│   ├── App_BMS.c/h       # BMS 业务逻辑
│   └── App_display.c/h   # OLED 显示管理
├── Com/                  # 通信层
│   └── Com_Bq769.c/h     # BQ76920 I2C 通信 (CRC-8)
├── Int/                  # 驱动层
│   ├── BQ76920/
│   │   ├── Int_BQ769.c/h       # BQ76920 寄存器读写
│   │   └── Int_BQ769_BSP.h     # 寄存器映射 + 阈值定义
│   └── OLED/
│       └── Int_OLED.c/h        # SSD1306 OLED 驱动
├── Core/                 # STM32 HAL 配置
├── Drivers/              # HAL 库
├── FreeRTOS/             # FreeRTOS 内核
├── BMS.ioc               # CubeMX 工程文件
└── README.md
```

## 构建

1. 使用 STM32CubeIDE 或 Keil MDK 打开 `BMS.ioc` / `MDK-ARM/BMS.uvprojx`
2. 编译目标：STM32F103C8T6
3. 下载工具：ST-Link / J-Link

## 关键参数

| 参数 | 值 |
|------|-----|
| I2C 速率 | 100 kHz (标准模式) |
| OLED 刷新周期 | 2 s |
| FreeRTOS 堆 | 12 KB |
| BMS 任务栈 | 512 words |
| 过压阈值 (OV) | 4.20 V |
| 欠压阈值 (UV) | 3.80 V |
| 均衡温差范围 | 0 ~ 40 °C |
| 均衡最低电压 | 3.70 V |

## 许可证

MIT License
