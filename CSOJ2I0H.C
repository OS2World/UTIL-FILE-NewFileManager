#define INCL_WIN
#define INCL_GPI
#define INCL_DOS

#include <os2.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "csoj2i0h.h"

/* Dimensioni di adattamento (Costanti) **************************************/
#define FRECCIA_Y         4
#define FRECCIA_CX        8
#define FRECCIA_CY        7

#define EXTRA_ALTEZZA_BTN 6
#define ID_FBAR           101
#define ID_CONTAINER      102
#define ID_BTN_RESIZE_ALL 103
#define SIZE_ARROW        4
#define SIZE_TRACK        2
#define MIN_SIZE_ITEM     FRECCIA_CX + 2

#define MAX_BTN           30
#define CFA_OLD_OWNERDRAW 0x01000000L

typedef struct {
  HWND   hwndCont;
  ULONG  offset;
  ULONG  tipo;
  BOOL   sort;
} SORTSTRUCT;
typedef SORTSTRUCT* PSORTSTRUCT;

typedef struct _PRIVATESTRUCT {
   HWND  hwndClass;
   ULONG idClass;
   HWND  hwndArea;
   HWND  hwndFBar;
   HWND  hwndCont;
   HWND  hwndResizeAll;
   LONG  ulId;
   LONG  ulOldId;
   BOOL  sort;  // TRUE=Descending FALSE=Ascending
   BOOL  fPrimaVolta;
   FONTMETRICS fmMetrics;
   ULONG hightBtn;
   BOOL  fViewDetail;
   BOOL  fOwnerDraw;

   PFIELDINFO pClassFieldInfo[MAX_BTN];
   ULONG      size[MAX_BTN];
   ULONG      start[MAX_BTN];
} PRIVATESTRUCT;
typedef PRIVATESTRUCT *PPRIVATESTRUCT;

#define RESIZE_ALL_ITEM   1
#define RESIZE_ITEM       2
#define RESIZE_ITEM_DELTA 3
#define RESIZE_ITEM_CX    4

/* Window Proc ***************************************************************/
MRESULT EXPENTRY CSOJ2I0H_Proc    ( HWND, ULONG, MPARAM, MPARAM );
BOOL    EXPENTRY CSOJ2I0H_REGISTRA( HAB );
MRESULT EXPENTRY FBarProc         ( HWND, ULONG, MPARAM, MPARAM );
MRESULT EXPENTRY BtnProc          ( HWND, ULONG, MPARAM, MPARAM );
MRESULT EXPENTRY EnhContProc      ( HWND, ULONG, MPARAM, MPARAM );

/* Funzioni interne **********************************************************/
VOID    EXPENTRY ScrollaFBar      ( PPRIVATESTRUCT, LONG );
VOID    EXPENTRY DrawButton       ( PPRIVATESTRUCT, PUSERBUTTON );
VOID    EXPENTRY DrawButtonReload ( PUSERBUTTON );
VOID    EXPENTRY CreaButton       ( PPRIVATESTRUCT, PFIELDINFO, PFIELDINFOINSERT );
VOID    EXPENTRY CancButton       ( PPRIVATESTRUCT, PFIELDINFO, PFIELDINFOINSERT );
ULONG   EXPENTRY CalcolaLenString ( HPS, PCHAR );
BOOL    EXPENTRY StoreRestorePos  ( PPRIVATESTRUCT, ULONG, HINI, PSZ, PSZ );
BOOL    EXPENTRY SortContainer    ( PPRIVATESTRUCT, ULONG, BOOL );
SHORT   EXPENTRY pfnCompare       ( PRECORDCORE, PRECORDCORE, PVOID );
LONG    EXPENTRY TrackRect        ( PPRIVATESTRUCT, HWND, SWP, USHORT );
VOID    EXPENTRY DimensionaItem   ( PPRIVATESTRUCT, ULONG, ULONG, LONG );
ULONG   EXPENTRY CalcolaWidth     ( PPRIVATESTRUCT, ULONG );
VOID    EXPENTRY CalcolaSizeItem  ( PPRIVATESTRUCT, POWNERITEM, PRECORDCORE, PFIELDINFO );

/* Static (non istanziate) ***************************************************/
static HAB         hab;
static CLASSINFO   clsiBtn;
static PFNWP       pfnwpCONTAINER;
static HPOINTER    hptrWait=NULLHANDLE;
static HMODULE     hmod;
static HBITMAP     hbmpDown=NULLHANDLE;
static HBITMAP     hbmpUp=NULLHANDLE;
static HBITMAP     hbmpReload=NULLHANDLE;
static HBITMAP     hbmpReloadR=NULLHANDLE;
static HPOINTER    hptrSepar=NULLHANDLE;
static COUNTRYINFO CtryInfo   = {0};

BOOL CSOJ2I0H_REGISTRA( HAB lochab )
{
   CLASSINFO   clsiTmp;
   COUNTRYCODE Country    = {0};
   ULONG       ulInfoLen  = 0;

   hab=lochab;

   /* Se classe gi… registrata */
   if( WinQueryClassInfo( hab,
                          (PSZ)"CSOJ2I0H",
                          &clsiTmp )==TRUE ) {
      return( TRUE );
   }

   if( ! WinRegisterClass( hab,
                           (PSZ)"CSOJ2I0H",
                           (PFNWP)CSOJ2I0H_Proc,
                           //CS_SIZEREDRAW | CS_SYNCPAINT,
                           CS_SIZEREDRAW,
                           4L ) ) {
      return( FALSE );
   }

   DosQueryModuleHandle("CSOJ2I0H", &hmod);

   WinQueryClassInfo( hab, WC_BUTTON, &clsiBtn );

   /* Class Floating Bar */
   WinRegisterClass( hab, "FBar", FBarProc, 0L, 4L );

   /* Classe Button */
   WinRegisterClass( hab, "Btn", BtnProc, CS_SIZEREDRAW, 4L );

   hptrWait = WinQuerySysPointer(HWND_DESKTOP, SPTR_WAIT, FALSE);

   DosQueryCtryInfo(sizeof(CtryInfo), &Country, &CtryInfo, &ulInfoLen);

   hptrSepar=WinQuerySysPointer(HWND_DESKTOP, SPTR_SIZEWE, FALSE);

   return( TRUE );
}

MRESULT EXPENTRY CSOJ2I0H_Proc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 )
{
   MRESULT mres;
   PPRIVATESTRUCT pClass;

   switch( msg )
   {
      case WM_CREATE:
      {
        PCREATESTRUCT pCreateStruct;
        HPS hps=NULLHANDLE;

        pCreateStruct=PVOIDFROMMP(mp2);
        DosAllocMem((PVOID)&pClass, sizeof(PRIVATESTRUCT), PAG_COMMIT | PAG_READ | PAG_WRITE);
        memset(pClass,0,sizeof(PRIVATESTRUCT));
        WinSetWindowPtr(hwnd, 0, pClass);

        pClass->hwndClass=hwnd;
        pClass->ulId=-1;
        pClass->sort=TRUE;
        pClass->fPrimaVolta=FALSE;
        pClass->idClass=pCreateStruct->id;

        hps = WinGetPS( hwnd );
        GpiQueryFontMetrics (hps, sizeof (FONTMETRICS), &pClass->fmMetrics);
        pClass->hightBtn  =pClass->fmMetrics.lMaxBaselineExt+EXTRA_ALTEZZA_BTN;
        WinReleasePS(hps);

        if (hbmpDown==NULLHANDLE) {
           hps=WinGetPS(hwnd);
           hbmpDown    = GpiLoadBitmap(hps, hmod, ID_BMP_DOWN,     0UL, 0UL);
           hbmpUp      = GpiLoadBitmap(hps, hmod, ID_BMP_UP,       0UL, 0UL);
           hbmpReload  = GpiLoadBitmap(hps, hmod, ID_BMP_RELOAD,   0UL, 0UL);
           hbmpReloadR = GpiLoadBitmap(hps, hmod, ID_BMP_RELOAD_R, 0UL, 0UL);
           WinReleasePS(hps);
        }

        pClass->hwndArea = WinCreateWindow( hwnd, WC_STATIC,
                                    "", WS_VISIBLE | WS_CLIPCHILDREN,
                                    (ULONG)0L,
                                    (ULONG)pCreateStruct->cy - pClass->hightBtn,
                                    (ULONG)pCreateStruct->cx - WinQuerySysValue(HWND_DESKTOP,SV_CXVSCROLL),
                                    (ULONG)pClass->hightBtn,
                                    hwnd,
                                    HWND_TOP,
                                    ID_FBAR,
                                    NULL, NULL);

        pClass->hwndResizeAll = WinCreateWindow( hwnd, WC_BUTTON,
                         "",
                         WS_VISIBLE | BS_NOPOINTERFOCUS | BS_USERBUTTON | BS_PUSHBUTTON,
                         (ULONG)pCreateStruct->cx-WinQuerySysValue(HWND_DESKTOP,SV_CXVSCROLL),
                         (ULONG)pCreateStruct->cy-pClass->hightBtn,
                         (ULONG)WinQuerySysValue(HWND_DESKTOP,SV_CXVSCROLL),
                         (ULONG)pClass->hightBtn,
                         hwnd,
                         HWND_TOP,
                         ID_BTN_RESIZE_ALL,
                         NULL, NULL);

        pClass->hwndFBar = WinCreateWindow( pClass->hwndArea, "FBar",
                                    "", WS_VISIBLE,
                                    (ULONG)0L,
                                    (ULONG)0L,
                                    (ULONG)1500,
                                    (ULONG)pClass->hightBtn,
                                    pClass->hwndArea,
                                    HWND_TOP,
                                    ID_FBAR,
                                    NULL, NULL);

        WinSetWindowPtr(pClass->hwndFBar, 0, pClass);

        pClass->hwndCont = WinCreateWindow( hwnd, WC_CONTAINER,
                                    "", pCreateStruct->flStyle | WS_SYNCPAINT,
                                    (ULONG)0L,
                                    (ULONG)0L,
                                    (ULONG)pCreateStruct->cx,
                                    (ULONG)pCreateStruct->cy-pClass->hightBtn,
                                    hwnd,
                                    HWND_TOP,
                                    ID_CONTAINER,
                                    NULL, NULL);
        WinSetWindowPtr(pClass->hwndCont, 0, pClass);
        pfnwpCONTAINER=WinSubclassWindow( pClass->hwndCont,(PFNWP)EnhContProc );
      }
      break;

      case WM_SIZE:
         {
           SWP  swp;
           RECTL rc;
           pClass = WinQueryWindowPtr(hwnd, 0);
           WinQueryWindowPos(pClass->hwndClass,&swp);
           if (pClass->fViewDetail) {
              WinSetWindowPos(pClass->hwndCont,NULLHANDLE,
                              0L,0L,
                              swp.cx,swp.cy-pClass->hightBtn,SWP_SIZE);
              WinSetWindowPos(pClass->hwndArea,NULLHANDLE,
                              0L,swp.cy-pClass->hightBtn,
                              swp.cx-WinQuerySysValue(HWND_DESKTOP,SV_CXVSCROLL),pClass->hightBtn,SWP_MOVE | SWP_SIZE);
              WinSetWindowPos(pClass->hwndResizeAll,NULLHANDLE,
                              (ULONG)swp.cx-WinQuerySysValue(HWND_DESKTOP,SV_CXVSCROLL),
                              (ULONG)swp.cy-pClass->hightBtn,
                              (ULONG)WinQuerySysValue(HWND_DESKTOP,SV_CXVSCROLL),
                              (ULONG)pClass->hightBtn,
                              SWP_MOVE | SWP_SIZE );
              WinSendMsg(pClass->hwndCont, CM_QUERYVIEWPORTRECT, MPFROMP(&rc), MPFROM2SHORT( CMA_WORKSPACE, FALSE ));
              WinSetWindowPos(pClass->hwndFBar, NULLHANDLE, -rc.xLeft, 0L, 0L, 0L, SWP_MOVE );
           } else {
              WinSetWindowPos(pClass->hwndCont,NULLHANDLE,
                              0L,0L,
                              swp.cx,swp.cy,SWP_SIZE);
           }
         }
      break;

      case WM_CONTROL:
         pClass = WinQueryWindowPtr(hwnd, 0);
         switch ( SHORT2FROMMP( mp1 ) ) {
           case CN_SCROLL:
              {
                 PNOTIFYSCROLL pScroll;
                 pScroll=PVOIDFROMMP(mp2);
                 if (pScroll->fScroll & CMA_HORIZONTAL) {
                    ScrollaFBar( pClass, -pScroll->lScrollInc );
                 }
              }
           break;

           case BN_PAINT:
              DrawButtonReload( MPFROMP(mp2) );
              return(MRESULT)0;
           break;

         }

         if (SHORT1FROMMP( mp1 )==ID_CONTAINER) {
            return( WinSendMsg( WinQueryWindow(hwnd,QW_OWNER),
                                WM_CONTROL,
                                MPFROM2SHORT(pClass->idClass,SHORT2FROMMP(mp1)),
                                mp2));
         }
      break;

      case CM_ALLOCDETAILFIELDINFO     :
      case CM_FREEDETAILFIELDINFO      :

      case CM_INVALIDATEDETAILFIELDINFO:
      case CM_QUERYDETAILFIELDINFO     :

      case CM_INSERTDETAILFIELDINFO    :
      case CM_REMOVEDETAILFIELDINFO    :

      case CM_ALLOCRECORD              :
      case CM_ARRANGE                  :
      case CM_ERASERECORD              :
      case CM_FILTER                   :
      case CM_FREERECORD               :
      case CM_HORZSCROLLSPLITWINDOW    :
      case CM_INSERTRECORD             :
      case CM_INVALIDATERECORD         :
      case CM_PAINTBACKGROUND          :
      case CM_QUERYCNRINFO             :
      case CM_QUERYDRAGIMAGE           :
      case CM_QUERYRECORD              :
      case CM_QUERYRECORDEMPHASIS      :
      case CM_QUERYRECORDFROMRECT      :
      case CM_QUERYRECORDRECT          :
      case CM_QUERYVIEWPORTRECT        :
      case CM_REMOVERECORD             :
      case CM_SCROLLWINDOW             :
      case CM_SEARCHSTRING             :
      case CM_SETCNRINFO               :
      case CM_SETRECORDEMPHASIS        :
      case CM_SORTRECORD               :
      case CM_OPENEDIT                 :
      case CM_CLOSEEDIT                :
      case CM_COLLAPSETREE             :
      case CM_EXPANDTREE               :
      case CM_QUERYRECORDINFO          :
      case CM_INSERTRECORDARRAY        :
      case CM_MOVETREE                 :
      case CM_SETTEXTVISIBILITY        :
         pClass = WinQueryWindowPtr(hwnd, 0);

         switch (msg) {
            case CM_SETCNRINFO:
               {
                  PCNRINFO pInfo = MPFROMP(mp1);
                  if (pInfo->flWindowAttr & CV_DETAIL) {
                     pClass->fViewDetail=TRUE;
                     pInfo->flWindowAttr = pInfo->flWindowAttr & ~CA_DETAILSVIEWTITLES;
                  } else {
                     pClass->fViewDetail=FALSE;
                  }
                  if (pInfo->flWindowAttr & CA_OWNERDRAW) {
                     pClass->fOwnerDraw=TRUE;
                  } else {
                     pClass->fOwnerDraw=FALSE;
                  }
                  WinShowWindow(pClass->hwndArea,pClass->fViewDetail);
                  WinShowWindow(pClass->hwndFBar,pClass->fViewDetail);
                  WinShowWindow(pClass->hwndResizeAll,pClass->fViewDetail);
                  WinSendMsg(hwnd,WM_SIZE,0L,0L);
               }
            break;

            case CM_INSERTRECORD:
               pClass->ulOldId=pClass->ulId;
               pClass->ulId=-1;
               WinInvalidateRect(WinWindowFromID(pClass->hwndFBar,pClass->ulOldId),NULL,TRUE);
               pClass->sort=TRUE;
            break;

            case CM_INVALIDATEDETAILFIELDINFO:
               {
                  RECTL rc;
                  mres=(MRESULT)(WinSendMsg(pClass->hwndCont,msg,mp1,mp2));
                  WinSendMsg(pClass->hwndCont, CM_QUERYVIEWPORTRECT, MPFROMP(&rc), MPFROM2SHORT( CMA_WORKSPACE, FALSE ));
                  WinSetWindowPos(pClass->hwndFBar, NULLHANDLE, -rc.xLeft, 0L, 0L, 0L, SWP_MOVE);
                  return(mres);
               }
            break;

            case CM_INVALIDATERECORD:
               {
                  LONG delta1,delta2,deltatot;
                  RECTL rc;
                  if (pClass->fPrimaVolta) {
                     pClass->fPrimaVolta=FALSE;
                     DimensionaItem( pClass, RESIZE_ALL_ITEM, 0L, 0L);
                  }
                  WinSendMsg(pClass->hwndCont, CM_QUERYVIEWPORTRECT, MPFROMP(&rc), MPFROM2SHORT( CMA_WORKSPACE, FALSE ));
                  delta1=rc.xLeft;
                  mres=(MRESULT)(WinSendMsg(pClass->hwndCont,msg,mp1,mp2));
                  WinSendMsg(pClass->hwndCont, CM_QUERYVIEWPORTRECT, MPFROMP(&rc), MPFROM2SHORT( CMA_WORKSPACE, FALSE ));
                  delta2=rc.xLeft;
                  deltatot=delta1-delta2;
                  if (deltatot!=0)
                     WinSendMsg(pClass->hwndCont, CM_SCROLLWINDOW, MPFROMSHORT( CMA_HORIZONTAL ), MPFROMLONG( deltatot ));
                  ScrollaFBar( pClass, deltatot );
                  return(mres);
               }
            break;

            case CM_INSERTDETAILFIELDINFO:
               {
                 PFIELDINFO pFieldInfo;
                 pFieldInfo=PVOIDFROMMP(mp1);
                 while (pFieldInfo) {
                   if (pFieldInfo->flData & CFA_OWNER) {
                     pFieldInfo->flData=pFieldInfo->flData | CFA_OWNER | CFA_OLD_OWNERDRAW;
                   } else {
                     pFieldInfo->flData=pFieldInfo->flData | CFA_OWNER;
                   }
                   pFieldInfo=pFieldInfo->pNextFieldInfo;
                 }
               }
            case CM_REMOVEDETAILFIELDINFO:
               mres=(MRESULT)(WinSendMsg(pClass->hwndCont,msg,mp1,mp2));
               CancButton( pClass, (PFIELDINFO)mp1, (PFIELDINFOINSERT)mp2 );
               CreaButton( pClass, (PFIELDINFO)mp1, (PFIELDINFOINSERT)mp2 );
               DimensionaItem( pClass, RESIZE_ALL_ITEM, 0L, 0L);
               return(mres);
            break;
         }
         return(MRESULT)(WinSendMsg(pClass->hwndCont,msg,mp1,mp2));
      break;

      case WM_COMMAND:
         switch ( SHORT1FROMMP( mp1 ) ) {
            case ID_BTN_RESIZE_ALL:
                pClass = WinQueryWindowPtr(hwnd, 0);
                DimensionaItem( pClass, RESIZE_ALL_ITEM, 0L, 0L);
            break;
         }
      break;

      case WM_DRAWITEM:
        {
           POWNERITEM pOwnerItem;
           BOOL       fOwnerDraw=FALSE;
           pOwnerItem=PVOIDFROMMP(mp2);
           pClass = WinQueryWindowPtr(hwnd, 0);
           /* Codice per details */
           if (pOwnerItem->idItem==0) {
             if ( ((CNRDRAWITEMINFO*)(pOwnerItem->hItem))->pFieldInfo->flData & CFA_OLD_OWNERDRAW)
                fOwnerDraw=TRUE;
             CalcolaSizeItem( WinQueryWindowPtr(hwnd, 0),
                              pOwnerItem,
                              ((CNRDRAWITEMINFO*)(pOwnerItem->hItem))->pRecord,
                              ((CNRDRAWITEMINFO*)(pOwnerItem->hItem))->pFieldInfo );
           } else {
             fOwnerDraw=pClass->fOwnerDraw;
           }
           if (fOwnerDraw) return( WinSendMsg( WinQueryWindow(hwnd,QW_OWNER),msg,MPFROMSHORT(pClass->idClass),mp2) );
                      else return( MRESULT ) FALSE;
        }
      break;

      case WM_DESTROY:
        DosFreeMem(WinQueryWindowPtr(hwnd, 0));
      break;

      case WM_CSOJ2I0H_STOREPOS:
      case WM_CSOJ2I0H_RESTOREPOS:
        return(MRESULT)StoreRestorePos(WinQueryWindowPtr(hwnd, 0),
                                       msg,
                                       ((PCSOJ2I0H_POS)mp1)->hini,
                                       ((PCSOJ2I0H_POS)mp1)->appl,
                                       ((PCSOJ2I0H_POS)mp1)->key );
      break;

      case WM_CSOJ2I0H_SORT:
        pClass = WinQueryWindowPtr(hwnd, 0);
        SortContainer( pClass, LONGFROMMP(mp1), LONGFROMMP(mp2));
      break;

      case WM_CSOJ2I0H_RESIZE:
        pClass = WinQueryWindowPtr(hwnd, 0);
        if (LONGFROMMP(mp1)==0)
           DimensionaItem( pClass, RESIZE_ALL_ITEM, 0L, 0L);
        else
           DimensionaItem( pClass, RESIZE_ITEM, LONGFROMMP(mp1), 0L);
        return(MRESULT)0;
      break;
   }
   return ( WinDefWindowProc(hwnd, msg, mp1, mp2) );
}

/******************************************************************************
*                                                                             *
******************************************************************************/
VOID DrawButton( PPRIVATESTRUCT pClass, PUSERBUTTON pBtn )
{
   RECTL    rcWin;
   RECTL    rcWinBmp;
   POINTL   ptl;
   PSZ      pszString;
   ULONG    cbString=0;

   WinQueryWindowRect( pBtn->hwnd , &rcWin );
   WinFillRect( pBtn->hps, &rcWin, CLR_PALEGRAY );

   rcWin.yTop--;
   rcWin.xRight--;

   if (pBtn->fsState & BDS_HILITED) {
      GpiSetColor(pBtn->hps,CLR_BLACK);
   } else {
      GpiSetColor(pBtn->hps,CLR_WHITE);
   }
   ptl.x = rcWin.xLeft;
   ptl.y = rcWin.yBottom+1;
   GpiMove(pBtn->hps, &ptl);
   ptl.y = rcWin.yTop;
   GpiLine(pBtn->hps, &ptl);
   ptl.x = rcWin.xRight-1;
   GpiLine(pBtn->hps, &ptl);

   if (pBtn->fsState & BDS_HILITED) {
      GpiSetColor(pBtn->hps,CLR_WHITE);
   } else {
      GpiSetColor(pBtn->hps,CLR_BLACK);
   }
   ptl.x = rcWin.xRight;
   ptl.y = rcWin.yTop;
   GpiMove(pBtn->hps, &ptl);
   ptl.y = rcWin.yBottom;
   GpiLine(pBtn->hps, &ptl);
   ptl.x = rcWin.xLeft;
   GpiLine(pBtn->hps, &ptl);

   rcWin.yTop--;
   rcWin.xRight--;
   rcWin.yBottom++;
   rcWin.xLeft++;

   if (pBtn->fsState & BDS_HILITED) {
      GpiSetColor(pBtn->hps,CLR_DARKGRAY);
   } else {
      GpiSetColor(pBtn->hps,CLR_PALEGRAY);
   }
   ptl.x = rcWin.xLeft;
   ptl.y = rcWin.yBottom;
   GpiMove(pBtn->hps, &ptl);
   ptl.y = rcWin.yTop;
   GpiLine(pBtn->hps, &ptl);
   ptl.x = rcWin.xRight;
   GpiLine(pBtn->hps, &ptl);

   if (pBtn->fsState & BDS_HILITED) {
      GpiSetColor(pBtn->hps,CLR_PALEGRAY);
   } else {
      GpiSetColor(pBtn->hps,CLR_DARKGRAY);
   }
   ptl.x = rcWin.xRight;
   ptl.y = rcWin.yTop;
   GpiMove(pBtn->hps, &ptl);
   ptl.y = rcWin.yBottom;
   GpiLine(pBtn->hps, &ptl);
   ptl.x = rcWin.xLeft;
   GpiLine(pBtn->hps, &ptl);

   rcWin.yTop--;
   rcWin.xRight--;
   rcWin.yBottom+=2;
   rcWin.xLeft+=2;

   if (pBtn->fsState & BDS_HILITED) {
      rcWin.yTop--;
      rcWin.yBottom--;
      rcWin.xLeft++;
   }

   rcWinBmp.xLeft   = rcWin.xRight - FRECCIA_CX - 1;
   rcWinBmp.xRight  = rcWinBmp.xLeft + FRECCIA_CX;
   rcWinBmp.yBottom = FRECCIA_Y;
   rcWinBmp.yTop    = rcWinBmp.yBottom + FRECCIA_CY;

   if (pBtn->fsState & BDS_HILITED) {
      rcWin.xRight++;
      rcWin.yTop--;

      rcWinBmp.xLeft++;
      rcWinBmp.xRight++;
      rcWinBmp.yBottom--;
      rcWinBmp.yTop--;
   }

   if (pClass->ulId==WinQueryWindowUShort(pBtn->hwnd,QWS_ID)) {
      if (pClass->sort) {
         WinDrawBitmap( pBtn->hps , hbmpDown, (PRECTL)NULL , (PPOINTL)&rcWinBmp, 0L , 0L , DBM_NORMAL );
      } else {
         WinDrawBitmap( pBtn->hps , hbmpUp, (PRECTL)NULL , (PPOINTL)&rcWinBmp, 0L , 0L , DBM_NORMAL );
      }
      rcWin.xRight = rcWin.xRight - FRECCIA_CX - 3;
   }

   cbString=WinQueryWindowTextLength(pBtn->hwnd)+1;
   DosAllocMem((PVOID)&pszString, cbString, PAG_COMMIT | PAG_READ | PAG_WRITE);
   WinQueryWindowText(pBtn->hwnd,cbString,pszString);

   rcWin.xRight-=6;
   rcWin.xLeft+=3;
   WinDrawText(pBtn->hps,-1L,pszString,&rcWin,CLR_BLACK,CLR_PALEGRAY, WinQueryWindowULong(pBtn->hwnd, QWL_USER) | DT_ERASERECT);
   DosFreeMem(pszString);

   return;
}

/******************************************************************************
*                                                                             *
******************************************************************************/
VOID DrawButtonReload( PUSERBUTTON pBtn )
{
   RECTL    rcWin;
   WinQueryWindowRect( pBtn->hwnd , &rcWin );
   WinFillRect( pBtn->hps, &rcWin, CLR_PALEGRAY );
   rcWin.yTop--;
   rcWin.xRight--;
   rcWin.yBottom++;
   rcWin.xLeft++;
   if (pBtn->fsState & BDS_HILITED) {
      rcWin.yTop--;
      rcWin.xLeft++;
   }
   if (pBtn->fsState & BDS_HILITED)
     WinDrawBitmap( pBtn->hps , hbmpReloadR, (PRECTL)NULL , (PPOINTL)&rcWin, 0L , 0L , DBM_NORMAL );
   else
     WinDrawBitmap( pBtn->hps , hbmpReload, (PRECTL)NULL , (PPOINTL)&rcWin, 0L , 0L , DBM_NORMAL );
   return;
}

/******************************************************************************
* Creazione bottoni                                                           *
******************************************************************************/
VOID CreaButton( PPRIVATESTRUCT pClass, PFIELDINFO pField, PFIELDINFOINSERT pInsert )
{
   ULONG  kx=0;
   ULONG  sizeBtn;
   ULONG i=0;
   ULONG cxWidth;
   HWND  hwndBtn;
   ULONG style;

   pField =(PFIELDINFO)WinSendMsg(pClass->hwndCont, CM_QUERYDETAILFIELDINFO, 0L, MPFROMSHORT(CMA_FIRST));

   while (pField!=NULLHANDLE && pField->flData>0) {

     cxWidth=MIN_SIZE_ITEM;
     if (pField->cxWidth==0 || pField->cxWidth<cxWidth) pField->cxWidth = cxWidth;
     sizeBtn = pField->cxWidth;

     hwndBtn = WinCreateWindow( pClass->hwndFBar, "Btn",
                      (pField->flData & CFA_BITMAPORICON) ? " " : pField->pTitleData,
                      WS_VISIBLE | BS_NOPOINTERFOCUS | BS_USERBUTTON | BS_PUSHBUTTON | WS_GROUP,
                      (ULONG)kx,
                      (ULONG)0,
                      (ULONG)sizeBtn,
                      (ULONG)pClass->hightBtn,
                      pClass->hwndFBar,
                      HWND_TOP,
                      i,
                      NULL, NULL);
     pClass->pClassFieldInfo[i]=pField;

     style=DT_LEFT | DT_BOTTOM; /* Default */
     if ((pField->flTitle & CFA_LEFT   )==CFA_LEFT   ) style |= DT_LEFT;
     if ((pField->flTitle & CFA_RIGHT  )==CFA_RIGHT  ) style |= DT_RIGHT;
     if ((pField->flTitle & CFA_CENTER )==CFA_CENTER ) style |= DT_CENTER;
     if ((pField->flTitle & CFA_TOP    )==CFA_TOP    ) style |= DT_TOP;
     if ((pField->flTitle & CFA_VCENTER)==CFA_VCENTER) style |= DT_VCENTER;
     if ((pField->flTitle & CFA_BOTTOM )==CFA_BOTTOM ) style |= DT_BOTTOM;
     WinSetWindowULong(hwndBtn, QWL_USER, style);

     kx+=sizeBtn;
     i++;
     pField = pField->pNextFieldInfo;
   };
   return;
}

/******************************************************************************
* Cancellazione bottoni                                                       *
******************************************************************************/
VOID CancButton( PPRIVATESTRUCT pClass, PFIELDINFO pField, PFIELDINFOINSERT pInsert )
{
   ULONG i=0;
   pField =(PFIELDINFO)WinSendMsg(pClass->hwndCont, CM_QUERYDETAILFIELDINFO, 0L, MPFROMSHORT(CMA_FIRST));
   while (pField!=NULLHANDLE && pField->flData>0) {
     WinDestroyWindow( WinWindowFromID(pClass->hwndFBar, i++ ) );
     pField = pField->pNextFieldInfo;
   };
   return;
}

ULONG CalcolaLenString ( HPS hps, PCHAR pString )
{
   POINTL aptl[TXTBOX_COUNT];
   GpiQueryTextBox(hps, strlen(pString), pString, TXTBOX_COUNT, aptl);
   return ( aptl[3].x - aptl[1].x );
}

MRESULT EXPENTRY BtnProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
   SWP swp;
   PPRIVATESTRUCT pClass;

   switch( msg )
   {
     case WM_MOUSEMOVE:
        WinQueryWindowPos(hwnd,&swp);
        if (SHORT1FROMMP(mp1)>=swp.cx-SIZE_ARROW) {
           WinSetPointer(HWND_DESKTOP,hptrSepar);
           return(MRESULT)0;
        }
     break;

     case WM_BUTTON1CLICK:
     case WM_BUTTON1DBLCLK:
        WinQueryWindowPos(hwnd,&swp);
        if (SHORT1FROMMP(mp1)>=swp.cx-SIZE_ARROW) {
           ULONG newSize;
           HWND  hwndParent;
           hwndParent=WinQueryWindow(hwnd,QW_PARENT);
           pClass = WinQueryWindowPtr(hwndParent, 0);
           newSize=CalcolaWidth(pClass, WinQueryWindowUShort(hwnd,QWS_ID));
           if (newSize!=0)
              DimensionaItem( pClass, RESIZE_ITEM_CX, WinQueryWindowUShort(hwnd,QWS_ID), newSize);
           return(MRESULT)0;
        }
     break;

     case WM_BUTTON1DOWN:
        WinQueryWindowPos(hwnd,&swp);
        if (SHORT1FROMMP(mp1)>=swp.cx-SIZE_ARROW) {
           LONG delta;
           HWND  hwndParent;
           hwndParent=WinQueryWindow(hwnd,QW_PARENT);
           pClass = WinQueryWindowPtr(hwndParent, 0);
           delta=TrackRect(pClass,hwnd,swp,swp.x+swp.cx);
           if (delta!=0)
              DimensionaItem( pClass, RESIZE_ITEM_DELTA, WinQueryWindowUShort(hwnd,QWS_ID), delta);
           return(MRESULT)0;
        }
     break;
   }
   return ( MRESULT )( * clsiBtn.pfnWindowProc )( hwnd, msg, mp1, mp2 );
}

LONG EXPENTRY TrackRect( PPRIVATESTRUCT pClass, HWND hwndBtn, SWP swpBtn, USHORT start )
{
   HPS hpsParent;
   HWND hwndParent;
   TRACKINFO tiStruct;

   //hwndParent = WinQueryWindow( hwndBtn , QW_PARENT );
   hwndParent=pClass->hwndFBar;

   tiStruct.cxBorder = 1;
   tiStruct.cyBorder = 1;
   tiStruct.cxGrid = 0;
   tiStruct.cyGrid = 0;

   tiStruct.rclTrack.xLeft   = start;
   tiStruct.rclTrack.xRight  = start + SIZE_TRACK;
   tiStruct.rclTrack.yBottom = swpBtn.y;
   tiStruct.rclTrack.yTop    = swpBtn.y + swpBtn.cy;

   WinQueryWindowRect( hwndParent, &tiStruct.rclBoundary);
   tiStruct.rclBoundary.xLeft=swpBtn.x+MIN_SIZE_ITEM;
   tiStruct.rclBoundary.yBottom=tiStruct.rclTrack.yBottom;
   tiStruct.rclBoundary.yTop=tiStruct.rclTrack.yTop;

   tiStruct.ptlMinTrackSize.x = 0;
   tiStruct.ptlMinTrackSize.y = 0;

   tiStruct.ptlMaxTrackSize.x = tiStruct.rclBoundary.xRight;
   tiStruct.ptlMaxTrackSize.y = tiStruct.rclBoundary.yTop;

   tiStruct.fs  = TF_STANDARD | TF_ALLINBOUNDARY | TF_MOVE;

   hpsParent=WinGetPS( hwndParent );
   WinTrackRect(hwndBtn, hpsParent, &tiStruct);
   WinReleasePS(hpsParent);

   if (tiStruct.rclTrack.xLeft!=start) {
      return(tiStruct.rclTrack.xRight - start);
   }
   return(0L);
}

/******************************************************************************/
/* In base al tipo di operazione vengono dimensionati i bottoni               */
/******************************************************************************/
VOID EXPENTRY DimensionaItem( PPRIVATESTRUCT pClass, ULONG tpOp, ULONG idItem, LONG delta )
{
   ULONG i=0;
   PFIELDINFO pField;
   RECTL rc;
   pField =(PFIELDINFO)WinSendMsg(pClass->hwndCont, CM_QUERYDETAILFIELDINFO, 0L, MPFROMSHORT(CMA_FIRST));
   while (pField) {
     switch (tpOp) {
       case RESIZE_ALL_ITEM:
         pField->cxWidth=CalcolaWidth(pClass,i);
       break;
       case RESIZE_ITEM:
         if (i==idItem) pField->cxWidth=CalcolaWidth(pClass,i);
       break;
       case RESIZE_ITEM_DELTA:
         if (i==idItem) pField->cxWidth += delta;
       break;
       case RESIZE_ITEM_CX:
         if (i==idItem) pField->cxWidth = delta;
       break;
     }
     i++;
     pField = pField->pNextFieldInfo;
   }
   WinSendMsg(pClass->hwndCont, CM_INVALIDATEDETAILFIELDINFO, 0L, 0L);
   WinSendMsg(pClass->hwndCont, CM_QUERYVIEWPORTRECT, MPFROMP(&rc), MPFROM2SHORT( CMA_WORKSPACE, FALSE ));
   WinSetWindowPos(pClass->hwndFBar, NULLHANDLE, -rc.xLeft, 0L, 0L, 0L, SWP_MOVE );
   WinInvalidateRect(pClass->hwndFBar,NULL,TRUE);
   return;
}

MRESULT EXPENTRY FBarProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
   PPRIVATESTRUCT pClass;

   switch( msg )
   {
     case WM_PAINT:
        {
           HPS hps;
           RECTL WinDim;
           pClass = WinQueryWindowPtr(hwnd, 0);
           hps = WinBeginPaint( hwnd, (HPS)NULL, &WinDim );
           if (pClass->pClassFieldInfo[0]!=NULL) { // Almeno un bottone
             WinFillRect(hps, &WinDim, CLR_PALEGRAY);
           } else {
             RECTL rc;
             POINTL ptl;
             SWP swp;
             WinQueryWindowPos( pClass->hwndClass, &swp);
             WinQueryWindowRect( hwnd , &rc );
             WinFillRect( hps, &rc, CLR_PALEGRAY );
             rc.yTop--;
             rc.xRight=rc.xLeft+swp.cx-WinQuerySysValue(HWND_DESKTOP,SV_CXVSCROLL)-1;

             GpiSetColor(hps,CLR_WHITE);
             ptl.x = rc.xLeft;
             ptl.y = rc.yBottom+1;
             GpiMove(hps, &ptl);
             ptl.y = rc.yTop;
             GpiLine(hps, &ptl);
             ptl.x = rc.xRight-1;
             GpiLine(hps, &ptl);

             GpiSetColor(hps,CLR_BLACK);
             ptl.x = rc.xRight;
             ptl.y = rc.yTop;
             GpiMove(hps, &ptl);
             ptl.y = rc.yBottom;
             GpiLine(hps, &ptl);
             ptl.x = rc.xLeft;
             GpiLine(hps, &ptl);

             rc.yTop--;
             rc.xRight--;
             rc.yBottom++;
             rc.xLeft++;

             GpiSetColor(hps,CLR_PALEGRAY);
             ptl.x = rc.xLeft;
             ptl.y = rc.yBottom;
             GpiMove(hps, &ptl);
             ptl.y = rc.yTop;
             GpiLine(hps, &ptl);
             ptl.x = rc.xRight;
             GpiLine(hps, &ptl);

             GpiSetColor(hps,CLR_DARKGRAY);
             ptl.x = rc.xRight;
             ptl.y = rc.yTop;
             GpiMove(hps, &ptl);
             ptl.y = rc.yBottom;
             GpiLine(hps, &ptl);
             ptl.x = rc.xLeft;
             GpiLine(hps, &ptl);
           }
           WinEndPaint( hps );
        }
     break;

     case WM_CONTROL:
        switch ( SHORT2FROMMP( mp1 ) ) {
          case BN_PAINT:
             pClass = WinQueryWindowPtr(hwnd, 0);
             DrawButton( pClass, MPFROMP(mp2) );
             return(MRESULT)0;
          break;
       }
     break;

     case WM_COMMAND:
        pClass = WinQueryWindowPtr(hwnd, 0);
        if (SHORT1FROMMP(mp1)!=pClass->ulId) {
           pClass->sort=TRUE;
        } else {
           if (pClass->sort) pClass->sort=FALSE;
                        else pClass->sort=TRUE;
        }
        SortContainer( pClass, SHORT1FROMMP(mp1), pClass->sort );
     break;
   }
   return ( WinDefWindowProc(hwnd, msg, mp1, mp2) );
}

SHORT pfnCompare( PRECORDCORE pTrg, PRECORDCORE pSrc, PVOID pStorage )
{
  ULONG pDataSrc;
  ULONG pDataTrg;
  PCHAR *pTextSrc;
  PCHAR *pTextTrg;
  PCDATE pDateSrc;
  PCDATE pDateTrg;
  PCTIME pTimeSrc;
  PCTIME pTimeTrg;
  LONG   lDiff;
  LONG   dateSrc;
  LONG   dateTrg;

  pDataSrc=(ULONG)(((PCHAR)pSrc)+((SORTSTRUCT*)pStorage)->offset);
  pDataTrg=(ULONG)(((PCHAR)pTrg)+((SORTSTRUCT*)pStorage)->offset);

  if (((SORTSTRUCT*)pStorage)->tipo & CFA_STRING) {
     pTextSrc=(PCHAR*)pDataSrc;
     pTextTrg=(PCHAR*)pDataTrg;
     if (((SORTSTRUCT*)pStorage)->sort) {
        return(strcmp(*pTextTrg,*pTextSrc));
     } else {
        return(strcmp(*pTextSrc,*pTextTrg));
     }
  } else {
     if (((SORTSTRUCT*)pStorage)->tipo & CFA_ULONG) {
        lDiff=0;
        if (((LONG)*(PULONG*)pDataTrg - (LONG)*(PULONG*)pDataSrc) > 0) lDiff=1;
        if (((LONG)*(PULONG*)pDataTrg - (LONG)*(PULONG*)pDataSrc) < 0) lDiff=-1;
        if (((SORTSTRUCT*)pStorage)->sort) {
           return(lDiff);
        } else {
           return(-lDiff);
        }
     } else {
        if (((SORTSTRUCT*)pStorage)->tipo & CFA_TIME) {
           pTimeSrc=(PCTIME)pDataSrc;
           pTimeTrg=(PCTIME)pDataTrg;
           if (((SORTSTRUCT*)pStorage)->sort) {
              return(memcmp(pTimeTrg,pTimeSrc,sizeof(CTIME)));
           } else {
              return(memcmp(pTimeSrc,pTimeTrg,sizeof(CTIME)));
           }
        } else {
           if (((SORTSTRUCT*)pStorage)->tipo & CFA_DATE) {
              pDateSrc=(PCDATE)pDataSrc;
              pDateTrg=(PCDATE)pDataTrg;
              dateSrc=pDateSrc->year*10000+pDateSrc->month*100+pDateSrc->day;
              dateTrg=pDateTrg->year*10000+pDateTrg->month*100+pDateTrg->day;
              lDiff=0;
              if (dateTrg > dateSrc) lDiff=1;
              if (dateTrg < dateSrc) lDiff=-1;
              if (((SORTSTRUCT*)pStorage)->sort) {
                 return(lDiff);
              } else {
                 return(-lDiff);
              }
           } else {
              if (((SORTSTRUCT*)pStorage)->tipo & CFA_BITMAPORICON) {
                 /* No sort */
                 pTextSrc=(PCHAR*)pDataSrc;
                 return(0);
              }
           }
        }
     }
  }
  return(0);
}

VOID EXPENTRY ScrollaFBar( PPRIVATESTRUCT pClass, LONG delta )
{
   SWP swp;
   WinQueryWindowPos(pClass->hwndFBar,&swp);
   WinSetWindowPos(pClass->hwndFBar, NULLHANDLE, swp.x+delta, swp.y, 0L, 0L, SWP_MOVE);
   WinInvalidateRect(pClass->hwndFBar,NULL,TRUE);
   return;
}

ULONG EXPENTRY CalcolaWidth( PPRIVATESTRUCT pClass, ULONG idItem )
{
   PFIELDINFO pField=NULLHANDLE;
   ULONG  i=0;
   PRECORDCORE pRecCore=NULL;
   ULONG  pData;
   PCHAR  *pText;
   PCHAR  *pImm;
   ULONG  num;
   CHAR   szString[35];
   PCDATE pDate;
   PCTIME pTime;
   ULONG  cxWidthNew=0;
   ULONG  cxWidthTitle=0;
   ULONG  cxWidth=0;
   HPS    hpsCont;

   WinSetPointer(HWND_DESKTOP,hptrWait);

   hpsCont=WinGetPS(pClass->hwndCont);

   pField =(PFIELDINFO)WinSendMsg(pClass->hwndCont, CM_QUERYDETAILFIELDINFO, 0L, MPFROMSHORT(CMA_FIRST));
   while (pField) {
     if (i==idItem) {
        pRecCore = (PRECORDCORE) WinSendMsg(pClass->hwndCont, CM_QUERYRECORD, 0L, MPFROM2SHORT( CMA_FIRST, CMA_ITEMORDER ));
        while (pRecCore!=NULLHANDLE) {
           pData=(ULONG)(((PCHAR)pRecCore)+pField->offStruct);
           cxWidthNew=MIN_SIZE_ITEM;
           if (pField->flData & CFA_STRING) {
              pText=(PCHAR*)pData;
              cxWidthNew=CalcolaLenString(hpsCont,*pText);
           } else {
              if (pField->flData & CFA_ULONG) {
                 ULONG miliardi,milioni,migliaia,unita;
                 num=(ULONG)(*(PULONG*)pData);
                 miliardi=num/1000000000;
                 num=num-miliardi*1000000000;
                 milioni=num/1000000;
                 num=num-milioni*1000000;
                 migliaia=num/1000;
                 num=num-migliaia*1000;
                 unita=num;
                      if (miliardi) sprintf(szString,"%d%s%03d%s%03d%s%03d",miliardi,CtryInfo.szThousandsSeparator,milioni,CtryInfo.szThousandsSeparator,migliaia,CtryInfo.szThousandsSeparator,unita);
                 else if (milioni)  sprintf(szString,"%d%s%03d%s%03d",milioni,CtryInfo.szThousandsSeparator,migliaia,CtryInfo.szThousandsSeparator,unita);
                 else if (migliaia) sprintf(szString,"%d%s%03d",migliaia,CtryInfo.szThousandsSeparator,unita);
                 else if (unita)    sprintf(szString,"%d",unita);
                 else sprintf(szString,"0");
                 cxWidthNew=CalcolaLenString(hpsCont,szString);
              } else {
                 if (pField->flData & CFA_TIME) {
                    pTime=(PCTIME)pData;
                    memset(szString,0,sizeof(szString));
                    switch (CtryInfo.fsTimeFmt) {
                       case 0:     /* 12:00:00 AM Inutile sofisticare, nessuno usa tale formato */
                          sprintf(szString,"%d%s%d%s%d AM", pTime->hours, CtryInfo.szTimeSeparator,
                                  pTime->minutes, CtryInfo.szTimeSeparator, pTime->seconds);
                       break;
                       default:                                               /* 24:00 */
                          sprintf(szString,"%d%s%d%s%d", pTime->hours, CtryInfo.szTimeSeparator,
                                  pTime->minutes, CtryInfo.szTimeSeparator, pTime->seconds);
                       break;
                    }
                    cxWidthNew=CalcolaLenString(hpsCont,szString);
                 } else {
                    if (pField->flData & CFA_DATE) {
                       pDate=(PCDATE)pData;
                       memset(szString,0,sizeof(szString));
                       switch (CtryInfo.fsDateFmt) {
                          case 1:                                              /* dd/mm/yy */
                             sprintf(szString,"%d%s%d%s%d", pDate->day, CtryInfo.szDateSeparator,
                                     pDate->month, CtryInfo.szDateSeparator, pDate->year);
                          break;
                          case 2:                                               /* yy/mm/dd */
                             sprintf(szString,"%d%s%d%s%d", pDate->year, CtryInfo.szDateSeparator,
                                     pDate->month, CtryInfo.szDateSeparator, pDate->day);
                          break;
                          default:                                               /* mm/dd/yy */
                             sprintf(szString,"%d%s%d%s%d", pDate->month, CtryInfo.szDateSeparator,
                             pDate->day, CtryInfo.szDateSeparator, pDate->year);
                          break;
                       }
                       cxWidthNew=CalcolaLenString(hpsCont,szString);
                    } else {
                       if (pField->flData & CFA_BITMAPORICON) {
                          POINTERINFO ptrInfo;
                          SIZEL       sizeBmp = { 0 };
                          HBITMAP     hbm;
                          ULONG       fStyle=0;
                          pImm=((PCHAR*)pData);
                          if (WinQueryPointerInfo((HPOINTER)*pImm,&ptrInfo)==TRUE) {
                             fStyle=WinQueryWindowULong(pClass->hwndClass,QWL_STYLE);
                             if (fStyle & CCS_MINIICONS) {
                               hbm=ptrInfo.hbmMiniPointer;
                             } else {
                               hbm=ptrInfo.hbmPointer;
                             }
                          } else {
                             hbm=(HBITMAP)*pImm;
                          }
                          GpiQueryBitmapDimension(hbm,&sizeBmp);
                          cxWidthNew=sizeBmp.cx;
                          if (cxWidthNew==0) {
                             cxWidthNew=WinQuerySysValue(HWND_DESKTOP,SV_CXICON);
                             if (fStyle & CCS_MINIICONS) cxWidthNew=cxWidthNew/2;
                          }
                       }
                    }
                 }
              }
           }
           if (cxWidthNew > cxWidth) cxWidth = cxWidthNew;
           pRecCore = (PRECORDCORE) WinSendMsg(pClass->hwndCont, CM_QUERYRECORD, MPFROMP(pRecCore), MPFROM2SHORT( CMA_NEXT, CMA_ITEMORDER ));
        }
        cxWidthTitle=CalcolaLenString(hpsCont,pField->pTitleData);
        if (cxWidthTitle>cxWidth) cxWidth=cxWidthTitle;
        WinReleasePS(hpsCont);
        return(cxWidth);
     }
     pField = pField->pNextFieldInfo;
     i++;
   };
   WinReleasePS(hpsCont);
   return(cxWidth);
}


BOOL StoreRestorePos( PPRIVATESTRUCT pClass, ULONG oper, HINI hini, PSZ appl, PSZ key )
{
  PFIELDINFO pField=NULLHANDLE;
  ULONG      i=0;
  ULONG      btnDim[MAX_BTN];
  ULONG      sizeBtnDim;
  ULONG      sizeBtn;
  ULONG      kx=0;
  SWP        swp;

  memset(btnDim,0,sizeof(btnDim));
  if (oper==WM_CSOJ2I0H_RESTOREPOS) {
     sizeBtnDim=sizeof(btnDim);
     if ( !PrfQueryProfileData(hini, appl, key, btnDim, &sizeBtnDim ))
       return(FALSE);
  }
  pField =(PFIELDINFO)WinSendMsg(pClass->hwndCont, CM_QUERYDETAILFIELDINFO, 0L, MPFROMSHORT(CMA_FIRST));
  while (pField!=NULLHANDLE && pField->flData>0) {
     if (oper==WM_CSOJ2I0H_STOREPOS) {
        btnDim[i]=pField->cxWidth;
     } else {
        pField->cxWidth=btnDim[i];
        sizeBtn = pField->cxWidth;
        WinQueryWindowPos(WinWindowFromID(pClass->hwndFBar,i),&swp);
        WinSetWindowPos(WinWindowFromID(pClass->hwndFBar,i), NULLHANDLE, kx, swp.y, sizeBtn, swp.cy, SWP_MOVE | SWP_SIZE);
        kx+=sizeBtn;
     }
     pField =(PFIELDINFO)WinSendMsg(pClass->hwndCont, CM_QUERYDETAILFIELDINFO, pField, MPFROMSHORT(CMA_NEXT));
     i++;
  };
  if (oper==WM_CSOJ2I0H_STOREPOS) {
     if ( !PrfWriteProfileData(hini, appl, key, btnDim, sizeof(btnDim) ))
       return(FALSE);
  } else {
     WinSendMsg(pClass->hwndCont, CM_INVALIDATEDETAILFIELDINFO, 0L, 0L);
  }
  return(TRUE);
}

BOOL EXPENTRY SortContainer( PPRIVATESTRUCT pClass, ULONG ulItem, BOOL fAsc )
{
   SORTSTRUCT sortStruct;
   PFIELDINFO pField=NULLHANDLE;
   ULONG i=0;
   RECTL rc;
   LONG delta1,delta2,deltatot;

   WinSetPointer(HWND_DESKTOP,hptrWait);

   if (ulItem!=pClass->ulId) {
      pClass->ulOldId=pClass->ulId;
      pClass->ulId=ulItem;
      WinInvalidateRect(WinWindowFromID(pClass->hwndFBar,pClass->ulOldId),NULL,TRUE);
   }
   WinInvalidateRect(WinWindowFromID(pClass->hwndFBar,pClass->ulId),NULL,TRUE);

   sortStruct.hwndCont=pClass->hwndCont;
   sortStruct.sort=fAsc;

   pField =(PFIELDINFO)WinSendMsg(pClass->hwndCont, CM_QUERYDETAILFIELDINFO, 0L, MPFROMSHORT(CMA_FIRST));
   while (pField!=NULLHANDLE && pField->flData>0) {
     if (i==pClass->ulId) {

        sortStruct.offset=pField->offStruct;
        sortStruct.tipo=pField->flData;

        WinSendMsg(pClass->hwndCont, CM_QUERYVIEWPORTRECT, MPFROMP(&rc), MPFROM2SHORT( CMA_WORKSPACE, FALSE ));
        delta1=rc.xLeft;
        WinSendMsg(pClass->hwndCont, CM_SORTRECORD, MPFROMP(pfnCompare), MPFROMP(&sortStruct) );
        /* KKKK */
        WinSendMsg(pClass->hwndCont, CM_QUERYVIEWPORTRECT, MPFROMP(&rc), MPFROM2SHORT( CMA_WORKSPACE, FALSE ));
        delta2=rc.xLeft;
        deltatot=delta1-delta2;
        if (deltatot!=0)
           WinSendMsg(pClass->hwndCont, CM_SCROLLWINDOW, MPFROMSHORT( CMA_HORIZONTAL ), MPFROMLONG( deltatot ));
        ScrollaFBar( pClass, deltatot );
        return(TRUE);
     }
     pField =(PFIELDINFO)WinSendMsg(pClass->hwndCont, CM_QUERYDETAILFIELDINFO, pField, MPFROMSHORT(CMA_NEXT));
     i++;
   };
   return(FALSE);
}

MRESULT EXPENTRY EnhContProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
   switch( msg )
   {
     case WM_PRESPARAMCHANGED:
        {
          ULONG ulPresParams;
          CHAR  buffer[40];
          PPRIVATESTRUCT pClass;
          static BOOL fRecurse=FALSE;
          if (fRecurse) break;
          fRecurse=TRUE;
          pClass = WinQueryWindowPtr(hwnd, 0);
          memset(buffer,0,sizeof(buffer));
          if (WinQueryPresParam(hwnd, LONGFROMMP(mp1),
                                0, &ulPresParams,
                                (ULONG)sizeof(buffer), (PVOID)buffer,
                               QPF_NOINHERIT)==0) {
             WinQueryPresParam(pClass->hwndClass, LONGFROMMP(mp1),
                               0, &ulPresParams,
                               (ULONG)sizeof(buffer), (PVOID)buffer,
                               QPF_NOINHERIT);
          }
          WinSetPresParam(pClass->hwndClass, LONGFROMMP(mp1),
                          (ULONG)sizeof(buffer),
                          (PVOID)buffer);
          if (LONGFROMMP(mp1)==PP_FONTNAMESIZE) {
             HPS   hps;
             RECTL rc;
             hps = WinGetPS( hwnd );
             GpiQueryFontMetrics (hps, sizeof (FONTMETRICS), &pClass->fmMetrics);
             pClass->hightBtn=pClass->fmMetrics.lMaxBaselineExt+EXTRA_ALTEZZA_BTN;
             WinReleasePS(hps);
             WinSendMsg(pClass->hwndClass,WM_SIZE,0L,0L);
             WinSendMsg(pClass->hwndCont, CM_QUERYVIEWPORTRECT, MPFROMP(&rc), MPFROM2SHORT( CMA_WORKSPACE, FALSE ));
             if (rc.xLeft>0)
                WinSendMsg(pClass->hwndCont, CM_SCROLLWINDOW, MPFROMLONG(CMA_HORIZONTAL), MPFROMLONG(-rc.xLeft));
             WinSetWindowPos(pClass->hwndFBar, NULLHANDLE, 0L, 0L, 1500L, pClass->hightBtn, SWP_SIZE );
             DimensionaItem( pClass, RESIZE_ALL_ITEM, 0L, 0L);
             WinPostMsg(pClass->hwndCont, CM_SCROLLWINDOW, MPFROMLONG(CMA_HORIZONTAL), MPFROMLONG(1500));
             WinPostMsg(pClass->hwndCont, CM_SCROLLWINDOW, MPFROMLONG(CMA_HORIZONTAL), MPFROMLONG(-1500));
          }
          fRecurse=FALSE;
        }
     break;
   }
   return( (  pfnwpCONTAINER( hwnd, msg, mp1, mp2 )) );
}

VOID EXPENTRY CalcolaSizeItem( PPRIVATESTRUCT pClass, POWNERITEM pOwnerItem, PRECORDCORE pRecCore, PFIELDINFO pField )
{
  ULONG i;
  LONG  size;
  SWP   swp;
  RECTL rc;
  WinSendMsg(pClass->hwndCont, CM_QUERYVIEWPORTRECT, MPFROMP(&rc), MPFROM2SHORT( CMA_WORKSPACE, FALSE ));
  pOwnerItem->rclItem.xLeft=pOwnerItem->rclItem.xLeft+rc.xLeft;
  pOwnerItem->rclItem.xRight=pOwnerItem->rclItem.xRight+rc.xLeft;

  size=pOwnerItem->rclItem.xRight-pOwnerItem->rclItem.xLeft;
  for (i=0;i<MAX_BTN;i++) {
     if (pClass->pClassFieldInfo[i]==pField) {
        if (pClass->size[i]!=size || pClass->start[i]!=pOwnerItem->rclItem.xLeft) {
           WinQueryWindowPos(WinWindowFromID(pClass->hwndFBar,i),&swp);
           swp.x=pOwnerItem->rclItem.xLeft;
           swp.y=0;
           swp.cx=size-1;
           swp.cy=pClass->hightBtn;
           WinSetWindowPos(WinWindowFromID(pClass->hwndFBar,i), NULLHANDLE, swp.x, swp.y, swp.cx, swp.cy, SWP_MOVE | SWP_SIZE );
           pClass->size[i]=size;
           pClass->start[i]=pOwnerItem->rclItem.xLeft;
        }
        return;
     }
  }
}
