#ifndef EEPROM_H
#define EEPROM_H

#include "typedef.h"
#include "drive.h"

#define	TPS_VAL		(MAIN_Fosc / 1000000)

//¼Ä´æÆ÷¶¨Òå
sfr IAP_DATA  = 0xC2;
sfr IAP_ADDRH = 0xC3;
sfr IAP_ADDRL = 0xC4;
sfr IAP_CMD   = 0xC5;
sfr IAP_TRIG  = 0xC6;
sfr IAP_CONTR = 0xC7;
sfr IAP_TPS   = 0xF5;

void EEPROM_SectorErase(u16 EE_address);
void EEPROM_ReadBytes(u16 EE_address,u8 *DataAddress,u16 number);
void EEPROM_WriteBytes(u16 EE_address,u8 *DataAddress,u16 number);

#endif
