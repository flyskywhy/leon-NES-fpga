/*******************************************************************
 *        Copyright (c) 2005,����ʿ��΢���ӹɷ����޹�˾            *
 *                   All rights reserved.                          *
 *******************************************************************
      �ļ����ƣ� SLNES_Data.h
      �ļ��汾�� 1.00
      ������Ա�� ����
      �������ڣ� 2005/05/08 08:00:00
      ���������� ���뵽������SDRAM�е����������ָ��SDRAM��ָ��
      �޸ļ�¼��
 *******************************************************************/

#ifndef SLNES_Data_H_INCLUDED
#define SLNES_Data_H_INCLUDED

#define SIZE_OF_gamefile 262160	// ֧��256KB��NES�ļ�
//#define SIZE_OF_gamefile 188416	// ֻ֧��BIN�ļ�

extern unsigned char gamefile[];	//NES��Ϸ���ݣ�.nes��.bin�ļ�

extern unsigned char PPU0[];	//PPU��棬����6λ����ɫ����ֵ
extern unsigned char PPU1[];
extern unsigned char PPU2[];

#if BITS_PER_SAMPLE == 8
extern unsigned char APU[];		//APU���
#else /* BITS_PER_SAMPLE */
extern short APU[];
#endif /* BITS_PER_SAMPLE */

extern unsigned char PTRAM[];	//ֻ����mapper2�Ĵ���VROM��8KB�ڴ�

// ����6502RAM��0x6000-0x7FFF��8KBһ������Ϸ�������̵��ڴ�
extern unsigned char SRAM[];

#endif /* !SLNES_Data_H_INCLUDED */
