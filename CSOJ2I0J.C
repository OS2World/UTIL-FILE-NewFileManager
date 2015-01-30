#define INCL_WIN
#define INCL_GPI
#define INCL_DOS

#include <os2.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "csoj2i0j.h"

// Messaggi non esportati
#define WM_HIGHTUPDATED   WM_USER + 100
#define WM_SETITEMCONTENT WM_USER + 1

#define SB_INCREMENTO     10
#define FRECCIA_CX        8
#define FRECCIA_CY        8
#define BMP_CX            20
#define BMP_CY            20
#define SB_WHIDTH         14
#define SCROLL_HIGH       1000
#define SIZE_CLOSE        22
#define MAX_ITEM          50
#define VELOCITA          4  //Minore Š piu veloce

typedef struct {
   HWND    hwndClass;
   HWND    hwndSB;
   HWND    hwndArea;
   HWND    hwndCtrl;
   ULONG   lastItem;
   ULONG   cy;

   LONG    maxy;
   LONG    miny;
   LONG    deltay;
   LONG    pagey;
   SWP     swpArea;
   SWP     swpCtrl;
} PRIVATESTRUCTCTRL;
typedef PRIVATESTRUCTCTRL *PPRIVATESTRUCTCTRL;

typedef struct {
   BOOL  fStatus;
   HWND  hwndItem;
   HWND  szTitle[256];
   HWND  szContent[1024];
   ULONG hightRow;
   ULONG numRow;
   ULONG sizeContent;
   HBITMAP hbmp;
} PRIVATESTRUCTITEM;
typedef PRIVATESTRUCTITEM *PPRIVATESTRUCTITEM;

/* Window Proc ***************************************************************/
MRESULT EXPENTRY CSOJ2I0J_Proc    ( HWND, ULONG, MPARAM, MPARAM );
MRESULT EXPENTRY Ctrl_Proc        ( HWND, ULONG, MPARAM, MPARAM );
MRESULT EXPENTRY Item_Proc        ( HWND, ULONG, MPARAM, MPARAM );

VOID             DrawItem         ( PPRIVATESTRUCTITEM );
VOID             ApriChiudi       ( PPRIVATESTRUCTITEM );
VOID             ImpostaDimensioni( PPRIVATESTRUCTCTRL );

/* Static (non istanziate) ***************************************************/
static HAB         hab;
static HPOINTER    hptrWait=NULLHANDLE;
static HMODULE     hmod;
static HBITMAP     hbmpDown=NULLHANDLE;
static HBITMAP     hbmpRight=NULLHANDLE;


BOOL CSOJ2I0J_REGISTRA( HAB lochab )
{
   CLASSINFO   clsiTmp;

   hab=lochab;

   /* Se classe gi… registrata */
   if( WinQueryClassInfo( hab,
                          (PSZ)"CSOJ2I0J",
                          &clsiTmp )==TRUE ) {
      return( TRUE );
   }

   WinRegisterClass( hab, (PSZ)"CSOJ2I0J", (PFNWP)CSOJ2I0J_Proc, CS_SIZEREDRAW, 4L );
   WinRegisterClass( hab, (PSZ)"ClCtrl", (PFNWP)Ctrl_Proc, CS_SYNCPAINT, 4L );
   WinRegisterClass( hab, (PSZ)"ClItem", (PFNWP)Item_Proc, 0L, 4L );

   DosQueryModuleHandle("CSOJ2I0J", &hmod);

   return( TRUE );
}

MRESULT EXPENTRY CSOJ2I0J_Proc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 )
{
   PPRIVATESTRUCTCTRL pCtrl;

   switch( msg )
   {
      case WM_CREATE:
      {
        PCREATESTRUCT pCreateStruct;
        pCreateStruct=PVOIDFROMMP(mp2);

        DosAllocMem((PVOID)&pCtrl, sizeof(PRIVATESTRUCTCTRL), PAG_COMMIT | PAG_READ | PAG_WRITE);

        memset(pCtrl,0,sizeof(PRIVATESTRUCTCTRL));
        WinSetWindowPtr(hwnd, 0, pCtrl);
        pCtrl->hwndClass=hwnd;
        pCtrl->cy=0;

        if (hbmpDown==NULLHANDLE) {
           HPS hps=NULLHANDLE;
           hps=WinGetPS(hwnd);
           hbmpDown    = GpiLoadBitmap(hps, hmod, ID_CSOJ2I0J_BMP_DOWN,  0UL, 0UL);
           hbmpRight   = GpiLoadBitmap(hps, hmod, ID_CSOJ2I0J_BMP_RIGHT, 0UL, 0UL);
           WinReleasePS(hps);
        }

        pCtrl->hwndSB = WinCreateWindow( hwnd,
                                    WC_SCROLLBAR,
                                    "",
                                    WS_VISIBLE | SBS_VERT | WS_CLIPCHILDREN,
                                    (LONG)pCreateStruct->cx-SB_WHIDTH,
                                    (LONG)0l,
                                    (LONG)SB_WHIDTH,
                                    (LONG)pCreateStruct->cy,
                                    hwnd,
                                    HWND_TOP,
                                    ID_CSOJ2I0J_VSB,
                                    NULL, NULL);

        pCtrl->hwndArea = WinCreateWindow( hwnd,
                                    WC_STATIC,
                                    "", WS_SYNCPAINT | WS_VISIBLE,
                                    (LONG)0l,
                                    (LONG)0l,
                                    (LONG)pCreateStruct->cx-SB_WHIDTH,
                                    (LONG)pCreateStruct->cy,
                                    hwnd,
                                    HWND_TOP,
                                    ID_CSOJ2I0J_AREA,
                                    NULL, NULL);

        pCtrl->hwndCtrl = WinCreateWindow( pCtrl->hwndArea,
                                    "ClCtrl",
                                    "",
                                    WS_VISIBLE,
                                    (LONG)0,
                                    (LONG)-SCROLL_HIGH+pCreateStruct->cy,
                                    (LONG)pCreateStruct->cx-SB_WHIDTH,
                                    (LONG)SCROLL_HIGH,
                                    pCtrl->hwndArea,
                                    HWND_BOTTOM,
                                    -1,
                                    NULL, NULL);
        WinSetWindowPtr(pCtrl->hwndCtrl, 0, pCtrl);
        WinSetPresParam(pCtrl->hwndCtrl, PP_FONTNAMESIZE,
                        (ULONG)(strlen("8.Helv")+(ULONG)1),
                        (PSZ)"8.Helv");

        ImpostaDimensioni( pCtrl );
      }
      break;

      case WM_SIZE:
         {
           SWP  swp,swpItem;
           HWND hwndItem;
           ULONG idNextItem;
           pCtrl = WinQueryWindowPtr(hwnd, 0);
           WinQueryWindowPos(pCtrl->hwndClass,&swp);
           WinSetWindowPos(pCtrl->hwndSB,NULLHANDLE,swp.cx-SB_WHIDTH,0L,SB_WHIDTH,swp.cy, SWP_SIZE | SWP_MOVE );
           WinSetWindowPos(pCtrl->hwndArea,NULLHANDLE,0L,0L,swp.cx-SB_WHIDTH,swp.cy, SWP_SIZE );
           WinQueryWindowPos(pCtrl->hwndCtrl,&swpItem);
           WinSetWindowPos(pCtrl->hwndCtrl, NULLHANDLE,
                           0L, 0L, swp.cx-SB_WHIDTH, swpItem.cy,
                           SWP_SIZE );
         //WinSetWindowPos(pCtrl->hwndCtrl, NULLHANDLE,
         //                0L, -SCROLL_HIGH+swp.cy, swp.cx-SB_WHIDTH, SCROLL_HIGH,
         //                SWP_MOVE | SWP_SIZE );
           ImpostaDimensioni( pCtrl );
           for (idNextItem=0;idNextItem<pCtrl->lastItem;idNextItem++) {
             hwndItem=WinWindowFromID(pCtrl->hwndCtrl,idNextItem);
             WinQueryWindowPos(hwndItem,&swpItem);
             WinSetWindowPos(hwndItem, NULLHANDLE,
                             0L, 0L, swp.cx-SB_WHIDTH-1, swpItem.cy,
                             SWP_SIZE );
           }
         }
      break;

      case WM_VSCROLL:
         {
           SHORT position;
           pCtrl = WinQueryWindowPtr(hwnd, 0);
           switch (SHORT2FROMMP(mp2)) {
             case SB_LINEUP:
                pCtrl->deltay=pCtrl->deltay+SB_INCREMENTO;
                position=(USHORT)((pCtrl->maxy-pCtrl->deltay)/SB_INCREMENTO)+1;
                WinSendMsg( pCtrl->hwndSB, SBM_SETPOS, MPFROMSHORT(position), 0L);
             break;
             case SB_LINEDOWN:
                pCtrl->deltay=pCtrl->deltay-SB_INCREMENTO;
                position=(USHORT)((pCtrl->maxy-pCtrl->deltay)/SB_INCREMENTO)+1;
                WinSendMsg( pCtrl->hwndSB, SBM_SETPOS, MPFROMSHORT(position), 0L);
             break;
             case SB_PAGEUP:
                pCtrl->deltay=pCtrl->deltay+pCtrl->pagey;
                position=(USHORT)((pCtrl->maxy-pCtrl->deltay)/SB_INCREMENTO)+1;
                WinSendMsg( pCtrl->hwndSB, SBM_SETPOS, MPFROMSHORT(position), 0L);
             break;
             case SB_PAGEDOWN:
                pCtrl->deltay=pCtrl->deltay-pCtrl->pagey;
                position=(USHORT)((pCtrl->maxy-pCtrl->deltay)/SB_INCREMENTO)+1;
                WinSendMsg( pCtrl->hwndSB, SBM_SETPOS, MPFROMSHORT(position), 0L);
             break;
             case SB_SLIDERPOSITION:
                if (SHORT1FROMMP(mp2)==1)
                   pCtrl->deltay=pCtrl->maxy;
                else
                   pCtrl->deltay=pCtrl->maxy-((double)(SHORT1FROMMP(mp2))*SB_INCREMENTO);
             break;
             case SB_SLIDERTRACK:
                if (SHORT1FROMMP(mp2)==1)
                   pCtrl->deltay=pCtrl->maxy;
                else
                   pCtrl->deltay=pCtrl->maxy-((double)(SHORT1FROMMP(mp2))*SB_INCREMENTO);
             break;
           }
           if (pCtrl->deltay>pCtrl->maxy) pCtrl->deltay=pCtrl->maxy;
           if (pCtrl->deltay<pCtrl->miny) pCtrl->deltay=pCtrl->miny;
           WinSetWindowPos(pCtrl->hwndCtrl, NULLHANDLE, 0, -pCtrl->deltay, 0L, 0L, SWP_MOVE);
         }
         break;

      case WM_DESTROY:
         DosFreeMem(WinQueryWindowPtr(hwnd, 0));
      break;

      case WM_CSOJ2I0J_INSERTITEM:
        {
           HWND hwndItem;
           pCtrl=WinQueryWindowPtr(hwnd, 0);
           hwndItem = WinCreateWindow( pCtrl->hwndCtrl,
                                       "ClItem",
                                       "",
                                       WS_VISIBLE,
                                       (LONG)0,
                                       (LONG)SCROLL_HIGH-pCtrl->cy-SIZE_CLOSE,
                                       (LONG)pCtrl->swpCtrl.cx-1,
                                       (LONG)SIZE_CLOSE,
                                       pCtrl->hwndCtrl,
                                       HWND_TOP,
                                       pCtrl->lastItem,
                                       NULL, NULL);
           pCtrl->lastItem++;
           pCtrl->cy+=SIZE_CLOSE;
           WinSendMsg(hwndItem,WM_SETITEMCONTENT,mp1,0L);
           WinSendMsg(hwnd,WM_HIGHTUPDATED,0L,0L);
        }
        return(MRFROMLONG(pCtrl->lastItem-1));
      break;

      case WM_CSOJ2I0J_SETITEM:
        {
           HWND hwndItem;
           PPRIVATESTRUCTITEM pItem;
           pCtrl=WinQueryWindowPtr(hwnd, 0);
           hwndItem = WinWindowFromID( pCtrl->hwndCtrl, LONGFROMMP(mp2) );
           pItem=WinQueryWindowPtr(hwndItem, 0);
           if (pItem->fStatus) ApriChiudi(pItem);
           WinSendMsg(hwndItem,WM_SETITEMCONTENT,mp1,0L);
           WinInvalidateRect(hwndItem,NULL,TRUE);
        }
        return(MRESULT)0;
      break;

      case WM_HIGHTUPDATED:
        ImpostaDimensioni( WinQueryWindowPtr(hwnd, 0) );
        return(MRESULT)0;
      break;

      case WM_PAINT:
         {
            static RECTL rc;
            static HPS hps;
            hps = WinBeginPaint( hwnd, (HPS)NULL, &rc );
            WinFillRect(hps, &rc, CLR_PALEGRAY);
            WinEndPaint( hps );
            return(MRESULT)0;
        }
      break;
   }
   return ( WinDefWindowProc(hwnd, msg, mp1, mp2) );
}

MRESULT EXPENTRY Ctrl_Proc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 )
{
   switch( msg )
   {
      case WM_PAINT:
         {
            static RECTL rc;
            static HPS hps;
            hps = WinBeginPaint( hwnd, (HPS)NULL, &rc );
            WinFillRect(hps, &rc, CLR_PALEGRAY);
            WinEndPaint( hps );
            return(MRESULT)0;
        }
      break;
   }
   return ( WinDefWindowProc(hwnd, msg, mp1, mp2) );
}

MRESULT EXPENTRY Item_Proc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 )
{
   PPRIVATESTRUCTITEM pItem;

   switch( msg )
   {
      case WM_CREATE:
      {
        PCREATESTRUCT pCreateStruct;
        pCreateStruct=PVOIDFROMMP(mp2);
        DosAllocMem((PVOID)&pItem, sizeof(PRIVATESTRUCTITEM), PAG_COMMIT | PAG_READ | PAG_WRITE);
        memset(pItem,0,sizeof(PRIVATESTRUCTITEM));
        WinSetWindowPtr(hwnd, 0, pItem);
        pItem->hwndItem=hwnd;
        pItem->fStatus=FALSE;
        pItem->sizeContent=0;
        pItem->hightRow=0;
        pItem->hbmp=hbmpDown;
      }
      break;

      case WM_DESTROY:
        DosFreeMem(WinQueryWindowPtr(hwnd, 0));
      break;

      case WM_BUTTON1CLICK:
        ApriChiudi(WinQueryWindowPtr(hwnd, 0));
      break;

      case WM_SETITEMCONTENT:
        {
           HPS hps;
           RECTL rc;
           PCSOJ2I0J_INSERTITEM pInsert;
           pInsert=PVOIDFROMMP(mp1);
           pItem=WinQueryWindowPtr(hwnd, 0);
           strncpy((PCHAR)pItem->szTitle,pInsert->szTitle,sizeof(pItem->szTitle));
           strncpy((PCHAR)pItem->szContent,pInsert->szContent,sizeof(pItem->szContent));
           pItem->hbmp=pInsert->hbmp;
           pItem->hightRow=0;
           pItem->numRow=0;
           pItem->sizeContent=0;
           if (strlen((PCHAR)pItem->szContent)>0) {
              LONG hTotalDrawn=0;
              LONG hDrawn=-1;
              POINTL aptl[TXTBOX_COUNT];
              hps=WinGetPS(pItem->hwndItem);
              while (hDrawn!=0) {
                 WinQueryWindowRect( hwnd , &rc );
                 rc.yTop=65000;
                 rc.yBottom=0;
                 hDrawn=WinDrawText(hps,-1L, (PCHAR)pItem->szContent+hTotalDrawn, &rc, 0L,0L,
                                    DT_WORDBREAK | DT_TOP | DT_LEFT | DT_TEXTATTRS | DT_QUERYEXTENT );
                 hTotalDrawn += hDrawn;
                 pItem->sizeContent=pItem->sizeContent+(rc.yTop-rc.yBottom);
                 pItem->hightRow=rc.yTop-rc.yBottom;
                 pItem->numRow++;
              }
              WinReleasePS(hps);
           } else {
              pItem->sizeContent=0;
           }
        }
      break;

      case WM_PAINT:
        DrawItem( WinQueryWindowPtr(hwnd, 0) );
        return(MRESULT)0;
      break;
   }
   return ( WinDefWindowProc(hwnd, msg, mp1, mp2) );
}

VOID DrawItem( PPRIVATESTRUCTITEM pItem )
{
   RECTL rc,rcFreccia,rcBmp,rcTitle,rcContent;
   HPS hps;
   POINTL ptl;

   hps = WinBeginPaint( pItem->hwndItem, (HPS)NULL, &rc );
   WinFillRect( hps, &rc, CLR_PALEGRAY );
   WinQueryWindowRect( pItem->hwndItem , &rc );
   rc.xRight--;
   rc.yTop--;

   /* Disegno titolo *******************************************************/
   rcTitle.xLeft   = rc.xLeft;
   rcTitle.xRight  = rc.xRight-(SIZE_CLOSE/2);
   rcTitle.yTop    = rc.yTop;
   rcTitle.yBottom = rcTitle.yTop - SIZE_CLOSE + 1;

   GpiSetColor(hps,CLR_WHITE);
   ptl.x = rcTitle.xLeft;
   ptl.y = rcTitle.yBottom;
   GpiMove(hps, &ptl);
   ptl.y = rcTitle.yTop;
   GpiLine(hps, &ptl);
   ptl.x = rcTitle.xRight;
   GpiLine(hps, &ptl);

   GpiSetColor(hps,CLR_DARKGRAY);
   ptl.x = rcTitle.xRight;
   ptl.y = rcTitle.yTop-1;
   GpiMove(hps, &ptl);
   ptl.y = rcTitle.yBottom;
   GpiLine(hps, &ptl);
   ptl.x = rcTitle.xLeft+1;
   GpiLine(hps, &ptl);

   rcBmp.xLeft   = rcTitle.xLeft + 4;
   rcBmp.xRight  = rcBmp.xLeft   + BMP_CX;
   rcBmp.yBottom = rcTitle.yBottom + 1;
   rcBmp.yTop    = rcBmp.yBottom + BMP_CY;
   WinDrawBitmap( hps, pItem->hbmp,  (PRECTL)NULL , (PPOINTL)&rcBmp, 0L , 0L , DBM_STRETCH );

   rcFreccia.xLeft   = rcTitle.xRight - FRECCIA_CX - 6;
   rcFreccia.xRight  = rcFreccia.xLeft   + FRECCIA_CX;
   rcFreccia.yBottom = (((rcTitle.yTop - rcTitle.yBottom) - FRECCIA_CY) / 2) + rcTitle.yBottom;
   rcFreccia.yTop    = rcFreccia.yBottom + FRECCIA_CY;
   if (pItem->numRow>0) {
      if (pItem->fStatus)
         WinDrawBitmap( hps, hbmpDown,  (PRECTL)NULL , (PPOINTL)&rcFreccia, 0L , 0L , DBM_NORMAL );
      else
         WinDrawBitmap( hps, hbmpRight, (PRECTL)NULL , (PPOINTL)&rcFreccia, 0L , 0L , DBM_NORMAL );
   }

   rcTitle.xRight = rcFreccia.xLeft - 2;
   rcTitle.xLeft =  rcBmp.xRight + 5;

   WinDrawText(hps,-1L,(PCHAR)pItem->szTitle, &rcTitle, CLR_BLACK , 0L, DT_VCENTER | DT_LEFT);

   /* Disegno Contenuto ****************************************************/
   if (pItem->sizeContent>0 && pItem->fStatus) {
      LONG hTotalDrawn=0;
      LONG hDrawn=-1;

      rcContent.xLeft   = rc.xLeft  + (SIZE_CLOSE/2);
      rcContent.xRight  = rc.xRight - 3;
      rcContent.yTop    = rc.yTop - SIZE_CLOSE - 2;
      rcContent.yBottom = rc.yBottom + 3;

      //WinFillRect( hps, &rcContent, -47 );
      WinFillRect( hps, &rcContent, CLR_WHITE );

      GpiSetColor(hps,CLR_DARKGRAY);
      ptl.x = rcContent.xLeft;
      ptl.y = rcContent.yBottom;
      GpiMove(hps, &ptl);
      ptl.y = rcContent.yTop;
      GpiLine(hps, &ptl);
      ptl.x = rcContent.xRight;
      GpiLine(hps, &ptl);

      //GpiSetColor(hps,CLR_WHITE);
      ptl.x = rcContent.xRight;
      ptl.y = rcContent.yTop-1;
      GpiMove(hps, &ptl);
      ptl.y = rcContent.yBottom;
      GpiLine(hps, &ptl);
      ptl.x = rcContent.xLeft+1;
      GpiLine(hps, &ptl);

      rcContent.xLeft   +=2;
      rcContent.xRight  -=2;
      rcContent.yTop    -=2;
      rcContent.yBottom +=2;

      while (hDrawn!=0) {
         hDrawn=WinDrawText(hps,-1L, (PCHAR)pItem->szContent+hTotalDrawn, &rcContent, CLR_BLACK ,0L,
                            DT_WORDBREAK | DT_TOP | DT_LEFT );
         rcContent.yTop -= pItem->hightRow;
         hTotalDrawn += hDrawn;
      }

   }
   WinEndPaint( hps );
}

VOID ApriChiudi( PPRIVATESTRUCTITEM pItem )
{
  ULONG  idNextItem;
  HWND   hwndOwner;
  HWND   hwndItem;
  SWP    swpItem,swpOwner;
  PPRIVATESTRUCTCTRL pCtrl;
  RECTL  rcScroll;
  double k,l;
  ULONG  moved=0;

  if (pItem->numRow<=0) {
     //WinAlarm(HWND_DESKTOP,WA_WARNING);
     DosBeep(80,90);
     return;
  }

  hwndOwner=WinQueryWindow(pItem->hwndItem,QW_OWNER);
  pCtrl=WinQueryWindowPtr(hwndOwner, 0);
  k=((double)pItem->sizeContent/(double)(pItem->numRow*VELOCITA));
  WinQueryWindowPos(pItem->hwndItem,&swpItem);

  WinQueryWindowPos(hwndOwner,&swpOwner);

  rcScroll.xLeft=0;
  rcScroll.xRight=swpOwner.cx;
  rcScroll.yBottom=1;

  if (!pItem->fStatus) {
     rcScroll.yTop=swpItem.y;
     for (l=0.0;l<=pItem->sizeContent;l+=k) {
        WinScrollWindow(hwndOwner, (LONG)0, -(LONG)(l-moved),
                        (PRECTL)&rcScroll, (PRECTL)NULL,
                        (HRGN)NULLHANDLE, (PRECTL)NULL, SW_INVALIDATERGN);
        moved=moved+(l-moved);
     }
     WinLockWindowUpdate(HWND_DESKTOP,hwndOwner);
     idNextItem=WinQueryWindowUShort(pItem->hwndItem, QWS_ID)+1;
     for (;idNextItem<pCtrl->lastItem;idNextItem++) {
        hwndItem=WinWindowFromID(hwndOwner,idNextItem);
        WinQueryWindowPos(hwndItem,&swpItem);
        WinSetWindowPos(hwndItem, NULLHANDLE,
                        swpItem.x, swpItem.y-pItem->sizeContent, 0L, 0L,
                        SWP_MOVE | SWP_NOADJUST);
     }
     WinQueryWindowPos(pItem->hwndItem,&swpItem);
     WinSetWindowPos(pItem->hwndItem, NULLHANDLE,
                     swpItem.x, swpItem.y-pItem->sizeContent,
                     swpItem.cx,swpItem.cy+pItem->sizeContent,
                     SWP_MOVE | SWP_SIZE);
     pItem->fStatus=TRUE;
     pCtrl->cy+=pItem->sizeContent;
  } else {
     rcScroll.yTop=swpItem.y+swpItem.cy-SIZE_CLOSE-7;
     for (l=0.0;l<=pItem->sizeContent;l+=k) {
        WinScrollWindow(hwndOwner, (LONG)0, (LONG)(l-moved),
                        (PRECTL)&rcScroll, (PRECTL)NULL,
                        (HRGN)NULLHANDLE, (PRECTL)NULL, SW_INVALIDATERGN);
        moved=moved+(l-moved);
     }
     WinLockWindowUpdate(HWND_DESKTOP,hwndOwner);
     idNextItem=WinQueryWindowUShort(pItem->hwndItem, QWS_ID)+1;
     for (;idNextItem<pCtrl->lastItem;idNextItem++) {
        hwndItem=WinWindowFromID(hwndOwner,idNextItem);
        WinQueryWindowPos(hwndItem,&swpItem);
        WinSetWindowPos(hwndItem, NULLHANDLE,
                        swpItem.x, swpItem.y+pItem->sizeContent, 0L, 0L,
                        SWP_MOVE | SWP_NOADJUST);
     }
     WinQueryWindowPos(pItem->hwndItem,&swpItem);
     WinSetWindowPos(pItem->hwndItem, NULLHANDLE,
                     swpItem.x, swpItem.y+pItem->sizeContent,
                     swpItem.cx,swpItem.cy-pItem->sizeContent,
                     SWP_MOVE | SWP_SIZE);
     pItem->fStatus=FALSE;
     pCtrl->cy-=pItem->sizeContent;
  }
  WinLockWindowUpdate(HWND_DESKTOP, NULLHANDLE);
  WinInvalidateRect(pItem->hwndItem,NULL,TRUE);
  WinSendMsg(pCtrl->hwndClass,WM_HIGHTUPDATED,0L,0L);
  return;
}

VOID ImpostaDimensioni( PPRIVATESTRUCTCTRL pCtrl )
{
  double  iArea;
  SHORT   position;

  WinQueryWindowPos(pCtrl->hwndArea,&pCtrl->swpArea);
  WinQueryWindowPos(pCtrl->hwndCtrl,&pCtrl->swpCtrl);

  if (pCtrl->cy > pCtrl->swpArea.cy) {
    WinShowWindow(pCtrl->hwndSB,TRUE);
  } else {
    WinShowWindow(pCtrl->hwndSB,FALSE);
  }

  pCtrl->maxy = pCtrl->swpCtrl.cy - pCtrl->swpArea.cy;
  pCtrl->miny = pCtrl->swpCtrl.cy - pCtrl->cy;

  if (pCtrl->deltay<pCtrl->miny || pCtrl->deltay>pCtrl->maxy) {
     LONG miny;
     if (pCtrl->cy<pCtrl->swpArea.cy) {
        miny=-SCROLL_HIGH+pCtrl->swpArea.cy;
     } else {
        miny=-SCROLL_HIGH+pCtrl->cy;
     }
     WinSetWindowPos(pCtrl->hwndCtrl, NULLHANDLE,
                     0L, miny, pCtrl->swpArea.cx-SB_WHIDTH, SCROLL_HIGH,
                     SWP_MOVE );
     WinQueryWindowPos(pCtrl->hwndCtrl,&pCtrl->swpCtrl);
     pCtrl->maxy = pCtrl->swpCtrl.cy - pCtrl->swpArea.cy;
     pCtrl->miny = pCtrl->swpCtrl.cy - pCtrl->cy;
     pCtrl->deltay = pCtrl->maxy;
  }

  if (pCtrl->swpArea.cy>150) pCtrl->pagey=150;
                 else pCtrl->pagey=pCtrl->swpArea.cy;

  iArea=(double)((((double)pCtrl->maxy-(double)pCtrl->miny)/
                     (double)SB_INCREMENTO)+0.4999);

  WinSendMsg( pCtrl->hwndSB, SBM_SETTHUMBSIZE,
              MPFROM2SHORT((USHORT)pCtrl->swpArea.cy, (USHORT)pCtrl->cy),0L );

  position=(USHORT)((pCtrl->maxy-pCtrl->deltay)/SB_INCREMENTO)+1;

  WinSendMsg( pCtrl->hwndSB, SBM_SETSCROLLBAR,
              MPFROMSHORT(position), MPFROM2SHORT(1,(USHORT)iArea+1));
}
