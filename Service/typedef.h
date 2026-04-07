#ifndef     TYPEDEF_H
#define     TYPEDEF_H

typedef     unsigned char   u8;
typedef     unsigned int    u16;
typedef     unsigned long   u32;

#define		SET_BIT0	0x01
#define		SET_BIT1	0x02
#define		SET_BIT2	0x04
#define		SET_BIT3	0x08
#define		SET_BIT4	0x10
#define		SET_BIT5	0x20
#define		SET_BIT6	0x40
#define		SET_BIT7	0x80

#define		CLEAR_BIT0	0xFE
#define		CLEAR_BIT1	0xFD
#define		CLEAR_BIT2	0xFB
#define		CLEAR_BIT3	0xF7
#define		CLEAR_BIT4	0xEF
#define		CLEAR_BIT5	0xDF
#define		CLEAR_BIT6	0xBF
#define		CLEAR_BIT7	0x7F

//串口状态
typedef enum
{
    enumIdle,
    enumSending,
    enumSended,
    enumReceiving,
    enumReceived,
}enumCommState;

//日期定义
typedef struct tagDate
{
	unsigned int Year;
	unsigned char Month;
	unsigned char Day;

	unsigned char Hour;
	unsigned char Min;
	unsigned char Sec;
}DateStructType;

typedef struct tagAdjustPoint
{
	//分段校准，两个点组成一条直线Y = kX + b
	unsigned int ValY;	//坐标系的Y(对于输入功率为功率值，对于输出功率为DAC值)
	unsigned int ValX;	//坐标系的X
}AdjustPointType;

typedef void (*InVoid_OutVoid)(void);
typedef void (*InU8_OutVoid)(unsigned char);

#endif
