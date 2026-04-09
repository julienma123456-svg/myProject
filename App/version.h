#ifndef VERSION_H
#define VERSION_H
#include "typedef.h"

#define __USEMASTERCOMM_SERVICE__
#define __USEMODBUS_SERVICE__

// 如需关闭全部调试输出，也可以注释掉下一行
#define DEBUG_ENABLE//日志打印开关

#ifdef DEBUG_ENABLE
// 1）仿真输出（Keil Simulator）：打开 USE_SIMULATOR
// 2）串口输出（实际板子或仿真串口转发）：关闭 USE_SIMULATOR
// #define USE_SIMULATOR//输出重定向位置 keil还是串口
#endif



//当前版本为1.10
#define		SW_VER_HIGH				0x01
#define		SW_VER_LOW				0x01

#endif
