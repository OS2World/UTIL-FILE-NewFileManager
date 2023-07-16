#define INCL_WIN
#define INCL_GPI
#define INCL_DOS
#define INCL_WINTRACKRECT

#include <os2.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "csoj2i0i.h"

#define TP_ORIZZONTALE 1
#define TP_VERTICALE   2

typedef struct {
  ULONG          tipo;           // TP_ORIZZONTALE  TP_VERTICALE
  ULONG          idClass;
  HWND           hwndClass;
  HWND           hwndParent;
  HWND           hwndOwner;
  ULONG          sizeBorder;
} AREA_CLASSE;
typedef AREA_CLASSE *PAREA_CLASSE;

static HAB hab;
static HPOINTER hptrO;
static HPOINTER hptrV;
static HPOINTER hptrN;

MRESULT EXPENTRY CSOJ2I0I_Proc       ( HWND, ULONG, MPARAM, MPARAM );
VOID    EXPENTRY DrawBarra           ( PAREA_CLASSE );
VOID    EXPENTRY TrackRect           ( PAREA_CLASSE, MPARAM );

/* DA LOCALIZZARE */
static BOOL fTrack=FALSE;
static TRACKINFO tiStruct;

BOOL CSOJ2I0I_REGISTRA( HAB lochab )
{
   CLASSINFO  clsiTmp;

   hab=lochab;

   /* Se classe gi… registrata */
   if( WinQueryClassInfo( hab,
                          (PSZ)"CSOJ2I0I",
                          &clsiTmp )==TRUE ) {
      return( TRUE );
   }

   WinRegisterClass( hab, (PSZ)"CSOJ2I0I", (PFNWP)CSOJ2I0I_Proc, CS_SIZEREDRAW, 4L );

   hptrN = WinQuerySysPointer(HWND_DESKTOP, SPTR_ARROW,  FALSE);
   hptrO = WinQuerySysPointer(HWND_DESKTOP, SPTR_SIZEWE, FALSE);
   hptrV = WinQuerySysPointer(HWND_DESKTOP, SPTR_SIZENS, FALSE);

   return( TRUE );
}

MRESULT EXPENTRY CSOJ2I0I_Proc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 )
{
   PAREA_CLASSE pArea = NULLHANDLE;

   switch( msg )
   {
      case WM_CREATE:
      {
        PCREATESTRUCT  pCreateStruct;
        pCreateStruct=PVOIDFROMMP(mp2);

        DosAllocMem((PVOID)&pArea, sizeof(AREA_CLASSE), PAG_COMMIT | PAG_READ | PAG_WRITE );
        memset(pArea,0,sizeof(AREA_CLASSE));
        if (pCreateStruct->cx>pCreateStruct->cy) {
           pArea->tipo=TP_ORIZZONTALE;
        } else {
           pArea->tipo=TP_VERTICALE;
        }
        pArea->hwndClass=hwnd;
        pArea->hwndParent=WinQueryWindow(hwnd,QW_PARENT);
        pArea->hwndOwner=WinQueryWindow(hwnd,QW_OWNER);
        pArea->idClass=WinQueryWindowUShort(hwnd,QWS_ID);

        WinSetWindowPtr(hwnd, 0, pArea);
      }
      break;

      case WM_DESTROY:
        DosFreeMem(WinQueryWindowPtr(hwnd, 0));
      break;

      case WM_CSOJ2I0I_SETSIZEBORDER:
        pArea = WinQueryWindowPtr(hwnd, 0);
        pArea->sizeBorder=LONGFROMMP(mp1);
      break;

      case WM_MOUSEMOVE:
        pArea = WinQueryWindowPtr(hwnd, 0);
        if (pArea->tipo==TP_VERTICALE) {
           WinSetPointer(HWND_DESKTOP,hptrO);
        } else {
           WinSetPointer(HWND_DESKTOP,hptrV);
        }
        return(MRESULT)0;
      break;

      case WM_BUTTON1DOWN:
        pArea = WinQueryWindowPtr(hwnd, 0);
        TrackRect(pArea,mp1);
      break;

      case WM_PAINT:
        DrawBarra(WinQueryWindowPtr(hwnd, 0));
        return(MRESULT)(FALSE);
      break;
   }
   return ( WinDefWindowProc(hwnd, msg, mp1, mp2) );
}

/******************************************************************************
*                                                                             *
******************************************************************************/
VOID DrawBarra( PAREA_CLASSE pArea  )
{
   RECTL    rcWin;
   HPS      hps;
   POINTL   ptl;

   hps = WinBeginPaint( pArea->hwndClass, (HPS)NULL, &rcWin );

   WinFillRect( hps, &rcWin, CLR_PALEGRAY );
   WinQueryWindowRect( pArea->hwndClass , &rcWin );

   rcWin.yTop--;
   rcWin.xRight--;
   //rcWin.yBottom++;
   //rcWin.xLeft++;

   if (pArea->tipo==TP_ORIZZONTALE) {
      GpiSetColor(hps,CLR_BLACK);
      ptl.x = rcWin.xLeft;
      ptl.y = rcWin.yBottom;
      GpiMove(hps, &ptl);
      ptl.x = rcWin.xRight;
      GpiLine(hps, &ptl);

      GpiSetColor(hps,CLR_PALEGRAY);
      ptl.x = rcWin.xRight;
      ptl.y = rcWin.yTop;
      GpiMove(hps, &ptl);
      ptl.x = rcWin.xLeft;
      GpiLine(hps, &ptl);

      rcWin.yTop--;
      rcWin.yBottom++;

      GpiSetColor(hps,CLR_DARKGRAY);
      ptl.x = rcWin.xLeft;
      ptl.y = rcWin.yBottom;
      GpiMove(hps, &ptl);
      ptl.x = rcWin.xRight;
      GpiLine(hps, &ptl);

      GpiSetColor(hps,CLR_WHITE);
      ptl.x = rcWin.xRight;
      ptl.y = rcWin.yTop;
      GpiLine(hps, &ptl);
      ptl.x = rcWin.xLeft;
      GpiLine(hps, &ptl);
   } else {
      GpiSetColor(hps,CLR_PALEGRAY);
      ptl.x = rcWin.xLeft;
      ptl.y = rcWin.yBottom;
      GpiMove(hps, &ptl);
      ptl.y = rcWin.yTop;
      GpiLine(hps, &ptl);

      GpiSetColor(hps,CLR_BLACK);
      ptl.x = rcWin.xRight;
      ptl.y = rcWin.yBottom;
      GpiMove(hps, &ptl);
      ptl.y = rcWin.yTop;
      GpiLine(hps, &ptl);

      rcWin.xRight--;
      rcWin.xLeft++;

      GpiSetColor(hps,CLR_WHITE);
      ptl.x = rcWin.xLeft;
      ptl.y = rcWin.yBottom;
      GpiMove(hps, &ptl);
      ptl.y = rcWin.yTop;
      GpiLine(hps, &ptl);

      GpiSetColor(hps,CLR_DARKGRAY);
      ptl.x = rcWin.xRight;
      ptl.y = rcWin.yBottom;
      GpiMove(hps, &ptl);
      ptl.y = rcWin.yTop;
      GpiLine(hps, &ptl);
   }
   WinEndPaint( hps );
   return;
}

VOID EXPENTRY TrackRect( PAREA_CLASSE pArea, MPARAM mp1 )
{
   HPS  hpsOwner;
   SWP  swpClass;
   LONG delta;

   tiStruct.cxBorder = 1;
   tiStruct.cyBorder = 1;
   tiStruct.cxGrid = 0;
   tiStruct.cyGrid = 0;

   WinQueryWindowPos(pArea->hwndClass,&swpClass);

   tiStruct.rclTrack.xLeft   = swpClass.x;
   tiStruct.rclTrack.xRight  = swpClass.x + swpClass.cx;
   tiStruct.rclTrack.yBottom = swpClass.y;
   tiStruct.rclTrack.yTop    = swpClass.y + swpClass.cy;

   WinQueryWindowRect( pArea->hwndOwner, &tiStruct.rclBoundary );
   if (pArea->tipo==TP_VERTICALE) {
      tiStruct.rclBoundary.xLeft   = tiStruct.rclBoundary.xLeft  + pArea->sizeBorder;
      tiStruct.rclBoundary.xRight  = tiStruct.rclBoundary.xRight - pArea->sizeBorder;
      tiStruct.rclBoundary.yBottom = tiStruct.rclTrack.yBottom;
      tiStruct.rclBoundary.yTop    = tiStruct.rclTrack.yTop;
   } else {
      tiStruct.rclBoundary.xLeft   = tiStruct.rclTrack.xLeft;
      tiStruct.rclBoundary.xRight  = tiStruct.rclTrack.xRight;
      tiStruct.rclBoundary.yBottom = tiStruct.rclBoundary.yBottom + pArea->sizeBorder;
      tiStruct.rclBoundary.yTop    = tiStruct.rclBoundary.yTop    - pArea->sizeBorder;
   }

   tiStruct.ptlMinTrackSize.x = 0;
   tiStruct.ptlMinTrackSize.y = 0;

   tiStruct.ptlMaxTrackSize.x = tiStruct.rclBoundary.xRight;
   tiStruct.ptlMaxTrackSize.y = tiStruct.rclBoundary.yTop;

   tiStruct.fs  = TF_STANDARD | TF_ALLINBOUNDARY | TF_MOVE;

   hpsOwner=WinGetPS( pArea->hwndOwner );
   WinTrackRect(pArea->hwndClass, hpsOwner, &tiStruct);
   WinReleasePS(hpsOwner);

   if (pArea->tipo==TP_VERTICALE) {
      if (tiStruct.rclTrack.xLeft != swpClass.x) {
         delta=tiStruct.rclTrack.xLeft - swpClass.x;
         WinSendMsg( pArea->hwndOwner, WM_CONTROL, MPFROM2SHORT(pArea->idClass,WN_CSOJ2I0I_MOVED), MPFROMLONG(delta));
         WinSetWindowPos( pArea->hwndClass, NULLHANDLE, swpClass.x+delta, swpClass.y, 0L, 0L , SWP_MOVE );
         WinInvalidateRect( pArea->hwndClass, NULL, TRUE );
      }
   } else {
      if (tiStruct.rclTrack.yBottom != swpClass.y) {
         delta=tiStruct.rclTrack.yBottom - swpClass.y;
         WinSendMsg( pArea->hwndOwner, WM_CONTROL, MPFROM2SHORT(pArea->idClass,WN_CSOJ2I0I_MOVED), MPFROMLONG(delta));
         WinSetWindowPos( pArea->hwndClass, NULLHANDLE, swpClass.x, swpClass.y+delta, 0L, 0L , SWP_MOVE );
         WinInvalidateRect( pArea->hwndClass, NULL, TRUE );
      }
   }
   return;
}
