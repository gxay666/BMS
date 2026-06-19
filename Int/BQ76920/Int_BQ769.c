#include "Int_BQ769.h"

RegisterGroup BQ769_RegisterGroup;
//gain
static float gain_mv = 0;
//offset
static int8_t offset_mv = 0;
//cell voltage
static float cell_voltage_v[5] = {0};
//pack voltage
float pack_voltage_v = 0;
//temperature
int8_t temp_c = 0;
//Current
float current_a = 0;
//SOC
float bat_soc = 0;

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

void Int_BQ769_LoadOffset(void)
{
    Int_BQ769_ReadReg(BQ_ADCOFFSET, (uint8_t *)&offset_mv, 1);
    printf("BQ769 Load Offset: offset_mv=%dmv/LSB\r\n", offset_mv);
}

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

void Int_BQ769_LoadPackVoltage(void)
{
    printf("--------------总电压-----------------\r\n");
    uint8_t pack_voltage_buff[2] = {0};
    Int_BQ769_ReadReg(BQ_BAT_HI, pack_voltage_buff, 2); 
    uint16_t pack_adc_value = ((pack_voltage_buff[0] & 0x3F) << 8) | pack_voltage_buff[1];
    pack_voltage_v = (pack_adc_value * gain_mv * 4 + offset_mv * Cell_Num) / 1000.0f;
    printf("Pack Voltage: adc_value=%d, voltage=%fv\r\n", pack_adc_value, pack_voltage_v);
}
/*
    CC Reading (in µV) = [16-bit 2’s Complement Value] × (8.44 µV/LSB)
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
/*
    VTSX = (ADC in Decimal) x 382 µV/LSB    电压值
    RTS = (10,000 × VTSX) ÷ (3.3 – VTSX)    热敏电阻阻值
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
