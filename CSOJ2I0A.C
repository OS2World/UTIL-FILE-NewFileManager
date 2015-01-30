#define INCL_DOSSEMAPHORES
#define INCL_WINSHELLDATA
#define INCL_WINLISTBOXES
#define INCL_BASE
#define INCL_SPL
#define INCL_SPLDOSPRINT
#define INCL_DOSMEMMGR
#define INCL_WIN
#define INCL_GPICONTROL
#define INCL_PM
#define INCL_WIN
#define INCL_GPI
#define INCL_DOS
#define INCL_DEV
#define INCL_GPICONTROL

#include <os2.h>
#include <mmioos2.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include "csoj2i0a.h"

#define LONGFromRGB(R,G,B) (LONG)(((LONG)R<<16)+((LONG)G<<8)+(LONG)B)
#define CLR_USERBACKGROUND  16
#define TOT_PAL             CLR_USERBACKGROUND + 1
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
 204, 204, 204,     // CLR_USERBACKGROUND
};

#define DRAW_ORIGINALE      0
#define DRAW_THUMBNAIL      1
#define DRAW_STRETCH        2

#define MAX_FILENAME_SIZE    CCHMAXPATH
#define MIN_RIDUZIONE        10
#define MAX_INGRANDIMENTO    300
#define MAX_COLORSFORBITBLT  32
#define MAX_ROWFORCOUNTCOLOR 10

#define INI_APPL            "APPL_MM"
#define INI_KEY             "LASTDIR"

#define WM_CSOJ2I0A_SETINDIC     WM_USER + 1
#define WM_CSOJ2I0A_AZZERAINDIC  WM_USER + 2

/******************************************************************************/
/* Strutture necessarie per conteggio colori                                  */
/******************************************************************************/
#define NON_ASSEGNATO -1
LONG   tab[MAX_COLORSFORBITBLT];

typedef struct {
  unsigned c0: 1;
  unsigned c1: 1;
  unsigned c2: 1;
  unsigned c3: 1;
  unsigned c4: 1;
  unsigned c5: 1;
  unsigned c6: 1;
  unsigned c7: 1;
} BIT1;
typedef struct {
  unsigned c0: 4;
  unsigned c1: 4;
} BIT4;
typedef struct {
   CHAR c;
} BIT8;
typedef struct {
   union {
     CHAR  c[3];
     ULONG cl;
   } un;
} BIT24;
/*****************************************************************************/

typedef struct _POSSTRUCT
   {
       ULONG  xClass;
       ULONG  yClass;
       ULONG  cxClass;
       ULONG  cyClass;

       ULONG  xBmp;
       ULONG  yBmp;
       ULONG  cxBmp;
       ULONG  cyBmp;

       ULONG  xBtn1;
       ULONG  yBtn1;
       ULONG  cxBtn1;
       ULONG  cyBtn1;

       ULONG  xBtn2;
       ULONG  yBtn2;
       ULONG  cxBtn2;
       ULONG  cyBtn2;

       ULONG  xBtn3;
       ULONG  yBtn3;
       ULONG  cxBtn3;
       ULONG  cyBtn3;

       ULONG  xSpin;
       ULONG  ySpin;
       ULONG  cxSpin;
       ULONG  cySpin;

       ULONG  xText;
       ULONG  yText;
       ULONG  cxText;
       ULONG  cyText;

       ULONG  xFolder;
       ULONG  yFolder;
       ULONG  cxFolder;
       ULONG  cyFolder;

       ULONG  xIndic;
       ULONG  yIndic;
       ULONG  cxIndic;
       ULONG  cyIndic;
   } POSSTRUCT;
typedef POSSTRUCT *PPOSSTRUCT;

typedef struct _PRIVATESTRUCT
   {
     HPS      hpsMemory;
     HDC      hdcMemory;
     HBITMAP  hbm;
     SIZEL    sizeBmp;

     ULONG    tipoDraw;      /* 0=ORIGINALE 1=THUMBNAIL 2=STRECTH */
     ULONG    tipo_ROP;      /* Opzione per BitBlt */
     ULONG    tipo_BBO;      /* Opzione per BitBlt */

     BOOL     bottoni;       /* TRUE=SI FALSE=NO Se compaiono i bottoncini sotto */
     BOOL     modificabile;  /* TRUE=SI FALSE=NO Se l'utente pu• modificare l'immagine */
     HPAL     hPal;

     HWND     hwndParent;
     HWND     hwndBmp;
     HWND     hwndBtnOriginale;
     HWND     hwndBtnThumbnail;
     HWND     hwndBtnStretch;
     HWND     hwndSpin;
     HWND     hwndTesto;
     HWND     hwndBtnFolder;
     HWND     hwndIndic;
     ULONG    hwndClass;
     ULONG    idClass;

     BOOL     fTrackeable;
     BOOL     fTrack;
     POINTL   ptl;
     POINTL   ptl2;
     LONG     deltax;
     LONG     deltay;
     float    coeff;
     SIZEL    ImageSize;
     CHAR     szFileName[MAX_FILENAME_SIZE];
     ULONG    alTable[TOT_PAL];

     ULONG    percParz;

   } PRIVATESTRUCT;
typedef PRIVATESTRUCT *PPRIVATESTRUCT;

MRESULT EXPENTRY CSOJ2I0A_Proc    ( HWND, ULONG, MPARAM, MPARAM );
MRESULT EXPENTRY Indicatore_Proc  ( HWND, ULONG, MPARAM, MPARAM );
MRESULT EXPENTRY Bmp_Proc         ( HWND, ULONG, MPARAM, MPARAM );
HBITMAP          GetBitmap        ( HWND, PSZ );
VOID             DrawBitMap       ( HWND );
HBITMAP          LoadBitmap       ( HAB, HDC, HPS *, PSZ , HWND );
VOID             DrawButton       ( SHORT, PUSERBUTTON );
PCHAR            CaseName         ( PCHAR );
BOOL    EXPENTRY OpenFileDialog   ( HWND, PSZ, PSZ, PSZ, PSZ );
VOID             CalcolaPosizioni ( HWND , PPOSSTRUCT );
VOID             SetPosWindows    ( HWND , PPRIVATESTRUCT, PPOSSTRUCT );
ULONG            ContaColori      ( PVOID , ULONG , ULONG );
VOID             DrawBorder       ( HPS , RECTL*, PSZ );
VOID             Cornice          ( HPS, ULONG, ULONG, ULONG, ULONG );
VOID             SetCoeff         ( PPRIVATESTRUCT, ULONG );
BOOL             CaricaFormati    ( VOID );
BOOL             CercaFormato     ( PSZ );
VOID             FourccToString   ( FOURCC, PSZ );
int              compareExt       ( const void *, const void * );
VOID             ChangePresParam  ( PPRIVATESTRUCT , HWND, ULONG );

static HAB hab;
typedef struct {
  char Ext[ sizeof(FOURCC) + 1 ];
} EXTEL;
EXTEL* pExtTab=NULL;
ULONG  elExtTab=0;

HBITMAP  hBmp1         = NULLHANDLE;
HBITMAP  hBmp1A        = NULLHANDLE;
HBITMAP  hBmp2         = NULLHANDLE;
HBITMAP  hBmp2A        = NULLHANDLE;
HBITMAP  hBmp3         = NULLHANDLE;
HBITMAP  hBmp3A        = NULLHANDLE;
HBITMAP  hBmpFolder    = NULLHANDLE;
HBITMAP  hBmpFolderA   = NULLHANDLE;
HMODULE  hmod;
HPOINTER hPtrA;
HPOINTER hPtrC;

BOOL CSOJ2I0A_REGISTRA( HAB lochab )
{
   CLASSINFO  clsiTmp;

   hab=lochab;

   /* Se classe gi… registrata */
   if( WinQueryClassInfo( hab,
                          (PSZ)"CSOJ2I0A",
                          &clsiTmp )==TRUE ) {
      return( TRUE );
   }

   if( ! WinRegisterClass( hab,
                           (PSZ)"CSOJ2I0A",
                           (PFNWP)CSOJ2I0A_Proc,
                           0L,
                           4L ) ) {
      WinMessageBox(HWND_DESKTOP,HWND_DESKTOP,"Not register class CSOJ2I0A","CSOJ2I0A",0L,0L);
      return( FALSE );
   }

   WinRegisterClass(hab,"clIndicatore",Indicatore_Proc, 0L, 4L);
   WinRegisterClass(hab,"clBmp",Bmp_Proc, 0L, 4L);

   DosQueryModuleHandle("CSOJ2I0A", &hmod);

   hPtrA = WinLoadPointer(HWND_DESKTOP,hmod,4451);
   hPtrC = WinLoadPointer(HWND_DESKTOP,hmod,4452);

   if (!CaricaFormati()) return( FALSE );

   return( TRUE );
}

MRESULT EXPENTRY CSOJ2I0A_Proc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 )
{
   PPRIVATESTRUCT pPrivateStruct;

   switch( msg )
   {
      case WM_CREATE:
         {
           PCREATESTRUCT pCreateStruct;
           POSSTRUCT ps;
           ULONG i;

           pCreateStruct=PVOIDFROMMP(mp2);

           DosAllocMem((PVOID)&pPrivateStruct, sizeof(PRIVATESTRUCT), PAG_COMMIT | PAG_READ | PAG_WRITE);
           memset(pPrivateStruct,0,sizeof(PRIVATESTRUCT));
           WinSetWindowPtr(hwnd, 0, pPrivateStruct);

           pPrivateStruct->hwndParent=pCreateStruct->hwndParent;
           pPrivateStruct->hwndClass=hwnd;
           pPrivateStruct->idClass=pCreateStruct->id;

           if (hBmp1==NULLHANDLE) {
              HPS hps=NULLHANDLE;
              hps=WinGetPS(hwnd);
              hBmp1       = GpiLoadBitmap(hps, hmod, 4401 , 0UL, 0UL);
              hBmp1A      = GpiLoadBitmap(hps, hmod, 4402 , 0UL, 0UL);
              hBmp2       = GpiLoadBitmap(hps, hmod, 4403 , 0UL, 0UL);
              hBmp2A      = GpiLoadBitmap(hps, hmod, 4404 , 0UL, 0UL);
              hBmp3       = GpiLoadBitmap(hps, hmod, 4405 , 0UL, 0UL);
              hBmp3A      = GpiLoadBitmap(hps, hmod, 4406 , 0UL, 0UL);
              hBmpFolder  = GpiLoadBitmap(hps, hmod, 4407 , 0UL, 0UL);
              hBmpFolderA = GpiLoadBitmap(hps, hmod, 4408 , 0UL, 0UL);
              WinReleasePS(hps);
           }

           pPrivateStruct->tipoDraw = DRAW_ORIGINALE;

           if ( (pCreateStruct->flStyle & WS_CSOJ2I0A_THUMBNAIL) == WS_CSOJ2I0A_THUMBNAIL) {
              pPrivateStruct->tipoDraw = DRAW_THUMBNAIL;
           }
           if ( (pCreateStruct->flStyle & WS_CSOJ2I0A_STRETCH) == WS_CSOJ2I0A_STRETCH) {
              pPrivateStruct->tipoDraw = DRAW_STRETCH;
           }

           if ( (pCreateStruct->flStyle & WS_CSOJ2I0A_BOTTONI) == WS_CSOJ2I0A_BOTTONI) {
              pPrivateStruct->bottoni = TRUE;
           } else {
              pPrivateStruct->bottoni = FALSE;
           }

           if ( (pCreateStruct->flStyle & WS_CSOJ2I0A_MODIFICABILE) == WS_CSOJ2I0A_MODIFICABILE) {
              pPrivateStruct->modificabile = TRUE;
           } else {
              pPrivateStruct->modificabile = FALSE;
           }

           pPrivateStruct->coeff=100;
           pPrivateStruct->deltax=0;
           pPrivateStruct->deltay=0;

           for (i=0;i<TOT_PAL;i++)
             pPrivateStruct->alTable[i] = PC_RESERVED * 16777216 + LONGFromRGB( rgb[i].bBlue, rgb[i].bGreen, rgb[i].bRed );
           pPrivateStruct->hPal=GpiCreatePalette(NULLHANDLE, LCOL_PURECOLOR, LCOLF_CONSECRGB, TOT_PAL, pPrivateStruct->alTable );

           CalcolaPosizioni(hwnd,&ps);

           pPrivateStruct->hwndBmp =
                 WinCreateWindow( hwnd, "clBmp", "",
                                  WS_SYNCPAINT | WS_VISIBLE,
                                  ps.xBmp, ps.yBmp, ps.cxBmp, ps.cyBmp,
                                  hwnd, HWND_TOP, ID_CSOJ2I0A_BITMAP, NULL, NULL);
           WinSetWindowPtr(pPrivateStruct->hwndBmp, 0, pPrivateStruct);

           pPrivateStruct->hwndIndic =
                 WinCreateWindow( hwnd, "clIndicatore", "",
                                  WS_SYNCPAINT | WS_VISIBLE,
                                  ps.xIndic, ps.yIndic, ps.cxIndic, ps.cyIndic,
                                  hwnd, HWND_TOP, ID_CSOJ2I0A_INDIC, NULL, NULL);
           WinSetWindowPtr(pPrivateStruct->hwndIndic, 0, pPrivateStruct);

           pPrivateStruct->hwndBtnOriginale =
                 WinCreateWindow( hwnd, WC_BUTTON, "#4401",
                                  WS_SYNCPAINT | WS_VISIBLE | BS_NOPOINTERFOCUS | BS_USERBUTTON | BS_PUSHBUTTON | WS_GROUP,
                                  ps.xBtn1, ps.yBtn1, ps.cxBtn1, ps.cyBtn1,
                                  hwnd, HWND_TOP, ID_CSOJ2I0A_BTN_ORIGINALE, NULL, NULL);

           pPrivateStruct->hwndBtnThumbnail =
                 WinCreateWindow( hwnd, WC_BUTTON, "#4402",
                                  WS_SYNCPAINT | WS_VISIBLE | BS_NOPOINTERFOCUS | BS_USERBUTTON | BS_PUSHBUTTON | WS_GROUP,
                                  ps.xBtn2, ps.yBtn2, ps.cxBtn2, ps.cyBtn2,
                                  hwnd, HWND_TOP, ID_CSOJ2I0A_BTN_THUMBNAIL, NULL, NULL);

           pPrivateStruct->hwndBtnStretch =
                 WinCreateWindow( hwnd, WC_BUTTON, "#4403",
                                  WS_SYNCPAINT | WS_VISIBLE | BS_NOPOINTERFOCUS | BS_USERBUTTON | BS_PUSHBUTTON | WS_GROUP,
                                  ps.xBtn3, ps.yBtn3, ps.cxBtn3, ps.cyBtn3,
                                  hwnd, HWND_TOP, ID_CSOJ2I0A_BTN_STRETCH, NULL, NULL);

           pPrivateStruct->hwndSpin =
                 WinCreateWindow( hwnd, WC_SPINBUTTON, "",
                                  WS_SYNCPAINT | WS_VISIBLE | SPBS_MASTER | SPBS_JUSTCENTER | SPBS_NUMERICONLY,
                                  ps.xSpin, ps.ySpin, ps.cxSpin, ps.cySpin,
                                  hwnd, HWND_TOP, ID_CSOJ2I0A_SPIN, NULL, NULL);
           WinSendMsg( pPrivateStruct->hwndSpin, SPBM_SETLIMITS, MPFROMLONG(MAX_INGRANDIMENTO), MPFROMLONG(MIN_RIDUZIONE) );
           WinSendMsg( pPrivateStruct->hwndSpin, SPBM_SETCURRENTVALUE, MPFROMLONG(100), 0L );

           pPrivateStruct->hwndTesto =
                 WinCreateWindow( hwnd, WC_STATIC, "Titolo della foto",
                                  WS_SYNCPAINT | WS_VISIBLE | SS_TEXT | DT_VCENTER | DT_LEFT,
                                  ps.xText, ps.yText, ps.cxText, ps.cyText,
                                  hwnd, HWND_TOP, ID_CSOJ2I0A_TESTO, NULL, NULL);

           if (pPrivateStruct->modificabile) {
              pPrivateStruct->hwndBtnFolder =
                    WinCreateWindow( hwnd, WC_BUTTON, "#4407",
                                     WS_SYNCPAINT | WS_VISIBLE | BS_NOPOINTERFOCUS | BS_USERBUTTON | BS_PUSHBUTTON | WS_GROUP,
                                     ps.xFolder, ps.yFolder, ps.cxFolder, ps.cyFolder,
                                     hwnd, HWND_TOP, ID_CSOJ2I0A_BTN_FOLDER, NULL, NULL);
           }

           if (pPrivateStruct->bottoni) {
              WinSendMsg(hwnd,WM_CSOJ2I0A_BOTTONI,MPFROMLONG(TRUE),0L);
           } else {
              WinSendMsg(hwnd,WM_CSOJ2I0A_BOTTONI,MPFROMLONG(FALSE),0L);
           }

           {
              CHAR szFontName[20];
              ULONG ulColor;
              memset(szFontName,0,sizeof(szFontName));
              strcpy (szFontName,"8.Helv");
              WinSetPresParam (hwnd, PP_FONTNAMESIZE,(ULONG)(strlen(szFontName)+(ULONG)1), (PSZ) szFontName);
              ulColor=CLR_PALEGRAY;
              WinSetPresParam (hwnd, PP_BACKGROUNDCOLORINDEX, (ULONG)sizeof(LONG), (PVOID)&ulColor);
              WinSetPresParam (pPrivateStruct->hwndTesto, PP_BACKGROUNDCOLORINDEX, (ULONG)sizeof(LONG), (PVOID)&ulColor);
              ulColor=CLR_BLACK;
              WinSetPresParam (hwnd, PP_FOREGROUNDCOLORINDEX, (ULONG)sizeof(LONG), (PVOID)&ulColor);
           }

           WinSetWindowPos( hwnd,
                            NULLHANDLE,
                            ps.xClass, ps.yClass, ps.cxClass, ps.cyClass,
                            SWP_SIZE | SWP_MOVE);

           switch (pPrivateStruct->tipoDraw) {
              case DRAW_ORIGINALE:
                 WinEnableWindow(pPrivateStruct->hwndSpin,TRUE);
              break;

              case DRAW_THUMBNAIL:
                 WinEnableWindow(pPrivateStruct->hwndSpin,FALSE);
              break;

              case DRAW_STRETCH:
                 WinEnableWindow(pPrivateStruct->hwndSpin,FALSE);
              break;
           }

           pCreateStruct->flStyle = WS_VISIBLE | WS_SYNCPAINT;
           WinSetWindowULong(hwnd,QWL_STYLE,WS_VISIBLE | WS_SYNCPAINT);

           GetBitmap( hwnd, pCreateStruct->pszText );
           WinSendMsg( hwnd, WM_SIZE , 0L, 0L );
         }
      break;

      case WM_PRESPARAMCHANGED:
           pPrivateStruct = WinQueryWindowPtr(hwnd, 0);
           ChangePresParam( pPrivateStruct, hwnd, LONGFROMMP(mp1) );
      break;

      case WM_PAINT:
         {
            HPS hps;
            RECTL WinDim;
            hps = WinBeginPaint( hwnd, (HPS)NULL, &WinDim );
            WinFillRect(hps, &WinDim, CLR_PALEGRAY);
            WinEndPaint( hps );
         }
      break;

      case WM_SIZE:
         {
            POSSTRUCT ps;
            CalcolaPosizioni(hwnd,&ps);
            pPrivateStruct = WinQueryWindowPtr(hwnd, 0);
            SetPosWindows(hwnd,pPrivateStruct,&ps);
            WinInvalidateRect(hwnd, NULL, TRUE);
         }
      break;

      case WM_CONTROL :
         switch ( SHORT2FROMMP ( mp1 ) ) {
             case BN_PAINT:
                 DrawButton( SHORT1FROMMP(mp1), ( PUSERBUTTON )mp2 );
             break ;

             case SPBN_DOWNARROW:
             case SPBN_UPARROW:
             case SPBN_CHANGE:
                 {
                    LONG coeff=0;
                    pPrivateStruct = WinQueryWindowPtr(hwnd, 0);
                    WinSendMsg( pPrivateStruct->hwndSpin, SPBM_QUERYVALUE,
                                MPFROMP(&coeff),
                                MPFROM2SHORT(0,SPBQ_UPDATEIFVALID) );
                    SetCoeff( pPrivateStruct, coeff );
                 }
             break;
         }
      break;

      case WM_COMMAND:
         switch ( SHORT1FROMMP( mp1 ) ) {
             case ID_CSOJ2I0A_BTN_ORIGINALE:
                pPrivateStruct = WinQueryWindowPtr(hwnd, 0);
                pPrivateStruct->tipoDraw = DRAW_ORIGINALE;
                WinInvalidateRect(pPrivateStruct->hwndBmp, NULL, TRUE);
                WinEnableWindow(pPrivateStruct->hwndSpin,TRUE);
             break;

             case ID_CSOJ2I0A_BTN_THUMBNAIL:
                pPrivateStruct = WinQueryWindowPtr(hwnd, 0);
                pPrivateStruct->tipoDraw = DRAW_THUMBNAIL;
                WinInvalidateRect(pPrivateStruct->hwndBmp, NULL, TRUE);
                WinEnableWindow(pPrivateStruct->hwndSpin,FALSE);
             break;

             case ID_CSOJ2I0A_BTN_STRETCH:
                pPrivateStruct = WinQueryWindowPtr(hwnd, 0);
                pPrivateStruct->tipoDraw = DRAW_STRETCH;
                WinInvalidateRect(pPrivateStruct->hwndBmp, NULL, TRUE);
                WinEnableWindow(pPrivateStruct->hwndSpin,FALSE);
             break;

             case ID_CSOJ2I0A_BTN_FOLDER:
                pPrivateStruct = WinQueryWindowPtr(hwnd, 0);
                {
                   PSZ pszFileName;
                   PSZ pszPathName;
                   PCHAR pChar;
                   HWND hwndDlg;
                   ULONG idCSOJ2I0A;
                   hwndDlg = WinQueryWindow(hwnd,QW_PARENT);
                   idCSOJ2I0A = WinQueryWindowUShort(hwnd,QWS_ID);

                   pszFileName = malloc ( MAX_FILENAME_SIZE );
                   if( pszFileName == NULL ) break;

                   pszPathName = malloc ( MAX_FILENAME_SIZE );
                   if( pszPathName == NULL ) break;

                   memset(pszPathName,0,MAX_FILENAME_SIZE);
                   if (PrfQueryProfileString( HINI_PROFILE, INI_APPL, INI_KEY, NULL,
                                          pszPathName, MAX_FILENAME_SIZE ))
                      strcat(pszPathName,"\\*.*");
                   else
                      strcat(pszPathName,"*.*");

                   if ( OpenFileDialog ( hwndDlg,
                                         "Selezionare un'immagine",
                                         pszPathName,
                                         "Aprire",
                                         pszFileName ) == TRUE ) {

                      memset(pszPathName,0,MAX_FILENAME_SIZE);
                      strcpy(pszPathName,pszFileName);
                      pChar=strrchr(pszPathName,'\\');
                      if (pChar) *pChar=0;
                      PrfWriteProfileString( HINI_PROFILE, INI_APPL, INI_KEY, pszPathName);

                      if (GetBitmap( hwnd, pszFileName )!=NULLHANDLE) {
                         WinSendMsg( hwndDlg, WM_CONTROL,
                                     MPFROM2SHORT(idCSOJ2I0A,WN_CSOJ2I0A_MODIFIED),
                                     MPFROMP(pszFileName));
                      } else {
                         strcpy(pszFileName,pPrivateStruct->szFileName);
                         GetBitmap( hwnd, pszFileName );
                      }
                   }
                   free ( pszFileName );
                   free ( pszPathName );
                }
             break;
         }
      break;

      case WM_DESTROY:
           pPrivateStruct = WinQueryWindowPtr(hwnd, 0);
           GpiDeletePalette(pPrivateStruct->hPal);
           DosFreeMem(pPrivateStruct);
      break;

      case WM_CSOJ2I0A_SET_FOTO:
           if ( GetBitmap( hwnd, MPFROMP(mp1)) == NULLHANDLE )
                   return((MRESULT)FALSE);
              else return((MRESULT)TRUE);
      break;

      case WM_CSOJ2I0A_STAMPA:
           ;
      break;

      case WM_CSOJ2I0A_GET_HBM:
           pPrivateStruct = WinQueryWindowPtr(hwnd, 0);
           return(MRFROMLONG(pPrivateStruct->hbm));
      break;

      case WM_CSOJ2I0A_SET_COEFF:
           pPrivateStruct = WinQueryWindowPtr(hwnd, 0);
           SetCoeff(pPrivateStruct,LONGFROMMP(mp1));
      break;

      case WM_CSOJ2I0A_TEST_EXT:
           return( (MRESULT)CercaFormato ( mp1 ));
      break;

      case WM_CSOJ2I0A_BOTTONI:
         {
            POSSTRUCT ps;
            pPrivateStruct = WinQueryWindowPtr(hwnd, 0);
            if (LONGFROMMP(mp1)==TRUE) {
              pPrivateStruct->bottoni=TRUE;
              WinShowWindow(pPrivateStruct->hwndBtnOriginale,TRUE);
              WinShowWindow(pPrivateStruct->hwndBtnThumbnail,TRUE);
              WinShowWindow(pPrivateStruct->hwndBtnStretch  ,TRUE);
              WinShowWindow(pPrivateStruct->hwndSpin        ,TRUE);
              WinShowWindow(pPrivateStruct->hwndTesto       ,TRUE);
              WinShowWindow(pPrivateStruct->hwndBtnFolder   ,TRUE);
            } else {
              pPrivateStruct->bottoni=FALSE;
              WinShowWindow(pPrivateStruct->hwndBtnOriginale,FALSE);
              WinShowWindow(pPrivateStruct->hwndBtnThumbnail,FALSE);
              WinShowWindow(pPrivateStruct->hwndBtnStretch  ,FALSE);
              WinShowWindow(pPrivateStruct->hwndSpin        ,FALSE);
              WinShowWindow(pPrivateStruct->hwndTesto       ,FALSE);
              WinShowWindow(pPrivateStruct->hwndBtnFolder   ,FALSE);
            }
            CalcolaPosizioni(hwnd,&ps);
            SetPosWindows(hwnd,pPrivateStruct,&ps);
            WinInvalidateRect(hwnd, NULL, TRUE);
         }
      break;

      case WM_BUTTON1DOWN:
          pPrivateStruct = WinQueryWindowPtr(hwnd, 0);
          if (pPrivateStruct->tipoDraw != DRAW_ORIGINALE) break;
          if (!pPrivateStruct->fTrackeable) break;
          WinQueryPointerPos(HWND_DESKTOP,&pPrivateStruct->ptl);
          pPrivateStruct->ptl.x += pPrivateStruct->deltax;
          pPrivateStruct->ptl.y += pPrivateStruct->deltay;
          WinSetCapture(HWND_DESKTOP, hwnd);
          pPrivateStruct->fTrack=TRUE;
          return(MRESULT)0;
      break;

      case WM_BUTTON1UP:
          pPrivateStruct = WinQueryWindowPtr(hwnd, 0);
          if (pPrivateStruct->tipoDraw != DRAW_ORIGINALE) break;
          if (!pPrivateStruct->fTrackeable) break;
          WinSetCapture(HWND_DESKTOP, NULLHANDLE);
          pPrivateStruct->fTrack=FALSE;
          return(MRESULT)0;
      break;

      case WM_MOUSEMOVE:
          pPrivateStruct = WinQueryWindowPtr(hwnd, 0);
          if (pPrivateStruct->fTrackeable) {
              WinQueryPointerPos(HWND_DESKTOP,&pPrivateStruct->ptl2);
              WinInvalidateRect(pPrivateStruct->hwndBmp, NULL, TRUE);
              WinSetPointer(HWND_DESKTOP,hPtrC);
              return(MRESULT)0;
          }
      break;

      case WM_BUTTON1DBLCLK:
          pPrivateStruct = WinQueryWindowPtr(hwnd, 0);
          if (pPrivateStruct->tipoDraw == DRAW_ORIGINALE) {
             pPrivateStruct->coeff=100;
             WinSendMsg( pPrivateStruct->hwndSpin, SPBM_SETCURRENTVALUE, MPFROMLONG(100), 0L );
             WinInvalidateRect(pPrivateStruct->hwndBmp, NULL, TRUE);
          }
      break;

   }
   return(WinDefWindowProc(hwnd, msg, mp1, mp2));
}

HBITMAP LoadBitmap ( HAB hab,
                     HDC hdc,
                     HPS *hps,
                     PSZ pszFileName,
                     HWND hwndClass )
{
    PCHAR         pChar;
    HBITMAP       hbm;
    MMIOINFO      mmioinfo;
    MMFORMATINFO  mmFormatInfo;
    HMMIO         hmmio;
    ULONG         ulImageHeaderLength;
    MMIMAGEHEADER mmImgHdr;
    ULONG         ulBytesRead;
    ULONG         dwNumRowBytes;
    PBYTE         pRowBuffer;
    ULONG         dwRowCount;
    SIZEL         ImageSize;
    ULONG         dwHeight, dwWidth;
    SHORT         wBitCount;
    FOURCC        fccStorageSystem;
    ULONG         dwPadBytes;
    ULONG         dwRowBits;
    ULONG         ulReturnCode;
    ULONG         dwReturnCode;
    FOURCC        fccIOProc;
    CHAR          szString[MAX_FILENAME_SIZE];
    CHAR          szPath[MAX_FILENAME_SIZE];
    ULONG         numColors,ulTmp;
    BOOL          fConta;
    PPRIVATESTRUCT pPrivateStruct;

    pPrivateStruct = WinQueryWindowPtr(hwndClass, 0);

    ulReturnCode = mmioIdentifyFile ( pszFileName,
                                      0L,
                                      &mmFormatInfo,
                                      &fccStorageSystem,
                                      0L,
                                      0L);

    /*
     *  If this file was NOT identified, then this function won't
     *  work, so return an error by indicating an empty bitmap.
     */

    if ( ulReturnCode == MMIO_ERROR ) {
        WinSetWindowText(pPrivateStruct->hwndTesto,"Immagine non disponibile");
        return (0L);
    }

    /*
     *  If mmioIdentifyFile did not find a custom-written IO proc which
     *  can understand the image file, then it will return the DOS IO Proc
     *  info because the image file IS a DOS file.
     */

    if( mmFormatInfo.fccIOProc == FOURCC_DOS ) {
        WinSetWindowText(pPrivateStruct->hwndTesto,"Immagine non disponibile");
        return ( 0L );
    }

    /*
     *  Ensure this is an IMAGE IOproc, and that it can read
     *  translated data
     */

    if ( (mmFormatInfo.ulMediaType != MMIO_MEDIATYPE_IMAGE) ||
         ((mmFormatInfo.ulFlags & MMIO_CANREADTRANSLATED) == 0) ) {
        WinSetWindowText(pPrivateStruct->hwndTesto,"Immagine non disponibile");
        return ( 0L );
    } else {
        fccIOProc = mmFormatInfo.fccIOProc;
    }

    /* Clear out and initialize mminfo structure */
    WinSendMsg( pPrivateStruct->hwndIndic, WM_CSOJ2I0A_SETINDIC, MPFROMLONG(6), MPFROMLONG(100) );

    memset ( &mmioinfo, 0L, sizeof ( MMIOINFO ) );
    mmioinfo.fccIOProc = fccIOProc;
    mmioinfo.ulTranslate = MMIO_TRANSLATEHEADER | MMIO_TRANSLATEDATA;

    hmmio = mmioOpen ( (PSZ) pszFileName,
                       &mmioinfo,
                       MMIO_READ | MMIO_DENYWRITE | MMIO_NOIDENTIFY );

    if ( ! hmmio ) {
        // If file could not be opened, return with error
        WinSetWindowText(pPrivateStruct->hwndTesto,"Immagine non disponibile");
        return (0L);
    }

    WinSendMsg( pPrivateStruct->hwndIndic, WM_CSOJ2I0A_SETINDIC, MPFROMLONG(12), MPFROMLONG(100) );

    dwReturnCode = mmioQueryHeaderLength ( hmmio,
                                         (PLONG)&ulImageHeaderLength,
                                          0L,
                                          0L);

    if ( ulImageHeaderLength != sizeof ( MMIMAGEHEADER ) ) {
        /* We have a problem.....possibly incompatible versions */
       mmioClose (hmmio, 0L);
       WinSetWindowText(pPrivateStruct->hwndTesto,"Immagine non disponibile");
       return (0L);
    }

    WinSendMsg( pPrivateStruct->hwndIndic, WM_CSOJ2I0A_SETINDIC, MPFROMLONG(18), MPFROMLONG(100) );

    ulReturnCode = mmioGetHeader ( hmmio,
                                   &mmImgHdr,
                                   (LONG) sizeof ( MMIMAGEHEADER ),
                                   (PLONG)&ulBytesRead,
                                   0L,
                                   0L);

    if ( ulReturnCode != MMIO_SUCCESS ) {
       /* Header unavailable */
       mmioClose (hmmio, 0L);
       WinSetWindowText(pPrivateStruct->hwndTesto,"Immagine non disponibile");
       return (0L);
    }

    WinSendMsg( pPrivateStruct->hwndIndic, WM_CSOJ2I0A_SETINDIC, MPFROMLONG(24), MPFROMLONG(100) );

    /*
     *  Determine the number of bytes required, per row.
     *      PLANES MUST ALWAYS BE = 1
     */

    dwHeight = mmImgHdr.mmXDIBHeader.BMPInfoHeader2.cy;
    dwWidth = mmImgHdr.mmXDIBHeader.BMPInfoHeader2.cx;
    wBitCount = mmImgHdr.mmXDIBHeader.BMPInfoHeader2.cBitCount;
    dwRowBits = dwWidth * mmImgHdr.mmXDIBHeader.BMPInfoHeader2.cBitCount;
    dwNumRowBytes = dwRowBits >> 3;

    WinSendMsg( pPrivateStruct->hwndIndic, WM_CSOJ2I0A_SETINDIC, MPFROMLONG(30), MPFROMLONG(100) );


    /*
     *  Account for odd bits used in 1bpp or 4bpp images that are
     *  NOT on byte boundaries.
     */

    if ( dwRowBits % 8 ) dwNumRowBytes++;

    /*
     *  Ensure the row length in bytes accounts for byte padding.
     *  All bitmap data rows must are aligned on LONG/4-BYTE boundaries.
     *  The data FROM an IOProc should always appear in this form.
     */

    dwPadBytes = ( dwNumRowBytes % 4 );

    if ( dwPadBytes ) dwNumRowBytes += 4 - dwPadBytes;

    /* Allocate space for ONE row of pels */
    if ( DosAllocMem( (PPVOID)&pRowBuffer, (ULONG)dwNumRowBytes, fALLOC)) {
       mmioClose (hmmio, 0L);
       WinSetWindowText(pPrivateStruct->hwndTesto,"Immagine non disponibile");
       return(0L);
    }

    WinSendMsg( pPrivateStruct->hwndIndic, WM_CSOJ2I0A_SETINDIC, MPFROMLONG(36), MPFROMLONG(100) );

    // ***************************************************
    // Create a memory presentation space that includes
    // the memory device context obtained above.
    // ***************************************************

    ImageSize.cx = dwWidth;
    ImageSize.cy = dwHeight;

    *hps = GpiCreatePS ( hab,
                         hdc,
                         &ImageSize,
                         PU_PELS | GPIF_DEFAULT | GPIT_MICRO | GPIA_ASSOC );

    if ( !*hps ) {
        mmioClose (hmmio, 0L);
        WinSetWindowText(pPrivateStruct->hwndTesto,"Immagine non disponibile");
        return(0L);
    }

    WinSendMsg( pPrivateStruct->hwndIndic, WM_CSOJ2I0A_SETINDIC, MPFROMLONG(42), MPFROMLONG(100) );

    // ***************************************************
    // Create an uninitialized bitmap.  This is where we
    // will put all of the bits once we read them in.
    // ***************************************************
    hbm = GpiCreateBitmap ( *hps,
                            &mmImgHdr.mmXDIBHeader.BMPInfoHeader2,
                            0L,
                            NULL,
                            NULL);

    if ( !hbm ) {
        mmioClose (hmmio, 0L);
        WinSetWindowText(pPrivateStruct->hwndTesto,"Immagine non disponibile");
        return(0L);
    }
    WinSendMsg( pPrivateStruct->hwndIndic, WM_CSOJ2I0A_SETINDIC, MPFROMLONG(48), MPFROMLONG(100) );

    // ***************************************************
    // Select the bitmap into the memory device context.
    // ***************************************************
    GpiSetBitmap ( *hps, hbm );

    //***************************************************************
    //  LOAD THE BITMAP DATA FROM THE FILE
    //      One line at a time, starting from the BOTTOM
    //*************************************************************** */
    for (numColors=0;numColors<MAX_COLORSFORBITBLT;numColors++)
       tab[numColors]=NON_ASSEGNATO;

    // Conto i colori se l'informazione non Š disponibile e
    // se i colori fisici sono maggiori di quelli richiesti
    if ((mmImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrUsed!=0) ||
        (pow(2,wBitCount) < MAX_COLORSFORBITBLT) ) {
       fConta=FALSE;
       numColors=mmImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrUsed;
    } else {
       fConta=TRUE;
       numColors=0;
    }

    for ( dwRowCount = 0; dwRowCount < dwHeight; dwRowCount++ ) {
         ulBytesRead = (ULONG) mmioRead ( hmmio,
                                          pRowBuffer,
                                          dwNumRowBytes );

         if ( !ulBytesRead ) break;

         /*
          *  Allow context switching while previewing.. Couldn't get
          *  it to work. Perhaps will get to it when time is available...
          */
         if (fConta) {
            if (dwRowCount>MAX_ROWFORCOUNTCOLOR) fConta=FALSE;
            ulTmp = ContaColori( pRowBuffer, ulBytesRead, mmImgHdr.mmXDIBHeader.BMPInfoHeader2.cBitCount);
            if (ulTmp>numColors) {
               numColors=ulTmp;
               if (numColors>=MAX_COLORSFORBITBLT) fConta=FALSE;
            }
         }

         WinSendMsg( pPrivateStruct->hwndIndic, WM_CSOJ2I0A_SETINDIC, MPFROMLONG(dwRowCount), MPFROMLONG(dwHeight) );

         GpiSetBitmapBits ( *hps,
                            (LONG) dwRowCount,
                            (LONG) 1,
                            (PBYTE) pRowBuffer,
                            (PBITMAPINFO2) &mmImgHdr.mmXDIBHeader.BMPInfoHeader2);
    }

    memset(szString,0,sizeof(szString));
    sprintf(szString,"[%dx%dx%d] ",
            mmImgHdr.mmXDIBHeader.BMPInfoHeader2.cx,
            mmImgHdr.mmXDIBHeader.BMPInfoHeader2.cy,
            //numColors
            mmImgHdr.mmXDIBHeader.BMPInfoHeader2.cBitCount
            //mmImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrUsed
            //mmImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrImportant
            );

    memset(szPath,0,sizeof(szPath));
    strcpy(szPath,pszFileName);
    pChar=strrchr(szPath,'\\');
    if (pChar) strcat(szString,CaseName(pChar+1));
    WinSetWindowText(pPrivateStruct->hwndTesto,szString);

    if (numColors < MAX_COLORSFORBITBLT) {
      if (wBitCount==1) {
         pPrivateStruct->tipo_BBO=BBO_OR;
      } else {
         pPrivateStruct->tipo_BBO=BBO_AND;
      }
    } else {
      pPrivateStruct->tipo_BBO=BBO_IGNORE;
    }

    pPrivateStruct->tipo_ROP=ROP_SRCCOPY;

    /* Bitmap OS2 o WinRGB 1bpp me li d… al contrario BOH ??? */
    if ( (wBitCount==1) &&
         (strncmp(mmFormatInfo.szDefaultFormatExt,"BMP",sizeof("BMP"))==0) ) {
       pPrivateStruct->tipo_ROP=ROP_NOTSRCCOPY;
       pPrivateStruct->tipo_BBO=BBO_AND;
    }

    mmioClose (hmmio, 0L);

    DosFreeMem(pRowBuffer);
    pPrivateStruct->hbm=hbm;
    pPrivateStruct->sizeBmp=ImageSize;

    return(hbm);
}


/*****************************************************************************/
/* Restituisce un HBITMAP da uno specifico file                              */
/*****************************************************************************/
HBITMAP GetBitmap ( HWND hwndClass, PSZ  pszFileName)
{
    HBITMAP hBmp;
    PPRIVATESTRUCT pPrivateStruct;
    pPrivateStruct = WinQueryWindowPtr(hwndClass, 0);

    /*
     *  Load Bitmap, which will then be drawn during WM_PAINT processing
     *
     *  First, we need a memory device context. We'll keep this around
     *  to reuse for subsequent preview operations
     */

    if ( !pPrivateStruct->hdcMemory ) {
         pPrivateStruct->hdcMemory = DevOpenDC ( hab,
                                                 OD_MEMORY,
                                                 "*",
                                                 0L,
                                                 NULL,
                                                 0);
    }

    /*
     *  Discard previous memory presentation space if present
     */

    if ( pPrivateStruct->hpsMemory ) {
         hBmp = GpiSetBitmap ( pPrivateStruct->hpsMemory, 0);
         GpiDestroyPS ( pPrivateStruct->hpsMemory );
         pPrivateStruct->hpsMemory = 0;
    }

    /*
     * Delete previously loaded bitmap if present
     */

    if ( hBmp && hBmp != HBM_ERROR ) {
         GpiDeleteBitmap ( hBmp );
    }

    WinSendMsg( pPrivateStruct->hwndIndic, WM_CSOJ2I0A_AZZERAINDIC, 0L, 0L );

    hBmp = LoadBitmap ( hab,
                        pPrivateStruct->hdcMemory,
                        &pPrivateStruct->hpsMemory,
                        pszFileName,
                        hwndClass );
    WinSendMsg( pPrivateStruct->hwndIndic, WM_CSOJ2I0A_AZZERAINDIC, 0L, 0L );

    if (!hBmp) {
         // Error loading bitmap
         WinInvalidateRect(pPrivateStruct->hwndBmp, NULL, TRUE);
         return hBmp;
    }

    GpiQueryPS ( pPrivateStruct->hpsMemory, &pPrivateStruct->ImageSize);
    memset(pPrivateStruct->szFileName,0,MAX_FILENAME_SIZE);
    strcpy(pPrivateStruct->szFileName,pszFileName);

    /*
     *  Be sure that the image gets repainted
     */
    WinInvalidateRect(pPrivateStruct->hwndBmp, NULL, TRUE);

    return hBmp;
}

/*****************************************************************************/
/* Draw the previously loaded bitmap in the rectangle                        */
/*****************************************************************************/
VOID DrawBitMap ( HWND hwndClass )
{
    SWP    swp;
    POINTL aptl[4];
    HPS    hps;
    SIZEL  ImageSize;
    ULONG  ulReturnCode;
    PPRIVATESTRUCT pPrivateStruct;
    RECTL  rcBmp;
    double center;

    pPrivateStruct = WinQueryWindowPtr(hwndClass, 0);

    /*
     *  Get position of image frame
     */
    WinQueryWindowPos ( pPrivateStruct->hwndBmp, &swp);

    /*
     *  Validate memory presentation space before attempting to draw bitmap
     */
    ulReturnCode = GpiQueryPS ( pPrivateStruct->hpsMemory, &ImageSize) ;

    if ( !ulReturnCode || swp.cy<1 || swp.cy<1) {
       hps = WinBeginPaint ( pPrivateStruct->hwndBmp, 0, &rcBmp);
       GpiSelectPalette(hps, pPrivateStruct->hPal);
       //rcBmp.xRight++;
       WinFillRect(hps, &rcBmp, CLR_USERBACKGROUND);
       WinEndPaint (hps);
       return;
    }

    hps = WinBeginPaint ( pPrivateStruct->hwndBmp,
                          0,
                          NULL);
    GpiSelectPalette(hps, pPrivateStruct->hPal);

    switch (pPrivateStruct->tipoDraw) {
        case DRAW_ORIGINALE:
            {
              LONG max_x = 0;
              LONG max_y = 0;

              pPrivateStruct->deltax = -(pPrivateStruct->ptl2.x - pPrivateStruct->ptl.x);
              pPrivateStruct->deltay = -(pPrivateStruct->ptl2.y - pPrivateStruct->ptl.y);

              max_x = pPrivateStruct->ImageSize.cx - (100 * swp.cx / pPrivateStruct->coeff );
              max_y = pPrivateStruct->ImageSize.cy - (100 * swp.cy / pPrivateStruct->coeff );

              if (pPrivateStruct->deltax > max_x) pPrivateStruct->deltax = max_x;
              if (pPrivateStruct->deltay > max_y) pPrivateStruct->deltay = max_y;
              if (pPrivateStruct->deltax < 0)     pPrivateStruct->deltax = 0;
              if (pPrivateStruct->deltay < 0)     pPrivateStruct->deltay = 0;

                                                // target lower left
              aptl[0].x = 0;
              aptl[0].y = 0;

              aptl[1].x = swp.cx;               // target upper right
              aptl[1].y = swp.cy;
                                                // source upper right
              aptl[2].x = pPrivateStruct->deltax;
              aptl[2].y = pPrivateStruct->deltay;

                                                // source upper right
              aptl[3].x = pPrivateStruct->deltax +
                          (float)((float)swp.cx * (float)100 / pPrivateStruct->coeff);
              aptl[3].y = pPrivateStruct->deltay +
                          (float)((float)swp.cy * (float)100 / pPrivateStruct->coeff);
            }
        break;

        case DRAW_THUMBNAIL:
                                            // target lower left
            aptl[0].x = 0;
            aptl[0].y = 0;

            if ( ((float)ImageSize.cx / (float)ImageSize.cy) > ((float)swp.cx / (float)swp.cy) ) {
               aptl[1].x = swp.cx;         // target upper right
               aptl[1].y = ( ImageSize.cy * swp.cx ) / ImageSize.cx;

                                            /* Center Position */
               center = ((double)swp.cy - (double)aptl[1].y) / (double)2;
               aptl[0].y += center;
               aptl[1].y += center;

               rcBmp.xLeft   = 0;
               rcBmp.xRight  = swp.cx;
               rcBmp.yBottom = 0;
               rcBmp.yTop    = aptl[0].y;
               WinFillRect(hps, &rcBmp, CLR_USERBACKGROUND);

               rcBmp.xLeft   = 0;
               rcBmp.xRight  = swp.cx;
               rcBmp.yBottom = aptl[1].y;
               rcBmp.yTop    = swp.cy;
               WinFillRect(hps, &rcBmp, CLR_USERBACKGROUND);
            } else {
                                           // target upper right
               aptl[1].x = ( ImageSize.cx * swp.cy ) / ImageSize.cy;
               aptl[1].y = swp.cy;

                                            /* Center Position */
               center = ((double)swp.cx - (double)aptl[1].x) / (double)2;
               aptl[0].x += center;
               aptl[1].x += center;

               rcBmp.xLeft   = 0;
               rcBmp.xRight  = aptl[0].x;
               rcBmp.yBottom = 0;
               rcBmp.yTop    = swp.cy;
               WinFillRect(hps, &rcBmp, CLR_USERBACKGROUND);

               rcBmp.xLeft   = aptl[1].x;
               rcBmp.xRight  = swp.cx;
               rcBmp.yBottom = 0;
               rcBmp.yTop    = swp.cy;
               WinFillRect(hps, &rcBmp, CLR_USERBACKGROUND);
            }

                                              // source upper right
            aptl[2].x = 0;
            aptl[2].y = 0;
                                              // source upper right
            aptl[3].x = ImageSize.cx;
            aptl[3].y = ImageSize.cy;

            Cornice(hps, aptl[0].x, aptl[0].y, --aptl[1].x, --aptl[1].y);
            aptl[0].x+=2;
            aptl[0].y+=2;
            aptl[1].x-=1;
            aptl[1].y-=1;
        break;

        case DRAW_STRETCH:
                                              // target lower left
            aptl[0].x = 0;
            aptl[0].y = 0;

            aptl[1].x = swp.cx;               // target upper right
            aptl[1].y = swp.cy;
                                              // source upper right
            aptl[2].x = 0;
            aptl[2].y = 0;
                                              // source upper right
            aptl[3].x = ImageSize.cx;
            aptl[3].y = ImageSize.cy;
        break;
    }

    /*
     *  Call GpiBitBlt and supply 4 aptl structures.  This tells
     *  it to stretch or compress the bitmap depending on what is
     *  in the aptl structures.  See above lines for their current
     *  settings.
     */

    GpiSetColor (hps, GpiQueryColor (pPrivateStruct->hpsMemory));
    GpiSetBackColor (hps, GpiQueryBackColor (pPrivateStruct->hpsMemory));
    //GpiBitBlt (hps, pPrivateStruct->hpsMemory, 4L, aptl, ROP_SRCCOPY, BBO_OR);

    GpiBitBlt (hps, pPrivateStruct->hpsMemory, 4L, aptl, pPrivateStruct->tipo_ROP, pPrivateStruct->tipo_BBO);

    pPrivateStruct->fTrackeable=TRUE;

    if (pPrivateStruct->tipoDraw==DRAW_ORIGINALE) {
       if (aptl[3].x > ImageSize.cx) {
          rcBmp.xLeft   = swp.cx - ((aptl[3].x - ImageSize.cx) * pPrivateStruct->coeff / 100);
          rcBmp.xRight  = swp.cx;
          rcBmp.yBottom = 0;
          rcBmp.yTop    = swp.cy;
          WinFillRect(hps, &rcBmp, CLR_USERBACKGROUND);
       }
       if (aptl[3].y > ImageSize.cy) {
          rcBmp.xLeft   = 0;
          rcBmp.xRight  = swp.cx;
          rcBmp.yBottom = swp.cy - ((aptl[3].y - ImageSize.cy) * pPrivateStruct->coeff / 100);
          rcBmp.yTop    = swp.cy;
          WinFillRect(hps, &rcBmp, CLR_USERBACKGROUND);
       }
       if (aptl[3].x > ImageSize.cx  &&
           aptl[3].y > ImageSize.cy) {
          pPrivateStruct->fTrackeable=FALSE;
       }
    }
    WinEndPaint (hps);
    return;
}

/**************************************************************************\
* Disegno effettivo del bottone                                            *
\**************************************************************************/
VOID DrawButton ( SHORT idItem , PUSERBUTTON pBtn )
{
   RECTL  WinDim;
   USHORT fsDBM=DBM_STRETCH;

   WinQueryWindowRect( pBtn->hwnd , &WinDim );
   WinFillRect( pBtn->hps , &WinDim, CLR_PALEGRAY );

   if (pBtn->fsState & BDS_DISABLED)
      fsDBM |= DBM_HALFTONE;

   if (pBtn->fsState & BDS_HILITED) {
      switch ( idItem ) {
        case ID_CSOJ2I0A_BTN_ORIGINALE : WinDrawBitmap( pBtn->hps , hBmp1A, (PRECTL)NULL , (PPOINTL)&WinDim , 0L, 0L, fsDBM ); break;
        case ID_CSOJ2I0A_BTN_THUMBNAIL : WinDrawBitmap( pBtn->hps , hBmp2A, (PRECTL)NULL , (PPOINTL)&WinDim , 0L, 0L, fsDBM ); break;
        case ID_CSOJ2I0A_BTN_STRETCH   : WinDrawBitmap( pBtn->hps , hBmp3A, (PRECTL)NULL , (PPOINTL)&WinDim , 0L, 0L, fsDBM ); break;
        case ID_CSOJ2I0A_BTN_FOLDER    : WinDrawBitmap( pBtn->hps , hBmpFolderA, (PRECTL)NULL , (PPOINTL)&WinDim , 0L, 0L, fsDBM ); break;
      }
   } else {
      switch ( idItem ) {
        case ID_CSOJ2I0A_BTN_ORIGINALE : WinDrawBitmap( pBtn->hps , hBmp1, (PRECTL)NULL , (PPOINTL)&WinDim , 0L, 0L, fsDBM ); break;
        case ID_CSOJ2I0A_BTN_THUMBNAIL : WinDrawBitmap( pBtn->hps , hBmp2, (PRECTL)NULL , (PPOINTL)&WinDim , 0L, 0L, fsDBM ); break;
        case ID_CSOJ2I0A_BTN_STRETCH   : WinDrawBitmap( pBtn->hps , hBmp3, (PRECTL)NULL , (PPOINTL)&WinDim , 0L, 0L, fsDBM ); break;
        case ID_CSOJ2I0A_BTN_FOLDER    : WinDrawBitmap( pBtn->hps , hBmpFolder, (PRECTL)NULL , (PPOINTL)&WinDim , 0L, 0L, fsDBM ); break;
      }
   }
   return;
}

/*****************************************************************************/
/*                                                                           */
/*****************************************************************************/
PCHAR CaseName( PCHAR pszString )
{
  PCHAR pChar;
  pChar=pszString;
  if (*pChar) {
     *pChar=WinUpperChar(hab,0,0,*pChar);
     for (++pChar;*pChar;pChar++) {
        if (*pChar>='A' && *pChar<='Z') *pChar=*pChar+32;
     }
  }
  return( pszString );
}

/**********************************************************************/
/*                                                                    */
/*  Name:   OpenFileDialog                                            */
/*                                                                    */
/*  Purpose: open the standard file open dialog as file extention     */
/*           and return the filename                                  */
/*                                                                    */
/*  Usage:   called when the user needs to supply a name for          */
/*           the file to be opened                                    */
/*                                                                    */
/*  Method:  calls the standard file open dialog to get the           */
/*           file name.                                               */
/*                                                                    */
/*  Parameters: HWD hwnd         Handle of the owner window.          */
/*              PSZ szTitle      Title of open dialog.                */
/*              PSZ pszFileExt   File extention. (for example : *.txt)*/
/*              PSZ pszFullPath  PSZ for returning the file name.     */
/*                                                                    */
/*  Returns: TRUE if successful in getting a file name, FALSE         */
/*              if not in pushing CANCEL                              */
/*           PSZ pszFullPath pointer to filename (full path)          */
/*                                                                    */
/**********************************************************************/

BOOL OpenFileDialog(HWND hwndOwner,
                    PSZ szTitle,
                    PSZ szFileExt,
                    PSZ szButton,
                    PSZ szFullPath )
{
    FILEDLG fdg;

    memset(&fdg, 0, sizeof(FILEDLG));
    fdg.cbSize = sizeof(FILEDLG);          /* Size of FILEDLG.                */

    fdg.pszTitle = szTitle;                /* String to display in title bar. */

    fdg.pszOKButton = szButton ;

    fdg.ulUser = 0L;                       /* User defined field.             */
    fdg.fl = FDS_CENTER | FDS_INCLUDE_EAS | FDS_FILTERUNION |
             FDS_OPEN_DIALOG;

    fdg.pfnDlgProc = NULL;
    fdg.lReturn = 0L;                      /* Result code from dialog dismissal. */
    fdg.lSRC = 0L;                         /* System return code.          */
    fdg.hMod = 0;                          /* Custom file dialog template. */
    fdg.usDlgId = 0;                       /* Custom file dialog ID.       */
    fdg.x = 100;                           /* X coordinate of the dialog.  */
    fdg.y = 100;                           /* Y coordinate of the dialog.  */

    /* set selected fully qualified path */
    strcpy( fdg.szFullFile, szFileExt);

    if ( !WinFileDlg ( HWND_DESKTOP,
                       hwndOwner,
                       (PFILEDLG)&fdg ) )
       return FALSE;

    /* copy file name into file name buffer */
    strcpy ( szFullPath, fdg.szFullFile );
    if (fdg.lReturn == DID_CANCEL) return( FALSE );
                              else return( TRUE );


}

/**********************************************************************/
/*                                                                    */
/**********************************************************************/
VOID CalcolaPosizioni( HWND hwndClass, PPOSSTRUCT ps )
{
   #define ALTEZZA_TESTO       18
   #define BTN1_CX             18
   #define SPIN_CX             50
   #define ALTEZZA_INDICATORE  1
   #define SPAZIO              2
   #define SPAZIOx3            6

   ULONG          kx;
   SWP            swpClass;
   PPRIVATESTRUCT pPrivateStruct;

   memset(ps,0,sizeof(POSSTRUCT));

   pPrivateStruct = WinQueryWindowPtr(hwndClass, 0);
   WinQueryWindowPos(hwndClass,&swpClass);

   ps->xClass    = swpClass.x;
   ps->yClass    = swpClass.y;
   ps->cxClass   = swpClass.cx;
   ps->cyClass   = swpClass.cy;

   /* Se non voglio vedere i bottoni */
   if (!pPrivateStruct->bottoni) {
     ps->xIndic  = 0;
     ps->yIndic  = 0;
     ps->cxIndic = swpClass.cx;
     ps->cyIndic = ALTEZZA_INDICATORE;

     ps->xBmp  = 0;
     ps->yBmp  = SPAZIO;
     ps->cxBmp = swpClass.cx;
     ps->cyBmp = swpClass.cy - SPAZIO;
   } else {
     ps->xIndic  = 0;
     ps->yIndic  = ALTEZZA_TESTO;
     ps->cxIndic = swpClass.cx;
     ps->cyIndic = ALTEZZA_INDICATORE;

     ps->xBmp      = 0;
     ps->yBmp      = ALTEZZA_TESTO + ALTEZZA_INDICATORE;
     ps->cxBmp     = swpClass.cx;
     ps->cyBmp     = swpClass.cy - ALTEZZA_TESTO - ALTEZZA_INDICATORE;
   }

   // Dimensioni originali
   kx = 0;

   ps->xBtn1     = kx;
   ps->yBtn1     = 0;
   ps->cxBtn1    = BTN1_CX;
   ps->cyBtn1    = ALTEZZA_TESTO;
   kx = kx + BTN1_CX + SPAZIO;

   ps->xSpin     = kx;
   ps->ySpin     = 0;
   ps->cxSpin    = SPIN_CX;
   ps->cySpin    = ALTEZZA_TESTO;
   kx = kx + SPIN_CX + SPAZIO;

   ps->xBtn2     = kx;
   ps->yBtn2     = 0;
   ps->cxBtn2    = BTN1_CX;
   ps->cyBtn2    = ALTEZZA_TESTO;
   kx = kx + BTN1_CX + SPAZIO;

   ps->xBtn3     = kx;
   ps->yBtn3     = 0;
   ps->cxBtn3    = BTN1_CX;
   ps->cyBtn3    = ALTEZZA_TESTO;
   kx = kx + BTN1_CX + SPAZIO;

   if (pPrivateStruct->modificabile) {
      ps->xFolder   = kx;
      ps->yFolder   = 0;
      ps->cxFolder  = BTN1_CX;
      ps->cyFolder  = ALTEZZA_TESTO;
      kx = kx + BTN1_CX + SPAZIO;
   } else {
      ps->xFolder   = 0;
      ps->yFolder   = 0;
      ps->cxFolder  = 0;
      ps->cyFolder  = 0;
   }

   ps->xText     = kx;
   ps->yText     = 0;
   ps->cxText    = swpClass.cx - kx - SPAZIOx3;
   ps->cyText    = ALTEZZA_TESTO;

// if (ps->cxClass<kx) ps->cxClass=kx;
// if (ps->cyClass<ALTEZZA_TESTO) ps->cyClass=ALTEZZA_TESTO;

   return;
}

/**********************************************************************/
/* Routine di conteggio colori effettivamente usati                   */
/**********************************************************************/
ULONG ContaColori( PVOID pBuff, ULONG ulTotBytes, ULONG nBits)
{
  /* 1 4 8 24 bpp    Formati Attualmente supportati */

  ULONG  k;
  ULONG  j;
  ULONG  tmp;
  double i;

  BIT1  *pChar1;
  BIT4  *pChar4;
  BIT8  *pChar8;
  BIT24 *pChar24;

  pChar1=(BIT1*)pBuff;
  pChar4=(BIT4*)pBuff;
  pChar8=(BIT8*)pBuff;
  pChar24=(BIT24*)pBuff;

  k=0;
  for (i=0;i<ulTotBytes;) {
     switch (nBits) {
     case 1:
        for (k=0;k<8;k++) {
           switch (k) {
              case 0: tmp=(ULONG)pChar1->c0; break;
              case 1: tmp=(ULONG)pChar1->c1; break;
              case 2: tmp=(ULONG)pChar1->c2; break;
              case 3: tmp=(ULONG)pChar1->c3; break;
              case 4: tmp=(ULONG)pChar1->c4; break;
              case 5: tmp=(ULONG)pChar1->c5; break;
              case 6: tmp=(ULONG)pChar1->c6; break;
              case 7: tmp=(ULONG)pChar1->c7; break;
           }
           for (j=0;j<MAX_COLORSFORBITBLT;j++) {
              if (j>=MAX_COLORSFORBITBLT) return( MAX_COLORSFORBITBLT );
              if (tab[j]==NON_ASSEGNATO || tab[j]==tmp) {
                 tab[j]=tmp;
                 break;
              }
           }
        }
        pChar1++;
        i++;
        break;

     case 4:
        for (k=0;k<2;k++) {
           switch (k) {
              case 0: tmp=(ULONG)pChar4->c0; break;
              case 1: tmp=(ULONG)pChar4->c1; break;
           }
           for (j=0;j<MAX_COLORSFORBITBLT;j++) {
              if (j>=MAX_COLORSFORBITBLT) return( MAX_COLORSFORBITBLT );
              if (tab[j]==NON_ASSEGNATO || tab[j]==tmp) {
                 tab[j]=tmp;
                 break;
              }
           }
        }
        pChar4++;
        i++;
        break;

     case 8:
        tmp=(ULONG)pChar8->c;
        for (j=0;j<MAX_COLORSFORBITBLT;j++) {
           if (j>=MAX_COLORSFORBITBLT) return( MAX_COLORSFORBITBLT );
           if (tab[j]==NON_ASSEGNATO || tab[j]==tmp) {
              tab[j]=tmp;
              break;
           }
        }
        pChar8++;
        i++;
        break;

     case 24:
        tmp=(ULONG)pChar24->un.cl;
        for (j=0;j<MAX_COLORSFORBITBLT;j++) {
           if (j>=MAX_COLORSFORBITBLT) return( MAX_COLORSFORBITBLT );
           if (tab[j]==NON_ASSEGNATO || tab[j]==tmp) {
              tab[j]=tmp;
              break;
           }
        }
        pChar24++;
        i+=3;
        break;
     }
  }

  for (j=0;j<MAX_COLORSFORBITBLT;j++)
     if (tab[j]==NON_ASSEGNATO) break;

  return j;
}

MRESULT EXPENTRY Indicatore_Proc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 )
{
   PPRIVATESTRUCT pPrivateStruct;

   switch( msg )
   {
      case WM_CSOJ2I0A_SETINDIC:
           {
              ULONG dividendo,divisore;
              float i;
              pPrivateStruct = WinQueryWindowPtr(hwnd, 0);
              dividendo=LONGFROMMP(mp1);
              divisore=LONGFROMMP(mp2);
              if (divisore==0) divisore=100;
              i=(float)dividendo/divisore;
              i=i*(float)100;
              if (i>100.0) return ( MRESULT )0;
              if (i<pPrivateStruct->percParz) return ( MRESULT )0;
              if ((i<94) && (i-pPrivateStruct->percParz) < 3) break;
              pPrivateStruct->percParz=(ULONG)i;
              WinInvalidateRect(hwnd,NULL,TRUE);
           }
         break;

      case WM_CSOJ2I0A_AZZERAINDIC:
              pPrivateStruct = WinQueryWindowPtr(hwnd, 0);
              pPrivateStruct->percParz=0;
              WinInvalidateRect(hwnd,NULL,TRUE);
         break;

      case WM_PAINT:
         {
           HPS hps;
           RECTL WinDim;
           ULONG pxlPerc;

           pPrivateStruct = WinQueryWindowPtr(hwnd, 0);
           hps = WinBeginPaint( hwnd, (HPS)NULL, &WinDim );
           //GpiSelectPalette(hps, pPrivateStruct->hPal);
           //WinQueryWindowRect( hwnd , &WinDim );
           //WinFillRect(hps, &WinDim, CLR_USERBACKGROUND);
           WinFillRect(hps, &WinDim, CLR_DARKGRAY);
           if (pPrivateStruct->percParz>0) {
              pxlPerc = ((WinDim.xRight-WinDim.xLeft)*pPrivateStruct->percParz)/100;
              WinDim.xRight=pxlPerc;
              WinFillRect(hps, &WinDim, CLR_RED);
           }
           WinEndPaint( hps );
         }
      break;
   }
   return(WinDefWindowProc(hwnd, msg, mp1, mp2));
}

MRESULT EXPENTRY Bmp_Proc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 )
{
   PPRIVATESTRUCT pPrivateStruct;

   switch( msg )
   {
      case WM_PAINT:
         DrawBitMap ( hwnd );
      break;

     case WM_MOUSEMOVE:
          pPrivateStruct = WinQueryWindowPtr(hwnd, 0);
          if (pPrivateStruct->tipoDraw==DRAW_ORIGINALE) {
              WinSetPointer(HWND_DESKTOP,hPtrA);
              return(MRESULT)0;
          }
        break;

      case WM_PRESPARAMCHANGED:
           pPrivateStruct = WinQueryWindowPtr(hwnd, 0);
           ChangePresParam( pPrivateStruct, hwnd, LONGFROMMP(mp1) );
      break;
   }
   return(WinDefWindowProc(hwnd, msg, mp1, mp2));
}

VOID SetPosWindows( HWND hwndClass, PPRIVATESTRUCT pPrivateStruct, PPOSSTRUCT ps )
{
   WinSetWindowPos( hwndClass,
                   NULLHANDLE,
                   ps->xClass, ps->yClass, ps->cxClass, ps->cyClass,
                   SWP_SIZE | SWP_MOVE);
   WinSetWindowPos( pPrivateStruct->hwndBmp,
                   NULLHANDLE,
                   ps->xBmp, ps->yBmp, ps->cxBmp, ps->cyBmp,
                   SWP_SIZE | SWP_MOVE);
   WinSetWindowPos( pPrivateStruct->hwndBtnOriginale,
                   NULLHANDLE,
                   ps->xBtn1, ps->yBtn1, ps->cxBtn1, ps->cyBtn1,
                   SWP_SIZE | SWP_MOVE);
   WinSetWindowPos( pPrivateStruct->hwndBtnThumbnail,
                   NULLHANDLE,
                   ps->xBtn2, ps->yBtn2, ps->cxBtn2, ps->cyBtn2,
                   SWP_SIZE | SWP_MOVE);
   WinSetWindowPos( pPrivateStruct->hwndBtnStretch,
                   NULLHANDLE,
                   ps->xBtn3, ps->yBtn3, ps->cxBtn3, ps->cyBtn3,
                   SWP_SIZE | SWP_MOVE);
   WinSetWindowPos( pPrivateStruct->hwndSpin,
                   NULLHANDLE,
                   ps->xSpin, ps->ySpin, ps->cxSpin, ps->cySpin,
                   SWP_SIZE | SWP_MOVE);
   WinSetWindowPos( pPrivateStruct->hwndTesto,
                   NULLHANDLE,
                   ps->xText, ps->yText, ps->cxText, ps->cyText,
                   SWP_SIZE | SWP_MOVE);
   WinSetWindowPos( pPrivateStruct->hwndBtnFolder,
                   NULLHANDLE,
                   ps->xFolder, ps->yFolder, ps->cxFolder, ps->cyFolder,
                   SWP_SIZE | SWP_MOVE);
   WinSetWindowPos( pPrivateStruct->hwndIndic,
                   NULLHANDLE,
                   ps->xIndic, ps->yIndic, ps->cxIndic, ps->cyIndic,
                   SWP_SIZE | SWP_MOVE);
}

VOID DrawBorder ( HPS hps, RECTL* prect, PSZ string )
{
   POINTL ptl;
   ptl.x=prect->xLeft;
   ptl.y=prect->yBottom;
   GpiMove(hps, &ptl);
   ptl.y = ptl.y + prect->yTop;
   GpiLine(hps, &ptl);
   ptl.x = ptl.x + prect->xRight;
   GpiLine(hps, &ptl);
   ptl.y=prect->yBottom;
   GpiLine(hps, &ptl);
   ptl.x=prect->xLeft;
   GpiLine(hps, &ptl);

   if (strlen(string)>0) {
      ptl.x=(prect->xRight-prect->xLeft)/25;
      ptl.y=prect->yBottom+(prect->yTop/2)-3;
      GpiCharStringAt(hps, &ptl, (LONG)strlen(string), string);
   }
   return;
}

VOID Cornice( HPS hps , ULONG xbl, ULONG ybl, ULONG xtr, ULONG ytr )
{
   POINTL  ptl;

   GpiSetColor(hps,CLR_DARKGRAY);
   ptl.x = xbl;
   ptl.y = ybl;
   GpiMove(hps, &ptl);
   ptl.y = ytr;
   GpiLine(hps, &ptl);
   ptl.x = xtr;
   GpiLine(hps, &ptl);

   GpiSetColor(hps,CLR_WHITE);
   ptl.y = ybl;
   GpiLine(hps, &ptl);
   ptl.x = xbl;
   GpiLine(hps, &ptl);

   xbl++; ybl++; xtr--; ytr--;

   ptl.x = xbl;
   ptl.y = ybl;
   GpiMove(hps, &ptl);
   ptl.y = ytr;
   GpiLine(hps, &ptl);
   ptl.x = xtr;
   GpiLine(hps, &ptl);

   GpiSetColor(hps,CLR_DARKGRAY);
   ptl.y = ybl;
   GpiLine(hps, &ptl);
   ptl.x = xbl;
   GpiLine(hps, &ptl);

   xbl++; ybl++; xtr--; ytr--;

   GpiSetColor(hps,CLR_USERBACKGROUND);
   ptl.x = xbl;
   ptl.y = ybl;
   GpiMove(hps, &ptl);
   ptl.y = ytr;
   GpiLine(hps, &ptl);
   ptl.x = xtr;
   GpiLine(hps, &ptl);
   ptl.y = ybl;
   GpiLine(hps, &ptl);
   ptl.x = xbl;
   GpiLine(hps, &ptl);

   GpiSetColor(hps,CLR_DEFAULT);
   return;
}

VOID SetCoeff ( PPRIVATESTRUCT pPrivateStruct, ULONG coeff )
{
  if (coeff<MIN_RIDUZIONE) coeff=MIN_RIDUZIONE;
  if (coeff>MAX_INGRANDIMENTO) coeff=MAX_INGRANDIMENTO;
  pPrivateStruct->coeff=coeff;
  WinInvalidateRect(pPrivateStruct->hwndBmp, NULL, TRUE);
  return;
}

int compareExt(const void *arg1, const void *arg2)
{
   return ( strcmp( (char *)arg1, (char *)arg2 ));
}

/*****************************************************************************/
/* Cerca nel vettore pExtTab (di dimensione elExtTab) il formato richiesto   */
/* Convertito in maiuscolo                                                   */
/*****************************************************************************/
BOOL CercaFormato( PSZ formato )
{
   char   *result;
   EXTEL  UpFormato;
   if (!formato || strlen(formato)==0 || strlen(formato)>=sizeof(EXTEL)) return( FALSE );
   strcpy((char*)&UpFormato,formato);
   result = (char*)bsearch( strupr((char*)&UpFormato), pExtTab, elExtTab, sizeof(EXTEL), compareExt);
   if (result != NULL) return( TRUE );
                  else return( FALSE );
}

/*****************************************************************************/
/* Carica il vettore pExtTab (di dimensione elExtTab) con i formati          */
/* supportati. Viene convertito in maiuscolo tolti i doppi e ordinato        */
/*****************************************************************************/
BOOL CaricaFormati( )
{
   CHAR          szBuffer[ sizeof( FOURCC ) + MAX_FILENAME_SIZE + 4 ];
   LONG          lNumIOProcs;
   MMFORMATINFO  mmFormatInfo;
   PMMFORMATINFO pmmFormatInfoArray;
   PMMFORMATINFO pmmFormatInfoArrayTmp;
   PSZ           pszFourccString;
   ULONG         ulReturnCode;
   LONG          lFormatsRead;
   LONG          lBytesRead;
   EXTEL*        pCurrExtTab=NULL;
   ULONG         i;

   /*
    * Call mmioQueryFormatCount, which will return the number
    * of formats which are supported.
    */
   memset( &mmFormatInfo, '\0', sizeof(MMFORMATINFO) );

   mmFormatInfo.ulMediaType |= MMIO_MEDIATYPE_IMAGE;

   ulReturnCode = mmioQueryFormatCount ( &mmFormatInfo, &lNumIOProcs, 0, 0 );
   if( ulReturnCode != MMIO_SUCCESS ) return( FALSE);

   pmmFormatInfoArrayTmp = pmmFormatInfoArray = (PVOID)malloc (lNumIOProcs * sizeof( MMFORMATINFO ) );
   if( pmmFormatInfoArray == NULL ) return( FALSE );

   ulReturnCode = mmioGetFormats( &mmFormatInfo,
                                  lNumIOProcs,
                                  pmmFormatInfoArray,
                                  &lFormatsRead,
                                  0,
                                  0 );

   if( ulReturnCode != MMIO_SUCCESS ) return( FALSE );

   if( lFormatsRead != lNumIOProcs ) return( FALSE );

   pszFourccString = (PVOID)malloc ( 4 );

   DosAllocMem((PVOID)&pExtTab, sizeof(EXTEL)*lNumIOProcs, PAG_COMMIT | PAG_READ | PAG_WRITE);
   memset(pExtTab,0,sizeof(EXTEL)*lNumIOProcs);
   elExtTab=lNumIOProcs;
   pCurrExtTab=pExtTab;

   for ( i=0; i<lNumIOProcs; i++ )
   {
        mmioGetFormatName(pmmFormatInfoArrayTmp, szBuffer, &lBytesRead, 0L, 0L);
        FourccToString ( pmmFormatInfoArrayTmp->fccIOProc,
                         pszFourccString );

        // Insert NULL string terminator
        *( szBuffer + lBytesRead ) = (CHAR)NULL;

        strcpy((char*)pCurrExtTab,strupr(pmmFormatInfoArrayTmp->szDefaultFormatExt));

        pCurrExtTab++;

        //  advance to next entry in mmFormatInfo array
        pmmFormatInfoArrayTmp++;
   }

   free( pmmFormatInfoArray  );
   free( pszFourccString );
   qsort((char *)pExtTab, elExtTab, sizeof(EXTEL), compareExt);

   /* Se piu di un elemento tolgo i doppi */
   if (lNumIOProcs>1) {
      pCurrExtTab=pExtTab;
      pCurrExtTab++;
      for (i=0;i<lNumIOProcs;i++) {
         if (compareExt(pCurrExtTab,pCurrExtTab-1)==0) {
            memset(pCurrExtTab-1,0Xff,sizeof(EXTEL));
            elExtTab--;
         }
         pCurrExtTab++;
      }
      qsort((char *)pExtTab, lNumIOProcs, sizeof(EXTEL), compareExt);
   }

   return( TRUE );
}

VOID FourccToString( FOURCC fcc, PSZ pszString )
{
    pszString[0] = (BYTE) fcc;
    pszString[1] = (BYTE) (fcc >> 8);
    pszString[2] = (BYTE) (fcc >> 16);
    pszString[3] = (BYTE) (fcc >> 24);
    pszString[4] = 0;
}

VOID ChangePresParam( PPRIVATESTRUCT pPrivateStruct, HWND hwnd, ULONG pptype )
{
   ULONG ulPresParams,ulColor=0;
   CHAR  buffer[40];
   HPS   hps;
   CSOJ2I0A_PRESPARAM pPresParam;
   static BOOL fRecurse=FALSE;
   if (fRecurse) return;
   fRecurse=TRUE;

   /* Espansione di qualsiasi PresParam sulla classe */
   memset(buffer,0,sizeof(buffer));
   if (WinQueryPresParam(hwnd, pptype,
                         0, &ulPresParams,
                         (ULONG)sizeof(buffer), (PVOID)buffer,
                         QPF_NOINHERIT)==0) {
      WinQueryPresParam(pPrivateStruct->hwndClass, pptype,
                        0, &ulPresParams,
                        (ULONG)sizeof(buffer), (PVOID)buffer,
                        QPF_NOINHERIT);
   }
   if (hwnd==pPrivateStruct->hwndClass) {
      WinSetPresParam(pPrivateStruct->hwndBmp, pptype,
                      (ULONG)sizeof(buffer),
                      (PVOID)buffer);
   } else {
      WinSetPresParam(pPrivateStruct->hwndClass, pptype,
                      (ULONG)sizeof(buffer),
                      (PVOID)buffer);
   }

   if (pptype==PP_BACKGROUNDCOLOR ||
       pptype==PP_BACKGROUNDCOLORINDEX ) {
      WinQueryPresParam(hwnd,
                        PP_BACKGROUNDCOLOR,
                        0, &ulPresParams,
                        (ULONG)sizeof(LONG), (PVOID)&ulColor,
                        QPF_PURERGBCOLOR | QPF_NOINHERIT);
      if (ulColor!=0) {
         hps=WinGetPS(pPrivateStruct->hwndBmp);
         pPrivateStruct->alTable[CLR_USERBACKGROUND] = PC_RESERVED * 16777216 + ulColor;
         GpiSetPaletteEntries(pPrivateStruct->hPal, LCOLF_CONSECRGB, 0, TOT_PAL, pPrivateStruct->alTable );
         WinReleasePS(hps);
      }
   }
   WinInvalidateRect(pPrivateStruct->hwndBmp, NULL, TRUE);
   WinInvalidateRect(pPrivateStruct->hwndIndic, NULL, TRUE);

   memset(&pPresParam,0,sizeof(pPresParam));
   pPresParam.type  =pptype;
   pPresParam.cbBuff=sizeof(buffer);
   pPresParam.pBuff =buffer;
   WinSendMsg(pPrivateStruct->hwndParent, WM_CONTROL,
              MPFROM2SHORT(pPrivateStruct->idClass,WN_CSOJ2I0A_PRESPARAM),
              MPFROMP(&pPresParam));

   fRecurse=FALSE;
}
