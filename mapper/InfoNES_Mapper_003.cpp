/*===================================================================*/
/*                                                                   */
/*                     Mapper 3 (VROM Switch)                        */
/*                                                                   */
/*===================================================================*/

/*-------------------------------------------------------------------*/
/*  Initialize Mapper 3                                              */
/*-------------------------------------------------------------------*/
void Map3_Init()
{
  int nPage;

  /* Initialize Mapper */
  MapperInit = Map3_Init;

  /* Write to Mapper */
  MapperWrite = Map3_Write;

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
  if ( ( NesHeader.byRomSize << 1 ) > 2 )
  {
    ROMBANK0 = ROMPAGE( 0 );
    ROMBANK1 = ROMPAGE( 1 );
    ROMBANK2 = ROMPAGE( 2 );
    ROMBANK3 = ROMPAGE( 3 );    
  } else {
    ROMBANK0 = ROMPAGE( 0 );
    ROMBANK1 = ROMPAGE( 1 );
    ROMBANK2 = ROMPAGE( 0 );
    ROMBANK3 = ROMPAGE( 1 );
  }

  /* Set PPU Banks */
  if ( NesHeader.byVRomSize > 0 )
  {
    for ( nPage = 0; nPage < 8; ++nPage )
    {
      PPUBANK[ nPage ] = VROMPAGE( nPage );
    }
#ifdef INES
			  NES_ChrGen = PPUBANK[ ( PPU_R0 & R0_BG_ADDR ) >> 2];
			  NES_SprGen = PPUBANK[ ( PPU_R0 & R0_SP_ADDR ) >> 1];
#else
//    InfoNES_SetupChr();
#endif /* INES */
  }

  /* Set up wiring of the interrupt pin */
  /* "DragonQuest" doesn't run if IRQ isn't made to occur in CLI */
  //K6502_Set_Int_Wiring( 1, 1 ); 
}

/*-------------------------------------------------------------------*/
/*  Mapper 3 Write Function                                          */
/*-------------------------------------------------------------------*/
void Map3_Write( WORD wAddr, BYTE byData )
{
  DWORD dwBase;

  /* Set PPU Banks */
  byData %= NesHeader.byVRomSize;
  dwBase = ( (DWORD)byData ) << 3;

  PPUBANK[ 0 ] = VROMPAGE( dwBase + 0 );
  PPUBANK[ 1 ] = VROMPAGE( dwBase + 1 );
  PPUBANK[ 2 ] = VROMPAGE( dwBase + 2 );
  PPUBANK[ 3 ] = VROMPAGE( dwBase + 3 );
  PPUBANK[ 4 ] = VROMPAGE( dwBase + 4 );
  PPUBANK[ 5 ] = VROMPAGE( dwBase + 5 );
  PPUBANK[ 6 ] = VROMPAGE( dwBase + 6 );
  PPUBANK[ 7 ] = VROMPAGE( dwBase + 7 );

#ifdef INES
			  NES_ChrGen = PPUBANK[ ( PPU_R0 & R0_BG_ADDR ) >> 2];
			  NES_SprGen = PPUBANK[ ( PPU_R0 & R0_SP_ADDR ) >> 1];
#else
//    InfoNES_SetupChr();
#endif /* INES */
}
