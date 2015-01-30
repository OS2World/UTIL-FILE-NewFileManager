#define INCL_WIN
#define INCL_GPI
#define INCL_DOS
#define INCL_OS2

#include <os2.h>
#include <stdlib.h>
#include <string.h>
#include "caoj2i03.h"

static HAB hab;

MRESULT EXPENTRY CAOJ2I03_Proc( HWND, ULONG, MPARAM, MPARAM );
void DrawEmbossed( HWND );
CLASSINFO  clsiEmb;
typedef struct _EMBSTRUCT
   {
     BOOL   fStatus;
     CHAR   szText[512];
     ULONG  ulProfondita;
     ULONG  ulAllineamento;
     ULONG  ulColorFore;
   } EMBSTRUCT;
typedef EMBSTRUCT *PEMBSTRUCT;

/******************************************************************************/
/*                                                                            */
/******************************************************************************/
BOOL CAOJ2I03_REGISTRA( HAB lochab )
{
   CLASSINFO  clsiTmp;

   hab=lochab;
   /* Se classe gi… registrata */

   if( WinQueryClassInfo( hab,
                          (PSZ)"CAOJ2I03",
                          &clsiTmp )==TRUE ) {
      return( TRUE );
   }

   if( ! WinRegisterClass( hab,
                           (PSZ)"CAOJ2I03",
                           (PFNWP)CAOJ2I03_Proc,
//                         clsiEmb.flClassStyle & ~CS_PUBLIC,
//                         clsiEmb.cbWindowData ) ) {
                           CS_SIZEREDRAW | CS_SYNCPAINT,
                           4UL ) ) {
      WinMessageBox(HWND_DESKTOP,HWND_DESKTOP,"Not register class CAOJ2I03","CAOJ2I03",0L,0L);
      return( FALSE );
   }

   return( TRUE );
}

MRESULT EXPENTRY CAOJ2I03_Proc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 )
{
   PEMBSTRUCT pUserStruct=NULL;
   PCREATESTRUCT pCreateStruct=NULL;

   switch( msg )
   {
      case WM_CREATE:
            pCreateStruct=PVOIDFROMMP(mp2);
            DosAllocMem((PVOID)&pUserStruct, sizeof(EMBSTRUCT), PAG_COMMIT | PAG_READ | PAG_WRITE);
            memset(pUserStruct,0,sizeof(EMBSTRUCT));
            if ( (pCreateStruct->flStyle & WS_INCASSATO) == WS_INCASSATO ) {
               pUserStruct->fStatus=TRUE;
            } else {
               pUserStruct->fStatus=FALSE;
            }
            pUserStruct->ulProfondita=1;
            if ( (pCreateStruct->flStyle & WS_PROFPER2) == WS_PROFPER2 ) {
               pUserStruct->ulProfondita=2;
            }
            if ( (pCreateStruct->flStyle & WS_PROFPER4) == WS_PROFPER4 ) {
               pUserStruct->ulProfondita=4;
            }
            strcpy(pUserStruct->szText,pCreateStruct->pszText);
            pUserStruct->ulColorFore=CLR_BLACK;
            pUserStruct->ulAllineamento=DT_LEFT;
            WinSetWindowPtr(hwnd, 0, pUserStruct);
         break;

      case WM_DESTROY:
            DosFreeMem( WinQueryWindowPtr(hwnd, 0) );
         break;

      case WM_PAINT:
            DrawEmbossed( hwnd );
            return(MRESULT)0;
         break;

      case WM_SETWINDOWPARAMS:
            {
               PWNDPARAMS pWndParams;
               pUserStruct = WinQueryWindowPtr(hwnd, 0);
               pWndParams = PVOIDFROMMP(mp1);
               if (strcmp(pUserStruct->szText,pWndParams->pszText)!=0) {
                  strcpy(pUserStruct->szText,pWndParams->pszText);
                  WinInvalidateRect(hwnd,NULL,TRUE);
               }
            }
         break;

      default:
         break;
   }
   return ( MRESULT )WinDefWindowProc( hwnd, msg, mp1, mp2 );
}

void DrawEmbossed( HWND hwnd )
{
   HPS hps;
   RECTL rcWin;
   POINTL ptl;
   PEMBSTRUCT pUserStruct;
   ULONG i;

   pUserStruct = WinQueryWindowPtr(hwnd, 0);
   hps = WinBeginPaint( hwnd, (HPS)NULL, &rcWin );
   WinQueryWindowRect( hwnd , &rcWin );
   rcWin.xRight -- ;
   rcWin.yTop -- ;

   if (pUserStruct->fStatus) {
     GpiSetColor(hps,CLR_WHITE);
   } else {
     GpiSetColor(hps,CLR_DARKGRAY);
   }

   for (i=0;i<pUserStruct->ulProfondita;i++) {
      ptl.x=rcWin.xLeft+i;
      ptl.y=rcWin.yBottom+i;
      GpiMove(hps, &ptl);
      ptl.x=rcWin.xRight;
      ptl.y=rcWin.yBottom+i;
      GpiLine(hps, &ptl);
   }

   for (i=0;i<pUserStruct->ulProfondita;i++) {
      ptl.x=rcWin.xRight-i;
      ptl.y=rcWin.yBottom;
      GpiMove(hps, &ptl);
      ptl.x=rcWin.xRight-i;
      ptl.y=rcWin.yTop-i;
      GpiLine(hps, &ptl);
   }

   if (pUserStruct->fStatus) {
     GpiSetColor(hps,CLR_DARKGRAY);
   } else {
     GpiSetColor(hps,CLR_WHITE);
   }

   for (i=0;i<pUserStruct->ulProfondita;i++) {
      ptl.x=rcWin.xLeft+i;
      ptl.y=rcWin.yBottom+i;
      GpiMove(hps, &ptl);
      ptl.x=rcWin.xLeft+i;
      ptl.y=rcWin.yTop;
      GpiLine(hps, &ptl);
   }

   for (i=0;i<pUserStruct->ulProfondita;i++) {
      ptl.x=rcWin.xLeft;
      ptl.y=rcWin.yTop-i;
      GpiMove(hps, &ptl);
      ptl.x=rcWin.xRight-i;
      ptl.y=rcWin.yTop-i;
      GpiLine(hps, &ptl);
   }

   if (strlen(pUserStruct->szText)>0) {
      rcWin.xLeft   = rcWin.xLeft   + pUserStruct->ulProfondita +2;
      rcWin.xRight  = rcWin.xRight  - pUserStruct->ulProfondita -1;
      rcWin.yBottom = rcWin.yBottom + pUserStruct->ulProfondita +1;
      rcWin.yTop    = rcWin.yTop    - pUserStruct->ulProfondita -1;
      WinDrawText(hps,-1L,pUserStruct->szText,&rcWin,pUserStruct->ulColorFore,CLR_PALEGRAY,pUserStruct->ulAllineamento | DT_VCENTER | DT_ERASERECT);
   }

   WinEndPaint( hps );

   return;
}
