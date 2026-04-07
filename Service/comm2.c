#include "comm2.h"
#include "uart1.h"
//???????????????UART1
static void Comm2ConfigRxMode(void);
static xdata Comm2StructType Comm2Struct;

//????????
static void Comm2StructInit(void)
{
    
    Comm2Struct.ComState = enumIdle;
    Comm2Struct.SendInx = 0;
    Comm2Struct.SendNum = 0;
    Comm2Struct.RecInx = 0;
    Comm2Struct.RecCount = 0;
	Comm2Struct.IdleCount = 0;

     #ifdef CHANGE_UART_BECAUSEOF_UART3ERR
    Uart3RegresiterCallback(Comm2SendOneDataOK, Comm2RecOneData);//注册回调函数，uart3绑定comm2 只发送日志
    Comm2Struct.fSendOneByte = UART3_SendOneData;
    #else
    Uart1RegresiterCallback(Comm2SendOneDataOK, Comm2RecOneData);//注册回调函数，uart1绑定comm2 只发送日志
    Comm2Struct.fSendOneByte = UART1_SendOneData;
    #endif
    Comm2ConfigRxMode();	//默认配置为接收模式
}

//Comm2配置为发送模式(DE为高)
static void Comm2ConfigTxMode(void)
{
    #ifdef CHANGE_UART_BECAUSEOF_UART3ERR
	GPIO_OutLow(enum485CTRL);//低电平光耦导通
    #endif
}

//Comm2配置为接收模式(DE为低)
static void Comm2ConfigRxMode(void)
{
    #ifdef CHANGE_UART_BECAUSEOF_UART3ERR
	GPIO_OutHigh(enum485CTRL);//高电平光耦不导通
    #endif
}

//??????????
static void Comm2SendOneData(unsigned char dataIn)
{
    Comm2Struct.fSendOneByte(dataIn);
}

//???????????????????(?????????ж????)
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
        Comm2Struct.ComState = enumSended;
		// Comm2Struct.ComState = enumIdle;
    }
}

//========================================================================
// ????: void   Comm2Init(void)
// ????: Comm2Init?????????.
// ????: none.
// ????: none.
// ?汾: V1.0, 2025-6-4
//========================================================================
void Comm2Init(void)
{
    Comm2StructInit();
}

//???????????
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

    Comm2ConfigTxMode();	//发送前先配置为发送模式
    MemCopy(Comm2Struct.SendArray,dataIn,sendLen);
    Comm2Struct.SendInx = 0;
    Comm2Struct.SendNum = sendLen;
    Comm2Struct.ComState = enumSending;
    Comm2SendOneData(Comm2Struct.SendArray[0]);

	return 1;
}

//1ms??????ú???
void Comm2Tick(void)
{
	//??????????????
	if(Comm2Struct.ComState == enumReceiving)
	{
		//??????????????
		Comm2Struct.IdleCount++;

		//?????????????????????????????????
		if(Comm2Struct.IdleCount >= REC_FRAME_DELAY)
		{
			Comm2Struct.RecCount = Comm2Struct.RecInx;
			Comm2Struct.RecInx = 0;
			Comm2Struct.ComState = enumReceived;
		}
	}
    //发送数据完成后，需要配置为接收模式(等待数据)
	if(Comm2Struct.ComState == enumSended)
	{
		Comm2ConfigRxMode();
		Comm2Struct.ComState = enumIdle;
	}
}


//???????????
void Comm2RecOneData(unsigned char recData)
{
    unsigned char xdata inx = 0;
    inx = Comm2Struct.RecInx;
    Comm2Struct.RecArray[inx] = recData;
    Comm2Struct.RecInx++;
	Comm2Struct.IdleCount = 0;

	//??????????
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

unsigned char Comm2GetRecData(unsigned char *dataIn,unsigned char *dataLen)
{
   unsigned char xdata recLen = 0;
   if(dataIn == 0)
       return 0;

   if(Comm2Struct.ComState != enumReceived)
       return 0;

   recLen = Comm2Struct.RecCount;
   *dataLen = Comm2Struct.RecCount;
   MemCopy(dataIn,Comm2Struct.RecArray,recLen);    

   Comm2Struct.ComState = enumIdle;

	return 1;
}
