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
#define INCL_WINCURSORS
#define INCL_WINPOINTERS
#define INCL_GPI
#define INCL_DOS

#include <os2.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "cspcsta3.h"

#define TOT_PAL            255
#define START_PAL_OWN        16
#define STOP_PAL_OWN         25
#define LONGFromRGB(R,G,B) (LONG)(((LONG)R<<16)+((LONG)G<<8)+(LONG)B)
#define MAKEFIXEDFROMDOUBLE(X) MAKEFIXED((ULONG)X, (ULONG)(((double)X-(ULONG)X)*65536) )
#define ANGOLO 45

RGB rgb[TOT_PAL] = {
   0,   0,   0,     // CLR_BACKGROUND
   0,   0, 255,     // CLR_BLUE
 255,   0,   0,     // CLR_RED
 255,   0, 255,     // CLR_PINK
   0, 255,   0,     // CLR_GREEN
   0, 255, 255,     // CLR_CYAN
 255, 255,   0,     // CLR_YELLOW
   0,   0,   0,     // CLR_NEUTRAL
 128, 128, 128,     // CLR_DARKGRAY
   0,   0, 128,     // CLR_DARKBLUE
 128,   0,   0,     // CLR_DARKRED
 128,   0, 128,     // CLR_DARKPINK
   0, 128,   0,     // CLR_DARKGREEN
   0, 128, 128,     // CLR_DARKCYAN
 128, 128,   0,     // CLR_BROWN
 204, 204, 204,     // CLR_PALEGRAY
  80, 216, 248,     // 16
 128, 216, 168,     // 17
 188, 144, 248,     // 18
 208, 180, 168,     // 19
 248, 252, 168,     // 20
 168, 144, 168,     // 21
 248, 144,  80,     // 22
 248, 216,  80,     // 23
 248, 180, 168,     // 24
 248, 216, 168,     // 25
};

typedef struct _INSERTITEM {
   double  dblPerc;
   CHAR    szLabel[CCHMAXPATH];
   ULONG   id;
   RECTL   rcText;
   struct  _INSERTITEM* pNext;
} INSERTITEM;
typedef INSERTITEM *PINSERTITEM;

typedef struct {
   HWND        hwndClass;
   ULONG       tipoVis;     // CSPCSTA3_DRAW_TORTA_3D
                            // CSPCSTA3_DRAW_BARRE_3D
                            // CSPCSTA3_DRAW_LINEE_3D
   ULONG       ulPercTot;
   ULONG       ulTotItem;
   PINSERTITEM pItem;
} PRIVATESTRUCT;
typedef PRIVATESTRUCT *PPRIVATESTRUCT;

/* Window Proc ***************************************************************/
MRESULT EXPENTRY CSPCSTA3_Proc    ( HWND, ULONG, MPARAM, MPARAM );
VOID             DrawClass_Torta  ( PPRIVATESTRUCT, HPS, RECTL* );
VOID             DrawClass_Barre  ( PPRIVATESTRUCT, HPS, RECTL* );
VOID             DrawClass_Linee  ( PPRIVATESTRUCT, HPS, RECTL* );
VOID             DrawArc          ( HPS, ULONG, POINTL, POINTL, double, double );
VOID             DrawBarre        ( HPS, ULONG, POINTL, ULONG, ULONG, ULONG );
VOID             DrawLinee        ( HPS, ULONG, POINTL, ULONG, ULONG, ULONG, ULONG );
VOID             Cornice          ( HPS, PRECTL );
VOID             DrawAngleText    ( HPS, PCHAR, USHORT, POINTL, SIZEL, PSZ );

/* Static (non istanziate) ***************************************************/
static HAB         hab;
ULONG  alTable[TOT_PAL];

BOOL CSPCSTA3_REGISTRA( HAB lochab )
{
   CLASSINFO   clsiTmp;
   ULONG       i,j;

   hab=lochab;

   /* Se classe gi… registrata */
   if( WinQueryClassInfo( hab,
                          (PSZ)"CSPCSTA3",
                          &clsiTmp )==TRUE ) {
      return( TRUE );
   }

   WinRegisterClass( hab, (PSZ)"CSPCSTA3", (PFNWP)CSPCSTA3_Proc, CS_SIZEREDRAW, 4L );

   j=0;
   for (i=0;i<TOT_PAL;i++,j++) {
     alTable[i] = PC_RESERVED * 16777216 + LONGFromRGB( rgb[j].bBlue, rgb[j].bGreen, rgb[j].bRed );
     if (j>=STOP_PAL_OWN) j=START_PAL_OWN-1;
   }

   return( TRUE );
}

MRESULT EXPENTRY CSPCSTA3_Proc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 )
{
   PPRIVATESTRUCT pArea;

   switch( msg )
   {
      case WM_CREATE:
        {
          PCREATESTRUCT pCreateStruct;
          pCreateStruct=PVOIDFROMMP(mp2);
          DosAllocMem((PVOID)&pArea, sizeof(PRIVATESTRUCT), PAG_COMMIT | PAG_READ | PAG_WRITE);
          memset(pArea,0,sizeof(PRIVATESTRUCT));
          if (pCreateStruct->flStyle & WS_CSPCSTA3_BARRE_3D) pArea->tipoVis = CSPCSTA3_DRAW_BARRE_3D;
          if (pCreateStruct->flStyle & WS_CSPCSTA3_LINEE_3D) pArea->tipoVis = CSPCSTA3_DRAW_LINEE_3D;
          pArea->hwndClass=hwnd;
          WinSetWindowPtr(hwnd, 0, pArea);
        }
      break;

      case WM_DESTROY:
        {
          PINSERTITEM pItem,pParent;
          pArea=WinQueryWindowPtr(hwnd, 0);
          pItem=pArea->pItem;
          while (pItem) {
             pParent=pItem;
             pItem=pItem->pNext;
             DosFreeMem(pParent);
          }
          DosFreeMem(pArea);
        }
      break;

      case WM_BUTTON1CLICK:
        {
           POINTL point;
           PINSERTITEM pItem;
           WinQueryPointerPos(HWND_DESKTOP,&point);
           WinMapWindowPoints(HWND_DESKTOP, hwnd, &point, 1);
           pArea=WinQueryWindowPtr(hwnd, 0);
           pItem=pArea->pItem;
           while (pItem) {
              if (WinPtInRect(hab,&pItem->rcText,&point)) {
                 WinSendMsg(WinQueryWindow(hwnd,QW_OWNER),
                            WM_CONTROL,
                            MPFROM2SHORT(WinQueryWindowUShort(hwnd,QWS_ID),WN_CSPCSTA3_SELECT),
                            MPFROMLONG(pItem->id));
              }
              pItem=pItem->pNext;
           }
        }
      break;

      case WM_PAINT:
        {
           RECTL rc;
           HPS  hps;
           pArea=WinQueryWindowPtr(hwnd, 0);
           hps = WinBeginPaint( pArea->hwndClass, (HPS)NULL, &rc);
           GpiCreateLogColorTable(hps, 0L, LCOLF_CONSECRGB, 0L, TOT_PAL, (PLONG)&alTable[0]);
           switch (pArea->tipoVis) {
             case CSPCSTA3_DRAW_TORTA_3D: DrawClass_Torta(pArea,hps,&rc); break;
             case CSPCSTA3_DRAW_BARRE_3D: DrawClass_Barre(pArea,hps,&rc); break;
             case CSPCSTA3_DRAW_LINEE_3D: DrawClass_Linee(pArea,hps,&rc); break;
           }
           WinEndPaint( hps );
           return(MRESULT)0;
        }
      break;

      case WM_CSPCSTA3_INSERTITEM:
        {
          PINSERTITEM pItem,pParent;
          PCSPCSTA3_INSERTITEM pInsert;
          pArea=WinQueryWindowPtr(hwnd, 0);
          pInsert=PVOIDFROMMP(mp1);
          if ( (pArea->ulPercTot + pInsert->dblPerc) > 100 )
             return(MRFROMLONG(-1));
          pItem=pArea->pItem;
          while (pItem) {
             pParent=pItem;
             pItem=pItem->pNext;
          }
          DosAllocMem((PVOID)&pItem, sizeof(INSERTITEM), PAG_COMMIT | PAG_READ | PAG_WRITE);
          memset(pItem,0,sizeof(pItem));
          pItem->dblPerc=pInsert->dblPerc;
          strcpy(pItem->szLabel,pInsert->szLabel);
          pItem->id=pArea->ulTotItem;
          pItem->pNext=NULL;
          if (pArea->pItem==NULL) {
             pArea->pItem=pItem;
          } else {
             pParent->pNext=pItem;
          }
          pArea->ulPercTot+=pInsert->dblPerc;
          pArea->ulTotItem++;
          WinInvalidateRect(hwnd,NULL,TRUE);
          return(MRFROMLONG(pItem->id));
        }
      break;

      case WM_CSPCSTA3_SETDRAW:
        pArea=WinQueryWindowPtr(hwnd, 0);
        if (LONGFROMMP(mp1)!=pArea->tipoVis) {
           pArea->tipoVis=LONGFROMMP(mp1);
           WinInvalidateRect(hwnd,NULL,TRUE);
        }
        return(MRESULT)0;
      break;

      case WM_CSPCSTA3_QUERYLABEL:
        {
          PINSERTITEM pItem;
          pArea=WinQueryWindowPtr(hwnd, 0);
          pItem=pArea->pItem;
          while (pItem) {
             if (pItem->id==SHORT1FROMMP(mp1)) {
                strncpy(PVOIDFROMMP(mp2),pItem->szLabel,SHORT2FROMMP(mp1));
                return(MRESULT)TRUE;
             }
             pItem=pItem->pNext;
          }
          return(MRESULT)FALSE;
        }
      break;

      case WM_CSPCSTA3_AZZERA:
        {
          PINSERTITEM pItem,pParent;
          pArea=WinQueryWindowPtr(hwnd, 0);
          pItem=pArea->pItem;
          while (pItem) {
             pParent=pItem;
             pItem=pItem->pNext;
             DosFreeMem(pParent);
          }
          pArea->ulPercTot=0;
          pArea->ulTotItem=0;
          pArea->pItem=NULL;
        }
      break;

   }
   return ( WinDefWindowProc(hwnd, msg, mp1, mp2) );
}

VOID DrawClass_Torta( PPRIVATESTRUCT pArea, HPS hps, RECTL* rc )
{
   #define      ID_PATH 1L
   #define      GRADI_RAD 0.01745329252
   RECTL        rcScrittaS,rcScrittaD,rcTmp;
   PINSERTITEM  pItem;
   ULONG        iColor;
   ARCPARAMS    arcp;
   double       x,y,cx,cy,kyScrittaS,kyScrittaD,yScrittaS,yScrittaD;
   POINTL       origine,origineombra;
   POINTL       ptl,ptlstart,ptlstop;
   double       prev,curr,medio;
   ULONG        sizeS,sizeD,numlabelS,numlabelD;

   WinFillRect( hps, rc, CLR_PALEGRAY );

   /* Condizione nessun record inserito ***********************************/
   if (pArea->ulTotItem==0) {
      return;
   }

   WinQueryWindowRect( pArea->hwndClass , rc );
   rc->xRight--;
   rc->yTop--;

   /* Calcolo delle dimensioni ********************************************/
   prev=90;
   pItem=pArea->pItem;
   sizeS=sizeD=0;
   numlabelS=numlabelD=0;
   while (pItem) {
      curr=(pItem->dblPerc*3.6);
      if (strlen(pItem->szLabel)>0) {
        rcTmp.xLeft=0;
        rcTmp.xRight=1;
        rcTmp.yBottom=0;
        rcTmp.yTop=1;
        WinDrawText(hps,-1L, (PCHAR)pItem->szLabel, &rcTmp, 0L,0L,
                             DT_CENTER | DT_VCENTER | DT_TEXTATTRS | DT_QUERYEXTENT );
        if ((prev+curr/2) <= 270) {
           if ((rcTmp.xRight-rcTmp.xLeft) > sizeS) sizeS=rcTmp.xRight-rcTmp.xLeft;
           numlabelS++;
        } else {
           if ((rcTmp.xRight-rcTmp.xLeft) > sizeD) sizeD=rcTmp.xRight-rcTmp.xLeft;
           numlabelD++;
        }
      }
      prev=prev+curr;
      pItem=pItem->pNext;
   }

   if (sizeS) sizeS+=10;
   if (sizeD) sizeD+=10;

   if ((sizeS+sizeD) > (rc->xRight-rc->xLeft)/2) {
      if (sizeS) sizeS=(rc->xRight-rc->xLeft)/4;
      if (sizeD) sizeD=(rc->xRight-rc->xLeft)/4;
   }

   rcScrittaD=*rc;
   rcScrittaD.xLeft=rcScrittaD.xRight-sizeD;

   rcScrittaS=*rc;
   rcScrittaS.xRight=rcScrittaS.xLeft+sizeS;

   /***********************************************************************/
   cx=(double)((rc->xRight-rc->xLeft)-sizeS-sizeD)*0.8;
   x=sizeS+(double)(rc->xRight-rc->xLeft-cx-sizeS-sizeD)/2.0;
   y=(double)(rc->yTop-rc->yBottom-cx)/2.0;
   cy=cx;

   origine.x=x+(cx/2);
   origine.y=y+(cy/1.8);
   origineombra.x=x+(cx/2);
   origineombra.y=y+(cy/2.2);

   GpiMove(hps, &origine);
   arcp.lP = (cx/2)-10;
   arcp.lQ = (cy/4);
   arcp.lR = 0;
   arcp.lS = 0;
   GpiSetArcParams(hps, &arcp);

   /* Disegno della torta *************************************************/
   iColor=START_PAL_OWN-1;
   prev=90;
   pItem=pArea->pItem;
   while (pItem) {
      curr=(pItem->dblPerc*3.6);
      DrawArc(hps,++iColor,origine,origineombra,prev,curr);
      prev=prev+curr;
      pItem=pItem->pNext;
   }
   curr=450-prev;
   DrawArc(hps,CLR_PALEGRAY,origine,origineombra,prev,curr);

   /* Disegno delle labels ************************************************/
   if (numlabelS) {
      kyScrittaS=(double)(rcScrittaS.yTop-rcScrittaS.yBottom)/(double)numlabelS;
   }
   if (numlabelD) {
      kyScrittaD=(double)(rcScrittaD.yTop-rcScrittaD.yBottom)/(double)numlabelD;
   }

   yScrittaS=rcScrittaS.yTop;
   yScrittaD=rcScrittaD.yBottom;
   iColor=START_PAL_OWN-1;
   prev=90;
   pItem=pArea->pItem;
   while (pItem) {
      curr=(pItem->dblPerc*3.6);
      medio=prev+curr/2;
      GpiSetColor(hps,++iColor);

      if (strlen(pItem->szLabel)>0) {
         if (medio <= 270) {
            rcScrittaS.yTop=yScrittaS;
            rcScrittaS.yBottom=rcScrittaS.yTop-kyScrittaS;
            if (kyScrittaS>22) {
               rcScrittaS.yBottom=rcScrittaS.yBottom+(rcScrittaS.yTop-rcScrittaS.yBottom-22)/2;
               rcScrittaS.yTop=rcScrittaS.yBottom+22;
            }
            yScrittaS-=kyScrittaS;
            rcTmp=rcScrittaS;
            ptlstart.x=rcScrittaS.xRight;
         } else {
            rcScrittaD.yBottom=yScrittaD;
            rcScrittaD.yTop=rcScrittaD.yBottom+kyScrittaD;
            if (kyScrittaD>22) {
               rcScrittaD.yBottom=rcScrittaD.yBottom+(rcScrittaD.yTop-rcScrittaD.yBottom-22)/2;
               rcScrittaD.yTop=rcScrittaD.yBottom+22;
            }
            yScrittaD+=kyScrittaD;
            rcTmp=rcScrittaD;
            ptlstart.x=rcScrittaD.xLeft;
         }

         ptlstart.y=(rcTmp.yTop-rcTmp.yBottom)/2+rcTmp.yBottom;
         pItem->rcText=rcTmp;
         Cornice(hps,&rcTmp);

         WinFillRect(hps,&rcTmp,iColor);
         WinDrawText(hps,-1L,pItem->szLabel,&rcTmp, 0L, 0L, DT_CENTER | DT_VCENTER );

         GpiSetColor(hps,CLR_BLACK);
         GpiSetLineType(hps, LINETYPE_INVISIBLE);
         GpiPartialArc(hps, &origine, MAKEFIXED(1, 0), MAKEFIXEDFROMDOUBLE(prev), MAKEFIXEDFROMDOUBLE(curr/2));
         GpiSetLineType(hps, LINETYPE_SOLID);
         GpiQueryCurrentPosition(hps,&ptlstop);

         GpiMove(hps,&ptlstart);

         if (medio>=180 && medio<=360) {
            ptlstop.y=ptlstop.y-(origine.y-origineombra.y)/2;
         }

         ptl.x=ptlstop.x+(double)(cos((medio)*GRADI_RAD)*5.0);
         ptl.y=ptlstop.y+(double)(sin((medio)*GRADI_RAD)*7.0);
         GpiLine(hps,&ptl);

         GpiLine(hps,&ptlstop);

         ptlstop.x++; GpiLine(hps,&ptlstop);
         ptlstop.y++; GpiLine(hps,&ptlstop);
         ptlstop.x--; GpiLine(hps,&ptlstop);
         ptlstop.y--; GpiLine(hps,&ptlstop);
      }
      prev=prev+curr;
      pItem=pItem->pNext;
   }
}

VOID DrawArc( HPS hps, ULONG color, POINTL origine, POINTL origineombra, double start, double whidth )
{
  double stop,miastart,miawhidth;
  POINTL ptl;

  GpiMove(hps, &origine);
  GpiBeginPath(hps, 1);
  GpiPartialArc(hps, &origine, MAKEFIXED(1, 0), MAKEFIXEDFROMDOUBLE(start), MAKEFIXEDFROMDOUBLE(whidth));
  GpiCloseFigure(hps);
  GpiEndPath(hps);
  GpiSetColor(hps, color );
  GpiFillPath(hps, 1, FPATH_ALTERNATE);

  miastart=start;
  stop=miastart+whidth;

  if ((stop>=180 && stop<=360) ||
      (miastart>=180 && miastart<=360) ||
      (miastart<=180 && stop>=360) ) {

     if (miastart<180) miastart=180;
     if (stop>360) stop=360;
     miawhidth=stop-miastart;

     GpiSetLineType(hps, LINETYPE_INVISIBLE);
     GpiPartialArc(hps, &origine, MAKEFIXED(1, 0), MAKEFIXEDFROMDOUBLE(miastart), MAKEFIXED(0, 0));
     GpiSetLineType(hps, LINETYPE_SOLID);

     GpiBeginPath(hps, 1);
     GpiQueryCurrentPosition(hps,&ptl);
     ptl.y=ptl.y-(origine.y-origineombra.y);
     GpiLine(hps,&ptl);

     GpiPartialArc(hps, &origineombra, MAKEFIXED(1, 0), MAKEFIXEDFROMDOUBLE(miastart), MAKEFIXEDFROMDOUBLE(miawhidth));

     GpiQueryCurrentPosition(hps,&ptl);
     ptl.y=ptl.y+(origine.y-origineombra.y);
     GpiLine(hps,&ptl);

     GpiPartialArc(hps, &origine, MAKEFIXED(1, 0), MAKEFIXEDFROMDOUBLE(miastart), MAKEFIXEDFROMDOUBLE(miawhidth));

     GpiEndPath(hps);
     GpiSetColor(hps, color );
     GpiFillPath(hps, 1, FPATH_ALTERNATE);

     GpiSetColor(hps, CLR_BLACK );
     GpiMove(hps,&origine);
     GpiSetLineType(hps, LINETYPE_INVISIBLE);
     GpiPartialArc(hps, &origine, MAKEFIXED(1, 0), MAKEFIXEDFROMDOUBLE(miastart), MAKEFIXED(0, 0));

     GpiSetLineType(hps, LINETYPE_SOLID);
     GpiQueryCurrentPosition(hps,&ptl);
     ptl.y=ptl.y-(origine.y-origineombra.y);
     GpiLine(hps,&ptl);

     GpiPartialArc(hps, &origineombra, MAKEFIXED(1, 0), MAKEFIXEDFROMDOUBLE(miastart), MAKEFIXEDFROMDOUBLE(miawhidth));

     GpiQueryCurrentPosition(hps,&ptl);
     ptl.y=ptl.y+(origine.y-origineombra.y);
     GpiLine(hps,&ptl);
  }

  GpiSetColor(hps, CLR_BLACK );
  GpiMove(hps,&origine);
  GpiPartialArc(hps, &origine, MAKEFIXED(1, 0), MAKEFIXEDFROMDOUBLE(start), MAKEFIXEDFROMDOUBLE(whidth));
  GpiLine(hps,&origine);

  return;
}
/*
VOID Cornice( HPS hps , PRECTL rc )
{
   POINTL  ptl;

   GpiSetColor(hps,CLR_DARKGRAY);
   ptl.x = rc->xLeft;
   ptl.y = rc->yBottom;
   GpiMove(hps, &ptl);
   ptl.y = rc->yTop;
   GpiLine(hps, &ptl);
   ptl.x = rc->xRight;
   GpiLine(hps, &ptl);

   GpiSetColor(hps,CLR_WHITE);
   ptl.x = rc->xRight;
   ptl.y = rc->yTop-1;
   GpiMove(hps, &ptl);
   ptl.y = rc->yBottom;
   GpiLine(hps, &ptl);
   ptl.x = rc->xLeft+1;
   GpiLine(hps, &ptl);

   rc->yBottom+=2;
   rc->yTop-=2;
   rc->xLeft+=2;
   rc->xRight-=2;

   GpiSetColor(hps,CLR_WHITE);
   ptl.x = rc->xLeft;
   ptl.y = rc->yBottom;
   GpiMove(hps, &ptl);
   ptl.y = rc->yTop;
   GpiLine(hps, &ptl);
   ptl.x = rc->xRight;
   GpiLine(hps, &ptl);

   GpiSetColor(hps,CLR_DARKGRAY);
   ptl.x = rc->xRight;
   ptl.y = rc->yTop-1;
   GpiMove(hps, &ptl);
   ptl.y = rc->yBottom;
   GpiLine(hps, &ptl);
   ptl.x = rc->xLeft+1;
   GpiLine(hps, &ptl);

   rc->xLeft++;
   rc->yBottom++;

   return;
}
*/
VOID Cornice( HPS hps , PRECTL rc )
{
   POINTL  ptl;

   GpiSetColor(hps,CLR_DARKGRAY);
   ptl.x = rc->xLeft;
   ptl.y = rc->yBottom;
   GpiMove(hps, &ptl);
   ptl.y = rc->yTop;
   GpiLine(hps, &ptl);
   ptl.x = rc->xRight;
   GpiLine(hps, &ptl);

   GpiSetColor(hps,CLR_WHITE);
   ptl.x = rc->xRight;
   ptl.y = rc->yTop-1;
   GpiMove(hps, &ptl);
   ptl.y = rc->yBottom;
   GpiLine(hps, &ptl);
   ptl.x = rc->xLeft+1;
   GpiLine(hps, &ptl);

   rc->yBottom++;
   rc->yTop--;
   rc->xLeft++;
   rc->xRight--;

   GpiSetColor(hps,CLR_WHITE);
   ptl.x = rc->xLeft;
   ptl.y = rc->yBottom;
   GpiMove(hps, &ptl);
   ptl.y = rc->yTop;
   GpiLine(hps, &ptl);
   ptl.x = rc->xRight;
   GpiLine(hps, &ptl);

   GpiSetColor(hps,CLR_DARKGRAY);
   ptl.x = rc->xRight;
   ptl.y = rc->yTop-1;
   GpiMove(hps, &ptl);
   ptl.y = rc->yBottom;
   GpiLine(hps, &ptl);
   ptl.x = rc->xLeft+1;
   GpiLine(hps, &ptl);

   rc->yBottom++;
   rc->yTop--;
   rc->xLeft++;
   rc->xRight--;

   return;
}

VOID DrawClass_Barre( PPRIVATESTRUCT pArea,HPS hps, RECTL* rc )
{
   #define      ID_PATH 1L
   #define      SPAZIO_SCRITTE  20L
   #define      SPAZIO_LATERALE 15L
   RECTL        rcTmp,rcArea,rcText;
   double       x,y,kx,ky,maxy,maxtext;
   POINTL       origine,ptl;
   ULONG        iColor;
   PINSERTITEM  pItem;
   ULONG        tick_y;
   CHAR         szPerc[5];
   SIZEL        size;

   WinFillRect( hps, rc, CLR_PALEGRAY );

   /* Condizione nessun record inserito ***********************************/
   if (pArea->ulTotItem==0) return;

   WinQueryWindowRect( pArea->hwndClass , rc );
   rc->xRight--;
   rc->yTop--;

   maxy=maxtext=0;
   pItem=pArea->pItem;
   while (pItem) {
      if (pItem->dblPerc>maxy) maxy=pItem->dblPerc;

      rcTmp.xLeft=0;
      rcTmp.xRight=1;
      rcTmp.yBottom=0;
      rcTmp.yTop=1;
      WinDrawText(hps,-1L, (PCHAR)pItem->szLabel, &rcTmp, 0L,0L,
                           DT_CENTER | DT_VCENTER | DT_TEXTATTRS | DT_QUERYEXTENT );
      if ((rcTmp.xRight-rcTmp.xLeft) > maxtext) maxtext=rcTmp.xRight-rcTmp.xLeft;

      pItem=pItem->pNext;
   }
   if (maxy==0) return;
   maxtext=maxtext * 1.3;
   if ( maxtext > ((rc->yTop-rc->yBottom)/2) ) maxtext = ((rc->yTop-rc->yBottom)/2);

   rcArea=*rc;
   rcArea.xLeft=SPAZIO_LATERALE*3;
   rcArea.xRight-=(SPAZIO_LATERALE*3);
   rcArea.yBottom=maxtext+SPAZIO_SCRITTE+SPAZIO_LATERALE;
   rcArea.yTop-=SPAZIO_LATERALE*2;

   rcText=*rc;
   rcText.yTop=maxtext;
  // rcText.yBottom=SPAZIO_LATERALE;

   kx=(double)(rcArea.xRight-rcArea.xLeft)/pArea->ulTotItem;
   if (kx>SPAZIO_LATERALE*3) kx=SPAZIO_LATERALE*3;
   ky=(double)(rcArea.yTop-rcArea.yBottom)/maxy;
   origine.x=rcArea.xLeft;
   origine.y=rcArea.yBottom;

   /* Disegno della griglia di sfondo *************************************/
   GpiSetColor(hps, CLR_BLACK );
   GpiSetLineType(hps,LINETYPE_SHORTDASH);

        if (maxy<25) tick_y=3;
   else if (maxy<50) tick_y=5;
   else              tick_y=10;

   for (y=0;y<maxy;y=y+tick_y) {
      ptl.x=rcArea.xRight+SPAZIO_LATERALE;
      ptl.y=rcArea.yBottom+(y*ky);

      memset(szPerc,0,sizeof(szPerc));
      sprintf(szPerc,"%d%%",(ULONG)y);
      rcTmp.xLeft    = ptl.x-SPAZIO_LATERALE;
      rcTmp.xRight   = ptl.x+SPAZIO_LATERALE;
      rcTmp.yBottom  = ptl.y+1;
      rcTmp.yTop     = rcTmp.yBottom+12;
      WinDrawText(hps,-1L, (PCHAR)szPerc, &rcTmp, 0L,0L,
                           DT_CENTER | DT_VCENTER | DT_TEXTATTRS );

      GpiMove(hps,&ptl);

      ptl.x=origine.x;
      GpiLine(hps,&ptl);

      ptl.x=ptl.x-cos(ANGOLO)*kx*0.5-cos(ANGOLO)*SPAZIO_LATERALE;
      ptl.y=ptl.y-sin(ANGOLO)*kx*0.5-sin(ANGOLO)*SPAZIO_LATERALE;
      GpiLine(hps,&ptl);
   }
   GpiSetLineType(hps,LINETYPE_DEFAULT);

   GpiMove(hps,&origine);
   ptl.x=origine.x;
   ptl.y=origine.y+(maxy*ky)+SPAZIO_LATERALE;
   GpiLine(hps,&ptl);

   GpiMove(hps,&origine);
   ptl.x=origine.x-cos(ANGOLO)*kx*0.5-cos(ANGOLO)*SPAZIO_LATERALE;
   ptl.y=origine.y-sin(ANGOLO)*kx*0.5-sin(ANGOLO)*SPAZIO_LATERALE;
   GpiLine(hps,&ptl);

   GpiMove(hps,&origine);
   ptl.x=rcArea.xRight+SPAZIO_LATERALE;
   ptl.y=origine.y;
   GpiLine(hps,&ptl);

   /* Disegno delle barre *************************************************/
   iColor=START_PAL_OWN-1;
   x=origine.x;
   pItem=pArea->pItem;
   while (pItem) {
      ptl.x=x;
      ptl.y=origine.y;
      DrawBarre(hps,++iColor,ptl,(ULONG)kx,(ULONG)kx*0.5,(ULONG)pItem->dblPerc*ky);

      ptl.x=ptl.x-cos(ANGOLO)*(kx*0.5);
      rcText.xLeft=ptl.x+1;
      rcText.xRight=ptl.x+kx-1;
      if (kx>22) {
         rcText.xLeft=rcText.xLeft+(rcText.xRight-rcText.xLeft-22)/2;
         rcText.xRight=rcText.xLeft+22;
      }
      rcTmp=rcText;
      Cornice(hps,&rcTmp);
      WinFillRect(hps,&rcTmp,iColor);

      GpiSetColor(hps,CLR_BLACK);
      ptl.x = rcTmp.xRight;
      ptl.y = rcTmp.yBottom;
      size.cx = rcTmp.yTop-rcTmp.yBottom;
      size.cy = rcTmp.xRight-rcTmp.xLeft;
      DrawAngleText(hps,pItem->szLabel, 90, ptl, size, "System VIO" );

      pItem->rcText=rcTmp;

      x+=kx;
      pItem=pItem->pNext;
   }
   return;
}

VOID DrawBarre( HPS hps, ULONG color, POINTL origine, ULONG cx, ULONG cy, ULONG cz )
{
  POINTL ptl;

  origine.x=origine.x-cos(ANGOLO)*cy;
  origine.y=origine.y-sin(ANGOLO)*cy;

  ptl=origine;

  GpiMove(hps, &ptl);
  GpiBeginPath(hps, 1);
  ptl.x+=cx;
  GpiLine(hps,&ptl);
  ptl.x=ptl.x+cos(ANGOLO)*cy;
  ptl.y=ptl.y+sin(ANGOLO)*cy;
  GpiLine(hps,&ptl);
  ptl.y+=cz;
  GpiLine(hps,&ptl);
  ptl.x-=cx;
  GpiLine(hps,&ptl);
  ptl.x=origine.x;
  ptl.y=origine.y+cz;
  GpiLine(hps,&ptl);
  GpiLine(hps,&origine);
  GpiCloseFigure(hps);
  GpiEndPath(hps);
  GpiSetColor(hps, color );
  GpiFillPath(hps, 1, FPATH_ALTERNATE);

  GpiSetColor(hps, CLR_BLACK );
  ptl=origine;
  GpiMove(hps, &ptl);
  ptl.x+=cx;
  GpiLine(hps,&ptl);
  ptl.x=ptl.x+cos(ANGOLO)*cy;
  ptl.y=ptl.y+sin(ANGOLO)*cy;
  GpiLine(hps,&ptl);
  ptl.y+=cz;
  GpiLine(hps,&ptl);
  ptl.x-=cx;
  GpiLine(hps,&ptl);
  ptl.x=origine.x;
  ptl.y=origine.y+cz;
  GpiLine(hps,&ptl);
  GpiLine(hps,&origine);

  origine.x+=cx;
  origine.y+=cz;

  GpiMove(hps, &origine);
  ptl.x=origine.x-cx;
  ptl.y=origine.y;
  GpiLine(hps,&ptl);

  GpiMove(hps, &origine);
  ptl.x=origine.x;
  ptl.y=origine.y-cz;
  GpiLine(hps,&ptl);

  GpiMove(hps, &origine);
  ptl.x=origine.x+cos(ANGOLO)*cy;
  ptl.y=origine.y+sin(ANGOLO)*cy;
  GpiLine(hps,&ptl);
  return;
}

VOID DrawAngleText(HPS hps, PCHAR pchText, USHORT usAngle, POINTL ptlRot, SIZEL size , PSZ facename )
{
   #define DW_LOG_FONT              1
   #define DW_MAX_ANGLE             90

   FATTRS   faFont;
   MATRIXLF mxRotate;
   FONTMETRICS fmFont;
   RECTL    rc;
   SIZEF    szfChar;

   usAngle %= (DW_MAX_ANGLE+1);

   faFont.usRecordLength=sizeof(faFont);
   faFont.fsSelection=0;
   faFont.lMatch=0;
   if (facename==NULL) strcpy(faFont.szFacename,"Helv");
                  else strcpy(faFont.szFacename,facename);
   faFont.idRegistry=0;
   faFont.usCodePage=GpiQueryCp(hps);
   faFont.lMaxBaselineExt=0;
   faFont.lAveCharWidth=0;
   faFont.fsType=0;
   faFont.fsFontUse=FATTR_FONTUSE_OUTLINE | FATTR_FONTUSE_TRANSFORMABLE;

   if (GpiSavePS(hps)==GPI_ERROR) goto stop;

   if (GpiCreateLogFont(hps,NULL,DW_LOG_FONT,&faFont)==GPI_ERROR) goto stop;

   if (!GpiSetCharSet(hps,DW_LOG_FONT)) goto stop;

   if (!GpiSetCharMode(hps,CM_MODE3)) goto stop;

   szfChar.cx=MAKEFIXED(size.cy/1.5,0);
   szfChar.cy=szfChar.cx;
   if (!GpiSetCharBox(hps,&szfChar)) goto stop;

   if (!GpiRotate(hps,
                  &mxRotate,
                  TRANSFORM_REPLACE,
                  MAKEFIXED(usAngle,0),
                  &ptlRot)) goto stop;

   if (!GpiSetModelTransformMatrix(hps,
                                   9,
                                   &mxRotate,
                                   TRANSFORM_REPLACE)) goto stop;

   //-------------------------------------------------------------------
   // We need the maximum descender size.
   //-------------------------------------------------------------------
   if (!GpiQueryFontMetrics(hps,sizeof(fmFont),&fmFont)) goto stop;

   //-------------------------------------------------------------------
   // Draw the text.
   //-------------------------------------------------------------------
   rc.xLeft  =ptlRot.x;
   rc.xRight =ptlRot.x+size.cx;
   rc.yBottom=ptlRot.y;
   rc.yTop   =ptlRot.y+size.cy;
   WinDrawText(hps,-1L,pchText, &rc, 0L, 0L, DT_CENTER | DT_VCENTER | DT_TEXTATTRS );

 stop:

   GpiSetCharSet(hps,LCID_DEFAULT);
   GpiDeleteSetId(hps,DW_LOG_FONT);
   GpiRestorePS(hps,-1);
   return;
}

VOID DrawClass_Linee( PPRIVATESTRUCT pArea, HPS hps, RECTL* rc )
{
   #define      ID_PATH 1L
   #define      SPAZIO_SCRITTE  20L
   #define      SPAZIO_LATERALE 15L
   RECTL        rcTmp,rcArea,rcText;
   double       x,y,kx,ky,maxy,maxtext;
   POINTL       origine,ptl;
   ULONG        iColor;
   PINSERTITEM  pItem;
   ULONG        tick_y;
   CHAR         szPerc[5];
   SIZEL        size;
   ULONG        percPrev;

   WinFillRect( hps, rc, CLR_PALEGRAY );

   /* Condizione nessun record inserito ***********************************/
   if (pArea->ulTotItem==0) return;

   WinQueryWindowRect( pArea->hwndClass , rc );
   rc->xRight--;
   rc->yTop--;

   maxy=maxtext=0;
   pItem=pArea->pItem;
   while (pItem) {
      if (pItem->dblPerc>maxy) maxy=pItem->dblPerc;

      rcTmp.xLeft=0;
      rcTmp.xRight=1;
      rcTmp.yBottom=0;
      rcTmp.yTop=1;
      WinDrawText(hps,-1L, (PCHAR)pItem->szLabel, &rcTmp, 0L,0L,
                           DT_CENTER | DT_VCENTER | DT_TEXTATTRS | DT_QUERYEXTENT );
      if ((rcTmp.xRight-rcTmp.xLeft) > maxtext) maxtext=rcTmp.xRight-rcTmp.xLeft;

      pItem=pItem->pNext;
   }
   if (maxy==0) return;
   maxtext=maxtext * 1.3;
   if ( maxtext > ((rc->yTop-rc->yBottom)/2) ) maxtext = ((rc->yTop-rc->yBottom)/2);

   rcArea=*rc;
   rcArea.xLeft=SPAZIO_LATERALE*3;
   rcArea.xRight-=(SPAZIO_LATERALE*3);
   rcArea.yBottom=maxtext+SPAZIO_SCRITTE+SPAZIO_LATERALE;
   rcArea.yTop-=SPAZIO_LATERALE*2;

   rcText=*rc;
   rcText.yTop=maxtext;
  // rcText.yBottom=SPAZIO_LATERALE;

   kx=(double)(rcArea.xRight-rcArea.xLeft)/pArea->ulTotItem;
   if (kx>SPAZIO_LATERALE*3) kx=SPAZIO_LATERALE*3;
   ky=(double)(rcArea.yTop-rcArea.yBottom)/maxy;
   origine.x=rcArea.xLeft;
   origine.y=rcArea.yBottom;

   /* Disegno della griglia di sfondo *************************************/
   GpiSetColor(hps, CLR_BLACK );
   GpiSetLineType(hps,LINETYPE_SHORTDASH);

        if (maxy<25) tick_y=3;
   else if (maxy<50) tick_y=5;
   else              tick_y=10;

   for (y=0;y<maxy;y=y+tick_y) {
      ptl.x=rcArea.xRight+SPAZIO_LATERALE;
      ptl.y=rcArea.yBottom+(y*ky);

      memset(szPerc,0,sizeof(szPerc));
      sprintf(szPerc,"%d%%",(ULONG)y);
      rcTmp.xLeft    = ptl.x-SPAZIO_LATERALE;
      rcTmp.xRight   = ptl.x+SPAZIO_LATERALE;
      rcTmp.yBottom  = ptl.y+1;
      rcTmp.yTop     = rcTmp.yBottom+12;
      WinDrawText(hps,-1L, (PCHAR)szPerc, &rcTmp, 0L,0L,
                           DT_CENTER | DT_VCENTER | DT_TEXTATTRS );

      GpiMove(hps,&ptl);

      ptl.x=origine.x;
      GpiLine(hps,&ptl);

      ptl.x=ptl.x-cos(ANGOLO)*kx*0.5-cos(ANGOLO)*SPAZIO_LATERALE;
      ptl.y=ptl.y-sin(ANGOLO)*kx*0.5-sin(ANGOLO)*SPAZIO_LATERALE;
      GpiLine(hps,&ptl);
   }
   GpiSetLineType(hps,LINETYPE_DEFAULT);

   GpiMove(hps,&origine);
   ptl.x=origine.x;
   ptl.y=origine.y+(maxy*ky)+SPAZIO_LATERALE;
   GpiLine(hps,&ptl);

   GpiMove(hps,&origine);
   ptl.x=origine.x-cos(ANGOLO)*kx*0.5-cos(ANGOLO)*SPAZIO_LATERALE;
   ptl.y=origine.y-sin(ANGOLO)*kx*0.5-sin(ANGOLO)*SPAZIO_LATERALE;
   GpiLine(hps,&ptl);

   GpiMove(hps,&origine);
   ptl.x=rcArea.xRight+SPAZIO_LATERALE;
   ptl.y=origine.y;
   GpiLine(hps,&ptl);

   /* Disegno delle barre *************************************************/
   iColor=START_PAL_OWN-1;
   x=origine.x;
   percPrev=0;
   pItem=pArea->pItem;
   while (pItem) {
      ptl.x=x;
      ptl.y=origine.y;
      DrawLinee(hps,++iColor,ptl,(ULONG)kx,(ULONG)kx*0.5,(ULONG)pItem->dblPerc*ky,percPrev);
      percPrev=(ULONG)pItem->dblPerc*ky;

      ptl.x=ptl.x-cos(ANGOLO)*(kx*0.5);
      rcText.xLeft=ptl.x+(kx*0.3)+1;
      rcText.xRight=ptl.x+kx+(kx*0.3)-1;
      if (kx>22) {
         rcText.xLeft=rcText.xLeft+(rcText.xRight-rcText.xLeft-22)/2;
         rcText.xRight=rcText.xLeft+22;
      }
      rcTmp=rcText;
      Cornice(hps,&rcTmp);
      WinFillRect(hps,&rcTmp,iColor);

      GpiSetColor(hps,CLR_BLACK);
      ptl.x = rcTmp.xRight;
      ptl.y = rcTmp.yBottom;
      size.cx = rcTmp.yTop-rcTmp.yBottom;
      size.cy = rcTmp.xRight-rcTmp.xLeft;
      DrawAngleText(hps,pItem->szLabel, 90, ptl, size, "System VIO" );

      pItem->rcText=rcTmp;

      x+=kx;
      pItem=pItem->pNext;
   }
}

VOID DrawLinee( HPS hps, ULONG color, POINTL origine, ULONG cx, ULONG cy, ULONG cz, ULONG czPrev )
{
  POINTL ptl;

  origine.x=origine.x+cx;

  ptl=origine;
  ptl.y=ptl.y+cz;
  GpiMove(hps, &ptl);

  GpiBeginPath(hps, 1);
  ptl.x=ptl.x-cos(ANGOLO)*cy;
  ptl.y=ptl.y-sin(ANGOLO)*cy;
  GpiLine(hps,&ptl);
  ptl.x=ptl.x-cx;
  ptl.y=ptl.y-(cz-czPrev);
  GpiLine(hps,&ptl);
  ptl.x=ptl.x+cos(ANGOLO)*cy;
  ptl.y=ptl.y+sin(ANGOLO)*cy;
  GpiLine(hps,&ptl);
  ptl.x=ptl.x+cx;
  ptl.y=ptl.y+(cz-czPrev);
  GpiLine(hps,&ptl);
  GpiCloseFigure(hps);
  GpiEndPath(hps);
  GpiSetColor(hps, color );
  GpiFillPath(hps, 1, FPATH_ALTERNATE);

  GpiSetColor(hps, CLR_BLACK );
  ptl=origine;
  ptl.y=ptl.y+cz;
  GpiMove(hps, &ptl);
  ptl.x=ptl.x-cos(ANGOLO)*cy;
  ptl.y=ptl.y-sin(ANGOLO)*cy;
  GpiLine(hps,&ptl);
  ptl.x=ptl.x-cx;
  ptl.y=ptl.y-(cz-czPrev);
  GpiLine(hps,&ptl);
  ptl.x=ptl.x+cos(ANGOLO)*cy;
  ptl.y=ptl.y+sin(ANGOLO)*cy;
  GpiLine(hps,&ptl);
  ptl.x=ptl.x+cx;
  ptl.y=ptl.y+(cz-czPrev);
  GpiLine(hps,&ptl);
  return;
}
