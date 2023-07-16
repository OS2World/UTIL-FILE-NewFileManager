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
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <ctype.h>
#include "csoj2i0k.h"

#define SIZE_EXT  4
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

typedef struct _PRIVATESTRUCT
   {
     HWND     hwndParent;
     HWND     hwndClass;
     HWND     hwndText;
     ULONG    idClass;

     HPAL     hPal;
     ULONG    alTable[TOT_PAL];
     FONTMETRICS fmMetrics;
     ULONG    cbMaxBuff;

     BOOL     fVis;
     CHAR     filename[CCHMAXPATH];

     ULONG    percParz;
     ULONG    ulRowCurr;
   } PRIVATESTRUCT;
typedef PRIVATESTRUCT *PPRIVATESTRUCT;

MRESULT EXPENTRY CSOJ2I0K_Proc    ( HWND, ULONG, MPARAM, MPARAM );
MRESULT EXPENTRY Text_Proc        ( HWND, ULONG, MPARAM, MPARAM );
BOOL             AggiungiFormato  ( PSZ, PSZ );
LONG             CercaFormato     ( PSZ );
int              compareExt       (const void *, const void *);
BOOL             ExecExtPgm       ( PPRIVATESTRUCT, PSZ, PSZ );
BOOL             ExecHexDump      ( PPRIVATESTRUCT, PSZ, BOOL );
VOID             ChangePresParam  ( PPRIVATESTRUCT , HWND, ULONG );

static HAB hab;
static HMODULE hmod;
static CLASSINFO clsiMLE;
typedef struct {
  char ext[ SIZE_EXT+1 ];
  char command[CCHMAXPATH];
} EXTEL;
EXTEL* pExtTab=NULL;
ULONG  elExtTab=0;

#define MAXEXT                   100

BOOL CSOJ2I0K_REGISTRA( HAB lochab )
{
   CLASSINFO  clsiTmp;

   hab=lochab;

   /* Se classe gi… registrata */
   if( WinQueryClassInfo( hab,
                          (PSZ)"CSOJ2I0K",
                          &clsiTmp )==TRUE ) {
      return( TRUE );
   }

   if( ! WinRegisterClass( hab,
                           (PSZ)"CSOJ2I0K",
                           (PFNWP)CSOJ2I0K_Proc,
                           CS_SIZEREDRAW,
                           4L ) ) {
      WinMessageBox(HWND_DESKTOP,HWND_DESKTOP,"Not register class CSOJ2I0K","CSOJ2I0K",0L,0L);
      return( FALSE );
   }

   WinQueryClassInfo( hab, WC_MLE, &clsiMLE );
   WinRegisterClass(hab,
                    "CSOJ2I0K_clText",
                    Text_Proc,
                    clsiMLE.flClassStyle & ~CS_PUBLIC,
                    clsiMLE.cbWindowData );

   DosQueryModuleHandle("CSOJ2I0K", &hmod);

   DosAllocMem((PVOID)&pExtTab, sizeof(EXTEL)*MAXEXT, PAG_READ | PAG_WRITE);

   return( TRUE );
}

MRESULT EXPENTRY CSOJ2I0K_Proc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 )
{
   PPRIVATESTRUCT pArea;

   switch( msg )
   {
      case WM_CREATE:
         {
           PCREATESTRUCT pCreateStruct;
           ULONG i;

           pCreateStruct=PVOIDFROMMP(mp2);

           DosAllocMem((PVOID)&pArea, sizeof(PRIVATESTRUCT), PAG_COMMIT | PAG_READ | PAG_WRITE);
           memset(pArea,0,sizeof(PRIVATESTRUCT));
           WinSetWindowPtr(hwnd, 0, pArea);
           pArea->hwndParent=pCreateStruct->hwndParent;
           pArea->ulRowCurr=0;
           pArea->cbMaxBuff=16384;
           pArea->idClass=pCreateStruct->id;
           pArea->hwndClass=hwnd;
           pArea->fVis=TRUE;

           for (i=0;i<TOT_PAL;i++)
             pArea->alTable[i] = PC_RESERVED * 16777216 + LONGFromRGB( rgb[i].bBlue, rgb[i].bGreen, rgb[i].bRed );
           pArea->hPal=GpiCreatePalette(NULLHANDLE, LCOL_PURECOLOR, LCOLF_CONSECRGB, TOT_PAL, pArea->alTable );

           pArea->hwndText =
                 WinCreateWindow( hwnd, "CSOJ2I0K_clText", "",
                                  WS_SYNCPAINT | WS_VISIBLE | MLS_HSCROLL | MLS_VSCROLL | MLS_READONLY | MLS_DISABLEUNDO,
                                  0L, 0L, pCreateStruct->cx, pCreateStruct->cy,
                                  hwnd, HWND_TOP, ID_CSOJ2I0K_TEXT, NULL, NULL);
           WinSetWindowPtr(pArea->hwndText, 0, pArea);
           WinSendMsg( pArea->hwndText, MLM_FORMAT, MPFROMSHORT(MLFIE_NOTRANS), 0L);
         }
      break;

      case WM_PRESPARAMCHANGED:
           pArea = WinQueryWindowPtr(hwnd, 0);
           ChangePresParam( pArea, hwnd, LONGFROMMP(mp1) );
      break;

      case WM_DESTROY:
           pArea = WinQueryWindowPtr(hwnd, 0);
           GpiDeletePalette(pArea->hPal);
           DosFreeMem(pArea);
      break;

      case WM_SIZE:
         {
            SWP swp;
            pArea = WinQueryWindowPtr(hwnd, 0);
            WinQueryWindowPos(pArea->hwndClass,&swp);
            WinSetWindowPos( pArea->hwndText,
                             NULLHANDLE,
                             0L, 0L, swp.cx, swp.cy,
                             SWP_SIZE);
         }
      break;

      case WM_CSOJ2I0K_ADD_EXT:
           AggiungiFormato(mp1,mp2);
      break;

      case WM_CSOJ2I0K_SET_TEXT:
          {
             LONG index;
             EXTEL* pCurrExtTab=NULL;
             pArea = WinQueryWindowPtr(hwnd, 0);
             index=CercaFormato(PVOIDFROMMP(mp1));
             if (index>=0) {
                pCurrExtTab=pExtTab+index;
                if (pCurrExtTab->command[0]=='*') {
                  ULONG tmpfVis=TRUE;
                  if (strstr(pCurrExtTab->command,"ASCII")) tmpfVis=FALSE;
                  ExecHexDump( pArea, PVOIDFROMMP(mp1), tmpfVis );
                } else {
                  ExecExtPgm( pArea, pCurrExtTab->command, PVOIDFROMMP(mp1) );
                }
             } else {
                ExecHexDump( pArea, PVOIDFROMMP(mp1), pArea->fVis );
             }
             strcpy(pArea->filename,PVOIDFROMMP(mp1));
             WinInvalidateRect(hwnd, NULL, TRUE);
          }
      break;

      case WM_CSOJ2I0K_SET_VIS:
           pArea = WinQueryWindowPtr(hwnd, 0);
           if (LONGFROMMP(mp1)==TRUE) pArea->fVis=TRUE;
                                 else pArea->fVis=FALSE;
           WinSendMsg(hwnd,WM_CSOJ2I0K_RELOAD,0L,0L);
      break;

      case WM_CSOJ2I0K_RELOAD:
          {
             LONG index;
             EXTEL* pCurrExtTab=NULL;
             pArea = WinQueryWindowPtr(hwnd, 0);
             index=CercaFormato(pArea->filename);
             if (index>=0) {
                pCurrExtTab=pExtTab+index;
                ExecExtPgm( pArea, pCurrExtTab->command, pArea->filename  );
             } else {
                ExecHexDump( pArea, pArea->filename, pArea->fVis );
             }
          }
      break;

   }
   return(WinDefWindowProc(hwnd, msg, mp1, mp2));
}

MRESULT EXPENTRY Text_Proc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 )
{
   PPRIVATESTRUCT pArea;

   switch( msg )
   {
      case WM_BUTTON1DOWN:
      case WM_BUTTON1UP:
      case WM_BUTTON2DOWN:
      case WM_BUTTON2UP:
      case WM_CHAR:
      case WM_MOUSEMOVE:
           return(WinDefWindowProc(hwnd, msg, mp1, mp2));
      break;

      case WM_PRESPARAMCHANGED:
           pArea = WinQueryWindowPtr(hwnd, 0);
           ChangePresParam( pArea, hwnd, LONGFROMMP(mp1) );
      break;
   }
   return ( MRESULT )( * clsiMLE.pfnWindowProc )( hwnd, msg, mp1, mp2 );
}


/*****************************************************************************/
/* Carica il vettore pExtTab (di dimensione elExtTab) con i formati          */
/* supportati. Viene convertito in maiuscolo tolti i doppi e ordinato        */
/*****************************************************************************/
BOOL AggiungiFormato( PSZ ext, PSZ commandline )
{
   EXTEL* pCurrExtTab=NULL;

   if (!ext || strlen(ext)==0 || strlen(ext)>SIZE_EXT) return( FALSE );
   if (!commandline || strlen(commandline)==0) return( FALSE );

   /* Gia presente */
   if (CercaFormato(ext)>=0) return( FALSE );
   elExtTab++;
   DosSetMem((PVOID)pExtTab, sizeof(EXTEL)*elExtTab, PAG_COMMIT | PAG_READ | PAG_WRITE);
   pCurrExtTab=pExtTab+elExtTab-1;
   memset(pCurrExtTab,0,sizeof(EXTEL));
   strcpy(pCurrExtTab->ext,ext);
   strcpy(pCurrExtTab->command,commandline);
   qsort((char *)pExtTab, elExtTab, sizeof(EXTEL), compareExt);

   return( TRUE );
}

/*****************************************************************************/
/* Cerca nel vettore pExtTab (di dimensione elExtTab) il formato richiesto   */
/* Convertito in maiuscolo                                                   */
/*****************************************************************************/
LONG CercaFormato( PSZ szFile )
{
   char   *result;
   result = strrchr(szFile,'.');
   if (result) result++;
          else result=szFile;
   if (!result || strlen(result)==0 || strlen(result)>SIZE_EXT) return( -1 );
   if (elExtTab==0) return( -1 );
   result = (char*)bsearch( result, pExtTab, elExtTab, sizeof(EXTEL), compareExt);
   if (result != NULL) return( ((char*)result-(char*)pExtTab)/sizeof(EXTEL) );
                  else return( -1 );
}

int compareExt(const void *arg1, const void *arg2)
{
   return ( stricmp( (char *)arg1, (char *)arg2 ));
}

/*****************************************************************************/
/* Esecuzione della riga comando con reindirizzamento di STDIN e STDERR      */
/*****************************************************************************/
BOOL ExecExtPgm( PPRIVATESTRUCT pArea, PSZ command, PSZ szFile )
{
  #define HF_STDOUT 1      /* Standard output handle */
  #define HF_STDERR 2      /* Standard error handle */
  #define PIPE_SIZE 32767

  PSZ  Envs = NULL;
  RESULTCODES ChildRC = {0};
  CHAR Args[CCHMAXPATH];
  CHAR PgmExe[CCHMAXPATH];
  CHAR *pchar;
  CHAR *param;
  IPT  lOffset = 0;
  APIRET rc;

  HPIPE hpR, hpW;
  ULONG ulRead,ulTotRead;
  CHAR  *achBuf;
  HFILE hfSaveOut = -1,
        hfSaveErr = -1,
        hfOut = HF_STDOUT,
        hfErr = HF_STDERR;

  param=strchr(command,' ');
  if (param) param++;
        else param=command;

  memset(PgmExe,0,sizeof(PgmExe));
  strcpy(PgmExe,command);
  pchar=strrchr(PgmExe,' ');
  if (pchar) *pchar=0;

  pchar=strrchr(PgmExe,'.');
  if (!pchar) strcat(PgmExe,".exe");

  if (CSOJ2I0K_CheckPgm(PgmExe)==FALSE)
    return( FALSE );

  memset(Args,0,sizeof(Args));
  strcpy(Args,PgmExe);
  pchar=Args+strlen(PgmExe)+1;
  sprintf(pchar,"%s %s",param,szFile);

  DosDupHandle(HF_STDOUT, &hfSaveOut);
  DosDupHandle(HF_STDERR, &hfSaveErr);
  DosCreatePipe(&hpR, &hpW, PIPE_SIZE);
  DosDupHandle(hpW, &hfOut);
  DosDupHandle(hpW, &hfErr);
  rc=DosExecPgm("", 0L, EXEC_BACKGROUND, Args, Envs, &ChildRC, PgmExe);
  DosClose(hpW);
  DosDupHandle(hfSaveOut, &hfOut);
  DosDupHandle(hfSaveErr, &hfErr);

  WinSendMsg( pArea->hwndText, MLM_DISABLEREFRESH, 0L, 0L);
  WinSendMsg( pArea->hwndText, MLM_SETSEL, MPFROMSHORT(NULL),
             (MPARAM)WinSendMsg(pArea->hwndText, MLM_QUERYTEXTLENGTH, NULL, NULL));
  WinSendMsg(pArea->hwndText, MLM_CLEAR, NULL, NULL);
  DosSleep(500);

  DosAllocMem((PVOID)&achBuf, PIPE_SIZE, PAG_COMMIT | PAG_READ | PAG_WRITE);
  ulTotRead=0;
  memset(achBuf,0,PIPE_SIZE);
  rc=DosRead(hpR, achBuf, PIPE_SIZE, &ulRead);
  while (ulRead>0 && rc==NO_ERROR) {
    ulTotRead+=ulRead;
    if (pArea->cbMaxBuff==0 ||
        ulTotRead<=pArea->cbMaxBuff) {
       for (pchar=achBuf;pchar<(achBuf+PIPE_SIZE);pchar++)
          if (*pchar=='\r' && *(pchar+1)=='\n') *pchar=' ';
       WinSendMsg( pArea->hwndText, MLM_SETIMPORTEXPORT, MPFROMP(achBuf), MPFROMLONG(ulRead));
       WinSendMsg( pArea->hwndText, MLM_IMPORT, MPFROMP( &lOffset ), MPFROMP( &ulRead ));
    }
    memset(achBuf,0,PIPE_SIZE);
    rc=DosRead(hpR, achBuf, PIPE_SIZE, &ulRead);
  }
  DosClose(hpR);
  DosClose(hfSaveOut);
  DosClose(hfSaveErr);
  DosFreeMem(achBuf);

  WinSendMsg( pArea->hwndText, MLM_ENABLEREFRESH, 0L, 0L);
  return(TRUE);
}

VOID ChangePresParam( PPRIVATESTRUCT pArea, HWND hwnd, ULONG pptype )
{
   ULONG ulPresParams,ulColor=0;
   CHAR  buffer[40];
   HPS   hps;
   CSOJ2I0K_PRESPARAM pPresParam;
   static BOOL fRecurse=FALSE;
   if (fRecurse) return;
   fRecurse=TRUE;

   /* Espansione di qualsiasi PresParam sulla classe */
   memset(buffer,0,sizeof(buffer));
   if (WinQueryPresParam(hwnd, pptype,
                         0, &ulPresParams,
                         (ULONG)sizeof(buffer), (PVOID)buffer,
                         QPF_NOINHERIT)==0) {
      WinQueryPresParam(pArea->hwndClass, pptype,
                        0, &ulPresParams,
                        (ULONG)sizeof(buffer), (PVOID)buffer,
                        QPF_NOINHERIT);
   }

   if (hwnd==pArea->hwndClass) {
      WinSetPresParam(pArea->hwndText, pptype,
                      (ULONG)sizeof(buffer),
                      (PVOID)buffer);
   } else {
      WinSetPresParam(pArea->hwndClass, pptype,
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
         hps=WinGetPS(pArea->hwndText);
         pArea->alTable[CLR_USERBACKGROUND] = PC_RESERVED * 16777216 + ulColor;
         GpiSetPaletteEntries(pArea->hPal, LCOLF_CONSECRGB, 0, TOT_PAL, pArea->alTable );
         WinReleasePS(hps);
      }
   }
   WinInvalidateRect(pArea->hwndText, NULL, TRUE);

   memset(&pPresParam,0,sizeof(pPresParam));
   pPresParam.type  =pptype;
   pPresParam.cbBuff=sizeof(buffer);
   pPresParam.pBuff =buffer;
   WinSendMsg(pArea->hwndParent, WM_CONTROL,
              MPFROM2SHORT(pArea->idClass,WN_CSOJ2I0K_PRESPARAM),
              MPFROMP(&pPresParam));

   fRecurse=FALSE;
}

/*****************************************************************************/
/* Dump esadecimale del file                                                 */
/*****************************************************************************/
BOOL ExecHexDump( PPRIVATESTRUCT pArea, PSZ filename, BOOL fVis )
{
  #define BUFF_SIZE 76
  FILE *fp;
  CHAR buff[16];
  CHAR ascii[17];
  CHAR *buffMLE;
  IPT  lOffset = 0;
  ULONG ulTransfer;
  int num,i;
  ULONG offs;

  DosAllocMem((PVOID)&buffMLE, BUFF_SIZE, PAG_COMMIT | PAG_READ | PAG_WRITE);

  //WinSendMsg( pArea->hwndText, MLM_DISABLEREFRESH, 0L, 0L);
  WinShowWindow( pArea->hwndText, FALSE );
  WinSendMsg( pArea->hwndText, MLM_SETSEL, MPFROMSHORT(NULL),
              (MPARAM)WinSendMsg(pArea->hwndText, MLM_QUERYTEXTLENGTH, NULL, NULL));
  WinSendMsg(pArea->hwndText, MLM_CLEAR, NULL, NULL);
  if (fVis) {
     WinSendMsg( pArea->hwndText, MLM_SETIMPORTEXPORT, MPFROMP(buffMLE), MPFROMLONG( BUFF_SIZE ) );
  } else {
     WinSendMsg( pArea->hwndText, MLM_SETIMPORTEXPORT, MPFROMP(&ascii), MPFROMLONG( sizeof(buff) ) );
  }

  fp=fopen(filename,"rb");

  if (fp) {
    offs=0;

    memset(buff,0,sizeof(buff));
    num=fread(buff,1,sizeof(buff),fp);

    while (num>0) {

      /* Condizione di superamento buffer */
      if (pArea->cbMaxBuff>0 && lOffset>pArea->cbMaxBuff) break;

      memset(ascii,0,sizeof(ascii));
      if (fVis) {
         memset(buffMLE,0,BUFF_SIZE);
         for (i=0;i<num;i++)
              if (iscntrl(buff[i])) ascii[i]=' ';
                               else ascii[i]=buff[i];
         sprintf(buffMLE,"%08X  %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X  %s\n",
                 offs,
                 buff[0]  , buff[1]  , buff[2]  , buff[3]  , buff[4]  ,
                 buff[5]  , buff[6]  , buff[7]  , buff[8]  , buff[9]  ,
                 buff[10] , buff[11] , buff[12] , buff[13] , buff[14] ,
                 buff[15] , ascii );

         ulTransfer=BUFF_SIZE;
         WinSendMsg( pArea->hwndText, MLM_IMPORT, MPFROMP( &lOffset ), MPFROMP( &ulTransfer ));
      } else {
         for (i=0;i<num;i++)
              if (iscntrl(buff[i]) && buff[i]!='\n') ascii[i]=' ';
                                                else ascii[i]=buff[i];
         ulTransfer=sizeof(buff);
         WinSendMsg( pArea->hwndText, MLM_IMPORT, MPFROMP( &lOffset ), MPFROMP( &ulTransfer ));
      }

      memset(buff,0,sizeof(buff));
      num=fread(buff,1,sizeof(buff),fp);
      offs+=sizeof(buff);
    }
    fclose(fp);
  }
  DosFreeMem(buffMLE);
  WinShowWindow( pArea->hwndText, TRUE );
  //WinSendMsg( pArea->hwndText, MLM_ENABLEREFRESH, 0L, 0L);

  return( TRUE );
}

/*****************************************************************************/
/* Check Pgm (successo solo per pgm NON API e NON DOS )                      */
/*****************************************************************************/
BOOL CSOJ2I0K_CheckPgm( PSZ pgmname )
{
  APIRET rc;
  ULONG flag=0;
  rc=DosQueryAppType(pgmname,&flag);
  if (rc) return( FALSE );
  if (flag & FAPPTYP_NOTSPEC) return( TRUE );
  if (flag & FAPPTYP_NOTWINDOWCOMPAT) return( TRUE );
  if (flag & FAPPTYP_WINDOWCOMPAT) return( TRUE );
  return( FALSE );
}
