#include "protocol.h"
#include <stdio.h>
#include <string.h>
#include "dac7311.h"
// CRC-8 Lookup Table
static uint8_t crc8_table[] = {
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

static PA_Data_t s_packdata;

// CRC计算函数
uint8_t calculate_crc(uint8_t* data, uint8_t length) {
    uint8_t crc = 0;
    for (uint8_t count = 0; count < length; count++) {
        crc = crc8_table[crc ^ data[count]];
    }
    return crc;
}

// 功率拆分：float → 档位 + 小数
static void power_to_bytes(float p, uint8_t *high, uint8_t *low)
{
    int temp = (int)(p * 10);   // 0.1W单位
    *high = temp / 100;         // 10W档
    *low  = temp % 100;         // 0.1W
}


// 回波损耗编码（补码）
static void return_loss_to_bytes(float rl, uint8_t *h, uint8_t *l)
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
static uint8_t temp_to_byte(float temp)
{
    if (temp < -25.0f) temp = -25.0f;
    if (temp > 150.0f) temp = 150.0f;
    return (uint8_t)(temp + 25+0.001f); // -25~150映射到0~175
}


// ================= 核心函数 =================
static uint8_t PA_BuildDataFrame(uint8_t *buf, PA_Data_t *d)
{
    uint8_t i = 0;

    buf[i++] = 0xA5;
    buf[i++] = 0xA5;

    uint8_t len_index = i++; // 先占位

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
    buf[i++] = data->reserved; // 预留


    buf[i++] = (uint8_t)(d->current * 10); // 电流（例如0x14=1.4A）
    
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

static uint8_t PA_BuildResetFrame(uint8_t *buf)
{
    uint8_t i = 0;

    buf[i++] = 0xA5;
    buf[i++] = 0xA5;

    uint8_t len_index = i++; // 先占位

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
    data->fault = 0xFF;
    data->version = 0x0103;
}

// 控制命令处理
void handle_control_command(uint8_t* data, uint8_t length)
{
    if(length != FUNC_CODE_CONTROL_LEN)
    {
        printf("控制命令长度不足\n");
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
        printf("参数超出范围: %02X\n", data[6]);
        return;
    }
    uint8_t gear = data[7];  // 第8字节 档位 (0x00-0x0A)
    uint8_t level = data[8]; // 第9字节 级别 (0x00-0x0F)
    float power = data[8] * 10.0 + data[7] * 0.1;  // PA功率 = Byte10*10W + Byte9*0.1W
    printf("设置PA功率: 档位=%02X, 级别=%02X, 功率=%.1f W\n", gear, level, power);
    OutputPower(power);
    //频率默认915MHZ 不处理
    //预留不处理

    uint8_t tx_buf[32];
    memset(tx_buf, 0, sizeof(tx_buf));
    update_packdata(&s_packdata);
    uint8_t len = PA_BuildDataFrame(tx_buf, &s_packdata);
    uart_send(tx_buf, len);
}

// 查询命令处理
static void handle_query_command(uint8_t* data, uint8_t length) {
    if(length != FUNC_CODE_DATA_LEN)
    {
        printf("控制命令长度不足\n");
        return;
    }
    uint8_t tx_buf[32];
    memset(tx_buf, 0, sizeof(tx_buf));
    update_packdata(&s_packdata);
    uint8_t len = PA_BuildDataFrame(tx_buf, &s_packdata);
    uart_send(tx_buf, len);
}

// 复位命令处理
static void handle_reset_command(uint8_t* data, uint8_t length) {
    //回复格式： A5A5 + 帧长+ B1 + 00 + 7D +1+ CRC校验
    if(length != FUNC_CODE_RESET_LEN)
    {
        printf("复位命令长度不足\n");
        return;
    }
    uint8_t tx_buf[32];
    memset(tx_buf, 0, sizeof(tx_buf));
    uint8_t len = PA_BuildResetFrame(tx_buf);
    uart_send(tx_buf, len);
}

// 处理接收到的帧数据
void process_frame(uint8_t* received_data, uint8_t length) {
    if (received_data[0] == FRAME_HEADER_1 && received_data[1] == FRAME_HEADER_2) {
        uint8_t frame_length = received_data[2];
        if(frame_length > length) 
        {
            printf("帧长度不匹配: %d\n", frame_length);
            return;
        }
        uint8_t crc = received_data[frame_length - 1];
        // CRC 校验
        if (calculate_crc(received_data, frame_length - 1) == crc) {
            if(received_data[3] != DEVICE_ADDRESS) {
                printf("地址不匹配: %02X\n", received_data[3]);
                return;
            }
            if(received_data[4] != FRAME_TYPE_DATA
                && received_data[4] != FRAME_TYPE_CMD) { // 这里假设帧类型为0x80或0x00
                printf("帧类型不匹配: %02X\n", received_data[4]);
                return;
            }
            // 根据功能码处理不同的命令
            uint8_t func_code = received_data[5];
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
                case FUNC_CODE_POWER_SET:
                    handle_power_set_command(received_data, frame_length);
                    break;
                default:
                    printf("未知功能码: %02X\n", func_code);
                    break;
            }
        } else {
            printf("CRC校验失败\n");
        }
    } else {
        printf("无效的帧头\n");
    }
}



