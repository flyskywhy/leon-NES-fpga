/*===================================================================*/
/*                                                                   */
/*  InfoNES_System_Linux.cpp : Linux specific File                   */
/*                                                                   */
/*  2001/05/18  InfoNES Project ( Sound is based on DarcNES )        */
/*                                                                   */
/*===================================================================*/

/*-------------------------------------------------------------------*/
/*  Include files                                                    */
/*-------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//lizheng #include <gtk/gtk.h>
//lizheng #include <gdk/gdkkeysyms.h>
//lizheng #include <pthread.h>

//#include <sys/types.h>
//#include <sys/stat.h>
//lizheng #include <fcntl.h>
//“Ù∆µ #include <sys/ioctl.h>
//#include <unistd.h>
//“Ù∆µ #include <sys/soundcard.h>

#include "../InfoNES.h"
#include "../InfoNES_System.h"
#include "../InfoNES_pAPU.h"

//LEON
#include "../gamefile/contra.h"
DWORD FrameCount = 0;

#define TRUE 1
#define FALSE 0

//#include <time.h>
//clock_t cFStart;
//clock_t cFEnd = 0;
//clock_t cFTime, cOTime;

/*-------------------------------------------------------------------*/
/*  ROM image file information                                       */
/*-------------------------------------------------------------------*/

char szRomName[ 256 ];
//lizheng char szSaveName[ 256 ];
//lizheng int nSRAM_SaveFlag;

/*-------------------------------------------------------------------*/
/*  Constants ( Linux specific )                                     */
/*-------------------------------------------------------------------*/

//lizheng #define VBOX_SIZE    7 
//lizheng #define SOUND_DEVICE "/dev/dsp"
#define VERSION      "InfoNES v0.91J"

/*-------------------------------------------------------------------*/
/*  Global Variables ( Linux specific )                              */
/*-------------------------------------------------------------------*/

/* GtkWidget is the storage type for widgets */
//lizheng GtkWidget *top, *vbox, *draw_area, *filew;

//lizheng GdkPixmap *pixmap;
//lizheng GdkBitmap *mask;
//lizheng GdkBitmap GtkStyle *style;

/* Emulation thread */
//lizheng pthread_t emulation_tid;
int bThread;

/* Pad state */
DWORD dwKeyPad1;
DWORD dwKeyPad2;
DWORD dwKeySystem;

/* For Sound Emulation */
//“Ù∆µ BYTE final_wave[2048];
//“Ù∆µ int waveptr;
//“Ù∆µ int wavflag;
//“Ù∆µ int sound_fd;

/*-------------------------------------------------------------------*/
/*  Function prototypes ( Linux specific )                           */
/*-------------------------------------------------------------------*/

//lizheng void *emulation_thread( void *args );
// ÷±˙ void add_key( GtkWidget *widget, GdkEventKey *event, gpointer callback_data );
// ÷±˙ void remove_key( GtkWidget *widget, GdkEventKey *event, gpointer callback_data );
//lizheng gint close_application( GtkWidget *widget, GdkEvent *event, gpointer data );
void reset_application( void );
//lizheng gint start_application_aux( GtkObject *fw );
void start_application( char *filename );
//lizheng void close_dialog( GtkWidget *widget, gpointer data );
//lizheng void closing_dialog( GtkWidget *widget, gpointer data );

//lizheng int LoadSRAM();
//lizheng int SaveSRAM();

/* Palette data */
WORD NesPalette[ 64 ] =
{
  0x39ce, 0x1071, 0x0015, 0x2013, 0x440e, 0x5402, 0x5000, 0x3c20,
  0x20a0, 0x0100, 0x0140, 0x00e2, 0x0ceb, 0x0000, 0x0000, 0x0000,
  0x5ef7, 0x01dd, 0x10fd, 0x401e, 0x5c17, 0x700b, 0x6ca0, 0x6521,
  0x45c0, 0x0240, 0x02a0, 0x0247, 0x0211, 0x0000, 0x0000, 0x0000,
  0x7fff, 0x1eff, 0x2e5f, 0x223f, 0x79ff, 0x7dd6, 0x7dcc, 0x7e67,
  0x7ae7, 0x4342, 0x2769, 0x2ff3, 0x03bb, 0x0000, 0x0000, 0x0000,
  0x7fff, 0x579f, 0x635f, 0x6b3f, 0x7f1f, 0x7f1b, 0x7ef6, 0x7f75,
  0x7f94, 0x73f4, 0x57d7, 0x5bf9, 0x4ffe, 0x0000, 0x0000, 0x0000
};

/*===================================================================*/
/*                                                                   */
/*                main() : Application main                          */
/*                                                                   */
/*===================================================================*/

/* Application main */
void main(void)
{                                  
  int argc=2;
  char *argv[2]={"InfoNES","mario.nes"};	

//  int       i;

  /*-------------------------------------------------------------------*/
  /*  Initialize GTK+/GDK                                              */
  /*-------------------------------------------------------------------*/

//lizheng   g_thread_init( NULL );
//lizheng   gtk_set_locale();
//lizheng   gtk_init(&argc, &argv);
//lizheng   gdk_rgb_init();

  /*-------------------------------------------------------------------*/
  /*  Create a top window                                              */
  /*-------------------------------------------------------------------*/

  /* Create a window */ 
//lizheng   top=gtk_window_new(GTK_WINDOW_TOPLEVEL);
//lizheng   gtk_widget_set_usize( GTK_WIDGET(top), NES_DISP_WIDTH + VBOX_SIZE, NES_DISP_HEIGHT + VBOX_SIZE );
//lizheng   gtk_window_set_title( GTK_WINDOW(top), VERSION );
//lizheng   gtk_widget_set_events( GTK_WIDGET(top), GDK_KEY_RELEASE_MASK );

  /* Destroy a window */
//lizheng         gtk_signal_connect( GTK_OBJECT(top), "destroy",
//lizheng       	      	      GTK_SIGNAL_FUNC( close_application ),
//lizheng		            NULL );
  
  /* Create a vbox */ 
//lizheng  vbox=gtk_vbox_new(FALSE, 5);
//lizheng  gtk_container_set_border_width(GTK_CONTAINER(vbox), 3);
//lizheng  gtk_container_add(GTK_CONTAINER(top), vbox);
  
  /* Create a drawing area */ 
//lizheng  draw_area = gtk_drawing_area_new();
//lizheng  gtk_box_pack_start(GTK_BOX (vbox), draw_area, TRUE, TRUE, 0);

//lizheng   gtk_widget_show_all(top);

  /*-------------------------------------------------------------------*/
  /*  Pad Control                                                      */
  /*-------------------------------------------------------------------*/

  /* Initialize a pad state */
  dwKeyPad1   = 0;
  dwKeyPad2   = 0;
  dwKeySystem = 0;

  /* Connecting to key event */
// ÷±˙  gtk_signal_connect( GTK_OBJECT(top), "key_press_event",
// ÷±˙		      GTK_SIGNAL_FUNC( add_key ),
// ÷±˙		      NULL );

// ÷±˙  gtk_signal_connect( GTK_OBJECT(top), "key_release_event",
// ÷±˙		      GTK_SIGNAL_FUNC( remove_key ),
// ÷±˙		      NULL );

  /*-------------------------------------------------------------------*/
  /*  Load Cassette & Create Thread                                    */
  /*-------------------------------------------------------------------*/

  /* Initialize thread state */
  bThread = FALSE;

  /* If a rom name specified, start it */
  if ( argc == 2 )
  {
    start_application( argv[ 1 ] );
  }

  /* show the window */
//lizheng   gdk_threads_enter();
//lizheng   gtk_main ();
//lizheng   gdk_threads_leave();

//  return(0);
}

/*===================================================================*/
/*                                                                   */
/*           emulation_thread() : Thread Hooking Routine             */
/*                                                                   */
/*===================================================================*/

//lizheng void *emulation_thread(void *args)
//lizheng {
//lizheng   InfoNES_Main();
//lizheng }

/*===================================================================*/
/*                                                                   */
/*          add_key() : Connecting to the key_press_event event      */
/*                                                                   */
/*===================================================================*/

/*void add_key( GtkWidget *widget, GdkEventKey *event, gpointer callback_data )
{
  switch ( event->keyval )
  {
    case GDK_Right:
      dwKeyPad1 |= ( 1 << 7 );
      break;

    case GDK_Left:
      dwKeyPad1 |= ( 1 << 6 );
      break;

    case GDK_Down:
      dwKeyPad1 |= ( 1 << 5 );
      break;
      
    case GDK_Up:
      dwKeyPad1 |= ( 1 << 4 );
      break;

    case 's':
    case 'S':*/// ÷±˙
      /* Start */
/*      dwKeyPad1 |= ( 1 << 3 );
      break;

    case 'a':
    case 'A':*/// ÷±˙
      /* Select */
/*      dwKeyPad1 |= ( 1 << 2 );
      break;

    case 'z':                     
    case 'Z':*/// ÷±˙                     
      /* 'A' */
/*      dwKeyPad1 |= ( 1 << 1 );
      break;

    case 'x': 
    case 'X': */// ÷±˙
      /* 'B' */
/*      dwKeyPad1 |= ( 1 << 0 );
      break;

    case 'c':
    case 'C':   */// ÷±˙
      /* Toggle up and down clipping */
/*      PPU_UpDown_Clip = ( PPU_UpDown_Clip ? 0 : 1 );
      break;

    case 'q':
    case 'Q':
      close_application( widget, NULL, NULL );
      break;

    case 'r':
    case 'R':*/// ÷±˙
      /* Reset the application */
/*      reset_application(); 
      break;*/// ÷±˙

//lizheng    case 'l':
//lizheng    case 'L':  
      /* If emulation thread runs, nothing here */
//lizheng      if ( bThread != TRUE )
//lizheng      {
	/* Create a file selection widget */
//lizheng	filew = gtk_file_selection_new( "Load" );
//lizheng	gtk_widget_show( filew );
	
        /* Connecting to button event */
//lizheng	gtk_signal_connect_object( GTK_OBJECT( GTK_FILE_SELECTION( filew )->ok_button ), 
//lizheng				   "clicked",
//lizheng				   ( GtkSignalFunc ) start_application_aux,
//lizheng				   GTK_OBJECT( filew ) );

//lizheng	gtk_signal_connect_object( GTK_OBJECT( GTK_FILE_SELECTION( filew )->cancel_button ), 
//lizheng				   "clicked",
//lizheng				   ( GtkSignalFunc ) gtk_widget_destroy,
//lizheng				   GTK_OBJECT( filew ) );
//lizheng      }
//lizheng      break;

// ÷±˙    case GDK_Page_Up:
      /* Increase Frame Skip */
// ÷±˙      FrameSkip++;
// ÷±˙      break;

// ÷±˙    case GDK_Page_Down:  
      /* Decrease Frame Skip */
// ÷±˙      if ( FrameSkip > 0 )
// ÷±˙      {
// ÷±˙	FrameSkip--;
// ÷±˙      }
// ÷±˙      break;
      
// ÷±˙    case 'm':
// ÷±˙    case 'M':
// ÷±˙      /* Toggle of sound mute */
// ÷±˙      APU_Mute = ( APU_Mute ? 0 : 1 );
// ÷±˙      break;
      
// ÷±˙          case 'i':
// ÷±˙    case 'I':
      /* If emulation thread doesn't run, nothing here */
// ÷±˙      if ( bThread )
// ÷±˙      {
// ÷±˙	InfoNES_MessageBox( "Mapper : %d\nPRG ROM : %dKB\nCHR ROM : %dKB\n" \
// ÷±˙			    "Mirroring : %s\nSRAM : %s\n4 Screen : %s\nTrainer : %s\n",
// ÷±˙			    MapperNo, NesHeader.byRomSize * 16, NesHeader.byVRomSize * 8,
// ÷±˙			    ( ROM_Mirroring ? "V" : "H" ), ( ROM_SRAM ? "Yes" : "No" ), 
// ÷±˙			    ( ROM_FourScr ? "Yes" : "No" ), ( ROM_Trainer ? "Yes" : "No" ) );
// ÷±˙      }
// ÷±˙      break;

// ÷±˙    case 'v':
// ÷±˙    case 'V':
// ÷±˙      /* Version Infomation */
// ÷±˙      InfoNES_MessageBox( "%s\nA fast and portable NES emulator\n"
// ÷±˙			  "Copyright (c) 1999-2003 Jay's Factory <jays_factory@excite.co.jp>",
// ÷±˙			  VERSION );
// ÷±˙      break;
// ÷±˙
// ÷±˙    defalut:  
// ÷±˙      break;
// ÷±˙  }
// ÷±˙}

/*===================================================================*/
/*                                                                   */
/*       remove_key() : Connecting to the key_release_event event    */
/*                                                                   */
/*===================================================================*/

//lizheng void remove_key( GtkWidget *widget, GdkEventKey *event, gpointer callback_data )
//lizheng {
//lizheng   switch ( event->keyval )
//lizheng   {
//lizheng     case GDK_Right:
//lizheng       dwKeyPad1 &= ~( 1 << 7 );
//lizheng       break;
//lizheng 
//lizheng     case GDK_Left:
//lizheng       dwKeyPad1 &= ~( 1 << 6 );
//lizheng       break;
//lizheng 
//lizheng     case GDK_Down:
//lizheng       dwKeyPad1 &= ~( 1 << 5 );
//lizheng       break;
//lizheng       
//lizheng     case GDK_Up:
//lizheng       dwKeyPad1 &= ~( 1 << 4 );
//lizheng       break;
//lizheng 
//lizheng     case 's':
//lizheng     case 'S':
//lizheng       /* Start */
//lizheng       dwKeyPad1 &= ~( 1 << 3 );
//lizheng       break;
//lizheng 
//lizheng     case 'a':
//lizheng     case 'A':
//lizheng       /* Select */
//lizheng       dwKeyPad1 &= ~( 1 << 2 );
//lizheng       break;
//lizheng 
//lizheng     case 'z':
//lizheng     case 'Z': 
//lizheng       /* 'A' */
//lizheng       dwKeyPad1 &= ~( 1 << 1 );
//lizheng       break;
//lizheng 
//lizheng     case 'x': 
//lizheng     case 'X': 
//lizheng       /* 'B' */
//lizheng       dwKeyPad1 &= ~( 1 << 0 );
//lizheng       break;
//lizheng 
//lizheng #if 0
//lizheng     case 'q':
//lizheng     case 'Q':
//lizheng       /* Terminate emulation thread */
//lizheng       dwKeySystem &= ~( PAD_SYS_QUIT );
//lizheng       break;
//lizheng #endif
//lizheng 
//lizheng     defalut:  
//lizheng       break;
//lizheng   }
//lizheng }
 
 /*===================================================================*/
/*                                                                   */
/*     start_application() : Start NES Hardware                      */
/*                                                                   */
/*===================================================================*/
void start_application( char *filename )
{
  /* Set a ROM image name */
  strcpy( szRomName, filename );

  /* Load cassette */
  if ( InfoNES_Load ( szRomName ) == 0 )
  { 
    /* Load SRAM */
//lizheng     LoadSRAM();

    /* Create Emulation Thread */
    bThread = TRUE;
//lizheng     pthread_create( &emulation_tid, NULL, emulation_thread, NULL );
    InfoNES_Main();
  }
}

/* Wrapper function for GTK file selection */
//lizheng gint start_application_aux( GtkObject *gfs )
//lizheng {
  /* Call actual function */
//lizheng   start_application( gtk_file_selection_get_filename( GTK_FILE_SELECTION( gfs ) ) );

  /* Destroy a file selection widget */
//lizheng   gtk_widget_destroy( GTK_WIDGET( gfs ) );
//lizheng }

/*===================================================================*/
/*                                                                   */
/*     close_application() : When invoked via signal delete_event    */
/*                                                                   */
/*===================================================================*/
//lizheng gint close_application( GtkWidget *widget, GdkEvent *event, gpointer data )
//lizheng {
  /* Terminate emulation thread */
//lizheng   bThread = FALSE;
//lizheng   dwKeySystem |= PAD_SYS_QUIT; 

  /* Leave Critical Section */
//lizheng  gdk_threads_leave();

  /* Waiting for Termination */
//lizheng  pthread_join( emulation_tid, NULL );
  
  /* Enter Critical Section */
//lizheng  gdk_threads_enter();

  /* Save SRAM*/
//lizheng   SaveSRAM();

  /* Terminates the application */
//lizheng   gtk_main_quit();

//lizheng   return( FALSE );
//lizheng }

/*===================================================================*/
/*                                                                   */
/*     reset_application() : Reset NES Hardware                      */
/*                                                                   */
/*===================================================================*/
void reset_application( void )
{
  /* Do nothing if emulation thread does not exists */
  if ( bThread == TRUE )
  {
    /* Terminate emulation thread */
    bThread = FALSE;
    dwKeySystem |= PAD_SYS_QUIT; 

    /* Leave Critical Section */
//lizheng     gdk_threads_leave();

    /* Waiting for Termination */
//lizheng     pthread_join( emulation_tid, NULL );
  
    /* Enter Critical Section */
//lizheng     gdk_threads_enter();

    /* Save SRAM File */
//lizheng     SaveSRAM();

    /* Load cassette */
    if ( InfoNES_Load ( szRomName ) == 0 )
    { 
      /* Load SRAM */
//lizheng       LoadSRAM();

      /* Create Emulation Thread */
      bThread = TRUE;
//lizheng       pthread_create( &emulation_tid, NULL, emulation_thread, NULL );
      InfoNES_Main();
    }
  }
}

/*===================================================================*/
/*                                                                   */
/*           LoadSRAM() : Load a SRAM                                */
/*                                                                   */
/*===================================================================*/
//lizheng int LoadSRAM()
//lizheng {
//lizheng /*
//lizheng  *  Load a SRAM
//lizheng  *
//lizheng  *  Return values
//lizheng  *     0 : Normally
//lizheng  *    -1 : SRAM data couldn't be read
//lizheng  */
//lizheng 
//lizheng   FILE *fp;
//lizheng   unsigned char pSrcBuf[ SRAM_SIZE ];
//lizheng   unsigned char chData;
//lizheng   unsigned char chTag;
//lizheng   int nRunLen;
//lizheng   int nDecoded;
//lizheng   int nDecLen;
//lizheng   int nIdx;
//lizheng 
//lizheng   // It doesn't need to save it
//lizheng   nSRAM_SaveFlag = 0;
//lizheng 
//lizheng   // It is finished if the ROM doesn't have SRAM
//lizheng   if ( !ROM_SRAM )
//lizheng     return 0;
//lizheng 
//lizheng   // There is necessity to save it
//lizheng   nSRAM_SaveFlag = 1;
//lizheng 
//lizheng   // The preparation of the SRAM file name
//lizheng   strcpy( szSaveName, szRomName );
//lizheng   strcpy( strrchr( szSaveName, '.' ) + 1, "srm" );
//lizheng 
//lizheng   /*-------------------------------------------------------------------*/
//lizheng   /*  Read a SRAM data                                                 */
//lizheng   /*-------------------------------------------------------------------*/
//lizheng 
//lizheng   // Open SRAM file
//lizheng   fp = fopen( szSaveName, "rb" );
//lizheng   if ( fp == NULL )
//lizheng     return -1;
//lizheng 
//lizheng   // Read SRAM data
//lizheng   fread( pSrcBuf, SRAM_SIZE, 1, fp );
//lizheng 
//lizheng   // Close SRAM file
//lizheng   fclose( fp );
//lizheng 
//lizheng   /*-------------------------------------------------------------------*/
//lizheng   /*  Extract a SRAM data                                              */
//lizheng   /*-------------------------------------------------------------------*/
//lizheng 
//lizheng   nDecoded = 0;
//lizheng   nDecLen = 0;
//lizheng 
//lizheng   chTag = pSrcBuf[ nDecoded++ ];
//lizheng 
//lizheng  while ( nDecLen < 8192 )
//lizheng  {
//lizheng    chData = pSrcBuf[ nDecoded++ ];
//lizheng
//lizheng    if ( chData == chTag )
//lizheng    {
//lizheng      chData = pSrcBuf[ nDecoded++ ];
//lizheng      nRunLen = pSrcBuf[ nDecoded++ ];
//lizheng      for ( nIdx = 0; nIdx < nRunLen + 1; ++nIdx )
//lizheng      {
//lizheng         SRAM[ nDecLen++ ] = chData;
//lizheng       }
//lizheng     }
//lizheng     else
//lizheng     {
//lizheng       SRAM[ nDecLen++ ] = chData;
//lizheng     }
//lizheng   }
//lizheng 
//lizheng   // Successful
//lizheng   return 0;
//lizheng }

/*===================================================================*/
/*                                                                   */
/*           SaveSRAM() : Save a SRAM                                */
/*                                                                   */
/*===================================================================*/
//lizheng int SaveSRAM()
//lizheng {
/*
 *  Save a SRAM
 *
 *  Return values
 *     0 : Normally
 *    -1 : SRAM data couldn't be written
 */

//lizheng   FILE *fp;
//lizheng   int nUsedTable[ 256 ];
//lizheng   unsigned char chData;
//lizheng   unsigned char chPrevData;
//lizheng   unsigned char chTag;
//lizheng   int nIdx;
//lizheng   int nEncoded;
//lizheng   int nEncLen;
//lizheng   int nRunLen;
//lizheng   unsigned char pDstBuf[ SRAM_SIZE ];

//lizheng   if ( !nSRAM_SaveFlag )
//lizheng     return 0;  // It doesn't need to save it

  /*-------------------------------------------------------------------*/
  /*  Compress a SRAM data                                             */
  /*-------------------------------------------------------------------*/

//lizheng   memset( nUsedTable, 0, sizeof nUsedTable );

//lizheng   for ( nIdx = 0; nIdx < SRAM_SIZE; ++nIdx )
//lizheng   {
//lizheng     ++nUsedTable[ SRAM[ nIdx++ ] ];
//lizheng   }
//lizheng   for ( nIdx = 1, chTag = 0; nIdx < 256; ++nIdx )
//lizheng   {
//lizheng     if ( nUsedTable[ nIdx ] < nUsedTable[ chTag ] )
//lizheng       chTag = nIdx;
//lizheng   }
//lizheng 
//lizheng   nEncoded = 0;
//lizheng   nEncLen = 0;
//lizheng   nRunLen = 1;
//lizheng 
//lizheng   pDstBuf[ nEncLen++ ] = chTag;
//lizheng 
//lizheng   chPrevData = SRAM[ nEncoded++ ];
//lizheng 
//lizheng   while ( nEncoded < SRAM_SIZE && nEncLen < SRAM_SIZE - 133 )
//lizheng   {
//lizheng     chData = SRAM[ nEncoded++ ];
//lizheng 
//lizheng     if ( chPrevData == chData && nRunLen < 256 )
//lizheng       ++nRunLen;
//lizheng     else
//lizheng     {
//lizheng       if ( nRunLen >= 4 || chPrevData == chTag )
//lizheng       {
//lizheng         pDstBuf[ nEncLen++ ] = chTag;
//lizheng         pDstBuf[ nEncLen++ ] = chPrevData;
//lizheng         pDstBuf[ nEncLen++ ] = nRunLen - 1;
//lizheng       }
//lizheng       else
//lizheng       {
//lizheng         for ( nIdx = 0; nIdx < nRunLen; ++nIdx )
//lizheng           pDstBuf[ nEncLen++ ] = chPrevData;
//lizheng       }
//lizheng 
//lizheng       chPrevData = chData;
//lizheng       nRunLen = 1;
//lizheng     }
//lizheng 
//lizheng   }
//lizheng   if ( nRunLen >= 4 || chPrevData == chTag )
//lizheng   {
//lizheng     pDstBuf[ nEncLen++ ] = chTag;
//lizheng     pDstBuf[ nEncLen++ ] = chPrevData;
//lizheng     pDstBuf[ nEncLen++ ] = nRunLen - 1;
//lizheng   }
//lizheng   else
//lizheng   {
//lizheng     for ( nIdx = 0; nIdx < nRunLen; ++nIdx )
//lizheng       pDstBuf[ nEncLen++ ] = chPrevData;
//lizheng   }
//lizheng 
//lizheng   /*-------------------------------------------------------------------*/
//lizheng   /*  Write a SRAM data                                                */
//lizheng   /*-------------------------------------------------------------------*/
//lizheng 
//lizheng   // Open SRAM file
//lizheng   fp = fopen( szSaveName, "wb" );
//lizheng   if ( fp == NULL )
//lizheng     return -1;
//lizheng 
//lizheng   // Write SRAM data
//lizheng   fwrite( pDstBuf, nEncLen, 1, fp );
//lizheng 
//lizheng   // Close SRAM file
//lizheng   fclose( fp );
//lizheng 
//lizheng   // Successful
//lizheng   return 0;
//lizheng }

/*===================================================================*/
/*                                                                   */
/*                  InfoNES_Menu() : Menu screen                     */
/*                                                                   */
/*===================================================================*/
int InfoNES_Menu()
{
/*
 *  Menu screen
 *
 *  Return values
 *     0 : Normally
 *    -1 : Exit InfoNES
 */

  /* If terminated */
  if ( bThread == FALSE )
  {
    return -1;
  }

  /* Nothing to do here */
  return 0;
}

/*===================================================================*/
/*                                                                   */
/*               InfoNES_ReadRom() : Read ROM image file             */
/*                                                                   */
/*===================================================================*/
int InfoNES_ReadRom( const char *pszFileName )
{
/*
 *  Read ROM image file
 *
 *  Parameters
 *    const char *pszFileName          (Read)
 *
 *  Return values
 *     0 : Normally
 *    -1 : Error
 */

//«∂»Î”Œœ∑Œƒº˛   FILE *fp;
  const unsigned char *fp;

  /* Open ROM file */
//«∂»Î”Œœ∑Œƒº˛   fp = fopen( pszFileName, "rb" );
  fp = &gamefile[0];
  if ( fp == NULL )
    return -1;

  /* Read ROM Header */
//ÃÊªªGccBugFunc   fread( &NesHeader, sizeof NesHeader, 1, fp );
  memcpy( &NesHeader, fp, 16 );
  fp += 16;
  if ( memcmp( NesHeader.byID, "NES\x1a", 4 ) != 0 )
  {
    /* not .nes file */
//lizheng    fclose( fp );
    return -1;
  }

  /* Clear SRAM */
  memset( SRAM, 0, SRAM_SIZE );

  /* If trainer presents Read Triner at 0x7000-0x71ff */
  if ( NesHeader.byInfo1 & 4 )
  {
//ÃÊªªGccBugFunc     fread( &SRAM[ 0x1000 ], 512, 1, fp );
    memcpy( &SRAM[ 0x1000 ], fp, 512 );
    fp += 512;
  }

  /* Allocate Memory for ROM Image */
  ROM = (BYTE *)malloc( NesHeader.byRomSize * 0x4000 );

  /* Read ROM Image */
//ÃÊªªGccBugFunc   fread( ROM, 0x4000, NesHeader.byRomSize, fp );
  memcpy(ROM, fp, 0x4000 * NesHeader.byRomSize);
  fp += 0x4000 * NesHeader.byRomSize;

  if ( NesHeader.byVRomSize > 0 )
  {
    /* Allocate Memory for VROM Image */
     VROM = (BYTE *)malloc( NesHeader.byVRomSize * 0x2000 );

    /* Read VROM Image */
//ÃÊªªGccBugFunc     fread( VROM, 0x2000, NesHeader.byVRomSize, fp );
  memcpy(VROM, fp, 0x2000 * NesHeader.byVRomSize);
  }

  /* File close */
//lizheng  fclose( fp );

  /* Successful */
  return 0;
}

/*===================================================================*/
/*                                                                   */
/*           InfoNES_ReleaseRom() : Release a memory for ROM         */
/*                                                                   */
/*===================================================================*/
void InfoNES_ReleaseRom()
{
/*
 *  Release a memory for ROM
 *
 */

  if ( ROM )
  {
    free( ROM );
    ROM = NULL;
  }

  if ( VROM )
  {
    free( VROM );
    VROM = NULL;
  }
}

/*===================================================================*/
/*                                                                   */
/*             InfoNES_MemoryCopy() : memcpy                         */
/*                                                                   */
/*===================================================================*/
void *InfoNES_MemoryCopy( void *dest, const void *src, int count )
{
/*
 *  memcpy
 *
 *  Parameters
 *    void *dest                       (Write)
 *      Points to the starting address of the copied block's destination
 *
 *    const void *src                  (Read)
 *      Points to the starting address of the block of memory to copy
 *
 *    int count                        (Read)
 *      Specifies the size, in bytes, of the block of memory to copy
 *
 *  Return values
 *    Pointer of destination
 */

  memcpy( dest, src, count );
  return dest;
}

/*===================================================================*/
/*                                                                   */
/*             InfoNES_MemorySet() : memset                          */
/*                                                                   */
/*===================================================================*/
void *InfoNES_MemorySet( void *dest, int c, int count )
{
/*
 *  memset
 *
 *  Parameters
 *    void *dest                       (Write)
 *      Points to the starting address of the block of memory to fill
 *
 *    int c                            (Read)
 *      Specifies the byte value with which to fill the memory block
 *
 *    int count                        (Read)
 *      Specifies the size, in bytes, of the block of memory to fill
 *
 *  Return values
 *    Pointer of destination
 */

  memset( dest, c, count);  
  return dest;
}

/*===================================================================*/
/*                                                                   */
/*      InfoNES_LoadFrame() :                                        */
/*           Transfer the contents of work frame on the screen       */
/*                                                                   */
/*===================================================================*/
void InfoNES_LoadFrame()
{
/*
 *  Transfer the contents of work frame on the screen
 *
 */
//lizheng   GdkGC       *gc;
//lizheng   GdkDrawable *drawable;
//lizheng   guchar  pbyRgbBuf[ NES_DISP_WIDTH * NES_DISP_HEIGHT * 3 ];
// ”∆µ   register guchar* pBuf;

  /* Enter Critical Section */
//lizheng   gdk_threads_enter();

  /* Get a drawable and graphic context */
//lizheng  drawable=draw_area->window;
//lizheng   gc=gdk_gc_new(drawable);

// ”∆µ   pBuf = pbyRgbBuf;

  /* Exchange 16-bit to 24-bit  */
// ”∆µ   for ( register int y = 0; y < NES_DISP_HEIGHT; y++ )
// ”∆µ   {
// ”∆µ     for ( register int x = 0; x < NES_DISP_WIDTH; x++ )
// ”∆µ     {  
// ”∆µ       WORD wColor = WorkFrame[ ( y << 8 ) + x ];
	  
// ”∆µ       *(pBuf++) = (guchar)( ( wColor & 0x7c00 ) >> 7 );
// ”∆µ       *(pBuf++) = (guchar)( ( wColor & 0x03e0 ) >> 2 );
// ”∆µ       *(pBuf++) = (guchar)( ( wColor & 0x001f ) << 3 );
// ”∆µ     }
// ”∆µ   }

  /* Blit screen */
//lizheng   gdk_draw_rgb_image( drawable, gc, 0, 0, NES_DISP_WIDTH, NES_DISP_HEIGHT,               
//lizheng 		      GDK_RGB_DITHER_NONE, pbyRgbBuf, NES_DISP_WIDTH * 3 );

//lizheng   gdk_gc_destroy(gc);

//cFStart = clock();
//cOTime = cFStart - cFEnd;

//  for ( register int y = 0; y < NES_DISP_HEIGHT; y++ ) 
//    for ( register int x = 0; x < NES_DISP_WIDTH; x++ )
//      PutPixel(x, y, (U8)((WorkFrame[ ( y << 8 ) + x ] & 0x7c00 ) >> 7));
//  for ( register int y = 110; y < 120; y++ ) 
//    for ( register int x = 0; x < NES_DISP_WIDTH; x++ )
//      PutPixel(x, y, (U8)((WorkFrame[ ( y << 8 ) + x ] & 0x7c00 ) >> 7));


//LEON	//”√printf()ƒ£ƒ‚¥Ú”°≥ˆ”Œœ∑ª≠√Êµƒ“ª≤ø∑÷
//if( FrameCount++ > 32)
//  for ( register int y = 130; y < 210; y++ ) 
//    for ( register int x = 0; x < 190; x++ )
//      if( x != 189 )
//      	printf( "%c", (BYTE)WorkFrame[ ( y << 8 ) + x ] );
//      else
//      	printf( "\n" );

#ifdef LEON
#ifdef PrintfFrameGraph
if( FrameCount++ > 32)
  for ( register int y = 130; y < 210; y++ ) 
    for ( register int x = 0; x < 190; x++ )
      if( x != 189 )
      	printf( "%c", WorkFrame[ y * NES_BACKBUF_WIDTH + 8 + x ] );
      else
      	printf( "\n" );
#endif /* PrintfFrameGraph */
#else
if( FrameCount++ > 32)
  for ( register int y = 130; y < 210; y++ ) 
    for ( register int x = 0; x < 190; x++ )
      if( x != 189 )
      	printf( "%c", (BYTE)WorkFrame[ y * NES_BACKBUF_WIDTH + 8 + x ] );
      else
      	printf( "\n" );
#endif /* LEON */


//	printf( "Outputing Frame %d\n", FrameCount++ );
//	printf( "Pixel 46080's RGB is %x\n", WorkFrame[ 46080 ] );

//cFEnd = clock();
//cFTime = cFEnd - cFStart;
      
//  Glib_FilledRectangle(0, NES_DISP_HEIGHT, 239, 319, 0xff);

  /* Leave Critical Section */
//lizheng   gdk_threads_leave();
}

//CLK:GPG4,OUT:GPG3,DQ0:GPG2,DQ1:GPG5;
/*int Get_GameKey(void)
{
	int i;
	unsigned BIT[8],GameKey=0x00;
	rPDATG|=(1<<3);
	rPDATG&=(~(1<<3));
	for(i=0;i<=7;i++)
	{
		rPDATG&=(~(1<<4));
		BIT[i]=((rPDATG&(1<<2))<<1);
		rPDATG|=(1<<4);
	}
	for(i=0;i<=7;i++)
	{
		GameKey|=(BIT[i]>>i);
	}
	return GameKey;
	Uart_Printf("you pressed %d",GameKey);
}*/

/*===================================================================*/
/*                                                                   */
/*             InfoNES_PadState() : Get a joypad state               */
/*                                                                   */
/*===================================================================*/
void InfoNES_PadState( DWORD *pdwPad1, DWORD *pdwPad2, DWORD *pdwSystem )
{
/*
 *  Get a joypad state
 *
 *  Parameters
 *    DWORD *pdwPad1                   (Write)
 *      Joypad 1 State
 *
 *    DWORD *pdwPad2                   (Write)
 *      Joypad 2 State
 *
 *    DWORD *pdwSystem                 (Write)
 *      Input for InfoNES
 *
 */

  /* Transfer joypad state */

//		dwKeyPad1=Get_GameKey();
		
//  *pdwPad1   = ~dwKeyPad1;
  *pdwPad1   = dwKeyPad1;
  *pdwPad2   = dwKeyPad2;
  *pdwSystem = dwKeySystem;
}

/*===================================================================*/
/*                                                                   */
/*        InfoNES_SoundInit() : Sound Emulation Initialize           */
/*                                                                   */
/*===================================================================*/
void InfoNES_SoundInit( void ) 
{
//“Ù∆µ   sound_fd = 0;
}

/*===================================================================*/
/*                                                                   */
/*        InfoNES_SoundOpen() : Sound Open                           */
/*                                                                   */
/*===================================================================*/
int InfoNES_SoundOpen( int samples_per_sync, int sample_rate ) 
{
//“Ù∆µ   int tmp;
//“Ù∆µ   int result;
//“Ù∆µ   int sound_rate;
//“Ù∆µ   int sound_frag;
//“Ù∆µ 
//“Ù∆µ   waveptr = 0;
//“Ù∆µ   wavflag = 0;
//“Ù∆µ 
//“Ù∆µ   /* Open sound device */
//“Ù∆µ   sound_fd = open( SOUND_DEVICE, O_WRONLY );
//“Ù∆µ   if ( sound_fd < 0 ) 
//“Ù∆µ   {
//“Ù∆µ     InfoNES_MessageBox("opening "SOUND_DEVICE"...failed");
//“Ù∆µ     sound_fd = 0;
//“Ù∆µ     return 0;
//“Ù∆µ   } else {
//“Ù∆µ 
//“Ù∆µ   }
//“Ù∆µ   
//“Ù∆µ   /* Setting unsigned 8-bit format */ 
//“Ù∆µ   tmp = AFMT_U8;
//“Ù∆µ   result = ioctl(sound_fd, SNDCTL_DSP_SETFMT, &tmp);
//“Ù∆µ   if ( result < 0 ) 
//“Ù∆µ   {
//“Ù∆µ     InfoNES_MessageBox("setting unsigned 8-bit format...failed");
//“Ù∆µ     close(sound_fd);
//“Ù∆µ     sound_fd = 0;
//“Ù∆µ     return 0;
//“Ù∆µ   } else {
//“Ù∆µ 
//“Ù∆µ   }
//“Ù∆µ     
//“Ù∆µ   /* Setting mono mode */
//“Ù∆µ   tmp = 0;
//“Ù∆µ   result = ioctl(sound_fd, SNDCTL_DSP_STEREO, &tmp);
//“Ù∆µ   if (result < 0) 
//“Ù∆µ   {
//“Ù∆µ     InfoNES_MessageBox("setting mono mode...failed");
//“Ù∆µ     close(sound_fd);
//“Ù∆µ     sound_fd = 0;
//“Ù∆µ     return 0;
//“Ù∆µ   } else {
//“Ù∆µ 
//“Ù∆µ   }
//“Ù∆µ     
//“Ù∆µ   sound_rate = sample_rate;
//“Ù∆µ   result = ioctl(sound_fd, SNDCTL_DSP_SPEED, &sound_rate);
//“Ù∆µ   if ( result < 0 ) 
//“Ù∆µ   {
//“Ù∆µ     InfoNES_MessageBox("setting sound rate...failed");
//“Ù∆µ     close(sound_fd);
//“Ù∆µ     sound_fd = 0;
//“Ù∆µ     return 0;
//“Ù∆µ   } else {
//“Ù∆µ 
//“Ù∆µ   }
//“Ù∆µ 
//“Ù∆µ   /* high word of sound_frag is number of frags, low word is frag size */
//“Ù∆µ   sound_frag = 0x00080008;
//“Ù∆µ   result = ioctl(sound_fd, SNDCTL_DSP_SETFRAGMENT, &sound_frag);
//“Ù∆µ   if (result < 0) 
//“Ù∆µ   {
//“Ù∆µ     InfoNES_MessageBox("setting soundfrags...failed");
//“Ù∆µ     close(sound_fd);
//“Ù∆µ     sound_fd = 0;
//“Ù∆µ     return 0;
//“Ù∆µ   } else {
//“Ù∆µ 
//“Ù∆µ   }
//“Ù∆µ 
  /* Successful */
  return 1;
}

/*===================================================================*/
/*                                                                   */
/*        InfoNES_SoundClose() : Sound Close                         */
/*                                                                   */
/*===================================================================*/
void InfoNES_SoundClose( void ) 
{
/*  if ( sound_fd ) 
  {
    close(sound_fd);
  }*///“Ù∆µ
}

/*===================================================================*/
/*                                                                   */
/*            InfoNES_SoundOutput() : Sound Output 5 Waves           */           
/*                                                                   */
/*===================================================================*/
void InfoNES_SoundOutput( int samples, BYTE *wave1, BYTE *wave2, BYTE *wave3, BYTE *wave4, BYTE *wave5 )
{
/*  int i;

  if ( sound_fd ) 
  {
    for (i = 0; i < samples; i++) 
    {
#if 1
      final_wave[ waveptr ] = 
	( wave1[i] + wave2[i] + wave3[i] + wave4[i] + wave5[i] ) / 5;
#else
      final_wave[ waveptr ] = wave4[i];
#endif


      waveptr++;
      if ( waveptr == 2048 ) 
      {
	waveptr = 0;
	wavflag = 2;
      } 
      else if ( waveptr == 1024 )
      {
	wavflag = 1;
      }
    }
	
    if ( wavflag )
    {
      if ( write( sound_fd, &final_wave[(wavflag - 1) << 10], 1024) < 1024 ) 
      {
	InfoNES_MessageBox( "wrote less than 1024 bytes\n" );
      }
      wavflag = 0;
    }
  }*///“Ù∆µ
}

/*===================================================================*/
/*                                                                   */
/*            InfoNES_Wait() : Wait Emulation if required            */
/*                                                                   */
/*===================================================================*/
void InfoNES_Wait() {}

/*===================================================================*/
/*                                                                   */
/*            InfoNES_MessageBox() : Print System Message            */
/*                                                                   */
/*===================================================================*/
void InfoNES_MessageBox( char *pszMsg, ... )
{
/*  char pszErr[ 1024 ];
  va_list args;

  static GtkWidget *label;
  GtkWidget *button;
  GtkWidget *dialog_window;

  // Create the message body
  va_start( args, pszMsg );
  vsprintf( pszErr, pszMsg, args );  pszErr[ 1023 ] = '\0';
  va_end( args );

#if 0
  // Only for debug
  g_print( pszErr );
#else
  // Create a dialog window
  dialog_window = gtk_dialog_new();
  gtk_signal_connect( GTK_OBJECT( dialog_window ), "destroy", 
		      GTK_SIGNAL_FUNC( closing_dialog ), &dialog_window );
  gtk_window_set_title( GTK_WINDOW( dialog_window ), VERSION );
  gtk_container_border_width( GTK_CONTAINER( dialog_window ), 5 );

  // Create a label
  label = gtk_label_new( pszErr );
  gtk_misc_set_padding( GTK_MISC( label ), 10, 10 );
  gtk_box_pack_start( GTK_BOX( GTK_DIALOG( dialog_window )->vbox ), label, TRUE, TRUE, 0 );
  gtk_widget_show( label );

  // Create a OK button
  button = gtk_button_new_with_label( "OK" );
  gtk_signal_connect( GTK_OBJECT( button ), "clicked", GTK_SIGNAL_FUNC( close_dialog ), dialog_window );
  GTK_WIDGET_SET_FLAGS( button, GTK_CAN_DEFAULT);
  gtk_box_pack_start( GTK_BOX( GTK_DIALOG( dialog_window )->action_area ), button, TRUE, TRUE, 0 );

  gtk_widget_grab_default( button );
  gtk_widget_show( button );
  gtk_widget_show( dialog_window );
  gtk_grab_add( dialog_window );
#endif
}

void close_dialog( GtkWidget *widget, gpointer data )
{
  gtk_widget_destroy( GTK_WIDGET( data ) );
}

void closing_dialog( GtkWidget *widget, gpointer data )
{
  gtk_grab_remove( GTK_WIDGET( widget ) );*///lizheng
}

/*
 * End of InfoNES_System_Linux.cpp
 */
