#define INCL_WIN
#define INCL_DOS
#define INCL_GPI
#define INCL_BSE
#define INCL_ALL
#define INCL_SPLDOSPRINT
#define INCL_SPL
#define INCL_DEV
#define INCL_DOSDEVIOCTL
#define INCL_DOSMISC

#include <os2.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "nfmlib.h"

#define CWN_CHANGE      0x601
#define CWM_SETCOLOUR   0x602

typedef struct CWPARAM {
    USHORT      cb;
    RGB         rgb;
} CWPARAM;

typedef struct CWDATA {
    USHORT      updatectl;
    HWND        hwndCol;
    HWND        hwndSpinR;
    HWND        hwndSpinG;
    HWND        hwndSpinB;
    RGB         *rgb;
    RGB         rgbold;
} CWDATA;
typedef CWDATA *PCWDATA;

MRESULT  EXPENTRY  SettingsProc( HWND hwnd, ULONG  msg, MPARAM mp1, MPARAM mp2 );
MRESULT  EXPENTRY  ColorProc( HWND hwnd, ULONG  msg, MPARAM mp1, MPARAM mp2 );
BOOL     EXPENTRY  InitializeNoteBook( HWND );

HMODULE  hmod;
HWND     hwndParent;
COMMAREA cm;
HLIB     hlib;
HWND     hwndNB1;
CWPARAM  cwparam;

BOOL EXPENTRY NFMSETT( PCOMMAREA pcm )
{
  memcpy(&cm,pcm,sizeof(COMMAREA));
  DosQueryModuleHandle( (PCSZ) "NFMLIB", &hmod);
  WinDlgBox( HWND_DESKTOP, cm.hwndParent,
             (PFNWP)SettingsProc,
             (HMODULE)hmod,
             ID_SETTINGS,
             NULL);
  return(TRUE);
}

MRESULT EXPENTRY SettingsProc( HWND hwnd, ULONG  msg, MPARAM mp1, MPARAM mp2 )
{
  switch( msg )
  {
    case WM_INITDLG:
       hlib = WinLoadLibrary (cm.hab, (PCSZ) "WPCONFIG.DLL");
       InitializeNoteBook(hwnd);
    break;

    case WM_CLOSE:
       WinDestroyWindow(hwndNB1);
       WinDeleteLibrary(cm.hab, hlib);
    break;
  }
  return(WinDefDlgProc( hwnd, msg, mp1, mp2 ));
}

BOOL InitializeNoteBook( HWND hwnd )
{
   ULONG    ulPageId;

   /* Pagina 1 ************************************************************/
   ulPageId = (LONG)WinSendDlgItemMsg(hwnd, ID_NOTEBOOK,
        BKM_INSERTPAGE, NULL,
        MPFROM2SHORT((BKA_STATUSTEXTON | BKA_AUTOPAGESIZE | BKA_MAJOR),
        BKA_LAST));

   if (!ulPageId)
     return FALSE;

   if ( !WinSendDlgItemMsg(hwnd, ID_NOTEBOOK,
        BKM_SETTABTEXT, MPFROMLONG(ulPageId),
        MPFROMP("Font e colori") ))
     return FALSE;

   if (!hwndNB1)
     return FALSE;

   if ( !WinSendDlgItemMsg(hwnd, ID_NOTEBOOK,
         BKM_SETPAGEWINDOWHWND, MPFROMLONG(ulPageId),
         MPFROMHWND(hwndNB1)))
     return FALSE;

   return TRUE;
}


MRESULT EXPENTRY ColorProc( HWND hwnd, ULONG  msg, MPARAM mp1, MPARAM mp2 )
{
  CWDATA      *cwdata;
  HWND        hwndSpin;
  ULONG       newval;

  cwdata = (CWDATA *) WinQueryWindowULong (hwnd, QWL_USER);

  switch(msg)
  {
    case WM_INITDLG:
      if (!(cwdata=(PVOID)malloc(sizeof(CWDATA))) || !mp2 || !(cwdata->rgb = &(((CWPARAM *) mp2)->rgb)))
      {
          WinPostMsg (hwnd, WM_CLOSE, MPVOID, MPVOID);
          break;
      }

      WinSetWindowULong (hwnd, QWL_USER, (ULONG) cwdata);
      cwdata->updatectl = FALSE;
      cwdata->rgbold = *cwdata->rgb;
      cwdata->hwndCol   = WinWindowFromID (hwnd, ID_COL);
      cwdata->hwndSpinR = WinWindowFromID (hwnd, ID_SPINR);
      cwdata->hwndSpinG = WinWindowFromID (hwnd, ID_SPING);
      cwdata->hwndSpinB = WinWindowFromID (hwnd, ID_SPINB);
      WinSendMsg (cwdata->hwndSpinR, SPBM_SETLIMITS,       MPFROMSHORT(255), MPFROMSHORT(0));
      WinSendMsg (cwdata->hwndSpinR, SPBM_SETCURRENTVALUE, MPFROMSHORT(cwdata->rgb->bRed), MPVOID);
      WinSendMsg (cwdata->hwndSpinG, SPBM_SETLIMITS,       MPFROMSHORT(255), MPFROMSHORT(0));
      WinSendMsg (cwdata->hwndSpinG, SPBM_SETCURRENTVALUE, MPFROMSHORT(cwdata->rgb->bGreen), MPVOID);
      WinSendMsg (cwdata->hwndSpinB, SPBM_SETLIMITS,       MPFROMSHORT(255), MPFROMSHORT(0));
      WinSendMsg (cwdata->hwndSpinB, SPBM_SETCURRENTVALUE, MPFROMSHORT(cwdata->rgb->bBlue), MPVOID);
      cwdata->updatectl = TRUE;
    break;

    case CWN_CHANGE:
        {
           ULONG color;
           *cwdata->rgb = *((RGB *) &mp1);
           WinSendMsg (cwdata->hwndSpinR, SPBM_SETCURRENTVALUE,
               (MPARAM) cwdata->rgb->bRed, MPVOID);
           WinSendMsg (cwdata->hwndSpinG, SPBM_SETCURRENTVALUE,
               (MPARAM) cwdata->rgb->bGreen, MPVOID);
           WinSendMsg (cwdata->hwndSpinB, SPBM_SETCURRENTVALUE,
               (MPARAM) cwdata->rgb->bBlue, MPVOID);
           color=*((ULONG*)&cwdata->rgb);
         //WinSetPresParam (cm.hwndContFiles, PP_FOREGROUNDCOLOR,
         //                (ULONG)sizeof(LONG),
         //                MPFROMLONG(color));
        }
    break;

    case WM_CONTROL:
        switch(SHORT1FROMMP(mp1))
        {
          case ID_SPINR:
          case ID_SPING:
          case ID_SPINB:
            hwndSpin = WinWindowFromID (hwnd, SHORT1FROMMP(mp1));
            switch(SHORT2FROMMP(mp1))
            {
              case SPBN_CHANGE:
              case SPBN_KILLFOCUS:
                if (cwdata->updatectl)
                {
                    WinSendMsg (hwndSpin, SPBM_QUERYVALUE,
                               (MPARAM) &newval,
                               MPFROM2SHORT(0, SPBQ_ALWAYSUPDATE));
                    switch(SHORT1FROMMP(mp1))
                    {
                        case ID_SPINR:
                              cwdata->rgb->bRed = (BYTE) newval;
                            break;

                        case ID_SPING:
                              cwdata->rgb->bGreen = (BYTE) newval;
                            break;

                        case ID_SPINB:
                              cwdata->rgb->bBlue = (BYTE) newval;
                    }
                }
                WinSendMsg (cwdata->hwndCol, CWM_SETCOLOUR,
                            MPFROMLONG(*((ULONG *) cwdata->rgb) &
                            0xFFFFFF), MPVOID);
            }
        }
    break;

    case WM_DESTROY:
        free(cwdata);
  }

  return (WinDefDlgProc (hwnd, msg, mp1, mp2));
}

BOOL EXPENTRY SetColor( HAB hab, HWND hwndCntrl, ULONG ppColor )
{
   HWND hwndColor;
   hlib = WinLoadLibrary (cm.hab, (PCSZ) "WPCONFIG.DLL");
   cwparam.cb = sizeof(CWPARAM);
   cwparam.rgb.bRed   = 63;
   cwparam.rgb.bGreen = 127;
   cwparam.rgb.bBlue  = 255;
   hwndColor  = WinLoadDlg( HWND_DESKTOP,
                            hwndCntrl,
                            (PFNWP)ColorProc,
                            (HMODULE)hmod,
                            ID_SETTINGS_A,
                            &cwparam);
   WinDeleteLibrary(cm.hab, hlib);
   return( TRUE );
}
