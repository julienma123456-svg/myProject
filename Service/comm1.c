#include "comm1.h"
#include "uart3.h"

//485通信口，使用资源UART3

static Comm1StructType Comm1Struct;

//结构体初始化
static void Comm1StructInit(void)
{
    Comm1Struct.ComState = enumIdle;
    Comm1Struct.SendInx = 0;
    Comm1Struct.SendNum = 0;
    Comm1Struct.RecInx = 0;
    Comm1Struct.RecCount = 0;
	Comm1Struct.IdleCount = 0;
}

//Comm1配置为发送模式(DE为高)
static void Comm1ConfigTxMode(void)
{
	GPIO_OutLow(enum485CTRL);//低电平光耦导通
}

//Comm1配置为接收模式(DE为低)
static void Comm1ConfigRxMode(void)
{
	GPIO_OutHigh(enum485CTRL);//高电平光耦不导通
}

//发送一个字节
static void Comm1SendOneData(unsigned char dataIn)
{
    UART3_SendOneData(dataIn);
}

//发送完成一个字节回调函数(由发送完成中断调用)
void Comm1SendOneDataOK(void)
{
    unsigned char send = 0;
    Comm1Struct.SendInx++;
    if(Comm1Struct.SendInx < Comm1Struct.SendNum)	//发送下一个字节
    {
        send = Comm1Struct.SendArray[Comm1Struct.SendInx];
        Comm1SendOneData(send);
    }
    else	  //发送完成
    {
        Comm1Struct.ComState = enumSended;	//0~1ms内变为空闲状态
		//Comm1Struct.ComState = enumIdle;
    }
}

//========================================================================
// 函数: void   Comm1Init(void)
// 描述: Comm1初始化函数.
// 参数: none.
// 返回: none.
// 版本: V1.0, 2025-6-4
//========================================================================
void Comm1Init(void)
{
    Comm1StructInit();
}

//发送一个数组
unsigned char Comm1SendData(unsigned char *dataIn,unsigned char dataLen)
{
    unsigned char sendLen = 0;
    if(dataIn == 0)
        return 0;

    if(Comm1Struct.ComState != enumIdle)
        return 0;    

    if(dataLen > MAX_LEN_SEND_COMM1)
        sendLen = MAX_LEN_SEND_COMM1;
    else
        sendLen = dataLen;

	Comm1ConfigTxMode();	//发送前先配置为发送模式
    MemCopy(Comm1Struct.SendArray,dataIn,sendLen);
    Comm1Struct.SendInx = 0;
    Comm1Struct.SendNum = sendLen;
    Comm1Struct.ComState = enumSending;
    Comm1SendOneData(Comm1Struct.SendArray[0]);	 //开始发送数据(先发第一个数据)

	return 1;
}

//1ms周期调用函数
void Comm1Tick(void)
{
	//接收数据完成处理
	if(Comm1Struct.ComState == enumReceiving)
	{
		//收到数据时会清零
		Comm1Struct.IdleCount++;  

		//超过一定时间没收到数据，认为接收数据完成
		if(Comm1Struct.IdleCount >= REC_FRAME_DELAY)
		{
			Comm1Struct.RecCount = Comm1Struct.RecInx;
			Comm1Struct.RecInx = 0;
			Comm1Struct.ComState = enumReceived;
		}
	}

	//发送数据完成后，需要配置为接收模式(等待数据)
	if(Comm1Struct.ComState == enumSended)
	{
		Comm1ConfigRxMode();
		Comm1Struct.ComState = enumIdle;
	}
}


//接收到一个字节
void Comm1RecOneData(unsigned char recData)
{
    unsigned char inx = 0;
    inx = Comm1Struct.RecInx;
    Comm1Struct.RecArray[inx] = recData;
    Comm1Struct.RecInx++;
	Comm1Struct.IdleCount = 0;

	//防止数据溢出
	if(Comm1Struct.RecInx >= MAX_LEN_REC_COMM1)
	{
		Comm1Struct.RecCount = Comm1Struct.RecInx;
		Comm1Struct.RecInx = 0;
		Comm1Struct.ComState = enumReceived;
	}
	else
	{
		Comm1Struct.ComState = enumReceiving;
	}
}

//获取接收数据
unsigned char Comm1GetRecData(unsigned char *dataIn,unsigned char *dataLen)
{
    unsigned char recLen = 0;
    if(dataIn == 0)
        return 0;

    if(Comm1Struct.ComState != enumReceived)
        return 0;

    recLen = Comm1Struct.RecCount;
    *dataLen = Comm1Struct.RecCount;
    MemCopy(dataIn,Comm1Struct.RecArray,recLen);    

    Comm1Struct.ComState = enumIdle;

	return 1;
}

