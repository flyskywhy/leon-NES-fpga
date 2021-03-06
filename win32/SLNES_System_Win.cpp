/*=================================================================*/
/*                                                                 */
/*  SLNES_System_Win.cpp : Windows specific File                   */
/*                                                                 */
/*  2004/07/28  SLNES Project                                      */
/*                                                                 */
/*=================================================================*/

/*-----------------------------------------------------------------*/
/*  Include files                                                  */
/*-----------------------------------------------------------------*/
#include <windows.h>
#include <mmsystem.h>
#include <limits.h>
#include <stdio.h>
#include <crtdbg.h>
#include <stdarg.h>

#include <time.h>

#include "../SLNES.h"
#include "../SLNES_System.h"
#include "../SLNES_Data.h"

#include "SLNES_Resource_Win.h"
#include "SLNES_Sound_Win.h"

/*-----------------------------------------------------------------*/
/*  ROM image file information                                       */
/*-----------------------------------------------------------------*/

char szRomName[256];
char szSaveName[256];
int nSRAM_SaveFlag;

/*-----------------------------------------------------------------*/
/*  Variables for Windows                                            */
/*-----------------------------------------------------------------*/
#define APP_NAME     "SLNES v1.00"
 
HWND hWndMain;
WNDCLASS wc;
HACCEL hAccel;

byte *pScreenMem;
HBITMAP hScreenBmp;
LOGPALETTE *plpal;
BITMAPINFO *bmi;

// Palette data
WORD NesPalette[64] =
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

// Screen Size Magnification
WORD wScreenMagnification = 1;
#define NES_MENU_HEIGHT     54
#define NES_MENU_WIDTH			8

/*-----------------------------------------------------------------*/
/*  Variables for Emulation Thread                                   */
/*-----------------------------------------------------------------*/
HANDLE m_hThread;
DWORD m_ThreadID = NULL;

/*-----------------------------------------------------------------*/
/*  Variables for Timer & Wait loop                                  */
/*-----------------------------------------------------------------*/
#define LINE_PER_TIMER      789
#define TIMER_PER_LINE      50

WORD wLines;
WORD wLinePerTimer;
MMRESULT uTimerID;
BOOL bWaitFlag;
CRITICAL_SECTION WaitFlagCriticalSection;
BOOL bAutoFrameskip = TRUE;

/*-----------------------------------------------------------------*/
/*  Variables for Sound Emulation                                    */
/*-----------------------------------------------------------------*/
DIRSOUND* lpSndDevice = NULL;

/*-----------------------------------------------------------------*/
/*  Variables for Expiration                                         */
/*-----------------------------------------------------------------*/
#define EXPIRED_YEAR    2001
#define EXPIRED_MONTH   3
#define EXPIRED_MSG     "This software has been expired.\nPlease download newer one."     

/*-----------------------------------------------------------------*/
/*  Function prototypes (Windows specific)                         */
/*-----------------------------------------------------------------*/

LRESULT CALLBACK MainWndproc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
//#if 0
//LRESULT CALLBACK AboutDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
//#endif
void ShowTitle(HWND hWnd);
void SetWindowSize(WORD wMag);
int LoadSRAM();
int SaveSRAM();
int CreateScreen(HWND hWnd);
void DestroyScreen();
static void SLNES_StartTimer(); 
static void SLNES_StopTimer();
static void CALLBACK TimerFunc(UINT nID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2);

/*=================================================================*/
/*                                                                   */
/*                WinMain() : Application main                       */
/*                                                                   */
/*=================================================================*/
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{	
  /*-----------------------------------------------------------------*/
  /*  Create a window                                                  */
  /*-----------------------------------------------------------------*/

  wc.style = 0;
  wc.lpfnWndProc = MainWndproc;
  wc.cbClsExtra = 0;
  wc.cbWndExtra = 0;
  wc.hInstance = hInstance;
  wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON));
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
  wc.lpszMenuName = MAKEINTRESOURCE(IDR_MENU);
  wc.lpszClassName = "SLNESClass";
  if (!RegisterClass(&wc))
    return FALSE;

  hWndMain = CreateWindowEx(0,
                             "SLNESClass",
                             APP_NAME,
                             WS_VISIBLE | WS_POPUP | WS_OVERLAPPEDWINDOW,
                             200,
                             120,
                             NES_DISP_WIDTH * wScreenMagnification + NES_MENU_WIDTH,
                             NES_DISP_HEIGHT * wScreenMagnification + NES_MENU_HEIGHT,
                             NULL,
                             NULL,
                             hInstance,
                             NULL);

  if (!hWndMain)
    return FALSE;

  ShowWindow(hWndMain, nCmdShow);
  UpdateWindow(hWndMain);

//#if 0
//  /*-----------------------------------------------------------------*/
//  /*  Expired or Not?                                                  */
//  /*-----------------------------------------------------------------*/
//  SYSTEMTIME st;
//  GetLocalTime(&st);
//
//  if (st.wYear > EXPIRED_YEAR || st.wMonth > EXPIRED_MONTH)
//  {
//    SLNES_MessageBox(EXPIRED_MSG);
//    exit(-1);
//  }
//#endif

  /*-----------------------------------------------------------------*/
  /*  Init Resources                                                   */
  /*-----------------------------------------------------------------*/
  SLNES_StartTimer();
  CreateScreen(hWndMain);
  ShowTitle(hWndMain);

  /*-----------------------------------------------------------------*/
  /*  For Drag and Drop Function                                       */
  /*-----------------------------------------------------------------*/
  if (lpCmdLine[0] != '\0')
  {
    // If included space characters, strip dobule quote marks
    if (lpCmdLine[0] == '"')
    {
      lpCmdLine[strlen(lpCmdLine) - 1] = '\0';
      lpCmdLine++;
    }

    // Load cassette
    if (SLNES_Load(lpCmdLine) == 0)
    {
      // Set a ROM image name
      strcpy(szRomName, lpCmdLine);

      // Load SRAM
      LoadSRAM();

	    // Create Emulation Thread
		  m_hThread = CreateThread((LPSECURITY_ATTRIBUTES)NULL, (DWORD)0,
			  (LPTHREAD_START_ROUTINE)SLNES_Main, (LPVOID)NULL, (DWORD)0, &m_ThreadID);
    }
  }

  /*-----------------------------------------------------------------*/
  /*  The Message Pump                                                 */
  /*-----------------------------------------------------------------*/
	MSG	msg;

  while (TRUE)
  {
    if (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
    {
      if (!GetMessage(&msg, NULL, 0, 0))
				break;
	  
			// Translate and dispatch the message
			if (0 == TranslateAccelerator(hWndMain, hAccel, &msg))
			{
				TranslateMessage(&msg); 
				DispatchMessage(&msg);
			}  
    } 
		else
		{
      // Make sure we go to sleep if we have nothing else to do
      WaitMessage();
		}
  }

  /*-----------------------------------------------------------------*/
  /*  Release Resources                                                */
  /*-----------------------------------------------------------------*/
  if (NULL != m_hThread) {
    // Terminate Emulation Thread
	  TerminateThread(m_hThread, (DWORD)0);  m_hThread=NULL;
		SaveSRAM();
    SLNES_Fin();
  }
  DestroyScreen();
  SLNES_StopTimer();
  return 0;
}

/*=================================================================*/
/*                                                                   */
/*                MainWndProc() : Window procedure                   */
/*                                                                   */
/*=================================================================*/
LRESULT CALLBACK MainWndproc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  OPENFILENAME ofn;
  char szFileName[256];

  switch (message)
  {
    case WM_ERASEBKGND:
      return 1;

    case WM_DESTROY:
      PostQuitMessage(0);
      break;
    
    case WM_ACTIVATE:
      // Show title screen if emulation thread dosent exist
      if (NULL == m_hThread)
        ShowTitle(hWnd);
      break;

    case WM_COMMAND:
      switch (LOWORD(wParam))
      {
        case IDC_BTN_OPEN:
          /*-----------------------------------------------------------------*/
          /*  Open button                                                      */
          /*-----------------------------------------------------------------*/
					
					// Do nothing if emulation thread exists
					if (NULL != m_hThread)
						break;

          memset(&ofn, 0, sizeof ofn);
          szFileName[0] = '\0';
          ofn.lStructSize = sizeof ofn;
          ofn.hwndOwner = hWnd;
          ofn.hInstance = wc.hInstance;

          ofn.lpstrFilter = NULL; 
          ofn.lpstrCustomFilter = NULL; 
          ofn.nMaxCustFilter = 0; 
          ofn.nFilterIndex = 0; 
          ofn.lpstrFile = szFileName; 
          ofn.nMaxFile = sizeof szFileName; 
          ofn.lpstrFileTitle = NULL; 
          ofn.nMaxFileTitle = 0; 
          ofn.lpstrInitialDir = NULL;
          ofn.lpstrTitle = NULL; 
          ofn.Flags = 0; 
          ofn.nFileOffset; 
          ofn.nFileExtension = 0; 
          ofn.lpstrDefExt = NULL; 
          ofn.lCustData = 0; 
          ofn.lpfnHook = NULL; 
          ofn.lpTemplateName = NULL; 

          if (GetOpenFileName(&ofn))
          {
            // Load cassette
            if (SLNES_Load(szFileName) == 0)
            {
              // Set a ROM image name
              strcpy(szRomName, szFileName);

              // Load SRAM
              LoadSRAM();

							// Create Emulation Thread
							m_hThread=CreateThread((LPSECURITY_ATTRIBUTES)NULL, (DWORD)0,
								(LPTHREAD_START_ROUTINE)SLNES_Main, (LPVOID)NULL, (DWORD)0, &m_ThreadID);
            }
          }
          break;

        case IDC_BTN_STOP:
          /*-----------------------------------------------------------------*/
          /*  Stop button                                                      */
          /*-----------------------------------------------------------------*/

					if (NULL != m_hThread)
					{
					  // Terminate Emulation Thread
						TerminateThread(m_hThread, (DWORD)0);  m_hThread=NULL;
					  SaveSRAM();
            SLNES_Fin();
            SLNES_StopTimer();
            DestroyScreen();

            // Preperations
            CreateScreen(hWndMain);
            SLNES_StartTimer();
          } 
          // Show Title Screen
          ShowTitle(hWnd);
          break;

        case IDC_BTN_RESET:
          /*-----------------------------------------------------------------*/
          /*  Reset button                                                     */
          /*-----------------------------------------------------------------*/
 
					// Do nothing if emulation thread does not exists
					if (NULL != m_hThread)
					{
            // Terminate Emulation Thread
						TerminateThread(m_hThread, (DWORD)0);  m_hThread=NULL;
						SaveSRAM();
            SLNES_Fin();
            SLNES_StopTimer();
            DestroyScreen();

            // Create Emulation Thread
            CreateScreen(hWndMain);
            SLNES_StartTimer();
            SLNES_Load(szRomName);
            LoadSRAM();
						m_hThread=CreateThread((LPSECURITY_ATTRIBUTES)NULL, (DWORD)0,
							(LPTHREAD_START_ROUTINE)SLNES_Main, (LPVOID)NULL, (DWORD)0, &m_ThreadID);
          } else {
            // Show Title Screen
            ShowTitle(hWnd);
          }
          break;
        
				case IDC_BTN_EXIT:
          /*-----------------------------------------------------------------*/
          /*  Exit button                                                      */
          /*-----------------------------------------------------------------*/
					if (NULL != m_hThread)
          {
            // Terminate Emulation Thread
						TerminateThread(m_hThread, (DWORD)0);  m_hThread=NULL;
					  SaveSRAM();
            SLNES_Fin();
            SLNES_StopTimer();   
            DestroyScreen();
          }
					// Received key/menu command to exit app
          PostMessage(hWnd, WM_CLOSE, 0, 0);
					break;

				case IDC_BTN_SINGLE:
          /*-----------------------------------------------------------------*/
          /*  Screen Size x1, x2, x3 button                                    */
          /*-----------------------------------------------------------------*/  
          SetWindowSize(1);

          // Check x1 button
          CheckMenuItem(GetMenu(hWndMain), IDC_BTN_SINGLE, MF_CHECKED);
          CheckMenuItem(GetMenu(hWndMain), IDC_BTN_DOUBLE, MF_UNCHECKED);
          CheckMenuItem(GetMenu(hWndMain), IDC_BTN_TRIPLE, MF_UNCHECKED);

          break;

        case IDC_BTN_DOUBLE:
          SetWindowSize(2);

          // Check x2 button
          CheckMenuItem(GetMenu(hWndMain), IDC_BTN_SINGLE, MF_UNCHECKED);
          CheckMenuItem(GetMenu(hWndMain), IDC_BTN_DOUBLE, MF_CHECKED);
          CheckMenuItem(GetMenu(hWndMain), IDC_BTN_TRIPLE, MF_UNCHECKED);

          break;

        case IDC_BTN_TRIPLE:
          SetWindowSize(3);

          // Check x3 button
          CheckMenuItem(GetMenu(hWndMain), IDC_BTN_SINGLE, MF_UNCHECKED);
          CheckMenuItem(GetMenu(hWndMain), IDC_BTN_DOUBLE, MF_UNCHECKED);
          CheckMenuItem(GetMenu(hWndMain), IDC_BTN_TRIPLE, MF_CHECKED);

          break;

        case IDC_BTN_INFO:
					/*-----------------------------------------------------------------*/
          /*  ROM information                                                  */
          /*-----------------------------------------------------------------*/
          if (NULL != m_hThread) 
          {
            char pszInfo[1024];
            sprintf(pszInfo, "Mapper\t\t%d\nPRG ROM\t\t%dKB\nCHR ROM\t\t%dKB\n" \
                              "Mirroring\t\t%s\nSRAM\t\t%s",
                              MapperNo, RomSize * 16, VRomSize * 8,
                              (ROM_Mirroring ? "V" : "H"), (ROM_SRAM ? "Yes" : "No"));
            MessageBox(hWndMain, pszInfo, APP_NAME, MB_OK | MB_ICONINFORMATION);              
          } else {
            // Show Title Screen
            ShowTitle(hWnd);
          }
          break;

		case IDC_BTN_ABOUT:
			/*-----------------------------------------------------------------*/
			/*  About button                                                     */
			/*-----------------------------------------------------------------*/
			{
				/* Version Infomation */
				char pszInfo[1024];
				sprintf(pszInfo, "%s\nA fast and portable NES emulator\n"
					"Copyright (C) 2004-2005 Silan Co.,Ltd <www.silan.com.cn>",
					APP_NAME);
				MessageBox(hWndMain, pszInfo, APP_NAME, MB_OK | MB_ICONINFORMATION);   
			}
          break;
      }
			break;

    default:
      return DefWindowProc(hWnd, message, wParam, lParam);
  }
  return 0;
}

/*=================================================================*/
/*                                                                   */
/*      ShowTitle() : Show Title Screen Procedure                    */
/*                                                                   */
/*=================================================================*/
void ShowTitle(HWND hWnd)
{
  HDC hDC = GetDC(hWnd);
  HDC hMemDC = CreateCompatibleDC(hDC);
  HBITMAP hTitleBmp = LoadBitmap(wc.hInstance, MAKEINTRESOURCE(IDB_BITMAP));

  // Blt Title Bitmap
  SelectObject(hMemDC, hTitleBmp);

  StretchBlt(hDC, 0, 0, NES_DISP_WIDTH * wScreenMagnification, 
              NES_DISP_HEIGHT * wScreenMagnification, hMemDC, 
              0, 0, NES_DISP_WIDTH, NES_DISP_HEIGHT, SRCCOPY);

  SelectObject(hMemDC, hTitleBmp);

  DeleteDC(hMemDC);
  ReleaseDC(hWnd, hDC);
}

/*=================================================================*/
/*                                                                   */
/*            CreateScreen() : Create SLNES screen                 */
/*                                                                   */
/*=================================================================*/
int CreateScreen(HWND hWnd)
{
  /*-----------------------------------------------------------------*/
  /*  Create a SLNES screen                                          */
  /*-----------------------------------------------------------------*/
  HDC hDC = GetDC(hWnd);

  BITMAPINFOHEADER bi;

  bi.biSize = sizeof(BITMAPINFOHEADER);
  bi.biWidth = NES_DISP_WIDTH;
  bi.biHeight = NES_DISP_HEIGHT * -1;
  bi.biPlanes = 1;

  bi.biBitCount = 16;

  bi.biCompression = BI_RGB;
  bi.biSizeImage = NES_DISP_WIDTH * NES_DISP_HEIGHT * 2;
  bi.biXPelsPerMeter = 0;
  bi.biYPelsPerMeter = 0;
  bi.biClrUsed = 0;
  bi.biClrImportant = 0;

  hScreenBmp = CreateDIBSection(hDC, 
                                 (BITMAPINFO *)&bi,
                                 DIB_RGB_COLORS, 
                                 (void **)&pScreenMem, 
                                 0,
                                 0); 
  ReleaseDC(hWnd, hDC);

  if (!hScreenBmp) { return -1; } 
  else {  return 0; }
}

/*=================================================================*/
/*                                                                   */
/*          DestroyScreen() : Destroy SLNES screen                 */
/*                                                                   */
/*=================================================================*/
void DestroyScreen()
{
  if (!hScreenBmp) { DeleteObject(hScreenBmp); }
}

/*=================================================================*/
/*                                                                   */
/*              SLNES_Main() : The main loop of SLNES            */
/*                                                                   */
/*=================================================================*/
int SLNES_Main()
{
	/*
	*  The main loop of SLNES
	*
	*/
	unsigned int frame = 1;
	long cur_time, last_frame_time;
	long BaseTime = clock();

	// Main loop
	for (;;)
	{
		/*-----------------------------------------------------------------*/
		/*  To the menu screen                                               */
		/*-----------------------------------------------------------------*/
		if (SLNES_Menu() == -1)
			break;  // Quit

		/*-----------------------------------------------------------------*/
		/*  Start a NES emulation                                            */
		/*-----------------------------------------------------------------*/

		SLNES(PPU0);

		last_frame_time = BaseTime + (frame++) * (FRAME_SKIP + 1) * SAMPLE_PER_FRAME * 1000 / SAMPLE_PER_SEC;
		for (;;)
		{
			cur_time = clock();
			if(last_frame_time <= cur_time)
				break;
		}
		if (frame == 1000)
		{
			frame = 1;
			BaseTime = clock();
		}
	}

	// Completion treatment
	SLNES_Fin();

	return 0;
}

/*=================================================================*/
/*                                                                   */
/*           LoadSRAM() : Load a SRAM                                */
/*                                                                   */
/*=================================================================*/
int LoadSRAM()
{
/*
 *  Load a SRAM
 *
 *  Return values
 *     0 : Normally
 *    -1 : SRAM data couldn't be read
 */

  FILE *fp;
  unsigned char pSrcBuf[SRAM_SIZE];
  unsigned char chData;
  unsigned char chTag;
  int nRunLen;
  int nDecoded;
  int nDecLen;
  int nIdx;

  // It doesn't need to save it
  nSRAM_SaveFlag = 0;

  // It is finished if the ROM doesn't have SRAM
  if (!ROM_SRAM)
    return 0;

  // There is necessity to save it
  nSRAM_SaveFlag = 1;

  // The preparation of the SRAM file name
  strcpy(szSaveName, szRomName);
  strcpy(strrchr(szSaveName, '.') + 1, "srm");

  /*-----------------------------------------------------------------*/
  /*  Read a SRAM data                                                 */
  /*-----------------------------------------------------------------*/

  // Open SRAM file
  fp = fopen(szSaveName, "rb");
  if (fp == NULL)
    return -1;

  // Read SRAM data
  fread(pSrcBuf, SRAM_SIZE, 1, fp);

  // Close SRAM file
  fclose(fp);

  /*-----------------------------------------------------------------*/
  /*  Extract a SRAM data                                              */
  /*-----------------------------------------------------------------*/

  nDecoded = 0;
  nDecLen = 0;

  chTag = pSrcBuf[nDecoded++];

  while (nDecLen < 8192)
  {
    chData = pSrcBuf[nDecoded++];

    if (chData == chTag)
    {
      chData = pSrcBuf[nDecoded++];
      nRunLen = pSrcBuf[nDecoded++];
      for (nIdx = 0; nIdx < nRunLen + 1; ++nIdx)
      {
        SRAM[nDecLen++] = chData;
      }
    }
    else
    {
      SRAM[nDecLen++] = chData;
    }
  }

  // Successful
  return 0;
}

/*=================================================================*/
/*                                                                   */
/*           SaveSRAM() : Save a SRAM                                */
/*                                                                   */
/*=================================================================*/
int SaveSRAM()
{
/*
 *  Save a SRAM
 *
 *  Return values
 *     0 : Normally
 *    -1 : SRAM data couldn't be written
 */

  FILE *fp;
  int nUsedTable[256];
  unsigned char chData;
  unsigned char chPrevData;
  unsigned char chTag;
  int nIdx;
  int nEncoded;
  int nEncLen;
  int nRunLen;
  unsigned char pDstBuf[SRAM_SIZE];

  if (!nSRAM_SaveFlag)
    return 0;  // It doesn't need to save it

  /*-----------------------------------------------------------------*/
  /*  Compress a SRAM data                                             */
  /*-----------------------------------------------------------------*/

  memset(nUsedTable, 0, sizeof nUsedTable);

  for (nIdx = 0; nIdx < SRAM_SIZE; ++nIdx)
  {
    ++nUsedTable[SRAM[nIdx++]];
  }
  for (nIdx = 1, chTag = 0; nIdx < 256; ++nIdx)
  {
    if (nUsedTable[nIdx] < nUsedTable[chTag])
      chTag = nIdx;
  }

  nEncoded = 0;
  nEncLen = 0;
  nRunLen = 1;

  pDstBuf[nEncLen++] = chTag;

  chPrevData = SRAM[nEncoded++];

  while (nEncoded < SRAM_SIZE && nEncLen < SRAM_SIZE - 133)
  {
    chData = SRAM[nEncoded++];

    if (chPrevData == chData && nRunLen < 256)
      ++nRunLen;
    else
    {
      if (nRunLen >= 4 || chPrevData == chTag)
      {
        pDstBuf[nEncLen++] = chTag;
        pDstBuf[nEncLen++] = chPrevData;
        pDstBuf[nEncLen++] = nRunLen - 1;
      }
      else
      {
        for (nIdx = 0; nIdx < nRunLen; ++nIdx)
          pDstBuf[nEncLen++] = chPrevData;
      }

      chPrevData = chData;
      nRunLen = 1;
    }

  }
  if (nRunLen >= 4 || chPrevData == chTag)
  {
    pDstBuf[nEncLen++] = chTag;
    pDstBuf[nEncLen++] = chPrevData;
    pDstBuf[nEncLen++] = nRunLen - 1;
  }
  else
  {
    for (nIdx = 0; nIdx < nRunLen; ++nIdx)
      pDstBuf[nEncLen++] = chPrevData;
  }

  /*-----------------------------------------------------------------*/
  /*  Write a SRAM data                                                */
  /*-----------------------------------------------------------------*/

  // Open SRAM file
  fp = fopen(szSaveName, "wb");
  if (fp == NULL)
    return -1;

  // Write SRAM data
  fwrite(pDstBuf, nEncLen, 1, fp);

  // Close SRAM file
  fclose(fp);

  // Successful
  return 0;
}

/*=================================================================*/
/*                                                                   */
/*                  SLNES_Menu() : Menu screen                     */
/*                                                                   */
/*=================================================================*/
int SLNES_Menu()
{
/*
 *  Menu screen
 *
 *  Return values
 *     0 : Normally
 *    -1 : Exit SLNES
 */

	// Nothing to do here
  return 0;
}

/*=================================================================*/
/*                                                                   */
/*               SLNES_ReadRom() : Read ROM image file             */
/*                                                                   */
/*=================================================================*/
int SLNES_ReadRom(const char *pszFileName)
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

	FILE *fp;

	/* Open ROM file */
	fp = fopen(pszFileName, "rb");
	if (fp == NULL)
		return -1;


	fread(gamefile, 1, SIZE_OF_gamefile, fp);
	if(SLNES_Init() == -1)
		return -1;

	/* File close */
	fclose(fp);

	/* Successful */
	return 0;
}

/*=================================================================*/
/*                                                                   */
/*           SLNES_ReleaseRom() : Release a memory for ROM         */
/*                                                                   */
/*=================================================================*/
void SLNES_ReleaseRom()
{
/*
 *  Release a memory for ROM
 *
 */
	ROM = NULL;
	VROM = NULL;
}

/*=================================================================*/
/*                                                                   */
/*      SLNES_LoadFrame() :                                        */
/*           Transfer the contents of work frame on the screen       */
/*                                                                   */
/*=================================================================*/
void SLNES_LoadFrame()
{
/*
 *  Transfer the contents of work frame on the screen
 *
 */

  // Set screen data
  for (int i = 0; i < NES_DISP_HEIGHT; i++)
	for (int j = 0; j < NES_DISP_WIDTH; j++)
		*((WORD*)pScreenMem + i * NES_DISP_WIDTH + j) = NesPalette[PPU0[i * NES_DISP_WIDTH + j]];

  // Screen update
  HDC hDC = GetDC(hWndMain);

  HDC hMemDC = CreateCompatibleDC(hDC);

  HBITMAP hOldBmp = (HBITMAP)SelectObject(hMemDC, hScreenBmp);

  StretchBlt(hDC, 0, 0, NES_DISP_WIDTH * wScreenMagnification, 
              NES_DISP_HEIGHT * wScreenMagnification, hMemDC, 
              0, 0, NES_DISP_WIDTH, NES_DISP_HEIGHT, SRCCOPY);

  SelectObject(hMemDC, hOldBmp);

  DeleteDC(hMemDC);
  ReleaseDC(hWndMain, hDC);
}

/*=================================================================*/
/*                                                                   */
/*             SLNES_PadState() : Get a joypad state               */
/*                                                                   */
/*=================================================================*/
void SLNES_PadState(unsigned int *pdwPad1, unsigned int *pdwPad2, unsigned int *pdwSystem)
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
 *      Input for SLNES
 *
 */

  static DWORD dwSysOld;
  DWORD dwTemp;

  /* Joypad 1 */
  *pdwPad1 =   (GetAsyncKeyState('K')        < 0) |
             ((GetAsyncKeyState('J')        < 0) << 1) |
             ((GetAsyncKeyState('T')        < 0) << 2) |
             ((GetAsyncKeyState('Y')        < 0) << 3) |
             ((GetAsyncKeyState('E')      < 0) << 4) |
             ((GetAsyncKeyState('D')    < 0) << 5) |
             ((GetAsyncKeyState('S')    < 0) << 6) |
             ((GetAsyncKeyState('F')   < 0) << 7);

//  *pdwPad1 = *pdwPad1 | (*pdwPad1 << 8);

  /* Joypad 2 */
  //*pdwPad2 = 0;
  *pdwPad2 =   (GetAsyncKeyState('M')        < 0) |
             ((GetAsyncKeyState('N')        < 0) << 1) |
             ((GetAsyncKeyState('Z')        < 0) << 2) |
             ((GetAsyncKeyState('X')        < 0) << 3) |
             ((GetAsyncKeyState('G')      < 0) << 4) |
             ((GetAsyncKeyState('V')    < 0) << 5) |
             ((GetAsyncKeyState('C')    < 0) << 6) |
             ((GetAsyncKeyState('B')   < 0) << 7);

//  *pdwPad2 = *pdwPad2 | (*pdwPad2 << 8);

  /* Input for SLNES */
  dwTemp = (GetAsyncKeyState(VK_ESCAPE)  < 0);
  
  /* Only the button pushed newly should be inputted */
  *pdwSystem = ~dwSysOld & dwTemp;
  
  /* keep this input */
  dwSysOld = dwTemp;

  /* Deal with a message */
  MSG msg;
  while(PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
  {
    if (GetMessage(&msg, NULL, 0, 0))
    {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  }
}

/*=================================================================*/
/*                                                                   */
/*             SLNES_MemoryCopy() : memcpy                         */
/*                                                                   */
/*=================================================================*/
void *SLNES_MemoryCopy(void *dest, const void *src, int count)
{
/*
 *  memcpy
 *
 *  Parameters
 *    void *dest                       (Write)
 *      Points to the starting address of the copied block?fs destination
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

  CopyMemory(dest, src, count);
  return dest;
}

/*=================================================================*/
/*                                                                   */
/*             SLNES_MemorySet() : Get a joypad state              */
/*                                                                   */
/*=================================================================*/
void *SLNES_MemorySet(void *dest, int c, int count)
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

  FillMemory(dest, count, c); 
  return dest;
}

/*=================================================================*/
/*                                                                   */
/*                DebugPrint() : Print debug message                 */
/*                                                                   */
/*=================================================================*/
void SLNES_DebugPrint(char *pszMsg)
{
  _RPT0(_CRT_WARN, pszMsg);
}

/*=================================================================*/
/*                                                                   */
/*        SLNES_SoundOpen() : Sound Open                           */
/*                                                                   */
/*=================================================================*/
int SLNES_SoundOpen(int samples_per_sync, int sample_rate) 
{
  lpSndDevice = new DIRSOUND(hWndMain);

  if (!lpSndDevice->SoundOpen(samples_per_sync, sample_rate))
  {
    SLNES_MessageBox("SoundOpen() Failed.");
    exit(0);
  }

  //// if sound mute, stop sound
  //if (APU_Mute)
  //{
  //  if (!lpSndDevice->SoundMute(APU_Mute))
  //  {
  //    SLNES_MessageBox("SoundMute() Failed.");
  //    exit(0);
  //  }
  //}

  return(TRUE);
}

/*=================================================================*/
/*                                                                   */
/*        SLNES_SoundClose() : Sound Close                         */
/*                                                                   */
/*=================================================================*/
void SLNES_SoundClose(void) 
{
  lpSndDevice->SoundClose();
  delete lpSndDevice;
	lpSndDevice = NULL;
}

/*=================================================================*/
/*                                                                   */
/*            SLNES_SoundOutput() : Sound Output Waves             */           
/*                                                                   */
/*=================================================================*/
#if BITS_PER_SAMPLE == 8
void SLNES_SoundOutput(int samples, unsigned char *wave) 
#else /* BITS_PER_SAMPLE */
void SLNES_SoundOutput(int samples, short *wave) 
#endif /* BITS_PER_SAMPLE */
{
#if 1
  if (!lpSndDevice->SoundOutput(samples, wave))
#else
  if (!lpSndDevice->SoundOutput(samples, wave3))
#endif
  {
    SLNES_MessageBox("SoundOutput() Failed.");
    exit(0);
  }
}

/*=================================================================*/
/*                                                                   */
/*            SLNES_StartTimer() : Start MM Timer                  */           
/*                                                                   */
/*=================================================================*/
static void SLNES_StartTimer()
{
  TIMECAPS caps;

  timeGetDevCaps(&caps, sizeof(caps));
  timeBeginPeriod(caps.wPeriodMin);

  uTimerID = 
    timeSetEvent(caps.wPeriodMin * TIMER_PER_LINE, caps.wPeriodMin, TimerFunc, 0, (UINT)TIME_PERIODIC);

  // Calculate proper timing
  wLinePerTimer = LINE_PER_TIMER * caps.wPeriodMin;

  // Initialize timer variables
  wLines = 0;
  bWaitFlag = TRUE;

  // Initialize Critical Section Object
  InitializeCriticalSection(&WaitFlagCriticalSection);
}

/*=================================================================*/
/*                                                                   */
/*            SLNES_StopTimer() : Stop MM Timer                    */           
/*                                                                   */
/*=================================================================*/
static void SLNES_StopTimer()
{
  if (0 != uTimerID)
  {
    TIMECAPS caps;
    timeKillEvent(uTimerID);
    uTimerID = 0;
    timeGetDevCaps(&caps, sizeof(caps));
    timeEndPeriod(caps.wPeriodMin * TIMER_PER_LINE);
  }
  // Delete Critical Section Object
  DeleteCriticalSection(&WaitFlagCriticalSection);
}

/*=================================================================*/
/*                                                                   */
/*           TimerProc() : MM Timer Callback Function                */
/*                                                                   */
/*=================================================================*/
static void CALLBACK TimerFunc(UINT nID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2)
{
  if (NULL != m_hThread)
  {  
    EnterCriticalSection(&WaitFlagCriticalSection);
    bWaitFlag = FALSE;
    LeaveCriticalSection(&WaitFlagCriticalSection);
  }
}


/*=================================================================*/
/*                                                                   */
/*            SLNES_MessageBox() : Print System Message            */
/*                                                                   */
/*=================================================================*/
void SLNES_MessageBox(char *pszMsg, ...)
{
  char pszErr[1024];
  va_list args;

  va_start(args, pszMsg);
  vsprintf(pszErr, pszMsg, args);  pszErr[1023] = '\0';
  va_end(args);
  MessageBox(hWndMain, pszErr, APP_NAME, MB_OK | MB_ICONSTOP);
}

/*=================================================================*/
/*                                                                   */
/*            SetWindowSize() : Set Window Size                      */
/*                                                                   */
/*=================================================================*/
void SetWindowSize(WORD wMag)
{          
  wScreenMagnification = wMag;

  SetWindowPos(hWndMain, HWND_TOPMOST, 0, 0, 
								NES_DISP_WIDTH * wMag + NES_MENU_WIDTH, 
                NES_DISP_HEIGHT * wMag + NES_MENU_HEIGHT, SWP_NOMOVE);
          
  // Show title screen if emulation thread dosent exist
  if (NULL == m_hThread)
    ShowTitle(hWndMain);
}
