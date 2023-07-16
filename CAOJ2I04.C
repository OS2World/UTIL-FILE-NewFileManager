#define INCL_WIN
#define INCL_GPI
#define INCL_DOS

#include <os2.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include "caoj2i04.h"

MRESULT EXPENTRY CAOJ2I04_Proc( HWND, ULONG, MPARAM, MPARAM );

void DrawPercentuale( HWND );

/* Variabili globali */
HAB        hab;
APIRET     rc;

BOOL CAOJ2I04_REGISTRA( HAB lochab )
{
   CLASSINFO  clsiTmp;

   hab=lochab;

   /* Se classe gi… registrata */
   if( WinQueryClassInfo( hab,
                          (PSZ)"CAOJ2I04",
                          &clsiTmp )==TRUE ) {
      return( TRUE );
   }

   if( ! WinRegisterClass( hab,
                           (PSZ)"CAOJ2I04",
                           (PFNWP)CAOJ2I04_Proc,
                           CS_SYNCPAINT,
                           4L ) ) {
      return( FALSE );
   }
   return( TRUE );
}

MRESULT EXPENTRY CAOJ2I04_Proc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 )
{
   ULONG    dividendo,divisore;
   double   i;

   switch( msg )
   {
      case WM_CREATE:
         WinSetWindowULong(hwnd, QWL_USER, 0UL);
      break;

      case WM_PAINT:
         DrawPercentuale( hwnd );
      break;

      case WM_SETPERC:
         dividendo=LONGFROMMP(mp1);
         divisore=LONGFROMMP(mp2);
         if (divisore==0) divisore=100;
         i=(double)dividendo/divisore;
         i=(double)i*100;
         if (i>100.0) i=100;
         if (((ULONG)i)!=WinQueryWindowULong(hwnd,QWL_USER)) {
            WinSetWindowULong(hwnd, QWL_USER, (ULONG)i);
            WinInvalidateRect(hwnd,NULL,TRUE);
         }
         return ( MRESULT )0;
       break;

   }
   return ( WinDefWindowProc( hwnd, msg, mp1, mp2 ) );
}

void DrawPercentuale( HWND hwnd )
{
   HPS      hps;
   RECTL    rcOr,rc,rcPieno,rcVuoto;
   POINTL   ptl;
   CHAR     stringa[7];
   double   k;
   ULONG    percCorrente=0;

   percCorrente=(ULONG)WinQueryWindowULong(hwnd,QWL_USER);

   hps = WinBeginPaint( hwnd, (HPS)NULL, &rc );
   WinQueryWindowRect( hwnd , &rcOr );
   rc=rcOr;
   rc.yBottom++;
   rc.yTop-=2;
   rc.xLeft++;
   rc.xRight-=2;

   GpiSetColor(hps,CLR_DARKGRAY);
   ptl.x = rc.xLeft;
   ptl.y = rc.yBottom+1;
   GpiMove(hps, &ptl);
   ptl.y = rc.yTop;
   GpiLine(hps, &ptl);
   ptl.x = rc.xRight-1;
   GpiLine(hps, &ptl);

   GpiSetColor(hps,CLR_WHITE);
   ptl.x = rc.xRight;
   ptl.y = rc.yTop;
   GpiMove(hps, &ptl);
   ptl.y = rc.yBottom;
   GpiLine(hps, &ptl);
   ptl.x = rc.xLeft;
   GpiLine(hps, &ptl);

   rc.yBottom++;
   rc.yTop--;
   rc.xLeft++;
   rc.xRight--;

   GpiSetColor(hps,CLR_BLACK);
   ptl.x = rc.xLeft;
   ptl.y = rc.yBottom+1;
   GpiMove(hps, &ptl);
   ptl.y = rc.yTop;
   GpiLine(hps, &ptl);
   ptl.x = rc.xRight-1;
   GpiLine(hps, &ptl);

   GpiSetColor(hps,CLR_PALEGRAY);
   ptl.x = rc.xRight;
   ptl.y = rc.yTop;
   GpiMove(hps, &ptl);
   ptl.y = rc.yBottom;
   GpiLine(hps, &ptl);
   ptl.x = rc.xLeft;
   GpiLine(hps, &ptl);

   rc.yBottom++;
   rc.xLeft++;

   k=0.0;
   if (WinQueryWindowULong(hwnd,QWL_STYLE) & WS_CSOJ2I0A_VERTICALE) {
      if (percCorrente>0) {
        k=(double)(rc.yTop-rc.yBottom)*(double)percCorrente/100.0;
        if (k<2) k=2;
      }

      rcVuoto=rc;
      rcVuoto.yBottom=rc.yBottom+k;

      rcPieno=rc;
      rcPieno.yTop--;
      rcPieno.yTop=rc.yBottom+k;
      rcPieno.xRight--;
   } else {
      if (percCorrente>0) {
        k=(double)(rc.xRight-rc.xLeft)*(double)percCorrente/100.0;
        if (k<2) k=2;
      }

      rcVuoto=rc;
      rcVuoto.xLeft=rc.xLeft+k;

      rcPieno=rc;
      rcPieno.yTop--;
      rcPieno.xRight=rc.xLeft+k;
   }

   WinFillRect( hps, &rcVuoto, CLR_DARKCYAN );

   if (k>0.0) {
     GpiSetColor(hps,CLR_WHITE);
     ptl.x = rcPieno.xLeft;
     ptl.y = rcPieno.yBottom+1;
     GpiMove(hps, &ptl);
     ptl.y = rcPieno.yTop;
     GpiLine(hps, &ptl);
     ptl.x = rcPieno.xRight-1;
     GpiLine(hps, &ptl);

     GpiSetColor(hps,CLR_BLACK);
     ptl.x = rcPieno.xRight;
     ptl.y = rcPieno.yTop;
     GpiMove(hps, &ptl);
     ptl.y = rcPieno.yBottom;
     GpiLine(hps, &ptl);
     ptl.x = rcPieno.xLeft;
     GpiLine(hps, &ptl);

     rcPieno.yBottom++;
     rcPieno.xLeft++;

     WinFillRect(hps,&rcPieno,CLR_DARKGRAY);
   }

   if (WinQueryWindowULong(hwnd,QWL_STYLE) & WS_CSOJ2I0A_VIS_PERC) {
      memset(stringa,0,sizeof(stringa));
      sprintf(stringa,"%d %%",percCorrente);
      if (percCorrente>50) {
         WinDrawText(hps,-1L,stringa, &rcOr, CLR_WHITE, CLR_PALEGRAY, DT_CENTER | DT_VCENTER );
      } else {
         WinDrawText(hps,-1L,stringa, &rcOr, CLR_WHITE, CLR_PALEGRAY, DT_CENTER | DT_VCENTER );
      }
   }

   WinEndPaint( hps );
}
