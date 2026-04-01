#ifndef PROTOCOL_H
#define PROTOCOL_H


// 定义命令帧相关常量
#define FRAME_HEADER_1      0xA5
#define FRAME_HEADER_2      0xA5
#define DEVICE_ADDRESS      0xB1
#define FRAME_TYPE_DATA     0x80
#define FRAME_TYPE_CMD      0x00

// 功能码定义
#define FUNC_CODE_CONTROL 0x4D
#define FUNC_CODE_QUERY   0x5D
#define FUNC_CODE_DATA    0x6D
#define FUNC_CODE_RESET   0x7D

#define FUNC_CODE_CONTROL_LEN 13
#define FUNC_CODE_DATA_LEN 7
#define FUNC_CODE_RESET_LEN 7


// 数据帧最大长度
#define PA_FRAME_MAX_LEN 32

// 数据结构（只需要填这个结构体）
typedef struct
{
    unsigned char  work_status;   // Byte7
    float   temperature;   // 实际温度（-25~150）

    float    fwd_power;     // 正向功率 (W)
    float    rev_power;     // 反向功率 (W)

    float    return_loss;   // 回波损耗 (单位：dB，例如 -4.0)

    unsigned char  freq;          // 频率（0=915MHz）
    unsigned char reserved;      // 预留
    float  current;       // 电流（例如0x14=1.4A）

    unsigned char  fault;         // 故障码

    unsigned short version;       // 软件版本（例如0x0103）

} PA_Data_t;

// 函数声明
unsigned char IsMasterCommFrame(unsigned char data1, unsigned char data2);
void MasterCommService(unsigned char *pdta, unsigned char dataLen);
#endif // PROTOCOL_H