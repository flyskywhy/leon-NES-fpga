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
  MapperSram = Map0_Sram;

  /* Write to APU */
  MapperApu = Map0_Apu;

  /* Read from APU */
  MapperReadApu = Map0_ReadApu;

  /* Callback at VSync */
  MapperVSync = Map0_VSync;

  /* Callback at HSync */
  MapperHSync = Map0_HSync;

  /* Callback at PPU */
//加速  MapperPPU = Map0_PPU;

  /* Callback at Rendering Screen ( 1:BG, 0:Sprite ) */
//减容   MapperRenderScreen = Map0_RenderScreen;

  /* Set SRAM Banks */
  SRAMBANK = SRAM;

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
void Map0_Sram( WORD wAddr, BYTE byData )
{
/*
 *  Dummy Write to Sram
 *
 */
}

/*-------------------------------------------------------------------*/
/*  Mapper 0 Write to APU Function                                   */
/*-------------------------------------------------------------------*/
void Map0_Apu( WORD wAddr, BYTE byData )
{
/*
 *  Dummy Write to Apu
 *
 */
}

/*-------------------------------------------------------------------*/
/*  Mapper 0 Read from APU Function                                  */
/*-------------------------------------------------------------------*/
BYTE Map0_ReadApu( WORD wAddr )
{
/*
 *  Dummy Read from Apu
 *
 */
  return ( wAddr >> 8 );
}

/*-------------------------------------------------------------------*/
/*  Mapper 0 V-Sync Function                                         */
/*-------------------------------------------------------------------*/
void Map0_VSync()
{
/*
 *  Dummy Callback at VSync
 *
 */
}

/*-------------------------------------------------------------------*/
/*  Mapper 0 H-Sync Function                                         */
/*-------------------------------------------------------------------*/
void Map0_HSync()
{
/*
 *  Dummy Callback at HSync
 *
 */
#if 0 
  // Frame IRQ
  FrameStep += STEP_PER_SCANLINE;
  if ( FrameStep > STEP_PER_FRAME && FrameIRQ_Enable )
  {
    FrameStep %= STEP_PER_FRAME;
    IRQ_REQ;
    APU_Reg[ 0x4015 ] |= 0x40;
  }
#endif
}

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
