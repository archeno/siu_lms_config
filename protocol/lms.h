/*
 * LMS.h
 *
 * Created: 2016/11/18 9:08:30
 *  Author: Administrator
 */ 
#ifndef LMS_H_
#define LMS_H_
#include "stdint.h"
typedef uint8_t BYTE;
typedef uint16_t WORD;

#define MaxLMSNumInGroupe          30                   //��һ����������LMS��������
//TC1�������
#define MaxCaptureCurWidth_ms      19.5f                //unit:ms
#define MinCaptureCurWidth_ms      9.5f                 //unit:ms

#define TStd                       0.0001f              //Unit:Second,100us
#define PowerHalfCycle_ms          (1000/(PowerFreq*2))
//
// BYTE gSampleCTC=0;
// //
// BYTE gLMSType=0;				 //gLMSType=0x00=SIU,gLMSType=0x01=LMS
//******************ͨ�Ž���*******************//
//1.֡����
//ͬ��֡LS/SS(8bit,��3bitУ��λ)		: startBit(1) g2g1g0(3bit/���) ID��1bit/1=LS,0=SS�� ccc(3bit)
//����֡LS (16bit,��4bitУ��λ)		��startBit(1) ConsegmentAddr(7bit),
//									  OrderCode(4bit) CRC4(4bit).
//������LS OrderCode(4bit)
#define  ProgData0_Order          0x00             //SCM��������ΪProgData0�������
#define  ProgData1_Order          0x01             //SCM��������ΪProgData1�������
#define  OpenLamp_Order			  0x02             //SCM��������ΪLMS�ص�
#define  CloseLamp_Order          0x03             //SCM��������ΪLMS����

#define  FlashSync_Order          0x04

#define  Intensity0_Order         0x05
#define  Intensity1_Order         0x06
#define  Intensity2_Order         0x07
#define  Intensity3_Order         0x08
#define  Intensity5_Order         0x09


#define SHORT_CMD 0X00
#define LONG_CMD 0X01

extern BYTE const TxSyncFrame[];

BYTE GenSyncFrame(BYTE byGroupNo,BYTE IDType);
BYTE crc4_itu(BYTE * message,BYTE length);
WORD crc16_modbus(BYTE *message, BYTE length);
#endif /* LMS_H_ */
