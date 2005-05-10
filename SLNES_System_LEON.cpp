/*******************************************************************
 *        Copyright (c) 2005,��������΢���ӹɷ����޹�˾            *
 *                   All rights reserved.                          *
 *******************************************************************
      �ļ����ƣ� SLNES_System_LEON.c
      �ļ��汾�� 1.00
      ������Ա�� ����
      �������ڣ� 2005/05/08 08:00:00
      ���������� NESģ������LEONƽ̨�����е�ϵͳ����
      �޸ļ�¼��
 *******************************************************************/

/*-----------------------------------------------------------------*/
/*  Include files                                                  */
/*-----------------------------------------------------------------*/

#define P1				  0x08000	// the first reference frame
#define P2				  0x11480	// the second reference frame
#define P3				  0x1A900	// the B frame

#define DEMUX_BASE_ADDR				0x000
#define DEMUX_ENABLE				0x11

#define DECODE_BASE_ADDR			0x0A0
#define BFRAME_BASE_ADDR			0x0E
#define TV_ONOFF					0x0F
#define TV_MODE						0x10
#define DISPLAY_FRAME_BASE_ADDR		0x11
#define DISPLAY_FRAME_B				0x12

#define HOST8_BASE_ADDR				0x20000
#define MMUHOST8_CMD_ADDR			0x0

#include "SLNES.h"
#include "SLNES_System.h"
#include "SLNES_Data.h"

#ifndef TCNT2
#define TCNT2 	0xD0
//#define TCNT2 	TCNT1
#endif
#ifndef TRLD2
#define TRLD2 	0xD4
//#define TRLD2 	TRLD1
#endif
#ifndef TCTRL2
#define TCTRL2	0xD8
//#define TCTRL2	TCTRL1
#endif

/*******************************************************************
 *   �������ƣ� main����������ʱ����SLNES_Main�����ⲿ����ʱ��     *
 *   ������Ա�� ����                                               *
 *   �����汾�� 1.00                                               *
 *   �������ڣ� 2005/05/08 08:00:00                                *
 *   ���������� ģ������ϵͳ��������������ģ�����ĺ��ĳ���       *
 *   ��ڲ����� ��                                                 *
 *   ����ֵ  �� 0�������˳�                                        *
 *              -1���쳣�˳�                                       *
 *   �޸ļ�¼��                                                    *
 *******************************************************************/
//int SLNES_Main()
int main()
{
#ifndef SLNES_SIM
	/*GameInit();*/

	// Disable interrupt
	//DisableAllInterrupt();
	*(volatile int*)(IMASK + PREGS) = 0;
	*(volatile int*)(IMASK2 + PREGS) = 0;

#ifdef withMEMIO
	//EnableDemux(FALSE);
	*(volatile int*)((DEMUX_BASE_ADDR+DEMUX_ENABLE) * 4 + MEMIO) = 0;

	//EnableTVEncoder(FALSE);
	*(volatile int*)((DECODE_BASE_ADDR + TV_ONOFF) * 4 + MEMIO) = 0;
#endif /* withMEMIO */

	//InitTimer();
	*(volatile int*)(SCNT + PREGS) = SCALER_RELOAD;
	*(volatile int*)(SRLD + PREGS) = SCALER_RELOAD;
	*(volatile int*)(TCNT2 + PREGS) = 0xFFFFFFFF;
	*(volatile int*)(TRLD2 + PREGS) = 0xFFFFFFFF;
#ifdef PrintfFrameClock
	*(volatile int*)(TCNT0 + PREGS) = 0xFFFFFF;
	*(volatile int*)(TRLD0 + PREGS) = 0xFFFFFF;
#endif /* PrintfFrameClock */

	// init Sdram
	//InitSdram();
	//*(volatile int*)((HOST8_BASE_ADDR+MMUHOST8_CMD_ADDR) * 4 + MEMIO) = 0x00;
	//*(volatile int*)((HOST8_BASE_ADDR+MMUHOST8_CMD_ADDR) * 4 + MEMIO) = 0x01;
	//*(volatile int*)((HOST8_BASE_ADDR+MMUHOST8_CMD_ADDR) * 4 + MEMIO) = 0x02;
	//*(volatile int*)((HOST8_BASE_ADDR+MMUHOST8_CMD_ADDR) * 4 + MEMIO) = 0x02;
	//*(volatile int*)((HOST8_BASE_ADDR+MMUHOST8_CMD_ADDR) * 4 + MEMIO) = 0x03;
	//*(volatile int*)((HOST8_BASE_ADDR+MMUHOST8_CMD_ADDR) * 4 + MEMIO) = 0x04;
	//*(volatile int*)((HOST8_BASE_ADDR+MMUHOST8_CMD_ADDR) * 4 + MEMIO) = 0x00;
	//*(volatile int*)((HOST8_BASE_ADDR+MMUHOST8_CMD_ADDR) * 4 + MEMIO) = 0x10;

#ifdef withMEMIO
	/* Configure Display*/
	// Configure Display frame address for init picture.
	//SetTVMode(TRUE/*bGame*/, 0/*BarMode*/, TRUE/*bPAL*/,
	//	FALSE/*bSVideo*/,TRUE/*bPAL625*/, TRUE/*bMaster*/);
	*(volatile int*)((DECODE_BASE_ADDR + TV_MODE) * 4 + MEMIO) =
		1<<6 | 0<<4 | 1<<3 | 0<<2 | 1<<1 | 1;
	//SetDisplayFrameBase((unsigned char*)P1);
	*(volatile int*)((DECODE_BASE_ADDR+DISPLAY_FRAME_BASE_ADDR) * 4 + MEMIO) = /*0x8000*/(int)(PPU0)>>2 & 0xFFFFFF;
	//SetDisplayFrameTypeB(FALSE);
	*(volatile int*)((DECODE_BASE_ADDR+DISPLAY_FRAME_B)*4+MEMIO) = 0;
	// set a invariable B frame addr for buffer status diagnos
	//WriteRegister(DECODE_BASE_ADDR+BFRAME_BASE_ADDR,P3/*0x1A900*/);
	*(volatile int*)((DECODE_BASE_ADDR+BFRAME_BASE_ADDR)*4+MEMIO)=P3;
#endif /* withMEMIO */

#endif /* SLNES_SIM */

	if (SLNES_Load((char *)"szFileName") == -1)
		return -1;

#ifdef withMEMIO
	// �ֱ��ӿ�����	pio(0):DQ0; pio(1):DQ1; pio(2):OUT; pio(3):CLK
	*(volatile int*)(IODIR + PREGS) = 0xC;
#endif /* withMEMIO */

	for (;;)
	{
		unsigned int frame_count;
		unsigned int cur_time, last_frame_time;
		unsigned int base_time;

#ifndef SLNES_SIM

		// ����Timer3
		*(volatile int*)(TCTRL2 + PREGS) = 0x7;

#ifdef PrintfFrameClock
		*(volatile int*)(TCTRL0 + PREGS) = 0x7;
#endif /* PrintfFrameClock */

#ifdef withMEMIO
		//EnableTVEncoder(TRUE);
		*(volatile int*)((DECODE_BASE_ADDR+TV_ONOFF)*4 + MEMIO) = 1;
#endif /* withMEMIO */

#endif /* SLNES_SIM */

		frame_count = 1;

		// ����PCM����

//		base_time = *(volatile int*)(TCNT2 + PREGS) * (MICROSEC_PER_COUNT);
		base_time = *(volatile int*)(TCNT2 + PREGS);

		for (;;)
		{
#ifdef withMEMIO
			*(volatile int*)((DECODE_BASE_ADDR+DISPLAY_FRAME_BASE_ADDR)*4 + MEMIO) = /*0x8000*/(int)PPU0 >> 2 & 0xFFFFFF;
#endif /* withMEMIO */
			SLNES(PPU1);

			//��PCMӲ������ͬ��
//			last_frame_time = base_time - (frame_count++) * (FRAME_SKIP + 1) * SAMPLE_PER_FRAME * 1000000 / SAMPLE_PER_SEC;
//			last_frame_time = base_time - (frame_count++) * 120000;	// FRAME_SKIPΪ6��SAMPLE_PER_FRAMEΪ189��378��756
//			last_frame_time = base_time - (frame_count++) * 120000 / (MICROSEC_PER_COUNT);	// FRAME_SKIPΪ6��SAMPLE_PER_FRAMEΪ189��378��756
//			last_frame_time = base_time - (frame_count++) * (120000 / (SCALER_RELOAD + 1)) * LEON_CLK;	// FRAME_SKIPΪ6��SAMPLE_PER_FRAMEΪ189��378��756
			last_frame_time = base_time - (frame_count++) * (FRAME_SKIP + 1) * ((120000/7) / (SCALER_RELOAD + 1)) * LEON_CLK;	// SAMPLE_PER_FRAMEΪ189��378��756
			for (;;)
			{
//				cur_time = *(volatile int*)(TCNT2 + PREGS) * (MICROSEC_PER_COUNT);
				cur_time = *(volatile int*)(TCNT2 + PREGS);
				if(last_frame_time >= cur_time)
					break;
			}

#ifdef withMEMIO
			*(volatile int*)((DECODE_BASE_ADDR+DISPLAY_FRAME_BASE_ADDR)*4 + MEMIO) = /*0x18000*/(int)PPU1 >> 2 & 0xFFFFFF;
#endif /* withMEMIO */
			SLNES(PPU2);

			//��PCMӲ������ͬ��
//			last_frame_time = base_time - (frame_count++) * (FRAME_SKIP + 1) * SAMPLE_PER_FRAME * 1000000 / SAMPLE_PER_SEC;
//			last_frame_time = base_time - (frame_count++) * 120000;	// FRAME_SKIPΪ6��SAMPLE_PER_FRAMEΪ189��378��756
//			last_frame_time = base_time - (frame_count++) * 120000 / (MICROSEC_PER_COUNT);	// FRAME_SKIPΪ6��SAMPLE_PER_FRAMEΪ189��378��756
//			last_frame_time = base_time - (frame_count++) * (120000 / (SCALER_RELOAD + 1)) * LEON_CLK;	// FRAME_SKIPΪ6��SAMPLE_PER_FRAMEΪ189��378��756
			last_frame_time = base_time - (frame_count++) * (FRAME_SKIP + 1) * ((120000/7) / (SCALER_RELOAD + 1)) * LEON_CLK;	// SAMPLE_PER_FRAMEΪ189��378��756
			for (;;)
			{
//				cur_time = *(volatile int*)(TCNT2 + PREGS) * (MICROSEC_PER_COUNT);
				cur_time = *(volatile int*)(TCNT2 + PREGS);
				if(last_frame_time >= cur_time)
					break;
			}

#ifdef withMEMIO
			*(volatile int*)((DECODE_BASE_ADDR+DISPLAY_FRAME_BASE_ADDR)*4 + MEMIO) = /*0x28000*/(int)PPU2 >> 2 & 0xFFFFFF;
#endif /* withMEMIO */
			SLNES(PPU0);

			// ����ֱ���A��B��Select��Start�밴��
			// �൱��NES��Ϸ����reset�������½�����Ϸ
			if (PAD_System == 0x0F)
				break;
			// ���ң�����������˳������͹ر�ģ����
			if (PAD_System == 0xF0)
				return 0;

			//��PCMӲ������ͬ��
//			last_frame_time = base_time - (frame_count++) * (FRAME_SKIP + 1) * SAMPLE_PER_FRAME * 1000000 / SAMPLE_PER_SEC;
//			last_frame_time = base_time - (frame_count++) * 120000;	// FRAME_SKIPΪ6��SAMPLE_PER_FRAMEΪ189��378��756
//			last_frame_time = base_time - (frame_count++) * 120000 / (MICROSEC_PER_COUNT);	// FRAME_SKIPΪ6��SAMPLE_PER_FRAMEΪ189��378��756
//			last_frame_time = base_time - (frame_count++) * (120000 / (SCALER_RELOAD + 1)) * LEON_CLK;	// FRAME_SKIPΪ6��SAMPLE_PER_FRAMEΪ189��378��756
			last_frame_time = base_time - (frame_count++) * (FRAME_SKIP + 1) * ((120000/7) / (SCALER_RELOAD + 1)) * LEON_CLK;	// SAMPLE_PER_FRAMEΪ189��378��756
			for (;;)
			{
//				cur_time = *(volatile int*)(TCNT2 + PREGS) * (MICROSEC_PER_COUNT);
				cur_time = *(volatile int*)(TCNT2 + PREGS);
				if(last_frame_time >= cur_time)
					break;
			}

			// ���Timer��Ҫ����ˣ������ֶ����أ�
			// ����ʱ��ֻ��Timer��λ���йأ���prescaler reload�Ĵ�С�޹�
//			if (last_frame_time < 3 * (FRAME_SKIP + 1) * SAMPLE_PER_FRAME * 1000000 / SAMPLE_PER_SEC / (MICROSEC_PER_COUNT))	// ���Ż�
//			if (last_frame_time < 360000 / (MICROSEC_PER_COUNT))	// FRAME_SKIPΪ6��SAMPLE_PER_FRAMEΪ189��378��756
//			if (last_frame_time < 0xFFFFF)
//			if (last_frame_time < 0xFFFFF / (MICROSEC_PER_COUNT))
			if (last_frame_time < 0xFFFFFF)
//			if (frame_count  == 1000)
			{
#if 1
				printf("last_frame_time = %x;Timer reload.\n", last_frame_time);
#endif

				frame_count = 1;

				*(volatile int*)(TCTRL2 + PREGS) = 0x7;	// ����Timer
//				base_time = *(volatile int*)(TCNT2 + PREGS) * (MICROSEC_PER_COUNT);
				base_time = *(volatile int*)(TCNT2 + PREGS);
			}
		}

		SLNES_Reset();

		//SetDisplayFrameBase((unsigned char*)P1);
		*(volatile int*)((DECODE_BASE_ADDR+DISPLAY_FRAME_BASE_ADDR) * 4 + MEMIO) = /*0x8000*/(int)(PPU0)>>2 & 0xFFFFFF;
	}

	// �˳���Ϸģ����
	SLNES_Fin();

	return 0;
}

/*******************************************************************
 *   �������ƣ� SLNES_ReadRom                                      *
 *   ������Ա�� ����                                               *
 *   �����汾�� 1.00                                               *
 *   �������ڣ� 2005/05/08 08:00:00                                *
 *   ���������� ��ȡNES��Ϸ�ļ����ݵ��ڴ���                        *
 *   ��ڲ����� const char *pszFileName �ļ���                     *
 *   ����ֵ  �� 0������                                            *
 *              -1������                                           *
 *   �޸ļ�¼��                                                    *
 *******************************************************************/
int SLNES_ReadRom(const char *pszFileName)
{
	//FILE *fp;

	///* Open ROM file */
	//fp = fopen(pszFileName, "rb");
	//if (fp == NULL)
	//	return -1;


	//fread(gamefile, 1, 188416, fp);
	if(SLNES_Init() == -1)
		return -1;

	//ROM_SRAM = 0;
	///* Clear SRAM */
	//memset(SRAM, 0, SRAM_SIZE);

	///* File close */
	//fclose(fp);

	/* Successful */
	return 0;
}

/*******************************************************************
 *   �������ƣ� SLNES_ReadRom                                      *
 *   ������Ա�� ����                                               *
 *   �����汾�� 1.00                                               *
 *   �������ڣ� 2005/05/08 08:00:00                                *
 *   ���������� ��ȡNES��Ϸ�ļ�֮ǰ���˳�ģ����ʱ�����ڴ�          *
 *   ��ڲ����� ��                                                 *
 *   ����ֵ  �� ��                                                 *
 *   �޸ļ�¼��                                                    *
 *******************************************************************/
void SLNES_ReleaseRom()
{
	ROM = NULL;
	VROM = NULL;
}

/*******************************************************************
 *   �������ƣ� SLNES_PadState                                     *
 *   ������Ա�� ����                                               *
 *   �����汾�� 1.00                                               *
 *   �������ڣ� 2005/05/08 08:00:00                                *
 *   ���������� ��ȡ�ֱ���ǰ״̬������NESģ����Ĵ���ʹ�ã�        *
 *   ��ڲ����� unsigned int *pdwPad1 �����ֱ�һ״̬�ı���         *
 *              unsigned int *pdwPad2 �����ֱ���״̬�ı���         *
 *              unsigned int *pdwSystem ����λ���˳���Ϣ�ı���   *
 *   ����ֵ  �� ��                                                 *
 *   �޸ļ�¼��                                                    *
 *******************************************************************/
#define READ_GM_DATA0 *(volatile int*)(IOREG + PREGS) & 1			// pio(0):DQ0	��DQ0�˿ڶ����ƽ
#define READ_GM_DATA1 *(volatile int*)(IOREG + PREGS) & 2			// pio(1):DQ1	��DQ1�˿ڶ����ƽ
#define SET_GM_LATCH0 *(volatile int*)(IOREG + PREGS) |= 4			// pio(2):OUT	��OUT�˿�����ߵ�ƽ
#define CLEAR_GM_LATCH0 *(volatile int*)(IOREG + PREGS) &= 0xFB		// pio(2):OUT	��OUT�˿�����͵�ƽ
#define SET_GM_CLK0 *(volatile int*)(IOREG + PREGS) |= 8			// pio(3):CLK	��CLK�˿�����ߵ�ƽ
#define CLEAR_GM_CLK0 *(volatile int*)(IOREG + PREGS) &= 0xF7		// pio(3):CLK	��CLK�˿�����͵�ƽ
void SLNES_PadState(unsigned int *pdwPad1, unsigned int *pdwPad2,
					unsigned int *pdwSystem)
{
    int i;

	*pdwPad1 = *pdwPad2 = 0 ;

    SET_GM_LATCH0;		// ��OUT�˿�����ߵ�ƽ
    CLEAR_GM_LATCH0;	// ��OUT�˿�����͵�ƽ����ʱ�ֱ�״̬������
	for (i = 0; i < 8; i++)
	{
		CLEAR_GM_CLK0;	// ��CLK�˿�����͵�ƽ
		*pdwPad1 |= ((READ_GM_DATA0) == 0) << i;
		*pdwPad2 |= ((READ_GM_DATA1) == 0) << i;
		SET_GM_CLK0;	// ��CLK�˿�����ߵ�ƽ
	}

	*pdwSystem = *pdwPad1 ;
#ifdef IR_GAMEPAD
	if(GB_ir_key)	// �����ң������ť��ť��������ң��������
	{
		*pdwSystem = 0xF0/*GB_ir_key*/ ;
		return;
	}
#endif
}

/*******************************************************************
 *   �������ƣ� SLNES_Menu                                         *
 *   ������Ա�� ����                                               *
 *   �����汾�� 1.00                                               *
 *   �������ڣ� 2005/05/08 08:00:00                                *
 *   ���������� �ж��ⲿ�˵�������ﲻʹ�ã�                     *
 *   ��ڲ����� ��                                                 *
 *   ����ֵ  �� 0����������ģ����                                  *
 *              -1���˳�ģ����                                     *
 *   �޸ļ�¼��                                                    *
 *******************************************************************/
int SLNES_Menu()
{
	// Nothing to do here
	return 0;
}

/*******************************************************************
 *   �������ƣ� SLNES_SoundOpen                                    *
 *   ������Ա�� ����                                               *
 *   �����汾�� 1.00                                               *
 *   �������ڣ� 2005/05/08 08:00:00                                *
 *   ���������� ��������Ӳ������ʼ������������ֵ                   *
 *   ��ڲ����� ��                                                 *
 *   ����ֵ  �� int samples_per_sync ÿ�β��ŵĲ���ֵ����          *
 *              int sample_rate ���ŵĲ�����                       *
 *   �޸ļ�¼��                                                    *
 *******************************************************************/
int SLNES_SoundOpen(int samples_per_sync, int sample_rate)
{
	return 0;
}

/*******************************************************************
 *   �������ƣ� SLNES_SoundClose                                   *
 *   ������Ա�� ����                                               *
 *   �����汾�� 1.00                                               *
 *   �������ڣ� 2005/05/08 08:00:00                                *
 *   ���������� �ر�����Ӳ����ֹͣ������������ֵ                   *
 *   ��ڲ����� ��                                                 *
 *   ����ֵ  �� ��                                                 *
 *   �޸ļ�¼��                                                    *
 *******************************************************************/
void SLNES_SoundClose(void) 
{
}

/*******************************************************************
 *   �������ƣ� WriteDMA                                           *
 *   ������Ա�� ����                                               *
 *   �����汾�� 1.00                                               *
 *   �������ڣ� 2005/05/08 08:00:00                                *
 *   ���������� ��DMA�ķ�ʽ��PPU��桢APU�����ٶȽ���            *
 *              ��SDRAM�д�������                                  *
 *   ��ڲ����� int *Data Դ���ݶε��׵�ַ����8λ���㣩            *
 *              int Length Դ���ݶεĴ�С                          *
 *              int MemBaseAddr Ŀ�����ݶε��׵�ַ����32λ���㣩   *
 *   ����ֵ  �� ��                                                 *
 *   �޸ļ�¼��                                                    *
 *******************************************************************/
void WriteDMA(int *Data, int Length, int MemBaseAddr)
{
	int i;

	//Go on when DMACache Status is Idle
	//while(GetDMAStatue(ReadBackStatus) == 1)
	while(((*(volatile int*)(0x044*4 + 0x20000000)) & 1) == 1)
	{
	}

	//DMASaveCache(MemAddressToSDRam,MemBaseAddr);
	*(volatile int*)(0x040*4 + 0x20000000) = MemBaseAddr&0xFFFFFF;
	//DMAWriteCache(WriteReadCacheSetUp, 0, 0 ,Length);
	*(volatile int*)(0x041*4 + 0x20000000) = Length & 0x3F;

	for (i = 0 ; i < Length; i++)
	{
		//DMAWriteData(WriteDataToCache, *(Data + i));
		*(volatile int*)(0x042*4 + 0x20000000) = *(Data + i);
	}
}

/*
 * End of SLNES_System_LEON.c
 */
