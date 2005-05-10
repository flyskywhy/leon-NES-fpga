/*******************************************************************
 *        Copyright (c) 2005,��������΢���ӹɷ����޹�˾            *
 *                   All rights reserved.                          *
 *******************************************************************
      �ļ����ƣ� SLNES_Data.c
      �ļ��汾�� 1.00
      ������Ա�� ����
      �������ڣ� 2005/05/08 08:00:00
      ���������� ���뵽������SDRAM�е����������ָ��SDRAM��ָ��
      �޸ļ�¼��
 *******************************************************************/

#include "SLNES.h"
#include "SLNES_Data.h"

#include ".\gamefile\contra.h"	// ����ʱ��Ϸ�ļ����ֶ�ѡ��

unsigned char PPU0[NES_DISP_WIDTH * NES_DISP_HEIGHT];
unsigned char PPU1[NES_DISP_WIDTH * NES_DISP_HEIGHT];
unsigned char PPU2[NES_DISP_WIDTH * NES_DISP_HEIGHT];

#if BITS_PER_SAMPLE == 8
unsigned char APU[SAMPLE_PER_FRAME * APU_LOOPS];
#else /* BITS_PER_SAMPLE */
short APU[SAMPLE_PER_FRAME * APU_LOOPS];
#endif /* BITS_PER_SAMPLE */

unsigned char PTRAM[0x2000];	// ֻ����mapper2�Ĵ���VROM��8KB�ڴ�

// ����
// ����6502RAM��0x6000-0x7FFF��8KB�ڴ棬
// ��win32�µĲ��Ը�Ϊ1���ֽ�Ҳû���⣬
// ��ʵ������Ϸvcd������������ֻ����������Ϸ��
// ����˹���飨TETRIS���ͻ�ʽײ��Side Pocket����
// ˵�ǡ����ܡ�����Ϊ�������ص���Ϸ��һ����VCD�����ϵ�һģһ��
unsigned char SRAM[/*1*/  SRAM_SIZE ];


