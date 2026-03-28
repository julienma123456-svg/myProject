#include "comm2.h"
#include "uart1.h"

//打印调试口，使用资源UART1

static xdata Comm2StructType Comm2Struct;

//结构体初始化
static void Comm2StructInit(void)
{
    Comm2Struct.ComState = enumIdle;
    Comm2Struct.SendInx = 0;
    Comm2Struct.SendNum = 0;
    Comm2Struct.RecInx = 0;
    Comm2Struct.RecCount = 0;
	Comm2Struct.IdleCount = 0;
}

//发送一个字节
static void Comm2SendOneData(unsigned char dataIn)
{
    UART1_SendOneData(dataIn);
}

//发送完成一个字节回调函数(由发送完成中断调用)
void Comm2SendOneDataOK(void)
{
    unsigned char send = 0;
    Comm2Struct.SendInx++;
    if(Comm2Struct.SendInx < Comm2Struct.SendNum)
    {
        send = Comm2Struct.SendArray[Comm2Struct.SendInx];
        Comm2SendOneData(send);
    }
    else
    {
        //Comm2Struct.ComState = enumSended;
		Comm2Struct.ComState = enumIdle;
    }
}

//========================================================================
// 函数: void   Comm2Init(void)
// 描述: Comm2Init初始化函数.
// 参数: none.
// 返回: none.
// 版本: V1.0, 2025-6-4
//========================================================================
void Comm2Init(void)
{
    Comm2StructInit();
}

//发送一个数组
unsigned char Comm2SendData(unsigned char *dataIn,unsigned char dataLen)
{
    unsigned char xdata sendLen = 0;
    if(dataIn == 0)
        return 0;

    if(Comm2Struct.ComState != enumIdle)
        return 0;    

    if(dataLen > MAX_LEN_SEND_COMM2)
        sendLen = MAX_LEN_SEND_COMM2;
    else
        sendLen = dataLen;

    MemCopy(Comm2Struct.SendArray,dataIn,sendLen);
    Comm2Struct.SendInx = 0;
    Comm2Struct.SendNum = sendLen;
    Comm2Struct.ComState = enumSending;
    Comm2SendOneData(Comm2Struct.SendArray[0]);

	return 1;
}

//1ms周期调用函数
void Comm2Tick(void)
{
	//接收数据完成处理
	if(Comm2Struct.ComState == enumReceiving)
	{
		//收到数据时会清零
		Comm2Struct.IdleCount++;

		//超过一定时间没收到数据，认为接收数据完成
		if(Comm2Struct.IdleCount >= REC_FRAME_DELAY)
		{
			Comm2Struct.RecCount = Comm2Struct.RecInx;
			Comm2Struct.RecInx = 0;
			Comm2Struct.ComState = enumReceived;
		}
	}
}


//接收到一个字节
void Comm2RecOneData(unsigned char recData)
{
    unsigned char xdata inx = 0;
    inx = Comm2Struct.RecInx;
    Comm2Struct.RecArray[inx] = recData;
    Comm2Struct.RecInx++;
	Comm2Struct.IdleCount = 0;

	//防止数据溢出
	if(Comm2Struct.RecInx >= MAX_LEN_REC_COMM2)
	{
		Comm2Struct.RecCount = Comm2Struct.RecInx;
		Comm2Struct.RecInx = 0;
		Comm2Struct.ComState = enumReceived;
	}
	else
	{
		Comm2Struct.ComState = enumReceiving;
	}
}

//获取接收数据
//unsigned char Comm2GetRecData(unsigned char *dataIn,unsigned char *dataLen)
//{
//    unsigned char xdata recLen = 0;
//    if(dataIn == 0)
//        return 0;
//
//    if(Comm2Struct.ComState != enumReceived)
//        return 0;
//
//    recLen = Comm2Struct.RecCount;
//    *dataLen = Comm2Struct.RecCount;
//    MemCopy(dataIn,Comm2Struct.RecArray,recLen);    
//
//    Comm2Struct.ComState = enumIdle;
//
//	return 1;
//}

