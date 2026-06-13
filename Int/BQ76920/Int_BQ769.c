#include "Int_BQ769.h"

RegisterGroup BQ769_RegisterGroup;
//gain
static float gain_mv = 0;

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
    HAL_I2C_Mem_Write(&hi2c2, BQ769_I2C_ADDRESS_WRITE, reg_addr, I2C_MEMADD_SIZE_8BIT, send_buff, 2, 1000);
    //vTaskDelay(1);考虑写周期
}

/**
 * @brief 从BQ769的寄存器读取多个字节
 */
void Int_BQ769_ReadReg(uint8_t reg, uint8_t *buff, uint8_t read_len)
{
    // 读取数据和CRC
    uint8_t *rev_buff = pvPortCalloc(read_len * 2, sizeof(uint8_t));
    HAL_I2C_Mem_Read(&hi2c2, BQ769_I2C_ADDRESS_READ, reg, I2C_MEMADD_SIZE_8BIT, rev_buff, read_len * 2, 1000);

    for (size_t i = 0; i < read_len; i++)
    {
        uint8_t crc = 0;
        uint8_t data = rev_buff[i * 2];
        uint8_t crc_recv = rev_buff[i * 2 + 1];
        // 计算CRC-8校验值
        if (i == 0)
        {
            uint8_t crc_calc_buff[2] = {BQ769_I2C_ADDRESS_READ, reg};
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
        //释放内存
        vPortFree(rev_buff);
    }

}

void Int_BQ769_LoadGain(void)
{
    uint8_t gain1 = 0;
    uint8_t gain2 = 0;
    Int_BQ769_ReadReg(BQ_ADCGAIN1, &gain1, 1);
    Int_BQ769_ReadReg(BQ_ADCGAIN2, &gain2, 1);

    uint8_t gain_adc_value = ((gain1 & 0x0C) << 1) | (gain2 >> 5);

    gain_mv = (gain_adc_value + 365) / 1000.0f;
    printf("BQ769 Load Gain: gain_adc_value=%d, gain_mv=%.3f\r\n", gain_adc_value, gain_mv);
}