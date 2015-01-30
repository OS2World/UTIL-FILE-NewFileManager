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
#define INCL_DOSFILEMGR
#define INCL_DOSERRORS

#include <os2.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "cspcsta4.h"
#include "caoj2i03.h"
#include "caoj2i04.h"
#include "cspcsta3.h"

/* Window Proc ***************************************************************/
MRESULT EXPENTRY FrameProc     ( HWND, ULONG, MPARAM, MPARAM );
MRESULT EXPENTRY TitoliProc    ( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);

VOID    _System  Scandisci     ( ULONG );
ULONG            InsertPerc    ( HWND, PSZ, double, double );
VOID             ScanDirRecurse( PSZ, double *, double *, double *, double *, ULONG *, ULONG );
PCHAR            CaseName      ( PCHAR );
VOID             DrawButton    ( PUSERBUTTON );

/* Static ********************************************************************/
#define WM_SCANDISCI     WM_USER + 1

static HAB         hab;
static HWND        hwndParent;
static HWND        hwndDlg;
static HWND        hwndGrafico;
static HWND        hwndTitoli;
static HWND        hwndDetails;
static HWND        hwndPerc;
static CHAR        glpath[CCHMAXPATH];
static HMODULE     hmod;
static SWP         swpDlg;
static TID         tidScan;
static BOOL        fStop;
static COUNTRYINFO CtryInfo   = {0};
static ULONG       bordo=0;
static ULONG       spazioSopra=0;
static ULONG       cyTitle=0;
static ULONG       cyDetails=0;
static ULONG       xDetails=0;
static ULONG       cyPerc=0;

LONG idFiles;
LONG idAltreDir;
LONG idSpazioS;
LONG idSpazioL;
LONG idFileSystem;

BOOL CSPCSTA4( HAB loc_hab, HWND loc_hwndParent, PSZ loc_path, PSZ loc_applsave, PSZ loc_keysave )
{
   COUNTRYCODE Country    = {0};
   ULONG       ulInfoLen  = 0;
   SWP         swpTmp;

   hab=loc_hab;
   hwndParent=loc_hwndParent;
   strcpy(glpath,loc_path);

   CSPCSTA3_REGISTRA(hab);
   CAOJ2I03_REGISTRA(hab);
   CAOJ2I04_REGISTRA(hab);

   DosQueryModuleHandle("CSPCSTA4", &hmod);
   DosQueryCtryInfo(sizeof(CtryInfo), &Country, &CtryInfo, &ulInfoLen);

   bordo=WinQuerySysValue( HWND_DESKTOP, SV_CXDLGFRAME )*2;
   spazioSopra=bordo*2+WinQuerySysValue( HWND_DESKTOP, SV_CYTITLEBAR );

   hwndDlg = WinLoadDlg( HWND_DESKTOP,
                         hwndParent,
                         (PFNWP)FrameProc,
                         (HMODULE)hmod,
                         ID_CSPCSTA4_FRAME,
                         NULL);

   if (loc_applsave!=NULL && loc_keysave!=NULL)
     WinRestoreWindowPos(loc_applsave,loc_keysave,hwndDlg);

   WinQueryWindowPos(hwndDlg,&swpDlg);

   hwndTitoli = WinLoadDlg( hwndDlg, hwndDlg,
                           (PFNWP)TitoliProc, (HMODULE)hmod,
                           ID_CSPCSTA4_TITOLI, NULL);
   WinQueryWindowPos(hwndTitoli,&swpTmp);
   cyTitle=swpTmp.cy;
   WinSetWindowPos(hwndTitoli, NULLHANDLE,
              bordo, swpDlg.cy-cyTitle-spazioSopra+bordo,
              swpDlg.cx-bordo*2, cyTitle,
              SWP_MOVE | SWP_SIZE | SWP_SHOW );

   hwndDetails=WinWindowFromID(hwndTitoli,ID_CSPCSTA4_DETAILS);
   WinQueryWindowPos(hwndDetails,&swpTmp);
   cyDetails=swpTmp.cy;
   xDetails=swpTmp.x+16;
   WinSetWindowPos(hwndDetails, NULLHANDLE,
              0L, 0L, swpDlg.cx-xDetails, cyDetails,
              SWP_SIZE );

   hwndPerc=WinWindowFromID(hwndTitoli,ID_CSPCSTA4_PERC);
   WinQueryWindowPos(hwndPerc,&swpTmp);
   cyPerc=swpTmp.cy;
   WinSetWindowPos(hwndPerc, NULLHANDLE,
              0L, 0L, swpDlg.cx-bordo*2, cyPerc,
              SWP_SIZE );

   hwndGrafico = WinCreateWindow( hwndDlg, "CSPCSTA3", "",
                                  0L,
                                  bordo,
                                  bordo,
                                  swpDlg.cx-bordo*2,
                                  swpDlg.cy-spazioSopra-cyTitle,
                                  hwndDlg,
                                  HWND_TOP,
                                  ID_CSPCSTA4_GRAFICO, NULL, NULL);
   WinSetPresParam (hwndGrafico, PP_FONTNAMESIZE,(ULONG)(strlen("2.System VIO")+(ULONG)1), (PSZ)"2.System VIO");
   WinShowWindow(hwndPerc,FALSE);

   WinShowWindow(hwndDlg,TRUE);
   WinProcessDlg(hwndDlg);

   fStop=TRUE;

   if (loc_applsave!=NULL && loc_keysave!=NULL)
     WinStoreWindowPos(loc_applsave,loc_keysave,hwndDlg);

   WinDestroyWindow(hwndGrafico);
   WinDestroyWindow(hwndDlg);

   return( TRUE );
}

MRESULT EXPENTRY FrameProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 )
{
  switch ( msg )
  {
    case WM_INITDLG:
      hwndDlg=hwnd;
      WinSendMsg(hwnd,WM_SCANDISCI,0L,0L);
    break;

    case WM_CONTROL:
       switch ( SHORT2FROMMP( mp1 ) ) {
          case WN_CSPCSTA3_SELECT:
             {
                CHAR buff[CCHMAXPATH];
                SHORT id;
                CHAR  *chr;
                id=LONGFROMMP(mp2);

                if (id==idFiles      || id==idAltreDir   ||
                    id==idSpazioS    || id==idSpazioL    ||
                    id==idFileSystem ) break;

                memset(buff,0,sizeof(buff));
                WinSendMsg(hwndGrafico,WM_CSPCSTA3_QUERYLABEL,MPFROM2SHORT(id,sizeof(buff)),MPFROMP(buff));

                chr=strchr(buff,' ');
                if (chr) *chr=0;

                chr=strrchr(glpath,'\\');
                if (chr) {
                   *chr=0;
                   strcat(glpath,"\\");
                   strcat(glpath,buff);
                   WinSendMsg(hwndGrafico,WM_CSPCSTA3_AZZERA,0L,0L);
                   //WinSendMsg(hwnd,WM_SCANDISCI,0L,0L);
                }
             }
          break;
       }
    break;

    case WM_WINDOWPOSCHANGED:
      {
         PSWP pswpFrame;
         pswpFrame=MPFROMP(mp1);
         if ( pswpFrame->fl & SWP_SIZE ) {
             WinSetWindowPos(hwndGrafico,NULLHANDLE,
                             bordo, bordo,
                             pswpFrame->cx-bordo*2,
                             pswpFrame->cy-spazioSopra-cyTitle,
                             SWP_SIZE | SWP_NOADJUST );
             WinSetWindowPos(hwndTitoli, NULLHANDLE,
                             bordo, pswpFrame->cy-spazioSopra-cyTitle+bordo,
                             pswpFrame->cx-bordo*2,  cyTitle,
                             SWP_MOVE | SWP_SIZE | SWP_NOADJUST);
             WinSetWindowPos(hwndDetails, NULLHANDLE,
                             0L, 0L, pswpFrame->cx-xDetails, cyDetails,
                             SWP_SIZE );
             WinSetWindowPos(hwndPerc, NULLHANDLE,
                             0L, 0L, pswpFrame->cx-bordo*2, cyPerc,
                             SWP_SIZE );
         }
      }
    break;

    case WM_COMMAND:
      switch (SHORT1FROMMP(mp1)) {
        case ID_CSPCSTA4_TORTA:
             WinSendMsg(hwndGrafico,WM_CSPCSTA3_SETDRAW,MPFROMLONG(CSPCSTA3_DRAW_TORTA_3D),0L);
             return(MRESULT)0;
           break;
        case ID_CSPCSTA4_BARRE:
             WinSendMsg(hwndGrafico,WM_CSPCSTA3_SETDRAW,MPFROMLONG(CSPCSTA3_DRAW_BARRE_3D),0L);
             return(MRESULT)0;
           break;
        case ID_CSPCSTA4_LINEE:
             WinSendMsg(hwndGrafico,WM_CSPCSTA3_SETDRAW,MPFROMLONG(CSPCSTA3_DRAW_LINEE_3D),0L);
             return(MRESULT)0;
           break;
      }
    break;

    case WM_SCANDISCI:
      fStop=FALSE;
      DosCreateThread(&tidScan, (PFNTHREAD)Scandisci, 0UL, 0UL, 65535UL);
      return(MRESULT)0;
    break;
  }

  return WinDefDlgProc( hwnd, msg, mp1, mp2 );
}

MRESULT EXPENTRY TitoliProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  switch(msg)
  {
    case WM_COMMAND:
      WinSendMsg(hwndDlg,WM_COMMAND,mp1,mp2);
      return(MRESULT)0;
    break;

    case WM_CONTROL:
       switch ( SHORT2FROMMP( mp1 ) ) {
          case BN_PAINT:
             DrawButton( MPFROMP(mp2) );
          break;
       }
    break;
  }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

VOID ScanDirRecurse( PSZ dirBase,
                     double *spazioeffettivo,
                     double *spazioallocato,
                     double *directory,
                     double *files,
                     ULONG  *parziale,
                     ULONG  spazio100 )
{
   HDIR          hdirFindHandle = HDIR_CREATE;
   FILEFINDBUF3  FindBuffer     = {0};
   ULONG         ulFindCount    = 1;
   APIRET        rc             = NO_ERROR;
   CHAR          path[CCHMAXPATH] = {0};

   WinSetDlgItemText(hwndTitoli,ID_CSPCSTA4_DETAILS,dirBase);

   memset(path,0,sizeof(path));
   sprintf(path,"%s\\*.*",dirBase);
   rc = DosFindFirst( path,
                      &hdirFindHandle,
                      FILE_ARCHIVED | FILE_DIRECTORY | FILE_SYSTEM | FILE_HIDDEN | FILE_READONLY,
                      &FindBuffer,
                      sizeof(FindBuffer),
                      &ulFindCount,
                      FIL_STANDARD);
   while (rc==NO_ERROR && fStop==FALSE) {
      ulFindCount = 1;
      if (spazio100) WinSendMsg(hwndPerc,WM_SETPERC,MPFROMLONG(*parziale),MPFROMLONG(spazio100));
      *spazioeffettivo = *spazioeffettivo + FindBuffer.cbFile;
      *spazioallocato  = *spazioallocato  + FindBuffer.cbFileAlloc;
      *parziale        = (*parziale)      + FindBuffer.cbFile;
      if ( (FindBuffer.attrFile & FILE_DIRECTORY) &&
         (strcmp(FindBuffer.achName,".")!=0) &&
         (strcmp(FindBuffer.achName,"..")!=0) ) {
         memset(path,0,sizeof(path));
         sprintf(path,"%s\\%s",dirBase,FindBuffer.achName);
         ScanDirRecurse(path,spazioeffettivo,spazioallocato,directory,files,parziale,spazio100);
         (*directory)++;
      } else {
         (*files)++;
      }
      rc = DosFindNext(hdirFindHandle, &FindBuffer, sizeof(FindBuffer), &ulFindCount);
   }
   DosFindClose(hdirFindHandle);
   return;
}

VOID _System Scandisci( ULONG dummy )
{
  BOOL                fInteroDisco;
  FSALLOCATE          aFSAlloc;
  FSINFO              aFSInfo;
  APIRET              rc = NO_ERROR;
  ULONG               ulDriveNumber;
  CHAR                scandir[CCHMAXPATH];
  CHAR                path[CCHMAXPATH] = {0};
  HDIR                hdirFindHandle = HDIR_CREATE;
  FILEFINDBUF3        FindBuffer     = {0};
  ULONG               ulFindCount    = 1;
  HMQ                 lochmq;
  ULONG               parziale;
  CHAR                szString[255];

  double              spazioeffettivo,spazioallocato,directory,files;
  double              spaziodirtiny  = 0;  // Spazio occupato da directory
                                           // che occupano meno dell'1%
  double              spaziofiles  = 0;    // Spazio occupato dai files della
                                           // directory di partenza
  double              spazio100 = 0;       // Spazio totale per calcolo
                                           // percentuali
  double              spaziototaleocc = 0; // Spazio totale occupato
  double              spaziosprecato = 0;  // Spazio sprecato per cluster

  WinShowWindow(hwndPerc,FALSE);

  idFiles = idAltreDir = idSpazioS = idSpazioL = idFileSystem = -1;

  strcpy(scandir,CaseName(glpath));
  ulDriveNumber=scandir[0]-64;

  rc = DosQueryFSInfo(ulDriveNumber,
                      FSIL_ALLOC,
                      (PVOID)&aFSAlloc,
                      sizeof(aFSAlloc));
  if (rc != NO_ERROR) goto stop;

  rc = DosQueryFSInfo(ulDriveNumber,
                      FSIL_VOLSER,
                      (PVOID)&aFSInfo,
                      sizeof(aFSInfo));
  if (rc != NO_ERROR) goto stop;

  lochmq = WinCreateMsgQueue( hab, 0 );

  WinSetPointer(HWND_DESKTOP, WinQuerySysPointer(HWND_DESKTOP, SPTR_WAIT, FALSE));
  WinShowWindow(hwndGrafico,FALSE);

  /* Se intero disco vengono mostrate anche le informazioni relative
     a spazio: allocato, non allocato, sprecato, occupato dal file-system */
  if (strlen(scandir)==2) fInteroDisco=TRUE;
                     else fInteroDisco=FALSE;

  spazioeffettivo=spazioallocato=directory=files=0;
  parziale=0;
  ScanDirRecurse( scandir,&spazioeffettivo,&spazioallocato,&directory,&files,&parziale,0L);
  spaziototaleocc=spazioeffettivo;
  spaziosprecato=spazioallocato-spazioeffettivo;
  if (spaziosprecato<0) spaziosprecato=0;

  /* Calcolo del totale per proporzioni */
  if (fInteroDisco) {
    spazio100=aFSAlloc.cUnit * aFSAlloc.cSectorUnit * aFSAlloc.cbSector;
  } else {
    spazio100=spazioallocato;
  }
  if (spazio100==0) goto stop;

  parziale=aFSAlloc.cUnitAvail * aFSAlloc.cSectorUnit * aFSAlloc.cbSector;
  WinShowWindow(hwndPerc,TRUE);

  /* Parziali e percentuali */
  memset(path,0,sizeof(path));
  sprintf(path,"%s\\*.*",scandir);
  rc = DosFindFirst( path,
                     &hdirFindHandle,
                     FILE_ARCHIVED | FILE_DIRECTORY | FILE_SYSTEM | FILE_HIDDEN | FILE_READONLY,
                     &FindBuffer,
                     sizeof(FindBuffer),
                     &ulFindCount,
                     FIL_STANDARD);
  while (rc == NO_ERROR && fStop==FALSE) {
     ulFindCount = 1;

     if ( (FindBuffer.attrFile & FILE_DIRECTORY) &&
        (strcmp(FindBuffer.achName,".")!=0) &&
        (strcmp(FindBuffer.achName,"..")!=0) ) {

       memset(path,0,sizeof(path));
       sprintf(path,"%s\\%s",scandir,FindBuffer.achName);
       spazioeffettivo=spazioallocato=directory=files=0;
       ScanDirRecurse(path,&spazioeffettivo,&spazioallocato,&directory,&files,&parziale,spazio100);
       if ((spazioeffettivo*100.0/spazio100)<0.5) {
          spaziodirtiny=spaziodirtiny+spazioeffettivo;
       } else {
          InsertPerc(hwndGrafico,FindBuffer.achName,spazioeffettivo,spazio100);
       }
     } else {
       spaziofiles=spaziofiles+FindBuffer.cbFile;
     }
     rc = DosFindNext(hdirFindHandle, &FindBuffer, sizeof(FindBuffer), &ulFindCount);
  }
  DosFindClose(hdirFindHandle);

  idFiles    = InsertPerc(hwndGrafico,"Files",spaziofiles,spazio100);
  idAltreDir = InsertPerc(hwndGrafico,"Altre directory",spaziodirtiny,spazio100);
  idSpazioS  = InsertPerc(hwndGrafico,"Spazio sprecato", spaziosprecato, spazio100 );

  if (fInteroDisco) {
    idSpazioL = InsertPerc(hwndGrafico,"Spazio libero",
              (aFSAlloc.cUnitAvail * aFSAlloc.cSectorUnit * aFSAlloc.cbSector),spazio100);
    idFileSystem = InsertPerc(hwndGrafico,"File-system",
              (aFSAlloc.cUnit * aFSAlloc.cSectorUnit * aFSAlloc.cbSector) -
              spaziototaleocc - spaziosprecato  -
              (aFSAlloc.cUnitAvail * aFSAlloc.cSectorUnit * aFSAlloc.cbSector), spazio100 );
  }

stop:
  WinShowWindow(hwndPerc,FALSE);
  sprintf(szString,"Unit… \"[%c:] %s\" - Percorso Corrente \"%s\"",scandir[0],aFSInfo.vol.szVolLabel,scandir);
  WinSetDlgItemText(hwndTitoli,ID_CSPCSTA4_DETAILS,szString);
  WinShowWindow(hwndGrafico,TRUE);
  WinSetPointer(HWND_DESKTOP, WinQuerySysPointer(HWND_DESKTOP, SPTR_ARROW, FALSE));
  WinDestroyMsgQueue( lochmq );
  DosExit(EXIT_THREAD,0L);
  return;
}

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

ULONG InsertPerc( HWND hwndGrafico, PSZ szLabel, double perc, double tot )
{
  CSPCSTA3_INSERTITEM Insert;
  CHAR szString[35];
  ULONG num,miliardi,milioni,migliaia,unita,fTipo;

  if (perc<=0) return(FALSE);

  num=(ULONG)perc;
  if (num>1048576) {
    fTipo=1;
    num=num>>20;
  } else {
    if (num>1024) {
      fTipo=2;
      num=num>>10;
    } else {
      fTipo=3;
    }
  }

  miliardi=num/1000000000;
  num=num-miliardi*1000000000;
  milioni=num/1000000;
  num=num-milioni*1000000;
  migliaia=num/1000;
  num=num-migliaia*1000;
  unita=num;

       if (miliardi) sprintf(szString,"%d%s%3d%s%3d%s%3d",miliardi,CtryInfo.szThousandsSeparator,milioni,CtryInfo.szThousandsSeparator,migliaia,CtryInfo.szThousandsSeparator,unita);
  else if (milioni)  sprintf(szString,"%d%s%3d%s%3d",milioni,CtryInfo.szThousandsSeparator,migliaia,CtryInfo.szThousandsSeparator,unita);
  else if (migliaia) sprintf(szString,"%d%s%3d",migliaia,CtryInfo.szThousandsSeparator,unita);
  else if (unita)    sprintf(szString,"%d",unita);
  else sprintf(szString,"0");

  switch (fTipo) {
  case 1: strcat(szString," mb"); break;
  case 2: strcat(szString," kb"); break;
  case 3: strcat(szString," bytes"); break;
  }

  memset(&Insert,0,sizeof(Insert));
  Insert.dblPerc=(double)(perc*100/tot);
  //strncpy((PCHAR)Insert.szLabel,CaseName(szLabel),sizeof(Insert.szLabel));
  sprintf(Insert.szLabel,"%s %s - %4.1f%%",CaseName(szLabel),szString,Insert.dblPerc);
  return((ULONG)WinSendMsg(hwndGrafico,WM_CSPCSTA3_INSERTITEM,MPFROMP(&Insert),0l));
}

/******************************************************************************
*                                                                             *
******************************************************************************/
VOID DrawButton( PUSERBUTTON pBtn )
{
   RECTL    rcWin,rcBmp;
   POINTL   ptl;
   ULONG    idBmp;
   HBITMAP  hbm;
   static   BITMAPINFOHEADER BmpInfo;

   WinQueryWindowRect( pBtn->hwnd , &rcWin );

   rcBmp=rcWin;
   rcBmp.yTop--;
   rcBmp.xRight--;
   rcBmp.yBottom++;
   rcBmp.xLeft++;

   hbm=(HBITMAP)WinQueryWindowULong(pBtn->hwnd,QWL_USER);
   if (hbm==NULLHANDLE) {
      CHAR szStringBmp[10];
      memset(szStringBmp,0,sizeof(szStringBmp));
      WinQueryWindowText(pBtn->hwnd,sizeof(szStringBmp),szStringBmp);
      sscanf(strchr(szStringBmp,'#')+1,"%d",&idBmp);
      hbm = GpiLoadBitmap(pBtn->hps,hmod,idBmp,0,0);
      WinSetWindowULong(pBtn->hwnd,QWL_USER,(ULONG)hbm);
      GpiQueryBitmapParameters(hbm,&BmpInfo);
   }

   if (pBtn->fsState & BDS_HILITED) {
     rcBmp.yTop--;
     rcBmp.xRight++;
     rcBmp.yBottom--;
     rcBmp.xLeft++;
   }
   rcBmp.xLeft=rcBmp.xLeft+(rcBmp.xRight-rcBmp.xLeft-BmpInfo.cx)/2;
   rcBmp.yBottom=rcBmp.yBottom+(rcBmp.yTop-rcBmp.yBottom-BmpInfo.cy)/2;
   WinDrawBitmap(pBtn->hps, hbm, (PRECTL)NULL, (PPOINTL)&rcBmp, 0L, 0L, DBM_NORMAL );

   if (pBtn->fsState & BDS_HILITED) {
      GpiSetColor(pBtn->hps,CLR_BLACK);
   } else {
      GpiSetColor(pBtn->hps,CLR_WHITE);
   }
   ptl.x = rcWin.xLeft;
   ptl.y = rcWin.yBottom;
   GpiMove(pBtn->hps, &ptl);
   ptl.y = rcWin.yTop;
   GpiLine(pBtn->hps, &ptl);
   ptl.x = rcWin.xRight;
   GpiLine(pBtn->hps, &ptl);

   if (pBtn->fsState & BDS_HILITED) {
      GpiSetColor(pBtn->hps,CLR_WHITE);
   } else {
      GpiSetColor(pBtn->hps,CLR_BLACK);
   }
   ptl.x = rcWin.xRight;
   ptl.y = rcWin.yTop-1;
   GpiMove(pBtn->hps, &ptl);
   ptl.y = rcWin.yBottom;
   GpiLine(pBtn->hps, &ptl);
   ptl.x = rcWin.xLeft+1;
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
   ptl.y = rcWin.yTop-1;
   GpiMove(pBtn->hps, &ptl);
   ptl.y = rcWin.yBottom;
   GpiLine(pBtn->hps, &ptl);
   ptl.x = rcWin.xLeft+1;
   GpiLine(pBtn->hps, &ptl);
}
