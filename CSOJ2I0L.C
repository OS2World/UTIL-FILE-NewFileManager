/*

   Why doesn't exist a ComboBox ownerdraw on PM ?
   Grrr......

*/

#define INCL_WIN
#define INCL_WINCURSORS
#define INCL_WINPOINTERS
#define INCL_GPI
#define INCL_DOS

#include <os2.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include "csoj2i0l.h"

#define WM_CLOSELIST  WM_USER + 1
#define WM_OPENLIST   WM_USER + 2

#define LONGFromRGB(R,G,B) (LONG)(((LONG)R<<16)+((LONG)G<<8)+(LONG)B)

typedef struct _PRIVATESTRUCT
{
     HWND     hwndClass;
     ULONG    idClass;
     HWND     hwndBtn;
     HWND     hwndText;
     HWND     hwndList;
     ULONG    hightText;
     ULONG    hightBtn;
     ULONG    widthBtn;
     ULONG    hightBmp;
     ULONG    widthBmp;
     HBITMAP  hbmp;
     BOOL     fStatus;
     BOOL     fOwnerDraw;
     LONG     alTable[16];
     ULONG    ulColorBack;
     BOOL     focus;

} PRIVATESTRUCT, *PPRIVATESTRUCT;

MRESULT EXPENTRY CSOJ2I0L_Proc    ( HWND, ULONG, MPARAM, MPARAM );
MRESULT EXPENTRY Text_Proc        ( HWND, ULONG, MPARAM, MPARAM );
MRESULT EXPENTRY Btn_Proc         ( HWND, ULONG, MPARAM, MPARAM );
MRESULT EXPENTRY List_Proc        ( HWND, ULONG, MPARAM, MPARAM );
VOID             DrawButton       ( PPRIVATESTRUCT );

static HAB hab;
static PFNWP pfnwpListProc = NULL;

BOOL CSOJ2I0L_REGISTRA( HAB lochab )
{
   hab=lochab;

   if (! WinRegisterClass( hab,
                           "CSOJ2I0L",
                           CSOJ2I0L_Proc,
                           CS_HITTEST | CS_CLIPCHILDREN | CS_MOVENOTIFY,
                           4l )) {
      return( FALSE );
   }

   if (! WinRegisterClass(hab,
                          "CSOJ2I0L_clText",
                          Text_Proc,
                          0l,
                          4l )) {
      return( FALSE );
   }

   if (! WinRegisterClass(hab,
                          "CSOJ2I0L_clBtn",
                          Btn_Proc,
                          0l,
                          4l )) {
      return( FALSE );
   }

   return( TRUE );
}


MRESULT EXPENTRY CSOJ2I0L_Proc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 )
{
   PPRIVATESTRUCT pArea;

   switch( msg )
   {
      case WM_CREATE:
         {
           PCREATESTRUCT pCreateStruct;
           FONTMETRICS fmMetrics;
           HPS hps;
           BITMAPINFOHEADER bmpInfo;
           ULONG style;

           pCreateStruct=PVOIDFROMMP(mp2);

           DosAllocMem((PVOID)&pArea, sizeof(PRIVATESTRUCT), PAG_COMMIT | PAG_READ | PAG_WRITE);
           memset(pArea,0,sizeof(PRIVATESTRUCT));
           WinSetWindowPtr(hwnd, 0, pArea);

           pArea->hwndClass=hwnd;
           pArea->idClass=pCreateStruct->id;
           pArea->ulColorBack= LONGFromRGB(255,255,255);

           if ( WinQueryWindowULong(pArea->hwndClass,QWL_STYLE) & LS_OWNERDRAW ) {
              pArea->fOwnerDraw=TRUE;
              style = LS_OWNERDRAW;
           } else {
              pArea->fOwnerDraw=FALSE;
              style = 0;
           }

           /* Dimensioni Bitmap combo */
           pArea->hbmp=WinGetSysBitmap(HWND_DESKTOP,SBMP_COMBODOWN);
           GpiQueryBitmapParameters(pArea->hbmp,&bmpInfo);
           pArea->widthBmp=bmpInfo.cx;

           if ( pArea->fOwnerDraw ) {
              pArea->hightText = SHORT1FROMMR( WinSendMsg(WinQueryWindow(pArea->hwndClass,QW_OWNER),
                                               WM_MEASUREITEM, MPFROMSHORT(CBID_LIST), MPFROMSHORT(LIT_NONE)));
           } else {
              hps = WinGetPS( hwnd );
              GpiQueryFontMetrics (hps, sizeof (FONTMETRICS), &fmMetrics);
              pArea->hightText=fmMetrics.lMaxBaselineExt;
              WinReleasePS(hps);
           }

           /* Dimensioni Text combo */
           pArea->hbmp=WinGetSysBitmap(HWND_DESKTOP,SBMP_COMBODOWN);
           GpiQueryBitmapParameters(pArea->hbmp,&bmpInfo);

           pArea->widthBmp=bmpInfo.cx;
           pArea->hightBmp=bmpInfo.cy;

           pArea->hightBtn=bmpInfo.cy;
           if (pArea->hightText>pArea->hightBtn) pArea->hightBtn=pArea->hightText;

           pArea->widthBtn=bmpInfo.cx;

           pArea->widthBtn+=7;
           if (pArea->fOwnerDraw) {
             //pArea->hightBtn+=2;
             ;
           } else {
             pArea->hightBtn+=4;
           }
           pArea->hwndText = WinCreateWindow( hwnd, "CSOJ2I0L_clText",
                                       "", WS_VISIBLE | WS_SYNCPAINT,
                                       2L,
                                       pCreateStruct->cy - pArea->hightBtn - 2,
                                       pCreateStruct->cx - pArea->widthBtn - 4,
                                       pArea->hightBtn,
                                       hwnd,
                                       HWND_TOP,
                                       ID_CSOJ2I0L_TEXT,
                                       NULL, NULL);
           WinSetWindowPtr(pArea->hwndText, 0, pArea);

           pArea->hwndBtn  = WinCreateWindow( hwnd, "CSOJ2I0L_clBtn",
                                       "",
                                       WS_VISIBLE,
                                       pCreateStruct->cx - pArea->widthBtn - 2,
                                       pCreateStruct->cy - pArea->hightBtn - 2,
                                       pArea->widthBtn,
                                       pArea->hightBtn,
                                       hwnd,
                                       HWND_TOP,
                                       ID_CSOJ2I0L_BTN,
                                       NULL, NULL);
           WinSetWindowPtr(pArea->hwndBtn, 0, pArea);

           style = style | LS_NOADJUSTPOS | LS_HORZSCROLL |
                   WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_PARENTCLIP | WS_SAVEBITS | WS_SYNCPAINT;

           pArea->hwndList = WinCreateWindow( HWND_DESKTOP, WC_LISTBOX,
                                       "",
                                       style,
                                       0L,
                                       0L,
                                       pCreateStruct->cx,
                                       pCreateStruct->cy - pArea->hightBtn - 3,
                                       hwnd,
                                       HWND_TOP,
                                       CBID_LIST,
                                       NULL, NULL);
           WinSetWindowPtr(pArea->hwndList, 0, pArea);
           pfnwpListProc=WinSubclassWindow( pArea->hwndList, (PFNWP)List_Proc );
         }
      break;

      case WM_MEASUREITEM:
         pArea=WinQueryWindowPtr(hwnd, 0);
         if ( pArea->fOwnerDraw ) {
            return WinSendMsg(WinQueryWindow(pArea->hwndClass,QW_OWNER),msg,mp1,mp2);
         }
      break;

      case WM_DRAWITEM:
         pArea=WinQueryWindowPtr(hwnd, 0);
         if ( pArea->fOwnerDraw ) {
            POWNERITEM poiItem;
            poiItem=(POWNERITEM)MPFROMP(mp2);
            poiItem->hwnd=pArea->hwndClass;
            GpiQueryLogColorTable(poiItem->hps, 0L, 0L, 16L, pArea->alTable);
            pArea->alTable[CLR_BACKGROUND] = pArea->ulColorBack;
            GpiCreateLogColorTable(poiItem->hps, 0L, LCOLF_CONSECRGB, 0L, 16, pArea->alTable);
            return(WinSendMsg(WinQueryWindow(pArea->hwndClass,QW_OWNER),msg,mp1,mp2));
         }
      break;

      case WM_CONTROL:
         pArea=WinQueryWindowPtr(hwnd, 0);
         switch ( SHORT1FROMMP ( mp1 ) ) {
            case ID_CSOJ2I0L_LIST:
               {
                  USHORT usNotifica=0;
                  switch ( SHORT2FROMMP ( mp1 ) ) {
                     case LN_SELECT:
                        usNotifica=CBN_LBSELECT;
                     break;
                     case LN_SCROLL:
                        usNotifica=CBN_LBSCROLL;
                     break;
                     case LN_ENTER:
                        usNotifica=CBN_ENTER;
                        WinSendMsg(pArea->hwndClass,WM_CLOSELIST,0L,0L);
                     break;
                  }
                  if (usNotifica) {
                     WinSendMsg( WinQueryWindow(pArea->hwndClass,QW_OWNER),
                                 WM_CONTROL,
                                 MPFROM2SHORT(pArea->idClass,usNotifica),
                                 mp2 );
                  }
               }
            break;
         }
      break;

      case WM_PAINT:
         {
            HPS hps;
            RECTL rc;
            POINTL ptl;
            pArea=WinQueryWindowPtr(hwnd, 0);
            hps = WinBeginPaint( hwnd, (HPS)NULL, &rc );
            WinQueryWindowRect(hwnd,&rc);
            rc.yBottom = rc.yTop - pArea->hightBtn - 4;

            rc.yTop--;
            rc.xRight--;

            GpiSetColor(hps,CLR_DARKGRAY);
            ptl.x = rc.xLeft;
            ptl.y = rc.yBottom;
            GpiMove(hps, &ptl);
            ptl.y = rc.yTop;
            GpiLine(hps, &ptl);
            ptl.x = rc.xRight;
            GpiLine(hps, &ptl);

            GpiSetColor(hps,CLR_WHITE);
            ptl.x = rc.xRight;
            ptl.y = rc.yTop;
            GpiLine(hps, &ptl);
            ptl.y = rc.yBottom;
            GpiLine(hps, &ptl);
            ptl.x = rc.xLeft;
            GpiLine(hps, &ptl);

            rc.yTop--;
            rc.yBottom++;
            rc.xLeft++;
            rc.xRight--;

            GpiSetColor(hps,CLR_BLACK);
            ptl.x = rc.xLeft;
            ptl.y = rc.yBottom;
            GpiMove(hps, &ptl);
            ptl.y = rc.yTop;
            GpiLine(hps, &ptl);
            ptl.x = rc.xRight;
            GpiLine(hps, &ptl);

            GpiSetColor(hps,CLR_PALEGRAY);
            ptl.x = rc.xRight;
            ptl.y = rc.yTop;
            GpiMove(hps, &ptl);
            ptl.y = rc.yBottom;
            GpiLine(hps, &ptl);
            ptl.x = rc.xLeft;
            GpiLine(hps, &ptl);

            WinEndPaint( hps );
            return(MRESULT)0;
         }
      break;

      case LM_QUERYITEMCOUNT     :
      case LM_INSERTITEM         :
      case LM_SETTOPINDEX        :
      case LM_DELETEITEM         :
      case LM_SELECTITEM         :
      case LM_QUERYSELECTION     :
      case LM_SETITEMTEXT        :
      case LM_QUERYITEMTEXTLENGTH:
      case LM_QUERYITEMTEXT      :
      case LM_SETITEMHANDLE      :
      case LM_QUERYITEMHANDLE    :
      case LM_SEARCHSTRING       :
      case LM_SETITEMHEIGHT      :
      case LM_QUERYTOPINDEX      :
      case LM_DELETEALL          :
      case LM_INSERTMULTITEMS    :
      case LM_SETITEMWIDTH       :
          {
            MRESULT mr;
            pArea=WinQueryWindowPtr(hwnd, 0);
            mr=WinSendMsg(pArea->hwndList,msg,mp1,mp2);
            if (msg==LM_SELECTITEM)
               WinInvalidateRect(pArea->hwndText,NULL,TRUE);
            return(mr);
          }
      break;

      case WM_MOVE:
          pArea=WinQueryWindowPtr(hwnd, 0);
          if (pArea->fStatus) {
             RECTL newRc;
             WinQueryWindowRect(hwnd,&newRc);
             WinMapWindowPoints(hwnd,HWND_DESKTOP,(PPOINTL)&newRc,1);
             WinSetWindowPos(pArea->hwndList,HWND_TOP,
                             newRc.xLeft, newRc.yBottom, 0L, 0L,
                             SWP_MOVE );
          }
      break;

      case WM_HITTEST:
          {
             POINTL ptl;
             RECTL newRc;
             pArea=WinQueryWindowPtr(hwnd, 0);
             ptl.x=SHORT1FROMMP(mp1);
             ptl.y=SHORT2FROMMP(mp1);
             WinQueryWindowRect(pArea->hwndList,&newRc);
             if (ptl.x<=newRc.xRight && ptl.y<=newRc.yTop) {
                return(MRESULT)HT_TRANSPARENT;
             } else {
                return(MRESULT)HT_NORMAL;
             }
          }
      break;

      case WM_OPENLIST:
          {
             POINTL ptl;
             SHORT index;
             HWND  hwndOtherList;
             ptl.x=0;
             ptl.y=0;
             pArea=WinQueryWindowPtr(hwnd, 0);
             if (pArea->fStatus) break;

             hwndOtherList=WinWindowFromID(HWND_DESKTOP,CBID_LIST);
             if (hwndOtherList) {
                hwndOtherList=WinQueryWindow(hwndOtherList,QW_OWNER);
                if ((BOOL)WinSendMsg(hwndOtherList,CBM_ISLISTSHOWING,0L,0L)==TRUE)
                   WinSendMsg(hwndOtherList,CBM_SHOWLIST,MPFROMSHORT(FALSE),0L);
             }

             WinMapWindowPoints(pArea->hwndClass,HWND_DESKTOP,&ptl,1);
             index=(SHORT)WinSendMsg(pArea->hwndList,LM_QUERYSELECTION,MPFROMSHORT(LIT_FIRST),0L);
             if (index==LIT_NONE)
                WinSendMsg(pArea->hwndList,LM_SELECTITEM,MPFROMSHORT(0),MPFROMSHORT(TRUE));
             WinSetWindowPos(pArea->hwndList, HWND_TOP,
                             ptl.x, ptl.y, 0L, 0L,
                             SWP_MOVE | SWP_ZORDER | SWP_SHOW);
             pArea->fStatus=TRUE;
             WinInvalidateRect(pArea->hwndBtn,NULL,TRUE);
             WinSetFocus( HWND_DESKTOP, pArea->hwndClass);
          }
      break;

      case WM_SETFOCUS:
         pArea=WinQueryWindowPtr(hwnd, 0);
         if (SHORT1FROMMP(mp2)==FALSE) {
           pArea->focus=FALSE;
           if ( HWNDFROMMP(mp1)!=pArea->hwndList )
              WinSendMsg(pArea->hwndClass,WM_CLOSELIST,0L,0L);
         } else
           pArea->focus=TRUE;
         WinInvalidateRect(pArea->hwndText,NULL,TRUE);
      break;

      case WM_CLOSELIST:
          pArea=WinQueryWindowPtr(hwnd, 0);
          if (!pArea->fStatus) break;
          WinShowWindow(pArea->hwndList,FALSE);
          WinInvalidateRect(pArea->hwndBtn,NULL,TRUE);
          WinInvalidateRect(pArea->hwndText,NULL,TRUE);
          pArea->fStatus=FALSE;
      break;

      case WM_DESTROY:
          pArea=WinQueryWindowPtr(hwnd, 0);
          WinDestroyWindow(pArea->hwndBtn);
          WinDestroyWindow(pArea->hwndText);
          WinDestroyWindow(pArea->hwndList);
          DosFreeMem( pArea );
      break;

      case WM_PRESPARAMCHANGED:
          {
             CHAR  buffer[40];
             ULONG ulPresParams;
             memset(buffer,0,sizeof(buffer));
             pArea=WinQueryWindowPtr(hwnd, 0);

             WinQueryPresParam(hwnd, LONGFROMMP(mp1),
                               0, &ulPresParams,
                               (ULONG)sizeof(buffer), (PVOID)buffer,
                               QPF_NOINHERIT);

             if (LONGFROMMP(mp1)==PP_BACKGROUNDCOLOR)
                 pArea->ulColorBack=PC_RESERVED * 16777216 + *(ULONG*)buffer;

             WinSetPresParam(pArea->hwndText, LONGFROMMP(mp1),
                            (ULONG)sizeof(buffer),
                            (PVOID)buffer);
             WinSetPresParam(pArea->hwndList, LONGFROMMP(mp1),
                            (ULONG)sizeof(buffer),
                            (PVOID)buffer);
         }
      break;
   }
   return ( WinDefWindowProc(hwnd, msg, mp1, mp2) );
}

MRESULT EXPENTRY Text_Proc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 )
{
   PPRIVATESTRUCT pArea;

   switch( msg )
   {
      case WM_PAINT:
         {
            PCHAR pChar=NULL;
            SHORT cbText;
            OWNERITEM ownerItem;
            RECTL rcOr;

            pArea=WinQueryWindowPtr(hwnd, 0);
            memset(&ownerItem,0,sizeof(ownerItem));
            ownerItem.hwnd = pArea->hwndClass;
            ownerItem.hps = WinBeginPaint( hwnd, (HPS)NULL, &ownerItem.rclItem );
            ownerItem.idItem=(SHORT)WinSendMsg(pArea->hwndList,LM_QUERYSELECTION,MPFROMSHORT(LIT_FIRST),0L);

            if ( pArea->focus ) ownerItem.fsState=TRUE;
                           else ownerItem.fsState=FALSE;

            ownerItem.fsStateOld=TRUE;
            ownerItem.fsAttribute=FALSE;
            ownerItem.fsAttributeOld=TRUE;
            WinQueryWindowRect(hwnd,&ownerItem.rclItem);

            if ( pArea->fOwnerDraw ) {
              GpiQueryLogColorTable(ownerItem.hps, 0L, 0L, 16L, pArea->alTable);
              pArea->alTable[CLR_BACKGROUND] = pArea->ulColorBack;
              GpiCreateLogColorTable(ownerItem.hps, 0L, LCOLF_CONSECRGB, 0L, 16, pArea->alTable);
              ownerItem.rclItem.xRight--;
              ownerItem.rclItem.yTop--;
              rcOr=ownerItem.rclItem;
            }

            if ( pArea->fOwnerDraw &&
                 (WinSendMsg(WinQueryWindow(pArea->hwndClass,QW_OWNER),WM_DRAWITEM,MPFROMSHORT((SHORT)ownerItem.idItem),MPFROMP(&ownerItem))) ) {

              /* OwnerClass Painted */
              POINTL ptl;
              GpiSetColor(ownerItem.hps,CLR_BACKGROUND);
              ptl.x=rcOr.xLeft;
              ptl.y=rcOr.yTop;
              GpiMove( ownerItem.hps, &ptl );
              ptl.x=rcOr.xRight;
              GpiLine( ownerItem.hps, &ptl );
              ptl.y=rcOr.yBottom;
              GpiLine( ownerItem.hps, &ptl );

            } else {
              if (ownerItem.idItem!=LIT_NONE) {
                 cbText=(SHORT)WinSendMsg(pArea->hwndList,LM_QUERYITEMTEXTLENGTH,MPFROMSHORT((SHORT)ownerItem.idItem),0L);
                 if (cbText!=LIT_ERROR) {
                    DosAllocMem((PVOID)&pChar, cbText+1, PAG_COMMIT | PAG_READ | PAG_WRITE );
                    memset(pChar,0,cbText+1);
                    WinSendMsg(pArea->hwndList,LM_QUERYITEMTEXT,MPFROM2SHORT((SHORT)ownerItem.idItem,cbText),MPFROMP(pChar));
                 }
              }
              if (pChar) {
                WinDrawText(ownerItem.hps,-1L,pChar,&ownerItem.rclItem,CLR_BLACK,CLR_BACKGROUND,DT_LEFT | DT_VCENTER | DT_ERASERECT);
              } else {
                WinFillRect(ownerItem.hps,&ownerItem.rclItem,CLR_BACKGROUND);
              }
              DosFreeMem(pChar);
            }

            WinEndPaint( ownerItem.hps );
            return(MRESULT)0;
         }
      break;
   }
   return(WinDefWindowProc(hwnd, msg, mp1, mp2));
}

/**************************************************************************\
* Disegno del bottone                                                      *
\**************************************************************************/
VOID DrawButton ( PPRIVATESTRUCT pArea )
{
   RECTL  rc;
   POINTL ptl;
   HPS    hps;

   hps = WinBeginPaint( pArea->hwndBtn, (HPS)NULL, &rc );
   WinQueryWindowRect( pArea->hwndBtn , &rc );

   rc.yTop--;
   rc.xRight--;

   GpiSetColor(hps,CLR_DARKGRAY);
   ptl.x = rc.xLeft;
   ptl.y = rc.yBottom;
   GpiMove(hps, &ptl);
   ptl.y = rc.yTop;
   GpiLine(hps, &ptl);
   rc.xLeft++;

   if (pArea->fStatus)
      GpiSetColor(hps,CLR_DARKGRAY);
   else
      GpiSetColor(hps,CLR_WHITE);
   ptl.x = rc.xLeft;
   ptl.y = rc.yBottom;
   GpiMove(hps, &ptl);
   ptl.y = rc.yTop;
   GpiLine(hps, &ptl);
   ptl.x = rc.xRight;
   GpiLine(hps, &ptl);

   if (pArea->fStatus)
      GpiSetColor(hps,CLR_WHITE);
   else
      GpiSetColor(hps,CLR_DARKGRAY);
   ptl.x = rc.xRight;
   ptl.y = rc.yTop;
   GpiMove(hps, &ptl);
   ptl.y = rc.yBottom;
   GpiLine(hps, &ptl);
   ptl.x = rc.xLeft;
   GpiLine(hps, &ptl);

   rc.yTop--;
   rc.yBottom++;
   rc.xLeft++;
   rc.xRight--;

   if (pArea->fStatus)
      GpiSetColor(hps,CLR_DARKGRAY);
   else
      GpiSetColor(hps,CLR_WHITE);
   ptl.x = rc.xLeft;
   ptl.y = rc.yBottom;
   GpiMove(hps, &ptl);
   ptl.y = rc.yTop;
   GpiLine(hps, &ptl);
   ptl.x = rc.xRight;
   GpiLine(hps, &ptl);

   if (pArea->fStatus)
      GpiSetColor(hps,CLR_WHITE);
   else
      GpiSetColor(hps,CLR_DARKGRAY);
   ptl.x = rc.xRight;
   ptl.y = rc.yTop;
   GpiMove(hps, &ptl);
   ptl.y = rc.yBottom;
   GpiLine(hps, &ptl);
   ptl.x = rc.xLeft;
   GpiLine(hps, &ptl);

   rc.yBottom++;
   rc.xLeft++;

   WinFillRect(hps,&rc,CLR_PALEGRAY);

   if (pArea->fStatus) {
      rc.xLeft++;
      rc.xRight++;
      rc.yTop--;
      rc.yBottom--;
   }
   rc.xLeft=((rc.xRight-rc.xLeft-pArea->widthBmp)/2)+rc.xLeft;
   rc.yBottom=((rc.yTop-rc.yBottom-pArea->hightBmp)/2)+rc.yBottom;

   WinDrawBitmap( hps , pArea->hbmp, (PRECTL)NULL , (PPOINTL)&rc , 0L, 0L, DBM_NORMAL );

   WinEndPaint( hps );
   return;
}

MRESULT EXPENTRY Btn_Proc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 )
{
   PPRIVATESTRUCT pArea;

   switch( msg )
   {
     case WM_PAINT:
        DrawButton( WinQueryWindowPtr(hwnd, 0) );
     break;

     case WM_BUTTON1CLICK:
     case WM_BUTTON1DBLCLK:
        pArea=WinQueryWindowPtr(hwnd, 0);
        if (pArea->fStatus) {
           WinSendMsg(pArea->hwndClass,WM_CLOSELIST,0L,0L);
        } else {
           WinSendMsg(pArea->hwndClass,WM_OPENLIST, 0L,0L);
        }
     break;

   }
   return(WinDefWindowProc(hwnd, msg, mp1, mp2));
}

MRESULT EXPENTRY List_Proc( HWND hwnd, ULONG  msg, MPARAM mp1, MPARAM mp2 )
{
   PPRIVATESTRUCT pArea;

   switch (msg) {
  // case WM_BUTTON1CLICK:
     case WM_BUTTON1UP:
        pArea=WinQueryWindowPtr(hwnd, 0);
        WinSendMsg(pArea->hwndClass,WM_CLOSELIST,0L,0L);
        WinSendMsg( WinQueryWindow(pArea->hwndClass,QW_OWNER),
                    WM_CONTROL,
                    MPFROM2SHORT(pArea->idClass,CBN_ENTER),
                    MPFROMHWND( pArea->hwndClass ));
     break;

     case WM_SETFOCUS:
        pArea=WinQueryWindowPtr(hwnd, 0);
        WinSetFocus(HWND_DESKTOP,pArea->hwndClass);
        return(MRESULT)0;
     break;
   }

   return( pfnwpListProc( hwnd, msg, mp1, mp2 ) );
}
