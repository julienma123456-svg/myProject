#ifndef TIME_H
#define TIME_H

sfr TL0  = 0x8A;
sfr TL1  = 0x8B;
sfr TH0  = 0x8C;
sfr TH1  = 0x8D;
//¶¨Ê±Æ÷3¼Ä´æÆ÷
sfr T2H  = 0xD6;
sfr T2L  = 0xD7;
sfr T3H  = 0xD4;
sfr T3L  = 0xD5;
sfr T4H  = 0xD2;
sfr T4L  = 0xD3;
sfr T4T3M = 0xD1;

/*  TCON  */
sbit TF1  = 0x8F;
sbit TR1  = 0x8E;
sbit TF0  = 0x8D;
sbit TR0  = 0x8C;
sbit IE1  = 0x8B;
sbit IT1  = 0x8A;
sbit IE0  = 0x89;
sbit IT0  = 0x88;

#endif










