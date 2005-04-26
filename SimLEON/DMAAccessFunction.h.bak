//#include "jpeg_decoder.h"

//Add Reg Address and Mem Base Address
//1.Regs Used for DMA Cache (Read and Write)
#define MemAddressToSDRam		0x040*4 + 0x20000000
#define WriteReadCacheSetUp		0x041*4 + 0x20000000
#define WriteDataToCache		0x042*4 + 0x20000000
#define ReadDataFromCache		0x043*4 + 0x20000000
#define ReadBackStatus			0x044*4 + 0x20000000


#define RegAddr_DctCoefBufStart		0x40007C00
#define RegAddr_DctCoefBufEnd		0x40007C3F
#define RegAddr_PixelBufStart		0x40003000
#define RegAddr_PixelBufEnd			0x4000303F
#define RegAddr_IdctSignStart		0x40007400
#define RegAddr_IdctSignEnd			0x40007401

#define RegAddr_DecodeEnable        0x61*4 + 0x20000000
#define RegAddr_DecodeType			0x62*4 + 0x20000000
#define RegAddr_StartIdct			0x63*4 + 0x20000000
#define RegAddr_IdctCompleted		0x69*4 + 0x20000000
#define RegAddr_JpegCompleted		0x6A*4 + 0x20000000

#define BaseAddr_JPEGPicture		0x20000

//DMACacheStatue 0: idle 1: busy
int GetDMAStatue(int RegAddr);

//Set DMA Request Length and CacheBase Address
void DMAWriteCache(int RegAddr, int Length, int CacheBaseAddr);

//Write a data to DMA Cache
void DMAWriteData(int RegAddr, int *Data);

//Send a Command to Start Save Cache to Sdram
void DMASaveCache(int RegAddr, int SDRAMAddr);

//Send Command to Start Load Cache
void DMALoadCache(int RegAddr, int SDRAMAddr);

//Read Data From DMA Cache
int DMAReadData(int RegAddr, int *Data);

int WriteDataToSDRAM(int *Data, int Length, int MemBaseAddr);

int ReadDataFromSDRAM(int *Data, int Length, int MemBaseAddr);

