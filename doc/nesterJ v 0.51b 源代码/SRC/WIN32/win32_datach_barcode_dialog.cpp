#include <windows.h>
#include "win32_datach_barcode_dialog.h"
#include "resource.h"

BOOL CALLBACK DatachBarcodeDialogProc(HWND hDlg, UINT msg, WPARAM wparam, LPARAM lparam)
{
  static LPBARCODEVALUE lpBarValue;
  switch(msg)
  {
  case WM_CLOSE:
    EndDialog(hDlg, IDCANCEL);
    break;
  
  case WM_INITDIALOG:
    lpBarValue = (LPBARCODEVALUE)lparam;
    SendDlgItemMessage(hDlg, IDC_BARCODEVALUE, EM_SETLIMITTEXT, 13, 0);
    break;
  
  case WM_COMMAND:
    switch(LOWORD(wparam))
    {
    case IDC_BARCODEVALUE:
      {
        int result = 
              SendMessage((HWND)lparam, WM_GETTEXTLENGTH, 0, 0);
        EnableWindow(GetDlgItem(hDlg, IDOK), (result == 8 || result == 13) ? TRUE : FALSE );
        break;
      }

    case IDOK:
      {
        char sz[14], *p; 
        p = sz;
        int len = GetDlgItemText(hDlg, IDC_BARCODEVALUE, p, 14);
        
        if(len==8)
        {
          lpBarValue->value_high = 0;
        }
        else if(len==13)
        {
          char c;
          c = p[5]; p[5] = '\0';
          lpBarValue->value_high = (DWORD)atol(p);
          p[5] = c;
          p += 5;
        }
        else
          throw "Error on Datach Barcode";

        lpBarValue->value_low = (DWORD)atol(p);
        EndDialog(hDlg, IDOK);
      }
      break;
    case IDCANCEL:
      EndDialog(hDlg, IDCANCEL);
      break;
    
    default:
      return FALSE;
    }
    break;
  default:
    return FALSE;
  }
  return TRUE;
}
