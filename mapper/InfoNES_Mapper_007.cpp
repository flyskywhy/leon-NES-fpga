/*===================================================================*/
/*                                                                   */
/*                        Mapper 7 (AOROM)                           */
/*                                                                   */
/*===================================================================*/

/*-------------------------------------------------------------------*/
/*  Initialize Mapper 7                                              */
/*-------------------------------------------------------------------*/
void Map7_Init()
{
  /* Initialize Mapper */
  MapperInit = Map7_Init;

  /* Write to Mapper */
  MapperWrite = Map7_Write;

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
  ROMBANK2 = ROMPAGE( 2 );
  ROMBANK3 = ROMPAGE( 3 );

  /* Set up wiring of the interrupt pin */
  K6502_Set_Int_Wiring( 1, 1 ); 
}

/*-------------------------------------------------------------------*/
/*  Mapper 7 Write Function                                          */
/*-------------------------------------------------------------------*/
void Map7_Write( WORD wAddr, BYTE byData )
{
  BYTE byBank;

  /* Set ROM Banks */
  byBank = ( byData & 0x07 ) << 2;
  byBank %= ( NesHeader.byRomSize << 1 );

  ROMBANK0 = ROMPAGE( byBank );
  ROMBANK1 = ROMPAGE( byBank + 1 );
  ROMBANK2 = ROMPAGE( byBank + 2 );
  ROMBANK3 = ROMPAGE( byBank + 3 );

  /* Name Table Mirroring */
  InfoNES_Mirroring( byData & 0x10 ? 2 : 3 );
}
