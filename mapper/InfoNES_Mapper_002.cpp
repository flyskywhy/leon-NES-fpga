/*===================================================================*/
/*                                                                   */
/*                        Mapper 2 (UNROM)                           */
/*                                                                   */
/*===================================================================*/

/*-------------------------------------------------------------------*/
/*  Initialize Mapper 2                                              */
/*-------------------------------------------------------------------*/
void Map2_Init()
{
  /* Initialize Mapper */
  MapperInit = Map2_Init;

  /* Write to Mapper */
  MapperWrite = Map2_Write;

  /* Write to SRAM */
//加速   MapperSram = Map0_Sram;

  /* Write to APU */
//加速   MapperApu = Map0_Apu;

  /* Read from APU */
//加速  MapperReadApu = Map0_ReadApu;

  /* Callback at VSync */
//加速   MapperVSync = Map0_VSync;

  /* Callback at HSync */
//加速   MapperHSync = Map0_HSync;

  /* Callback at PPU */
//加速  MapperPPU = Map0_PPU;

  /* Callback at Rendering Screen ( 1:BG, 0:Sprite ) */
//减容   MapperRenderScreen = Map0_RenderScreen;

  /* Set SRAM Banks */
//减容   SRAMBANK = SRAM;

  /* Set ROM Banks */
  ROMBANK0 = ROMPAGE( 0 );
  ROMBANK1 = ROMPAGE( 1 );
  ROMBANK2 = ROMLASTPAGE( 1 );
  ROMBANK3 = ROMLASTPAGE( 0 );

  /* Set up wiring of the interrupt pin */
  //K6502_Set_Int_Wiring( 1, 1 ); 
}

/*-------------------------------------------------------------------*/
/*  Mapper 2 Write Function                                          */
/*-------------------------------------------------------------------*/
void Map2_Write( WORD wAddr, BYTE byData )
{
  /* Set ROM Banks */
  byData %= NesHeader.byRomSize;
  byData <<= 1;

  ROMBANK0 = ROMPAGE( byData );
  ROMBANK1 = ROMPAGE( byData + 1 );

#if PocketNES == 1
  memmap_tbl[ 4 ] = ROMBANK0 - 0x8000;		//这里- 0x8000是为了在encodePC中不用再做& 0x1FFF的运算了，下同
  memmap_tbl[ 5 ] = ROMBANK1 - 0xA000;
#endif
}
