#include "Int_BQ769.h"

/** @brief BQ76920 寄存器镜像结构体 (内存缓存) */
RegisterGroup BQ769_RegisterGroup;
/** @brief ADC 增益系数 (mV/LSB) */
static float gain_mv = 0;
/** @brief ADC 偏移值 (mV) */
static int8_t offset_mv = 0;
/** @brief 各节电芯电压数组 (V)，索引 0~4 */
static float cell_voltage_v[5] = {0};
/** @brief 电池组总电压 (V) */
float pack_voltage_v = 0;
/** @brief 电池温度 (°C) */
int8_t temp_c = 0;
/** @brief 充放电电流 (A)，正=放电，负=充电 */
float current_a = 0;
/** @brief 电池 SOC 百分比 (0.0~100.0) */
float bat_soc = 0;
/** @brief 各节电芯均衡标志，1=需要均衡，0=不需均衡 */
uint8_t need_balance[5] = {0};

/**
 * CRC-8 校验算法实现
 * 多项式: x^8 + x^2 + x + 1 (0x07)
 * 初始值: 0x00
 *
 * @param data 输入数据指针
 * @param size 数据长度
 * @return CRC-8 校验值
 */
static uint8_t crc8(uint8_t *data, uint16_t length)
{
    uint8_t i;
    uint8_t crc = 0;        // Initial value
    while(length--)
    {
        crc ^= *data++;        // crc ^= *data; data++;
        for ( i = 0; i < 8; i++ )
        {
            if ( crc & 0x80 )
                crc = (crc << 1) ^ 0x07;
            else
                crc <<= 1;
        }
    }
    return crc;
}

/**
 * @brief 唤醒
 */
void Int_BQ769_WakeUp(void)
{
    HAL_GPIO_WritePin(BQ76920_WKP_GPIO_Port, BQ76920_WKP_Pin, GPIO_PIN_SET);
    printf("BQ769 WakeUp\r\n");
    vTaskDelay(5); // 拉高5ms
    HAL_GPIO_WritePin(BQ76920_WKP_GPIO_Port, BQ76920_WKP_Pin, GPIO_PIN_RESET);
    vTaskDelay(3000); // 芯片启动延迟
}

/**
 * @brief 休眠
 * @note 休眠时需要先拉高SHUT_B再拉高SHUT_A，才能进入休眠状态。否则可能会进入一种不稳定的状态，导致无法唤醒。
 *       即先将SHUT_B置1，SHUT_A置0，再将SHUT_B置0，SHUT_A置1，最后等待100ms给芯片进入休眠状态的时间。
 */
void Int_BQ769_Ship(void)
{
    //   Int_BQ769_WriteReg(0x04, 0x01);
    //   Int_BQ769_WriteReg(0x04, 0x02);
    BQ769_RegisterGroup.SysCtrl1.SysCtrl1Byte = 0x00;
    Int_BQ769_WriteReg(BQ_SYS_CTRL1, BQ769_RegisterGroup.SysCtrl1.SysCtrl1Byte);

    BQ769_RegisterGroup.SysCtrl1.SysCtrl1Bit.SHUT_A = 0;
    BQ769_RegisterGroup.SysCtrl1.SysCtrl1Bit.SHUT_B = 1;
    Int_BQ769_WriteReg(BQ_SYS_CTRL1, BQ769_RegisterGroup.SysCtrl1.SysCtrl1Byte);

    BQ769_RegisterGroup.SysCtrl1.SysCtrl1Bit.SHUT_A = 1;
    BQ769_RegisterGroup.SysCtrl1.SysCtrl1Bit.SHUT_B = 0;
    Int_BQ769_WriteReg(BQ_SYS_CTRL1, BQ769_RegisterGroup.SysCtrl1.SysCtrl1Byte);
    vTaskDelay(100); // 给睡眠时间
}

/**
 * @brief 重置
 */
void Int_BQ769_Reset(void)
{
    Int_BQ769_Ship();

    Int_BQ769_WakeUp();
}

/**
 * @brief 向BQ769的寄存器写入数据（单个字节)
 */
void Int_BQ769_WriteReg(uint8_t reg_addr, uint8_t data)
{
    // 计算CRC-8校验值
    uint8_t crc_calc_buff[3] = {BQ769_I2C_ADDRESS_WRITE, reg_addr, data};
    uint8_t crc = crc8(crc_calc_buff, 3);
    // 将数据和CRC一起发送
    uint8_t send_buff[2] = {data, crc}; // 最后一个字节保留给CRC
    //添加临界区保护，防止在发送过程中被打断
    taskENTER_CRITICAL();
    HAL_I2C_Mem_Write(&hi2c2, BQ769_I2C_ADDRESS_WRITE, reg_addr, I2C_MEMADD_SIZE_8BIT, send_buff, 2, 1000);
    taskEXIT_CRITICAL();
     //vTaskDelay(1);考虑写周期
    
}

/**
 * @brief 从BQ769的寄存器读取多个字节
 */
void Int_BQ769_ReadReg(uint8_t reg, uint8_t *buff, uint8_t read_len)
{
    // 读取数据和CRC
    uint8_t *rev_buff = pvPortCalloc(read_len * 2, sizeof(uint8_t));
    //添加临界区保护，防止在读取过程中被打断
    taskENTER_CRITICAL();
    HAL_I2C_Mem_Read(&hi2c2, BQ769_I2C_ADDRESS_READ, reg, I2C_MEMADD_SIZE_8BIT, rev_buff, read_len * 2, 1000);
    taskEXIT_CRITICAL();

    for (size_t i = 0; i < read_len; i++)
    {
        uint8_t crc = 0;
        uint8_t data = rev_buff[i * 2];
        uint8_t crc_recv = rev_buff[i * 2 + 1];
        // 计算CRC-8校验值
        if (i == 0)
        {
            uint8_t crc_calc_buff[2] = {BQ769_I2C_ADDRESS_READ, data};
            crc = crc8(crc_calc_buff, 2);
        }
        else
        {
            crc = crc8(&data, 1);
        }
        // 验证CRC
        if (crc == crc_recv)
        {
            buff[i] = data;
        }
        else
        {
            buff[i] = 0xFF;
            printf("BQ769_ReadReg: CRC Error, data=0x%02X, crc=0x%02X, crc_recv=0x%02X\r\n", data, crc, crc_recv);
        }       
    }
    vPortFree(rev_buff);

}

/**
 * @brief 从 BQ76920 加载 ADC 增益值
 * @note  读取 ADCGAIN1 (0x50) 和 ADCGAIN2 (0x59)，合并为 5-bit 增益值，
 *        计算公式: gain_mv = (gain_adc_value + 365) / 1000 (mV/LSB)
 */
void Int_BQ769_LoadGain(void)
{
    uint8_t gain1 = 0;
    uint8_t gain2 = 0;
    Int_BQ769_ReadReg(BQ_ADCGAIN1, &gain1, 1);
    Int_BQ769_ReadReg(BQ_ADCGAIN2, &gain2, 1);

    uint8_t gain_adc_value = ((gain1 & 0x0C) << 1) | (gain2 >> 5);

    gain_mv = (gain_adc_value + 365) / 1000.0f;
    printf("BQ769 Load Gain: gain_adc_value=%d, gain_mv=%fmv/LSB\r\n", gain_adc_value, gain_mv);
}

/**
 * @brief 从 BQ76920 加载 ADC 偏移值
 * @note  读取 ADCOFFSET (0x51) 寄存器，该值为有符号 8-bit (mV)
 */
void Int_BQ769_LoadOffset(void)
{
    Int_BQ769_ReadReg(BQ_ADCOFFSET, (uint8_t *)&offset_mv, 1);
    printf("BQ769 Load Offset: offset_mv=%dmv/LSB\r\n", offset_mv);
}

/**
 * @brief 读取各节电芯电压并存入 cell_voltage_v[]
 * @note  从 VC1_HI (0x0C) 连续读取 10 字节 (5 节 × 2 字节)，
 *        14-bit ADC 值 (取高 6+8 位) × gain_mv + offset_mv 转换为 V
 */
void Int_BQ769_LoadCellVoltage(void)
{
    printf("--------------单个电芯电压-----------------\r\n");
    uint8_t cell_voltage_buff[10] = {0};
    Int_BQ769_ReadReg(BQ_VC1_HI, cell_voltage_buff, 10);
    for (size_t i = 0; i < 5; i++)
    {
        uint16_t cell_adc_value = ((cell_voltage_buff[i * 2] &0x3F) << 8) | cell_voltage_buff[i * 2 + 1];
        cell_voltage_v[i] = (cell_adc_value * gain_mv + offset_mv) / 1000.0f;
        printf("Cell%d Voltage: adc_value=%d, voltage=%fv\r\n", i + 1, cell_adc_value, cell_voltage_v[i]);
    }
}

/**
 * @brief 读取电池组总电压并存入 pack_voltage_v
 * @note  从 BAT_HI/BAT_LO (0x2A/0x2B) 读取 14-bit ADC 值，
 *        总电压 = (ADC × gain_mv × 4 + offset_mv × Cell_Num) / 1000 (V)
 */
void Int_BQ769_LoadPackVoltage(void)
{
    printf("--------------总电压-----------------\r\n");
    uint8_t pack_voltage_buff[2] = {0};
    Int_BQ769_ReadReg(BQ_BAT_HI, pack_voltage_buff, 2); 
    uint16_t pack_adc_value = ((pack_voltage_buff[0] & 0x3F) << 8) | pack_voltage_buff[1];
    pack_voltage_v = (pack_adc_value * gain_mv * 4 + offset_mv * Cell_Num) / 1000.0f;
    printf("Pack Voltage: adc_value=%d, voltage=%fv\r\n", pack_adc_value, pack_voltage_v);
}
/**
 * @brief 读取库仑计数器电流值并存入 current_a
 * @note  CC Reading (µV) = [16-bit 2’s Complement Value] × (8.44 µV/LSB)
 *        电流 (A) = CC 读数 (µV) / 采样电阻 (5mΩ) / 1000
 */
void Int_BQ769_LoadCurrent(void)
{
    printf("--------------电流-----------------\r\n");
    uint8_t current_buff[2] = {0};
    Int_BQ769_ReadReg(BQ_CC_HI, current_buff, 2); 
    uint16_t current_adc_value = ((current_buff[0] & 0x3F) << 8) | current_buff[1];
    current_a = current_adc_value * 8.44f / (5 * 1000.0f); // 将ADC值转换为电流值，单位为安培
    printf("Current: adc_value=%d, current=%fA\r\n", current_adc_value, current_a);
} 
/**
 * @brief 读取温度传感器并存入 temp_c
 * @note  VTSX (V) = ADC × 382 µV/LSB
 *        RTS (Ω) = (10000 × VTSX) ÷ (3.3 – VTSX)
 *        再通过查表 Com_BQ769_getTemperByResist() 转换为 °C
 */
void Int_BQ769_LoadTemp(void)
{
    printf("--------------温度-----------------\r\n");
    uint8_t temp_buff[2] = {0};
    Int_BQ769_ReadReg(BQ_TS1_HI, temp_buff, 2); 
    int16_t temp_adc_value = ((temp_buff[0] & 0x3F) << 8) | temp_buff[1];
    float VTSX = temp_adc_value * 0.000382f; // 将ADC值转换为电压值，单位为伏特
    float RTS = (10000.0f * VTSX) / (3.3f - VTSX); // 根据电压值计算热敏电阻的阻值
    temp_c = Com_BQ769_getTemperByResist((int)RTS); // 根据热敏电阻的阻值计算温度，单位为摄氏度
    printf("Temperature: adc_value=%d, temperature=%d°C\r\n", temp_adc_value, temp_c);
}

/**
 * @brief 估算电池 SOC 并存入 bat_soc
 * @note  读取各节电芯电压取平均值 (mV)，通过查表 Com_BQ769_getPercentByVoltage() 转为百分比
 */
void Int_BQ769_LoadBatSOC(void)
{
    printf("--------------电池SOC-----------------\r\n");
    // 直接读取电芯电压寄存器，不走LoadCellVoltage避免多余打印
    uint8_t cell_buff[10] = {0};
    Int_BQ769_ReadReg(BQ_VC1_HI, cell_buff, 10);
    float sum_cell_mv = 0;
    for (size_t i = 0; i < Cell_Num; i++)
    {
        uint16_t cell_adc = ((cell_buff[i * 2] & 0x3F) << 8) | cell_buff[i * 2 + 1];
        cell_voltage_v[i] = (cell_adc * gain_mv + offset_mv) / 1000.0f; // 同步更新
        sum_cell_mv += cell_voltage_v[i] * 1000.0f;
    }
    uint16_t avg_cell_mv = (uint16_t)(sum_cell_mv / Cell_Num);
    // 查表：电压 -> SOC百分比
    bat_soc = Com_BQ769_getPercentByVoltage(avg_cell_mv);
    printf("Battery SOC: avg_cell_voltage=%dmV, SOC=%f%%\r\n", avg_cell_mv, bat_soc);
}
/**
 * @brief 配置 BQ76920 全部保护寄存器
 * @note  依次配置:
 *        - SYS_CTRL1: ADC 使能、热敏电阻测温模式
 *        - SYS_CTRL2: CC 连续计数使能、充放电 MOSFET 默认开启
 *        - PROTECT1: 短路保护阈值 (10mV) / 延迟 (200µs)、RSNS 翻倍 (≥5mΩ)
 *        - PROTECT2: 过流保护阈值 (160mV) / 延迟 (640ms)
 *        - PROTECT3: 过压延迟 (4s)、欠压延迟 (4s)
 *        - OV_TRIP/UV_TRIP: 根据 gain/offset 计算阈值
 *        - CC_CFG: 连续 CC 计数阈值 (25mV)
 */
void Int_BQ769_ConfigReg(void)
{
    // 配置SYS_ctrl1寄存器
    BQ769_RegisterGroup.SysCtrl1.SysCtrl1Byte = 0x00;
    BQ769_RegisterGroup.SysCtrl1.SysCtrl1Bit.ADC_EN = 1;// 使能ADC
    BQ769_RegisterGroup.SysCtrl1.SysCtrl1Bit.TEMP_SEL = 1;// 选择热敏电阻测温
    Int_BQ769_WriteReg(BQ_SYS_CTRL1, BQ769_RegisterGroup.SysCtrl1.SysCtrl1Byte);
    // 配置SYS_ctrl2寄存器
    BQ769_RegisterGroup.SysCtrl2.SysCtrl2Byte = 0x00;
    BQ769_RegisterGroup.SysCtrl2.SysCtrl2Bit.DELAY_DIS = 0; // 使能过压欠压延迟
    BQ769_RegisterGroup.SysCtrl2.SysCtrl2Bit.CC_EN = 1; // 使能连续CC计数
    BQ769_RegisterGroup.SysCtrl2.SysCtrl2Bit.CHG_ON = 1; // 充电MOSFET默认开启
    BQ769_RegisterGroup.SysCtrl2.SysCtrl2Bit.DSG_ON = 1; // 放电MOSFET默认开启
    Int_BQ769_WriteReg(BQ_SYS_CTRL2, BQ769_RegisterGroup.SysCtrl2.SysCtrl2Byte);
    // 配置保护寄存器1
    BQ769_RegisterGroup.Protect1.Protect1Byte = 0x00;
    BQ769_RegisterGroup.Protect1.Protect1Bit.RSNS= 1; // 提高电流检测的阈值（被测电阻>=5mΩ时需要翻倍）
    BQ769_RegisterGroup.Protect1.Protect1Bit.SCD_DELAY = BMS_SCD_DELAY_200us; // 短路检测延迟时间设置为200us,具体得实验室测试验证，过短可能会误触发，过长可能会延迟保护响应·
    BQ769_RegisterGroup.Protect1.Protect1Bit.SCD_THRESH = 0x3; // 短路检测阈值设置为10mV,具体得实验室测试验证，过低可能会误触发，过高可能会延迟保护响应
    Int_BQ769_WriteReg(BQ_PROTECT1, BQ769_RegisterGroup.Protect1.Protect1Byte);
    // 配置保护寄存器2
    BQ769_RegisterGroup.Protect2.Protect2Byte = 0x00;
    BQ769_RegisterGroup.Protect2.Protect2Bit.OCD_DELAY = BMS_OCD_DELAY_640ms; // 过流检测延迟时间设置为1ms,具体得实验室测试验证，过短可能会误触发，过长可能会延迟保护响应
    BQ769_RegisterGroup.Protect2.Protect2Bit.OCD_THRESH = 0xB; // 过流检测阈值设置为160mV,具体得实验室测试验证，过低可能会误触发，过高可能会延迟保护响应
    Int_BQ769_WriteReg(BQ_PROTECT2, BQ769_RegisterGroup.Protect2.Protect2Byte);
    // 配置保护寄存器3
    BQ769_RegisterGroup.Protect3.Protect3Byte = 0x00;
    BQ769_RegisterGroup.Protect3.Protect3Bit.OV_DELAY = BMS_OV_DELAY_4s; // 过压检测延迟时间设置为4s,具体得实验室测试验证，过短可能会误触发，过长可能会延迟保护响应
    BQ769_RegisterGroup.Protect3.Protect3Bit.UV_DELAY = BMS_UV_DELAY_4s; // 欠压检测延迟时间设置为4s,具体得实验室测试验证，过短可能会误触发，过长可能会延迟保护响应
    Int_BQ769_WriteReg(BQ_PROTECT3, BQ769_RegisterGroup.Protect3.Protect3Byte);
    // 配置OVTrip和UVTrip寄存器
/*  
    OV_TRIP_FULL = (OV – ADCOFFSET) ÷ ADCGAIN
    UV_TRIP_FULL = (UV – ADCOFFSET) ÷ ADCGAIN
*/
    uint16_t OV_TRIP_FULL = (uint16_t)((OV_Trip * 1000 - offset_mv) / gain_mv);
    uint8_t OV_TRIP_REG = (OV_TRIP_FULL >> 4) & 0xFF; // 取中间8位
    Int_BQ769_WriteReg(BQ_OV_TRIP, OV_TRIP_REG); // 过压阈值设置为4.2V,具体得实验室测试验证，过低可能会误触发，过高可能会延迟保护响应
    uint16_t UV_TRIP_FULL = (uint16_t)((UV_Trip * 1000 - offset_mv) / gain_mv);
    uint8_t UV_TRIP_REG = (UV_TRIP_FULL >> 4) & 0xFF; // 取中间8位
    Int_BQ769_WriteReg(BQ_UV_TRIP, UV_TRIP_REG); // 欠压阈值设置为2.5V,具体得实验室测试验证，过低可能会误触发，过高可能会延迟保护响应
    // 配置CCCfg寄存器 
    Int_BQ769_WriteReg(BQ_CC_CFG, 0x19); // 连续CC计数阈值设置为25mV,具体得实验室测试验证，过低可能会误触发，过高可能会延迟保护响应
    
}
/**
 * @brief 电池被动均衡控制
 * @note  均衡策略:
 *        1. 温度不在 0~40°C 范围内或最低电压 < 3.7V 时关闭所有均衡
 *        2. 找出最低电压电芯，对其他电芯标记均衡需求
 *        3. 相邻电芯不同时均衡，优先保留电压更高的一路
 *        4. 最终通过 I2C 写入 CellBal1 寄存器生效
 */
void Int_BQ769_BatBalance(void)
{
    // 这里以均衡电芯1为例，其他电芯类似
    if (temp_c < 0 || temp_c > 40) // 如果温度在0~40℃范围内，开启均衡
    {
        BQ769_RegisterGroup.CellBal1.CellBal1Byte = 0x00;
        return;
    }
    // 找出电压最低的电芯
    uint8_t min_cell_index = 0;
    for (size_t i = 1; i < Cell_Num; i++)
    {
        if (cell_voltage_v[i] < cell_voltage_v[min_cell_index])
        {
            min_cell_index = i;
        }
    }

    printf("Battery Balance: min_cell_index=%d, min_cell_voltage=%fv\r\n", min_cell_index, cell_voltage_v[min_cell_index]);
    // 如果最低电池电压接近下限，则不进行均衡，考虑充电
    if (cell_voltage_v[min_cell_index] < 3.7) //
    {
        BQ769_RegisterGroup.CellBal1.CellBal1Byte = 0x00;
        printf("Battery Balance: Cell%d voltage is low, skip balancing\r\n", min_cell_index + 1);
        return;
    }

    for (size_t i = 0; i < 5; i++)
    {
        if (i == min_cell_index)
        {
            continue; // 跳过最低电压的电芯，不进行均衡
        }
        need_balance[i] = 1; // 标记需要均衡的电芯
    }
    //相邻电池不能同时进行均衡，因此只能均衡电芯1
    for (size_t i = 0; i < 4; i++)
    {
        //判断相邻电池是否都需要均衡
        if (need_balance[i] && need_balance[i + 1])
        {
           //判断谁的电压更高，优先均衡
            if (cell_voltage_v[i] > cell_voltage_v[i + 1])
            {
                need_balance[i+1] = 0; // 不均衡电芯i
            }
            else
            {
                need_balance[i] = 0; // 不均衡电芯i
            }
        }
       
    }

    BQ769_RegisterGroup.CellBal1.CellBal1Bit.CB1 = need_balance[0];
    BQ769_RegisterGroup.CellBal1.CellBal1Bit.CB2 = need_balance[1];
    BQ769_RegisterGroup.CellBal1.CellBal1Bit.CB3 = need_balance[2];
    BQ769_RegisterGroup.CellBal1.CellBal1Bit.CB4 = need_balance[3];
    BQ769_RegisterGroup.CellBal1.CellBal1Bit.CB5 = need_balance[4];
    Int_BQ769_WriteReg(BQ_CELLBAL1, BQ769_RegisterGroup.CellBal1.CellBal1Byte);
    
}
