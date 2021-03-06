#include "DMAAccessFunction.h"
#include "hostsim.h"

//DMACacheStatue 0: idle 1: busy
int GetDMAStatue(int RegAddr)
{
	int Statue;
	BasicReadReg32(RegAddr, &Statue);
	return (Statue&0x00000001);

}

//Set DMA Request Length and CacheBase Address
void DMAWriteCache(int RegAddr, int ReadOrWrite, int CacheBaseAddr, int Length)
{
	int Data = ((ReadOrWrite&0x3)<<11)|((CacheBaseAddr&0x1F)<<6)|(Length&0x3F);
	BasicWriteReg32(RegAddr, Data);
}

//Write a data to DMA Cache
void DMAWriteData(int RegAddr, int Data)
{
	BasicWriteReg32(RegAddr, Data);
}

//Set Sdram Address to DMA Cache
void DMASetSDRAMAddr(int RegAddr, int SDRAMAddr)
{
	int Data = SDRAMAddr&0xFFFFFF;
	BasicWriteReg32(RegAddr, Data);
}

//Read Data From DMA Cache
int DMAReadData(int RegAddr, int *Data)
{
	BasicReadReg32(RegAddr, Data);
	return *Data;
}

int WriteDataToSDRAM(int *Data, int Length, int MemBaseAddr)
{
	int i;
	//Go on when DMACache Status is Idle
	while(GetDMAStatue(ReadBackStatus) == 1)
	{
	}

	DMASetSDRAMAddr(MemAddressToSDRam,MemBaseAddr);
	DMAWriteCache(WriteReadCacheSetUp, 0, 0 ,Length);

	for( i = 0 ; i < Length; i++)
	{
		
		DMAWriteData(WriteDataToCache, *(Data + i));
	}
	return Length;

}

int ReadDataFromSDRAM(int *Data, int Length, int MemBaseAddr)
{
	int i;
	while(GetDMAStatue(ReadBackStatus) == 1)
	{
	}

	DMASetSDRAMAddr(MemAddressToSDRam,MemBaseAddr);
	DMAWriteCache(WriteReadCacheSetUp, 1, 0 ,Length);
	
	 //Go on when DMACache Status is Idle
	while(GetDMAStatue(ReadBackStatus) == 1)
	{
	}
	for(i = 0; i < Length; i ++)
	{
		DMAReadData(ReadDataFromCache , (Data + i));
	}
	return 1;
}

unsigned char clamp(int i)
{
  if (i & 0xFFFFFF00)
    i = (((~i) >> 31) & 0xFF);

  return (i);
}