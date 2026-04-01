#include "protocol.h"
#include <stdio.h>
#include <string.h>
#include "dac7311.h"
#include "debug.h"
// CRC-8 Lookup Table
static unsigned char crc8_table[] = {
    0, 94, 188, 226, 97, 63, 221, 131, 194, 156, 126, 32, 163, 253, 31, 65,  
    157, 195, 33, 127, 252, 162, 64, 30, 95, 1, 227, 189, 62, 96, 130, 220,  
    35, 125, 159, 193, 66, 28, 254, 160, 225, 191, 93, 3, 128, 222, 60, 98,  
    190, 224, 2, 92, 223, 129, 99, 61, 124, 34, 192, 158, 29, 67, 161, 255,  
    70, 24, 250, 164, 39, 121, 155, 197, 132, 218, 56, 102, 229, 187, 89, 7,  
    219, 133, 103, 57, 186, 228, 6, 88, 25, 71, 165, 251, 120, 38, 196, 154,  
    101, 59, 217, 135, 4, 90, 184, 230, 167, 249, 27, 69, 198, 152, 122, 36,  
    248, 166, 68, 26, 153, 199, 37, 123, 58, 100, 134, 216, 91, 5, 231, 185,  
    140, 210, 48, 110, 237, 179, 81, 15, 78, 16, 242, 172, 47, 113, 147, 205,  
    17, 79, 173, 243, 112, 46, 204, 146, 211, 141, 111, 49, 178, 236, 14, 80,  
    175, 241, 19, 77, 206, 144, 114, 44, 109, 51, 209, 143, 12, 82, 176, 238,  
    50, 108, 142, 208, 83, 13, 239, 177, 240, 174, 76, 18, 145, 207, 45, 115,  
    202, 148, 118, 40, 171, 245, 23, 73, 8, 86, 180, 234, 105, 55, 213, 139,  
    87, 9, 235, 181, 54, 104, 138, 212, 149, 203, 41, 119, 244, 170, 72, 22,  
    233, 183, 85, 11, 136, 214, 52, 106, 43, 117, 151, 201, 74, 20, 246, 168,  
    116, 42, 200, 150, 21, 75, 169, 247, 182, 232, 10, 84, 215, 137, 107, 53  
};
static unsigned char recArray[256] = {0};
static unsigned char sendArray[256] = {0};
static PA_Data_t s_packdata;

// CRC计算函数
unsigned char calculate_crc(unsigned char* data, unsigned char length) {
    unsigned char crc = 0;
    for (unsigned char count = 0; count < length; count++) {
        crc = crc8_table[crc ^ data[count]];
    }
    return crc;
}

// 功率拆分：float → 档位 + 小数
static void power_to_bytes(float p, unsigned char *high, unsigned char *low)
{
    int temp = (int)(p * 10);   // 0.1W单位
    *high = temp / 100;         // 10W档
    *low  = temp % 100;         // 0.1W
}


// 回波损耗编码（补码）
static void return_loss_to_bytes(float rl, unsigned char *h, unsigned char *l)
{
    int val = (int)(rl * 10); // ×10

    if (val >= 0)
    {
        *h = (val >> 8) & 0xFF;
        *l = val & 0xFF;
    }
    else
    {
        val = -val;
        val = (~val + 1) & 0xFFFF;
        *h = (val >> 8) & 0xFF;
        *l = val & 0xFF;
    }
}

// 温度编码
static unsigned char temp_to_byte(float temp)
{
    if (temp < -25.0f) temp = -25.0f;
    if (temp > 150.0f) temp = 150.0f;
    return (unsigned char)(temp + 25+0.001f); // -25~150映射到0~175
}

// 电流编码 (BCD格式：高四位整数，低四位小数*10)
static unsigned char current_to_bcd(float current)
{
    int integer = (int)current;
    int decimal = (int)((current - integer) * 10);
    return (unsigned char)((integer << 4) | decimal);
}


// ================= 核心函数 =================
static unsigned char PA_BuildDataFrame(unsigned char *buf, PA_Data_t *d)
{
    unsigned char i = 0;

    buf[i++] = 0xA5;
    buf[i++] = 0xA5;

    unsigned char len_index = i++; // 先占位

    buf[i++] = 0xB1;
    buf[i++] = 0x00;
    
    buf[i++] = FUNC_CODE_DATA;
    // ===== 数据区 =====
    buf[i++] = d->work_status;

    buf[i++] = temp_to_byte(d->temperature);

    power_to_bytes(d->fwd_power, &buf[i], &buf[i+1]);
    i += 2;

    power_to_bytes(d->rev_power, &buf[i], &buf[i+1]);
    i += 2;

    return_loss_to_bytes(d->return_loss, &buf[i], &buf[i+1]);
    i += 2;

    buf[i++] = d->freq;
    buf[i++] = d->reserved; // 预留


    buf[i++] = current_to_bcd(d->current); // 电流（例如0x14=1.4A）
    
    buf[i++] = d->fault;

    buf[i++] = (d->version >> 8) & 0xFF;

    buf[i++] = d->version & 0xFF;

    // ===== 长度 =====
    buf[len_index] = i + 1; // +CRC

    // ===== CRC =====
    buf[i] = PA_CRC8(&buf[len_index], i - len_index);
    i++;

    return i; // 返回帧长度
}

static unsigned char PA_BuildResetFrame(unsigned char *buf)
{
    unsigned char i = 0;

    buf[i++] = 0xA5;
    buf[i++] = 0xA5;

    unsigned char len_index = i++; // 先占位

    buf[i++] = 0xB1;
    buf[i++] = 0x00;
    
    buf[i++] = FUNC_CODE_RESET;

    // ===== 长度 =====
    buf[len_index] = i + 1; // +CRC

    // ===== CRC =====
    buf[i] = PA_CRC8(&buf[len_index], i - len_index);
    i++;

    return i; // 返回帧长度
}

static void update_packdata(PA_Data_t *data)
{
    data->work_status = (GPIO_GetIn(enumFREGSWONFF) ? 0x00 : 0x01); // 0=ON, 1=OFF
    data->temperature = MeasureGetAnalog(Analog_TEMP); // 实际温度
    data->fwd_power = MeasureGetAnalog(Analog_InPower); // 正向功率
    data->rev_power = MeasureGetAnalog(Analog_RefPower); // 反向功率
    data->return_loss = getReturnLoss(); // 回波损耗
    data->freq = 0x00;
    data->reserved = 0x13; // 预留
    data->current = MeasureGetAnalog(Analog_17A); // 电流
    data->fault = MeasureGetAlarmFlag(); // 故障码
    data->version = (unsigned short)SW_VER_HIGH << 8 | SW_VER_LOW;
}

// 控制命令处理
static void handle_control_command(unsigned char* data, unsigned char length)
{
    if(length != FUNC_CODE_CONTROL_LEN)
    {
        DEBUG_PRINT("控制命令长度不足\n");
        return;
    }
    if(data[6] == 0x00)
    {
        GPIO_OutHigh(enumFREGSWONFF);
    }
    else if(data[6] == 0x01)
    {
        GPIO_OutLow(enumFREGSWONFF);
    }
    else
    {
        DEBUG_PRINT("参数超出范围: %02X\n", data[6]);
        return;
    }
    unsigned char gear = data[7];  // 第8字节 档位 (10W单位，0x00-0x0A)
    unsigned char level = data[8]; // 第9字节 级别 (0.1W单位，0x00-0x0F)
    float power = gear * 10.0 + level * 0.1;  // PA功率 = 档位*10W + 级别*0.1W
    DEBUG_PRINT("设置PA功率: 档位=%02X, 级别=%02X, 功率=%.1f W\n", gear, level, power);
    unsigned char output_power_index = GetOutputPowerIndex(power);
    float outDAC = GetAdjustResult(output_power_index,power,&err);
    OutputPower(outDAC);
    //频率默认915MHZ 不处理
    //预留不处理

    unsigned char tx_buf[32];
    memset(tx_buf, 0, sizeof(tx_buf));
    update_packdata(&s_packdata);
    unsigned char len = PA_BuildDataFrame(tx_buf, &s_packdata);
    uart_send(tx_buf, len);
}

// 查询命令处理
static void handle_query_command(unsigned char* data, unsigned char length) {
    // 查询命令的长度应该固定，不需要严格检查（取决于协议）
    // if(length < MINIMUM_FRAME_LENGTH) {
    //     DEBUG_PRINT("查询命令长度不足\n");
    //     return;
    // }
    unsigned char tx_buf[32];
    memset(tx_buf, 0, sizeof(tx_buf));
    update_packdata(&s_packdata);
    unsigned char len = PA_BuildDataFrame(tx_buf, &s_packdata);
    uart_send(tx_buf, len);
}

// 复位命令处理
static void handle_reset_command(unsigned char* data, unsigned char length) {
    //回复格式： A5A5 + 帧长+ B1 + 00 + 7D +1+ CRC校验
    if(length != FUNC_CODE_RESET_LEN)
    {
        DEBUG_PRINT("复位命令长度不足\n");
        return;
    }
    unsigned char tx_buf[32];
    memset(tx_buf, 0, sizeof(tx_buf));
    unsigned char len = PA_BuildResetFrame(tx_buf);
    uart_send(tx_buf, len);
    Trap(); // 复位设备
}

// 处理接收到的帧数据
static void process_frame(unsigned char* received_data, unsigned char length) {
    // 最小长度检查：帧头(2) + 长度(1) + 地址(1) + 类型(1) + 功能码(1) + CRC(1) = 7字节
    if (length < 7) {
        DEBUG_PRINT("帧长度过短: %d\n", length);
        return;
    }
    
    if (received_data[0] == FRAME_HEADER_1 && received_data[1] == FRAME_HEADER_2) {
        unsigned char frame_length = received_data[2];
        if(frame_length > length) 
        {
            DEBUG_PRINT("帧长度不匹配: %d\n", frame_length);
            return;
        }
        unsigned char crc = received_data[frame_length - 1];
        // CRC 校验
        if (calculate_crc(received_data, frame_length - 1) == crc) {
            if(received_data[3] != DEVICE_ADDRESS) {
                DEBUG_PRINT("地址不匹配: %02X\n", received_data[3]);
                return;
            }
            if(received_data[4] != FRAME_TYPE_DATA
                && received_data[4] != FRAME_TYPE_CMD) { // 这里假设帧类型为0x80或0x00
                DEBUG_PRINT("帧类型不匹配: %02X\n", received_data[4]);
                return;
            }
            // 根据功能码处理不同的命令
            unsigned char func_code = received_data[5];
            switch (func_code) {
                case FUNC_CODE_CONTROL:
                    handle_control_command(received_data, frame_length);
                    break;
                case FUNC_CODE_QUERY:
                    handle_query_command(received_data, frame_length);
                    break;
                case FUNC_CODE_RESET:
                    handle_reset_command(received_data, frame_length);
                    break;
                default:
                    DEBUG_PRINT("未知功能码: %02X\n", func_code);
                    break;
            }
        } else {
            DEBUG_PRINT("CRC校验失败\n");
        }
    } else {
        DEBUG_PRINT("无效的帧头\n");
    }
}

unsigned char IsMasterCommFrame(unsigned char data1, unsigned char data2)
{
    if(data1 == FRAME_HEADER_1 && data2 == FRAME_HEADER_2)
    {
        return 1;
    }
    return 0;
}

void MasterCommService(unsigned char *pdta, unsigned char dataLen)
{
    if(pdta == 0 || dataLen == 0)
        return;
	
    PrintfArray(pdta,dataLen);
	process_frame(pdta, dataLen);
}

