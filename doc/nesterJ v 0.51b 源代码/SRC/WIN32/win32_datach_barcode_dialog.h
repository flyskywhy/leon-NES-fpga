#ifndef WIN32_DATACH_BARCODE_DIALOG_
#define WIN32_DATACH_BARCODE_DIALOG_

typedef struct
{
  DWORD value_low;
  DWORD value_high;
} BARCODEVALUE, *LPBARCODEVALUE;

BOOL CALLBACK DatachBarcodeDialogProc(HWND,UINT,WPARAM,LPARAM);

#endif