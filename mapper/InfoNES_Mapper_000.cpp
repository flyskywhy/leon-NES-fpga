/*===================================================================*/
/*                                                                   */
/*                            Mapper 0                               */
/*                                                                   */
/*===================================================================*/

/*-------------------------------------------------------------------*/
/*  Initialize Mapper 0                                              */
/*-------------------------------------------------------------------*/
void Map0_Init()
{
  /* Initialize Mapper */
  MapperInit = Map0_Init;

  /* Write to Mapper */
  MapperWrite = Map0_Write;

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
  if ( NesHeader.byRomSize > 1 )
  {
    ROMBANK0 = ROMPAGE( 0 );
    ROMBANK1 = ROMPAGE( 1 );
    ROMBANK2 = ROMPAGE( 2 );
    ROMBANK3 = ROMPAGE( 3 );
  }
  else if ( NesHeader.byRomSize > 0 )
  {
    ROMBANK0 = ROMPAGE( 0 );
    ROMBANK1 = ROMPAGE( 1 );
    ROMBANK2 = ROMPAGE( 0 );
    ROMBANK3 = ROMPAGE( 1 );
  } else {
    ROMBANK0 = ROMPAGE( 0 );
    ROMBANK1 = ROMPAGE( 0 );
    ROMBANK2 = ROMPAGE( 0 );
    ROMBANK3 = ROMPAGE( 0 );
  }

  /* Set PPU Banks */
  if ( NesHeader.byVRomSize > 0 )
  {
    for ( int nPage = 0; nPage < 8; ++nPage )
      PPUBANK[ nPage ] = VROMPAGE( nPage );
    InfoNES_SetupChr();
  }

  /* Set up wiring of the interrupt pin */
  K6502_Set_Int_Wiring( 1, 1 ); 
}

/*-------------------------------------------------------------------*/
/*  Mapper 0 Write Function                                          */
/*-------------------------------------------------------------------*/
void Map0_Write( WORD wAddr, BYTE byData )
{
/*
 *  Dummy Write to Mapper
 *
 */
}

/*-------------------------------------------------------------------*/
/*  Mapper 0 Write to SRAM Function                                  */
/*-------------------------------------------------------------------*/
//加速 void Map0_Sram( WORD wAddr, BYTE byData )
//加速 {
/*
 *  Dummy Write to Sram
 *
 */
//加速 }

/*-------------------------------------------------------------------*/
/*  Mapper 0 Write to APU Function                                   */
/*-------------------------------------------------------------------*/
//加速 void Map0_Apu( WORD wAddr, BYTE byData )
//加速 {
/*
 *  Dummy Write to Apu
 *
 */
//加速 }

/*-------------------------------------------------------------------*/
/*  Mapper 0 Read from APU Function                                  */
/*-------------------------------------------------------------------*/
//加速BYTE Map0_ReadApu( WORD wAddr )
//加速{
/*
 *  Dummy Read from Apu
 *
 */
//加速  return ( wAddr >> 8 );
//加速}

/*-------------------------------------------------------------------*/
/*  Mapper 0 V-Sync Function                                         */
/*-------------------------------------------------------------------*/
//加速 void Map0_VSync()
//加速 {
/*
 *  Dummy Callback at VSync
 *
 */
//加速 }

/*-------------------------------------------------------------------*/
/*  Mapper 0 H-Sync Function                                         */
/*-------------------------------------------------------------------*/
//加速 void Map0_HSync()
//加速 {
/*
 *  Dummy Callback at HSync
 *
 */
//加速 #if 0 
  // Frame IRQ
//加速   FrameStep += STEP_PER_SCANLINE;
//加速   if ( FrameStep > STEP_PER_FRAME && FrameIRQ_Enable )
//加速   {
//加速     FrameStep %= STEP_PER_FRAME;
//加速     IRQ_REQ;
//加速     APU_Reg[ 0x4015 ] |= 0x40;
//加速   }
//加速 #endif
//加速 }

/*-------------------------------------------------------------------*/
/*  Mapper 0 PPU Function                                            */
/*-------------------------------------------------------------------*/
//加速 void Map0_PPU( WORD wAddr )
//加速 {
/*
 *  Dummy Callback at PPU
 *
 */
//加速 }

/*-------------------------------------------------------------------*/
/*  Mapper 0 Rendering Screen Function                               */
/*-------------------------------------------------------------------*/
//减容 void Map0_RenderScreen( BYTE byMode )
//减容 {
/*
 *  Dummy Callback at Rendering Screen
 *
 */
//减容 }
