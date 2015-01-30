#define INCL_WIN
#define INCL_WINPOINTERS
#define INCL_DOS
#define INCL_GPI
#define INCL_BSE
#define INCL_ALL
#define INCL_SPLDOSPRINT
#define INCL_SPL
#define INCL_DEV
#define INCL_DOSDEVIOCTL
#define INCL_DOSMISC
#define INCL_DOSERRORS

#include <os2.h>
#include <mmioos2.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <mmioos2.h>
#include <dive.h>

#include "nfm.h"
#include "nfmlib.h"
#include "caoj2i03.h"
#include "csoj2i0a.h"
#include "csoj2i0k.h"
#include "csoj2i0h.h"
#include "csoj2i0i.h"
#include "cspcsta4.h"
#include "csoj2i0l.h"

#define RECEMPHASIS( hwnd, after ) (PVOID)WinSendMsg( hwnd ,                    \
                                                      CM_QUERYRECORDEMPHASIS,   \
                                                      MPFROMP( after ),         \
                                                      MPFROMSHORT(CRA_SELECTED))



#define MAX_DISK                26
#define SIZE_EXT                4

#define FILE_DRIVE              0x1000
#define FILE_DRIVE_CASESENS     0x2000
#define FILE_DRIVE_CASENOTSENS  0x4000
#define FILE_DIR_HAVE_ICON      0x8000

#define SELECT_ALL              0x0001
#define SELECT_NONE             0x0002
#define SELECT_INVERT           0x0003

#define PREVIEW_FILEFROMEXT      0
#define PREVIEW_FORCEFILEFROMEXT 1
#define PREVIEW_FORCENULLGRAPH   2
#define PREVIEW_FORCENULLTEXT    3

typedef struct _USERRECTREE
{
  RECORDCORE  recordCore;                  /* Occhio alle prime 4 posizioni  */
  CHAR        szFullName[CCHMAXPATH];      /* devono essere uguali           */
  ULONG       ulattr;                      /* a USERRECFILES                 */
  CHAR        szTitle[CCHMAXPATH];
} USERRECTREE, *PUSERRECTREE;

typedef struct _USERRECFILES
{
  RECORDCORE  recordCore;
  CHAR        szFullName[CCHMAXPATH];
  ULONG       ulattr;
  CHAR        szNomeFile[CCHMAXPATH];

  HPOINTER    hptr;
  PCHAR       pszNomeFile;
  CDATE       date;
  CTIME       time;
  ULONG       size;
  PCHAR       ptype;
  CHAR        type[SIZE_EXT+1];
  PCHAR       pattr;
  CHAR        attr[5];
  PCHAR       pvuoto;
  CHAR        vuoto;
} USERRECFILES, *PUSERRECFILES;

MRESULT       EXPENTRY FrameProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT       EXPENTRY BarraMenuProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT       EXPENTRY BarraStatoProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT       EXPENTRY ClientProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT       EXPENTRY VaiAProc( HWND hwnd, ULONG  msg, MPARAM mp1, MPARAM mp2 );
MRESULT       EXPENTRY MsgProc( HWND hwnd, ULONG  msg, MPARAM mp1, MPARAM mp2 );

VOID          EXPENTRY DrawButton( PUSERBUTTON );
BOOL          EXPENTRY SubDir( PSZ pszDir );
VOID          EXPENTRY InizializzaContainer( VOID );
VOID          EXPENTRY SetContTree( HWND, ULONG );
VOID          EXPENTRY SetContFiles( HWND, ULONG );
VOID          EXPENTRY InsertRecDummy( PUSERRECTREE );
VOID          EXPENTRY CaricaRamo( PUSERRECTREE );
VOID          EXPENTRY CaricaLista( PSZ );
VOID          EXPENTRY InserisciFile( PSZ, FILEFINDBUF4* );
PCHAR         EXPENTRY CaseName( PCHAR );

VOID          EXPENTRY ImpostazioniLeggi( VOID );
VOID          EXPENTRY ImpostazioniRipristina( VOID );
VOID          EXPENTRY ImpostazioniSalva( VOID );

BOOL          EXPENTRY VaiADirectory( PSZ, BOOL );
PUSERRECTREE  EXPENTRY SearchDirectory( PSZ, PUSERRECTREE );
SHORT         EXPENTRY pfnCompareTree( PRECORDCORE, PRECORDCORE, PVOID );
SHORT         EXPENTRY pfnCompareFiles( PRECORDCORE, PRECORDCORE, PVOID );
MRESULT       EXPENTRY ProcessCommand( HWND, USHORT );
APIRET        EXPENTRY CancellaFile( PSZ, HWND );
BOOL          EXPENTRY RinominaFile( PSZ, PSZ );
CHAR *        EXPENTRY FormatNum( ULONG, ULONG );
VOID          EXPENTRY InfoDisk( PSZ );
VOID          EXPENTRY InfoFile( VOID );
VOID          EXPENTRY ExecExtPgm( PSZ, PSZ );
VOID          EXPENTRY ApiretMsg( APIRET, PSZ );
VOID          EXPENTRY MakeTitleFromFullName( PVOID );
VOID          EXPENTRY SelectContainer( HWND, ULONG );
APIRET        EXPENTRY TogliAttrFile( PSZ );
VOID          EXPENTRY PreviewFile( PUSERRECFILES, ULONG ); /* Asincrono non torna RC */
VOID          EXPENTRY GestBarraMenu( BOOL );
VOID          EXPENTRY GestBarraStato( BOOL );

/* DEBUG *********************************************************************/
#include "perfutil.h"
#define LL2F(high, low) (4294967296.0*(high)+(low))
VOID timedebug( PSZ stringa , BOOL flag);

/* From EDM2 *****************************************************************/
const char* printEA(const char* pBase, char* buffout);
const char* printAsciiValue(const char* pBase, char* buffout);
const char* printMVMTValue(const char* pBase, char* buffout);
BOOL        listEAs(const char* name, const char* type, char* longname);
/*****************************************************************************/

static PUSERRECTREE pRecDisk[MAX_DISK];
static CHAR         pComboTab[MAX_DISK];
static PUSERRECTREE pRecFiglio;
static ULONG  cbRecordDataTree  = (sizeof(USERRECTREE)  - sizeof(RECORDCORE));
static ULONG  cbRecordDataFiles = (sizeof(USERRECFILES) - sizeof(RECORDCORE));
static RECORDINSERT recordInsert;
static BOOL primavolta=TRUE;

static HPOINTER hptrDiskA;
static HPOINTER hptrCDRom;
static HPOINTER hptrDisk;
static HPOINTER hptrLanDisk;
static HPOINTER hptrFolderO;
static HPOINTER hptrSysFile;

static CHAR        szCurrDir[CCHMAXPATH] = { 0 }; // Directory corrente
static ULONG fView=0; /* 0=nopreview  1=graph  2=text */

#define OPEN_UNKNOWN      -1
#define OPEN_DEFAULT       0
#define OPEN_CONTENTS      1
#define OPEN_SETTINGS      2
#define OPEN_HELP          3
#define OPEN_RUNNING       4
#define OPEN_PROMPTDLG     5
#define OPEN_PALETTE       121
#define CLOSED_ICON        122
#define OPEN_USER          0x6500
#define OPEN_TREE          101
#define OPEN_DETAILS       102

#define BORDER_MIACLIENT  2
#define SIZE_BARRA_SEP    6

HAB         hab;
HMQ         hmq;
HWND        hwndFrame;
HWND        hwndContTree;
HWND        hwndContFiles;
HWND        hwndClient;
HWND        hwndMiaClient;
HWND        hwndBarraMenu;
HWND        hwndBarraSepX;
HWND        hwndBarraSepY;
HWND        hwndBarraStato;
HWND        hwndMenu;
HWND        hwndMenuContNew;
HWND        hwndMenuContDrives;
HWND        hwndMenuContFiles;
HWND        hwndMenuContTrees;
HWND        hwndComboDrive;
HWND        hwndTreeTop;
HWND        hwndFilesTop;
HWND        hwndTreeBottom;
HWND        hwndFilesBottom;
USHORT      idViewSchiacciato;
ULONG       idDefaultView;
HWND        hwndPreviewBmp;
HWND        hwndPreviewTxt;
HWND        hwndLastCont;
PFIELDINFO  pFieldInfoText;

SWP         swpBarraMenu;
SWP         swpBarraStato;
SWP         swpClient;
SWP         swpMiaClient;

BOOL        fSepY;
ULONG       tipoPreviewBmp;
BOOL        tipoPreviewTxt;
ULONG       posSepX=0;
ULONG       posSepY=0;
ULONG       filemask=0;
COUNTRYINFO CtryInfo = {0};
CHAR        defFont[]="8.Helv";

int main( int argc, char *argv[] )
{
  QMSG  qmsg;
  ULONG flCreate = FCF_STANDARD;
  ULONG kx,ky;
  SWP   swpTmp;
  COUNTRYCODE Country    = {0};
  ULONG       ulInfoLen  = 0;

  DosError(FERR_DISABLEHARDERR);

  hab = WinInitialize( 0 );
  hmq = WinCreateMsgQueue( hab, 0 );

//{
//  DIVE_CAPS   caps;
//  DiveQueryCaps(&caps, DIVE_BUFFER_SCREEN);
//  if (caps.ulDepth<8) {
//     WinMessageBox(HWND_DESKTOP,HWND_DESKTOP,
//                  "Spiacente, per far funzionare questo programma occorrono almeno 256 colori.","Errore",0L,MB_APPLMODAL | MB_MOVEABLE | MB_ERROR | MB_OK);
//     WinDestroyMsgQueue( hmq );
//     WinTerminate( hab );
//     return(0);
//  }
//}

  DosQueryCtryInfo(sizeof(CtryInfo), &Country, &CtryInfo, &ulInfoLen);

  CAOJ2I03_REGISTRA( hab ); /* Embossed */
  CSOJ2I0A_REGISTRA( hab ); /* Immagine */
  CSOJ2I0K_REGISTRA( hab ); /* Testo */
  CSOJ2I0H_REGISTRA( hab ); /* Container enhanced */
  CSOJ2I0I_REGISTRA( hab ); /* Barra di spostamento */
  CSOJ2I0L_REGISTRA( hab ); /* ComboBox drives */

  hptrDiskA    = WinLoadPointer(HWND_DESKTOP,NULLHANDLE,ID_ICON_DISKA  );
  hptrCDRom    = WinLoadPointer(HWND_DESKTOP,NULLHANDLE,ID_ICON_CDROM  );
  hptrDisk     = WinLoadPointer(HWND_DESKTOP,NULLHANDLE,ID_ICON_DISK   );
  hptrLanDisk  = WinLoadPointer(HWND_DESKTOP,NULLHANDLE,ID_ICON_LANDISK);
  hptrFolderO  = WinLoadPointer(HWND_DESKTOP,NULLHANDLE,ID_ICON_FOLDERO);
  hptrSysFile  = WinQuerySysPointer(HWND_DESKTOP,SPTR_FILE,FALSE);

  /* Creazione frame window *************************************************/
  WinRegisterClass( hab, "FrameWindow", FrameProc, CS_SIZEREDRAW, 0 );
  hwndFrame = WinCreateStdWindow( HWND_DESKTOP,
                                  0L,
                                  &flCreate,
                                  "FrameWindow",
                                  "Nuovo File Manager",
                                  0L,
                                  (HMODULE)NULL,
                                  ID_FRAME,
                                  &hwndClient);
  WinSetPresParam (hwndFrame, PP_FONTNAMESIZE,
                  (ULONG)(strlen(defFont)+1),
                  (PSZ)defFont);
  WinSetWindowPos(hwndFrame, NULLHANDLE, 5L, 5L,
                  WinQuerySysValue(HWND_DESKTOP, SV_CXSCREEN) - 10,
                  WinQuerySysValue(HWND_DESKTOP, SV_CYSCREEN) - 10,
                  SWP_MOVE | SWP_SIZE );
  WinSetPresParam (hwndClient, PP_FONTNAMESIZE,
                  (ULONG)(strlen(defFont)+1),
                  (PSZ)defFont);
  WinQueryWindowPos(hwndClient,&swpClient);

  /* Menu *******************************************************************/
  hwndMenu = WinWindowFromID(hwndFrame,FID_MENU);
  WinSetPresParam (hwndMenu, PP_FONTNAMESIZE,
                  (ULONG)(strlen(defFont)+1),
                  (PSZ)defFont);

  hwndMenuContNew    = WinLoadMenu(HWND_DESKTOP, NULLHANDLE, ID_FRAME);
  WinSetPresParam(hwndMenuContNew, PP_FONTNAMESIZE, (ULONG)(strlen(defFont)+1), (PSZ)defFont);
  hwndMenuContDrives = WinLoadMenu(HWND_DESKTOP, NULLHANDLE, ID_MN_CONTDRIVES);
  WinSetPresParam(hwndMenuContDrives, PP_FONTNAMESIZE, (ULONG)(strlen(defFont)+1), (PSZ)defFont);
  hwndMenuContFiles  = WinLoadMenu(HWND_DESKTOP, NULLHANDLE, ID_MN_CONTFILES);
  WinSetPresParam(hwndMenuContFiles, PP_FONTNAMESIZE, (ULONG)(strlen(defFont)+1), (PSZ)defFont);
  hwndMenuContTrees  = WinLoadMenu(HWND_DESKTOP, NULLHANDLE, ID_MN_CONTTREES);
  WinSetPresParam(hwndMenuContTrees, PP_FONTNAMESIZE, (ULONG)(strlen(defFont)+1), (PSZ)defFont);

  ImpostazioniLeggi();

  /* Barra con pulsanti *****************************************************/
  hwndBarraMenu = WinLoadDlg( hwndClient,
                              hwndClient,
                              (PFNWP)BarraMenuProc,
                              (HMODULE)0,
                              ID_BARRA_MENU,
                              NULL);
  hwndComboDrive = WinWindowFromID(hwndBarraMenu,ID_COMBO_DRIVE);
  hwndTreeTop    = WinWindowFromID(hwndBarraMenu,ID_TREE_TOP);
  hwndFilesTop   = WinWindowFromID(hwndBarraMenu,ID_FILES_TOP);
  GestBarraMenu( TRUE );

  /* Barra di stato *********************************************************/
  hwndBarraStato = WinLoadDlg( hwndClient,
                               hwndClient,
                               (PFNWP)BarraStatoProc,
                               (HMODULE)0,
                               ID_BARRA_STATO,
                               NULL);
  hwndTreeBottom = WinWindowFromID(hwndBarraStato,ID_TREE_BOTTOM);
  hwndFilesBottom= WinWindowFromID(hwndBarraStato,ID_FILES_BOTTOM);
  GestBarraStato( TRUE );

  /* Creazione client window (MIA) ******************************************/
  WinRegisterClass( hab, "ClientWindow", ClientProc, CS_SIZEREDRAW, 0 );
  hwndMiaClient = WinCreateWindow( hwndClient,"ClientWindow", "",
                                   WS_VISIBLE,
                                   0L, swpBarraStato.cy,
                                   swpClient.cx, swpClient.cy-swpBarraMenu.cy-swpBarraStato.cy,
                                   hwndClient,
                                   HWND_TOP,
                                   -1, NULL, NULL);
  WinQueryWindowPos(hwndMiaClient,&swpMiaClient);

  /* Container Tree *********************************************************/
  hwndContTree = WinCreateWindow( hwndMiaClient, WC_CONTAINER, "",
                                  //WS_VISIBLE | CCS_MINIICONS,
                                  WS_VISIBLE,
                                  BORDER_MIACLIENT,
                                  BORDER_MIACLIENT,
                                  posSepX,
                                  swpMiaClient.cy-(BORDER_MIACLIENT*2),
                                  hwndMiaClient,
                                  HWND_BOTTOM,
                                  ID_CONT_TREE, NULL, NULL);
  kx=posSepX;

  /* Barra separatrice X ****************************************************/
  hwndBarraSepX = WinCreateWindow( hwndMiaClient, "CSOJ2I0I", "",
                                  WS_VISIBLE,
                                  kx,
                                  BORDER_MIACLIENT,
                                  SIZE_BARRA_SEP,
                                  swpMiaClient.cy-(BORDER_MIACLIENT*2),
                                  hwndMiaClient,
                                  HWND_TOP,
                                  ID_BARRA_SEPX, NULL, NULL);
  WinSendMsg(hwndBarraSepX, WM_CSOJ2I0I_SETSIZEBORDER, MPFROMLONG(BORDER_MIACLIENT), 0L );

  WinQueryWindowPos(hwndTreeTop,&swpTmp);
  WinSetWindowPos(hwndTreeTop, NULLHANDLE,
                  0L, 0L, kx+1, swpTmp.cy, SWP_SIZE);
  WinQueryWindowPos(hwndTreeBottom,&swpTmp);
  WinSetWindowPos(hwndTreeBottom, NULLHANDLE,
                  0L, 0L, kx+1, swpTmp.cy, SWP_SIZE);

  kx=kx+SIZE_BARRA_SEP;

  WinQueryWindowPos(hwndFilesTop,&swpTmp);
  WinSetWindowPos(hwndFilesTop, NULLHANDLE,
                  kx-1, swpTmp.y,
                  swpMiaClient.cx-kx, swpTmp.cy,
                  SWP_MOVE | SWP_SIZE);
  WinQueryWindowPos(hwndFilesBottom,&swpTmp);
  WinSetWindowPos(hwndFilesBottom, NULLHANDLE,
                  kx-1, swpTmp.y,
                  swpMiaClient.cx-kx, swpTmp.cy,
                  SWP_MOVE | SWP_SIZE);

  /* Anteprima **************************************************************/
  hwndPreviewBmp =WinCreateWindow( hwndMiaClient, "CSOJ2I0A", "",
                                 //  WS_VISIBLE | WS_CSOJ2I0A_THUMBNAIL,
                                   WS_VISIBLE | WS_CSOJ2I0A_BOTTONI | WS_CSOJ2I0A_THUMBNAIL,
                                   kx,
                                   BORDER_MIACLIENT,
                                   swpMiaClient.cx-kx-BORDER_MIACLIENT,
                                   posSepY,
                                   hwndMiaClient,
                                   HWND_BOTTOM,
                                   ID_PREVIEW_BMP, NULL, NULL);
  hwndPreviewTxt =WinCreateWindow( hwndMiaClient, "CSOJ2I0K", "",
                                   WS_VISIBLE | DT_VCENTER | DT_LEFT,
                                   kx,
                                   BORDER_MIACLIENT,
                                   swpMiaClient.cx-kx-BORDER_MIACLIENT,
                                   posSepY,
                                   hwndMiaClient,
                                   HWND_BOTTOM,
                                   ID_PREVIEW_TXT, NULL, NULL);
  ky=posSepY;

  WinSendMsg(hwndPreviewTxt,WM_CSOJ2I0K_ADD_EXT,MPFROMP("txt" ),MPFROMP("* ASCII"));
  WinSendMsg(hwndPreviewTxt,WM_CSOJ2I0K_ADD_EXT,MPFROMP("cfg" ),MPFROMP("* ASCII"));
  WinSendMsg(hwndPreviewTxt,WM_CSOJ2I0K_ADD_EXT,MPFROMP("cmd" ),MPFROMP("* ASCII"));
  WinSendMsg(hwndPreviewTxt,WM_CSOJ2I0K_ADD_EXT,MPFROMP("bat" ),MPFROMP("* ASCII"));
  WinSendMsg(hwndPreviewTxt,WM_CSOJ2I0K_ADD_EXT,MPFROMP("cbl" ),MPFROMP("* ASCII"));
  WinSendMsg(hwndPreviewTxt,WM_CSOJ2I0K_ADD_EXT,MPFROMP("zip" ),MPFROMP("unzip -l"));
  WinSendMsg(hwndPreviewTxt,WM_CSOJ2I0K_ADD_EXT,MPFROMP("exe" ),MPFROMP("bldlevel"));
  WinSendMsg(hwndPreviewTxt,WM_CSOJ2I0K_ADD_EXT,MPFROMP("dll" ),MPFROMP("bldlevel"));

  /* Barra separatrice Y ****************************************************/
  hwndBarraSepY = WinCreateWindow( hwndMiaClient, "CSOJ2I0I", "",
                                  WS_VISIBLE,
                                  kx,
                                  ky,
                                  swpMiaClient.cx-kx-BORDER_MIACLIENT,
                                  SIZE_BARRA_SEP,
                                  hwndMiaClient,
                                  HWND_TOP,
                                  ID_BARRA_SEPY, NULL, NULL);
  WinSendMsg(hwndBarraSepY, WM_CSOJ2I0I_SETSIZEBORDER, MPFROMLONG(BORDER_MIACLIENT), 0L );
  ky=ky+SIZE_BARRA_SEP;

  /* Container Files ********************************************************/
  hwndContFiles = WinCreateWindow( hwndMiaClient, "CSOJ2I0H", "",
                                   WS_VISIBLE | CCS_AUTOPOSITION | CCS_EXTENDSEL | CCS_MINIICONS,
                                   kx,
                                   ky,
                                   swpMiaClient.cx-kx-BORDER_MIACLIENT,
                                   swpMiaClient.cy-BORDER_MIACLIENT-ky,
                                   hwndMiaClient,
                                   HWND_BOTTOM,
                                   ID_CONT_FILES, NULL, NULL);
  ky=ky+posSepY;

  memset(szCurrDir,0,sizeof(szCurrDir));
  SetContTree( hwndContTree, CV_MINI | CA_DRAWICON );
  InizializzaContainer();

  ImpostazioniRipristina();

  WinSendMsg(hwndFrame,WM_COMMAND,MPFROMSHORT(idDefaultView),MPFROM2SHORT(0,0));
  fSepY=!fSepY;
  WinSendMsg(hwndFrame,WM_COMMAND,MPFROMSHORT(ID_PREVIEW),MPFROM2SHORT(0,0));
  WinSendMsg(hwndPreviewBmp,WM_COMMAND,MPFROMSHORT(tipoPreviewBmp),0L);

  if (argc==2) VaiADirectory(argv[1],TRUE);

  while( WinGetMsg( hab, &qmsg, (HWND)NULL, 0, 0 ) )
        WinDispatchMsg( hab, &qmsg );

  ImpostazioniSalva();

  WinDestroyWindow(hwndFrame);

  WinDestroyMsgQueue( hmq );
  WinTerminate( hab );
  DosError(FERR_ENABLEHARDERR);
  return(0);
}

MRESULT EXPENTRY FrameProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
   switch(msg)
   {
     case WM_CREATE:
        ;
     break;

     case WM_MINMAXFRAME:
       {
          PSWP pswpFrame;
          pswpFrame=PVOIDFROMMP(mp1);
          if ((pswpFrame->fl & SWP_RESTORE) || (pswpFrame->fl & SWP_MAXIMIZE)) {
             if (swpBarraMenu.cx>0) {
                WinSetWindowPos(hwndBarraMenu, NULLHANDLE,
                                swpBarraMenu.x,  swpBarraMenu.y,
                                swpBarraMenu.cx, swpBarraMenu.cy,
                                SWP_MOVE | SWP_SIZE | SWP_SHOW );
             }
          }
       }
     break;

     case WM_SIZE:
       {
         SWP swpBarraSepX,swpBarraSepY,swp;
         ULONG kx,ky;
         WinQueryWindowPos(hwndClient,&swpClient);

         if (swpBarraMenu.cx>0) {
           WinSetWindowPos(hwndBarraMenu, NULLHANDLE,
                           0L,              swpClient.cy-swpBarraMenu.cy,
                           swpClient.cx+2,  swpBarraMenu.cy,
                           SWP_MOVE | SWP_SIZE );
         }

         if (swpBarraStato.cx>0) {
            WinSetWindowPos(hwndBarraStato, NULLHANDLE,
                            0L,              0L,
                            swpClient.cx+2,  swpBarraStato.cy,
                            SWP_MOVE | SWP_SIZE );
         }

         WinSetWindowPos( hwndMiaClient, NULLHANDLE,
                          0L,             swpBarraStato.cy,
                          swpClient.cx,   swpClient.cy-swpBarraMenu.cy-swpBarraStato.cy,
                          SWP_MOVE | SWP_SIZE );
         WinQueryWindowPos(hwndMiaClient,&swpMiaClient);

         memset(&swpBarraSepX,0,sizeof(swpBarraSepX));
         WinQueryWindowPos(hwndBarraSepX,&swpBarraSepX);
         WinSetWindowPos( hwndBarraSepX,   NULLHANDLE,
                          0l, 0l,
                          SIZE_BARRA_SEP, swpMiaClient.cy-(BORDER_MIACLIENT*2),
                          SWP_SIZE );

         WinSetWindowPos( hwndContTree, NULLHANDLE,
                          BORDER_MIACLIENT, BORDER_MIACLIENT,
                          swpBarraSepX.x-BORDER_MIACLIENT, swpMiaClient.cy-(BORDER_MIACLIENT*2),
                          SWP_MOVE | SWP_SIZE );
         kx=swpBarraSepX.x+swpBarraSepX.cx;

         WinSetWindowPos( hwndBarraSepY,   NULLHANDLE,
                          0l, 0l,
                          swpMiaClient.cx-kx-BORDER_MIACLIENT, SIZE_BARRA_SEP,
                          SWP_SIZE );
         memset(&swpBarraSepY,0,sizeof(swpBarraSepY));
         WinQueryWindowPos(hwndBarraSepY,&swpBarraSepY);
         if (fSepY) {
           ky=swpBarraSepY.y+swpBarraSepY.cy;
         } else {
           ky=BORDER_MIACLIENT;
         }

         WinSetWindowPos( hwndContFiles, NULLHANDLE,
                          0l, 0l,
                          swpMiaClient.cx-kx-BORDER_MIACLIENT, swpMiaClient.cy-ky-BORDER_MIACLIENT,
                          SWP_SIZE );

         WinSetWindowPos( hwndPreviewBmp, NULLHANDLE,
                          0l, 0l,
                          swpMiaClient.cx-kx-BORDER_MIACLIENT, swpBarraSepY.y-BORDER_MIACLIENT,
                          SWP_SIZE );
         WinSetWindowPos( hwndPreviewTxt, NULLHANDLE,
                          0l, 0l,
                          swpMiaClient.cx-kx-BORDER_MIACLIENT, swpBarraSepY.y-BORDER_MIACLIENT,
                          SWP_SIZE );

         WinQueryWindowPos(hwndFilesTop,&swp);
         WinSetWindowPos( hwndFilesTop, NULLHANDLE,
                          0l, 0l,
                          swpMiaClient.cx-kx, swp.cy,
                          SWP_SIZE );

         WinQueryWindowPos(hwndFilesBottom,&swp);
         WinSetWindowPos( hwndFilesBottom, NULLHANDLE,
                          0l, 0l,
                          swpMiaClient.cx-kx, swp.cy,
                          SWP_SIZE );
       }
     break;

     case WM_COMMAND:
       return(ProcessCommand(hwnd,SHORT1FROMMP(mp1)));
     break;
   }
   return(WinDefWindowProc(hwnd, msg, mp1, mp2));
}

BOOL SubDir( PSZ pszDir )
{
   HDIR hdirFindHandle = HDIR_CREATE;
   FILEFINDBUF3  FindBuffer = {0};
   ULONG         ulResultBufLen = sizeof(FILEFINDBUF3);
   ULONG         ulFindCount;
   ULONG         ulFindFound    = 0;
   APIRET        rc             = 0;
   CHAR          szString[CCHMAXPATH];

   memset(&szString,0,sizeof(szString));
   sprintf(szString,"%s\\*.*",pszDir);

   memset(&FindBuffer,0,sizeof(FindBuffer));
   ulResultBufLen = sizeof(FILEFINDBUF3);
   ulFindCount = 1;
   rc = DosFindFirst( szString,
                      &hdirFindHandle,
                      MUST_HAVE_DIRECTORY | FILE_DIRECTORY | filemask,
                      &FindBuffer,
                      ulResultBufLen,
                      &ulFindCount,
                      FIL_STANDARD);
   while (rc==0 && ulFindFound==0) {
      if (strcmp(FindBuffer.achName,".")!=0 &&
          strcmp(FindBuffer.achName,"..")!=0) {
         ulFindFound++;
      }
      ulFindCount = 1;
      rc = DosFindNext(hdirFindHandle,
                       &FindBuffer,
                       ulResultBufLen,
                       &ulFindCount);
   }
   DosFindClose(hdirFindHandle);
   if (ulFindFound==0) {
      return( FALSE );
   } else {
      return( TRUE );
   }
}

/******************************************************************************
* Inizializzazione del TreeContainer e della ComboDrive                       *
******************************************************************************/
VOID InizializzaContainer( )
{
  FSINFO      aFSInfo;
  ULONG       cbBuffer   = sizeof(FSQBUFFER2) + (3 * CCHMAXPATH);
  ULONG       cbBufferOut;
  PFSQBUFFER2 pfsqBuffer;
  ULONG       ulDriveNumber=0;
  ULONG       ulComboNumber=0;
  SHORT       index=0;
  APIRET      rc = 0;
  CHAR        szDisk[3];

  memset(pComboTab,0,sizeof(pComboTab));
  memset(pRecDisk,0,sizeof(pRecDisk));

  DosAllocMem((PVOID)&pfsqBuffer, cbBuffer, PAG_COMMIT | PAG_READ | PAG_WRITE);

  // Azzeramento container e deallocazione totale
  WinSendMsg(hwndComboDrive,LM_DELETEALL,0L,0L);
  WinSendMsg(hwndContTree,
             CM_REMOVERECORD,
             0L, MPFROM2SHORT(0, CMA_FREE ));
  WinSendMsg(hwndContFiles,
             CM_REMOVERECORD,
             0L, MPFROM2SHORT(0, CMA_FREE ));
  WinSendMsg(hwndContFiles,
             CM_REMOVEDETAILFIELDINFO,
             0L, MPFROM2SHORT(0, CMA_FREE | CMA_INVALIDATE ));
  primavolta=TRUE;

  /**************************************************************************/
  for (ulDriveNumber=0;ulDriveNumber<MAX_DISK;ulDriveNumber++) {
      sprintf(szDisk,"%c:",ulDriveNumber+'A');

      memset(pfsqBuffer,0,cbBuffer);
      cbBufferOut=cbBuffer;
      rc=DosQueryFSAttach(szDisk,
                          0L,
                          FSAIL_QUERYNAME,
                          pfsqBuffer,
                          &cbBufferOut);
      if (rc==0 || rc==ERROR_NOT_READY) {
         memset(&aFSInfo,0,sizeof(aFSInfo));
         /* Se il drive non Š pronto Š inutile appesantire con inutili letture */
         if (rc==0) {
            DosQueryFSInfo(ulDriveNumber+1,
                           FSIL_VOLSER,
                           &aFSInfo,
                           sizeof(FSINFO));
         }

         pRecDisk[ulDriveNumber] =
                 WinSendMsg(hwndContTree,
                            CM_ALLOCRECORD,
                            MPFROMLONG(cbRecordDataTree), MPFROMSHORT(1));

         memset(pRecDisk[ulDriveNumber],0,cbRecordDataTree);
         pRecDisk[ulDriveNumber]->recordCore.cb = sizeof(RECORDCORE);
         pRecDisk[ulDriveNumber]->recordCore.flRecordAttr = CRA_DROPONABLE | CRA_RECORDREADONLY;

         memset(pRecDisk[ulDriveNumber]->szFullName,0,sizeof(pRecDisk[ulDriveNumber]->szFullName));
         strcpy(pRecDisk[ulDriveNumber]->szFullName,szDisk);

         memset(pRecDisk[ulDriveNumber]->szTitle   ,0,sizeof(pRecDisk[ulDriveNumber]->szTitle   ));
         sprintf(pRecDisk[ulDriveNumber]->szTitle,"[%s] %s",szDisk,CaseName(aFSInfo.vol.szVolLabel));
         pRecDisk[ulDriveNumber]->ulattr=FILE_DRIVE | FILE_DRIVE_CASENOTSENS;

         pRecDisk[ulDriveNumber]->recordCore.pszTree=pRecDisk[ulDriveNumber]->szTitle;
         pRecDisk[ulDriveNumber]->recordCore.preccNextRecord=NULLHANDLE;

         pRecDisk[ulDriveNumber]->recordCore.hptrIcon = hptrDisk;

         if (pfsqBuffer->iType==4) {
           pRecDisk[ulDriveNumber]->recordCore.hptrIcon = hptrLanDisk;
         } else {
           BIOSPARAMETERBLOCK Data;
           struct {
              UCHAR CommandInformation;
              UCHAR DriveUnit;
           } Params;
           ULONG ulParamsLen;
           ULONG ulDataLen;
           ulParamsLen=sizeof(Params);
           ulDataLen=0;
           Params.CommandInformation = 0;
           Params.DriveUnit = ulDriveNumber;
           DosDevIOCtl((ULONG)-1,
                        IOCTL_DISK,
                        DSK_GETDEVICEPARAMS,
                        (PVOID)&Params,
                        sizeof(Params),
                        &ulParamsLen,
                        (PVOID)&Data,
                        sizeof(Data),
                        &ulDataLen);
            /* Se non Š un disco fisso */
            if (Data.bDeviceType!=5) {
               if (ulDriveNumber<3) {
                  /* Floppy */
                  pRecDisk[ulDriveNumber]->recordCore.hptrIcon = hptrDiskA;
               } else {
                  /* CDRom ?? */
                  pRecDisk[ulDriveNumber]->recordCore.hptrIcon = hptrCDRom;
               }
            }
         }

         recordInsert.cb                = sizeof(RECORDINSERT);
         recordInsert.pRecordParent     = (PRECORDCORE)NULL;
         recordInsert.pRecordOrder      = (PRECORDCORE)CMA_END;
         recordInsert.zOrder            = CMA_TOP;
         recordInsert.cRecordsInsert    = 1;
         recordInsert.fInvalidateRecord = FALSE;

         WinSendMsg(hwndContTree,
                    CM_INSERTRECORD,
                    (PRECORDCORE)pRecDisk[ulDriveNumber], &recordInsert);

         pComboTab[ulComboNumber++]=(char)(ulDriveNumber+'A');
         index=(SHORT)WinSendMsg(hwndComboDrive,LM_INSERTITEM,MPFROMSHORT(LIT_END),MPFROMP(pRecDisk[ulDriveNumber]->szTitle));

         // Per default mi posiziono su C:
         if (ulDriveNumber==2)
            WinSendMsg(hwndComboDrive,LM_SELECTITEM,MPFROMSHORT(index),MPFROMLONG(TRUE));

         if (SubDir(pRecDisk[ulDriveNumber]->szFullName))
            InsertRecDummy( pRecDisk[ulDriveNumber] );
      }
  }

  WinSendMsg(hwndContTree,
             CM_INVALIDATERECORD,
             0L, MPFROM2SHORT(0,CMA_NOREPOSITION));

  WinSendMsg(hwndContTree,
             CM_SETRECORDEMPHASIS,
             MPFROMP(pRecDisk[2]), MPFROM2SHORT(TRUE,CRA_SELECTED));

  DosFreeMem(pfsqBuffer);
  return;
}

VOID SetContTree( HWND hwndContainer, ULONG style )
{
   CNRINFO cnrinfo;
   ULONG   aspetto;

   memset(&cnrinfo,0,sizeof(cnrinfo));
   cnrinfo.cb                     = sizeof(CNRINFO);
   cnrinfo.pSortRecord            = (PVOID)pfnCompareTree;
   cnrinfo.pFieldInfoLast         = NULL;
   cnrinfo.pFieldInfoObject       = NULL;
   cnrinfo.pszCnrTitle            = "";
   cnrinfo.flWindowAttr           = style | CV_TREE | CA_TREELINE | CA_ORDEREDTARGETEMPH | CA_MIXEDTARGETEMPH | CA_OWNERDRAW;
   cnrinfo.ptlOrigin.x            = 0;
   cnrinfo.ptlOrigin.y            = 0;
   cnrinfo.cDelta                 = 0;
   cnrinfo.cRecords               = 0;
   cnrinfo.slBitmapOrIcon.cx      = 14;
   cnrinfo.slBitmapOrIcon.cy      = 16;
   cnrinfo.slTreeBitmapOrIcon.cx  = 13;
   cnrinfo.slTreeBitmapOrIcon.cy  = 13;
   cnrinfo.hbmExpanded            = NULLHANDLE;
   cnrinfo.hbmCollapsed           = NULLHANDLE;
   cnrinfo.hptrExpanded           = NULLHANDLE;
   cnrinfo.hptrCollapsed          = NULLHANDLE;
   cnrinfo.cyLineSpacing          = 0;
   cnrinfo.cxTreeIndent           = 20;
   cnrinfo.cxTreeLine             = 0;
   cnrinfo.cFields                = 0;
   cnrinfo.xVertSplitbar          = 0;

   aspetto = CMA_PSORTRECORD |
             CMA_FLWINDOWATTR |
             CMA_SLTREEBITMAPORICON |
             CMA_LINESPACING |
             CMA_CXTREEINDENT;

   if (style & CV_MINI) aspetto = aspetto | CMA_SLBITMAPORICON;

   WinSendMsg(hwndContainer,CM_SETCNRINFO,&cnrinfo, MPFROMLONG(aspetto));
}

VOID SetContFiles( HWND hwndContainer, ULONG style )
{
   CNRINFO cnrinfo;
   ULONG   aspetto;

   memset(&cnrinfo,0,sizeof(cnrinfo));
   cnrinfo.cb                     = sizeof(CNRINFO);
   cnrinfo.pSortRecord            = (PVOID)pfnCompareFiles;
   cnrinfo.pFieldInfoLast         = NULL;
   cnrinfo.pFieldInfoObject       = NULL;
   cnrinfo.pszCnrTitle            = "";
   cnrinfo.flWindowAttr           = style | CA_ORDEREDTARGETEMPH | CA_MIXEDTARGETEMPH | CA_OWNERDRAW;
   cnrinfo.ptlOrigin.x            = 0;
   cnrinfo.ptlOrigin.y            = 0;
   cnrinfo.cDelta                 = 0;
   cnrinfo.cRecords               = 0;
   cnrinfo.slBitmapOrIcon.cx      = 0;
   cnrinfo.slBitmapOrIcon.cy      = 0;
   cnrinfo.slTreeBitmapOrIcon.cx  = 12;
   cnrinfo.slTreeBitmapOrIcon.cy  = 12;
   cnrinfo.hbmExpanded            = NULLHANDLE;
   cnrinfo.hbmCollapsed           = NULLHANDLE;
   cnrinfo.hptrExpanded           = NULLHANDLE;
   cnrinfo.hptrCollapsed          = NULLHANDLE;
   cnrinfo.cyLineSpacing          = 7;
   cnrinfo.cxTreeIndent           = 0;
   cnrinfo.cxTreeLine             = 0;
   cnrinfo.cFields                = 0;
   cnrinfo.xVertSplitbar          = 0;

   aspetto = CMA_PSORTRECORD |
             CMA_FLWINDOWATTR |
             CMA_LINESPACING;

   if (style & CV_MINI) aspetto = aspetto | CMA_SLBITMAPORICON;

   WinSendMsg(hwndContainer,CM_SETCNRINFO,&cnrinfo, MPFROMLONG(aspetto));
}

VOID InsertRecDummy( PUSERRECTREE pRecPadre )
{
   PUSERRECTREE pRecDummy;
   pRecDummy = WinSendMsg(hwndContTree,
                          CM_ALLOCRECORD,
                          MPFROMLONG(cbRecordDataTree), MPFROMSHORT(1));
   memset(pRecDummy,0,cbRecordDataTree);
   pRecDummy->recordCore.cb = sizeof(RECORDCORE);
   pRecDummy->recordCore.pszTree = "";
   pRecDummy->recordCore.preccNextRecord=NULLHANDLE;
   recordInsert.cb = sizeof(RECORDINSERT);
   recordInsert.pRecordParent     = (PRECORDCORE)pRecPadre;
   recordInsert.pRecordOrder      = (PRECORDCORE)CMA_END;
   recordInsert.zOrder            = CMA_TOP;
   recordInsert.cRecordsInsert    = 1;
   recordInsert.fInvalidateRecord = FALSE;
   WinSendMsg(hwndContTree,
              CM_INSERTRECORD,
              (PRECORDCORE)pRecDummy, &recordInsert);
   return;
}

VOID CaricaRamo( PUSERRECTREE pRecPadre )
{
   HDIR          hdirFindHandle = HDIR_CREATE;
   FILEFINDBUF4  FindBuffer = {0};
   ULONG         ulResultBufLen = sizeof(FILEFINDBUF4);
   ULONG         ulFindCount;
   APIRET        rc             = 0;
   CHAR          szString[CCHMAXPATH];
   PUSERRECTREE  pNext[1] = { 0 };

   if (strlen(pRecPadre->szFullName)==0) return;

   pNext[0]=(PUSERRECTREE)WinSendMsg(hwndContTree,
                                 CM_QUERYRECORD,
                                 pRecPadre,
                                 MPFROM2SHORT( CMA_FIRSTCHILD , CMA_ITEMORDER ));
   while (pNext[0]!=NULLHANDLE) {
      WinFreeFileIcon(pNext[0]->recordCore.hptrIcon);
      WinSendMsg(hwndContTree,
                 CM_REMOVERECORD,
                 pNext,
                 MPFROM2SHORT( 1, CMA_FREE ));
      pNext[0]=(PUSERRECTREE)WinSendMsg(hwndContTree,
                                      CM_QUERYRECORD,
                                     pRecPadre,
                                     MPFROM2SHORT( CMA_FIRSTCHILD, CMA_ITEMORDER ));
   }

   memset(&szString,0,sizeof(szString));
   sprintf(szString,"%s\\*.*",pRecPadre->szFullName);

   memset(&FindBuffer,0,sizeof(FindBuffer));
   ulResultBufLen = sizeof(FILEFINDBUF4);
   ulFindCount = 1;
   rc = DosFindFirst( szString,
                      &hdirFindHandle,
                      MUST_HAVE_DIRECTORY | FILE_DIRECTORY | filemask,
                      &FindBuffer,
                      ulResultBufLen,
                      &ulFindCount,
                      FIL_QUERYEASIZE);
   while (rc == 0) {
      if (strcmp(FindBuffer.achName,".")!=0 &&
          strcmp(FindBuffer.achName,"..")!=0)  {

          pRecFiglio = WinSendMsg(hwndContTree,
                                  CM_ALLOCRECORD,
                                  MPFROMLONG(cbRecordDataTree), MPFROMSHORT(1));

          memset(pRecFiglio,0,cbRecordDataTree);
          pRecFiglio->recordCore.cb = sizeof(RECORDCORE);
          pRecFiglio->recordCore.flRecordAttr = CRA_DROPONABLE;

          memset(pRecFiglio->szFullName,0,sizeof(pRecFiglio->szFullName));
          sprintf(pRecFiglio->szFullName,"%s\\%s",pRecPadre->szFullName,FindBuffer.achName);

          pRecFiglio->ulattr=FindBuffer.attrFile;

          memset(pRecFiglio->szTitle,0,sizeof(pRecFiglio->szTitle));
          if (FindBuffer.cbList>4) {
            listEAs(pRecFiglio->szFullName, ".LONGNAME", pRecFiglio->szTitle);
            if (listEAs(pRecFiglio->szFullName, ".ICON", NULL)) pRecFiglio->ulattr |= FILE_DIR_HAVE_ICON;
          }
          if (strlen(pRecFiglio->szTitle)==0) {
            strcpy(pRecFiglio->szTitle,FindBuffer.achName);
          }
          CaseName(pRecFiglio->szTitle);

          pRecFiglio->recordCore.pszTree = pRecFiglio->szTitle;
          pRecFiglio->recordCore.preccNextRecord=NULLHANDLE;

          pRecFiglio->recordCore.hptrIcon = WinLoadFileIcon(pRecFiglio->szFullName,FALSE);

          recordInsert.cb = sizeof(RECORDINSERT);
          recordInsert.pRecordParent     = (PRECORDCORE)pRecPadre;
          recordInsert.pRecordOrder      = (PRECORDCORE)CMA_END;
          recordInsert.zOrder            = CMA_TOP;
          recordInsert.cRecordsInsert    = 1;
          recordInsert.fInvalidateRecord = FALSE;
          WinSendMsg(hwndContTree,
                     CM_INSERTRECORD,
                     (PRECORDCORE)pRecFiglio, &recordInsert);
          if (SubDir(pRecFiglio->szFullName))
             InsertRecDummy( pRecFiglio );

      }
      ulFindCount = 1;
      rc = DosFindNext(hdirFindHandle,
                       &FindBuffer,
                       ulResultBufLen,
                       &ulFindCount);
   }
   DosFindClose(hdirFindHandle);

   WinSendMsg(hwndContTree,
              CM_INVALIDATERECORD,
              0L, MPFROM2SHORT(0,CMA_REPOSITION));
   return;
}

VOID CaricaLista( PSZ pszDir )
{
   HDIR          hdirFindHandle = HDIR_CREATE;
   PFILEFINDBUF4 pFindBuffer;
   ULONG         ulResultBufLen = sizeof(FILEFINDBUF4);
   ULONG         ulFindCount;
   APIRET        rc             = 0;
   CHAR          szString[CCHMAXPATH*2];
   ULONG         count=0,size=0;
   CHAR          name[CCHMAXPATH];
   CHAR *        pchar;
   PUSERRECFILES pRec;

   WinSetPointer(HWND_DESKTOP,WinQuerySysPointer(HWND_DESKTOP, SPTR_WAIT, FALSE));

   pRec=(PUSERRECFILES)WinSendMsg(hwndContFiles,
                                  CM_QUERYRECORD,
                                  0L,
                                  MPFROM2SHORT( CMA_FIRST , CMA_ITEMORDER ));
   while (pRec) {
      WinFreeFileIcon(pRec->hptr);
      pRec=(PUSERRECFILES)WinSendMsg(hwndContFiles,
                                      CM_QUERYRECORD,
                                      pRec,
                                      MPFROM2SHORT( CMA_NEXT, CMA_ITEMORDER ));
   }

   WinSendMsg(hwndContFiles,
              CM_REMOVERECORD,
              0L, MPFROM2SHORT(0,CMA_FREE ));


   DosAllocMem((PVOID)&pFindBuffer, ulResultBufLen, PAG_COMMIT | PAG_READ | PAG_WRITE);

   memset(&szString,0,sizeof(szString));
   sprintf(szString,"%s\\*.*",pszDir);

   ulFindCount = 1;
   rc = DosFindFirst( szString,
                      &hdirFindHandle,
                      FILE_DIRECTORY | filemask,
                      pFindBuffer,
                      ulResultBufLen,
                      &ulFindCount,
                      FIL_QUERYEASIZE);

   while (rc==0) {
      if (strcmp(pFindBuffer->achName,".")!=0
           &&
          strcmp(pFindBuffer->achName,"..")!=0)  {
        memset(&szString,0,sizeof(szString));
        size+=pFindBuffer->cbFile;
        count++;
        InserisciFile( pszDir, pFindBuffer );
      }
      rc = DosFindNext(hdirFindHandle,
                       pFindBuffer,
                       ulResultBufLen,
                       &ulFindCount);
   }

   DosFindClose(hdirFindHandle);
   DosFreeMem(pFindBuffer);

   pchar=strrchr(pszDir,'\\');
   if (pchar) pchar++;
         else pchar=pszDir;
   strcpy(name,pchar);
   sprintf(szString,"Contenuto di  \"%s\"  (%s files  %s bytes)",
                    CaseName(name), FormatNum(count,1), FormatNum(size,2));
   WinSetWindowText(hwndFilesTop,szString);

   SelectContainer( hwndContFiles, SELECT_NONE );

// {
//   QUERYRECORDRECT queryRecordRect;
//   RECTL rectlRec,rectlCont;
//   LONG spostamento=0;
//   pRec=(PUSERRECFILES)WinSendMsg(hwndContFiles,
//                                  CM_QUERYRECORD,
//                                  0L,
//                                  MPFROM2SHORT( CMA_FIRST , CMA_ITEMORDER ));
//   WinQueryWindowRect(hwndContFiles,&rectlCont);
//   memset(&queryRecordRect,0,sizeof(queryRecordRect));
//   queryRecordRect.cb=sizeof(queryRecordRect);
//   queryRecordRect.pRecord=(PRECORDCORE)pRec;
//   queryRecordRect.fsExtent=CMA_TEXT;
//   WinSendMsg(hwndContFiles,
//              CM_QUERYRECORDRECT,
//              MPFROMP( &rectlRec ), MPFROMP( &queryRecordRect ));
//   spostamento = (LONG) - ( rectlRec.yTop - (rectlCont.yTop - rectlCont.yBottom));
//   WinSendMsg(hwndContFiles,
//              CM_SCROLLWINDOW,
//              MPFROMSHORT( CMA_VERTICAL ), MPFROMLONG( spostamento ));
// }

   WinSendMsg(hwndContFiles,
              CM_INVALIDATERECORD,
              0L, MPFROM2SHORT(0,CMA_REPOSITION));
   WinSetPointer(HWND_DESKTOP,WinQuerySysPointer(HWND_DESKTOP, SPTR_ARROW, FALSE));

   return;
}

VOID InserisciFile( PSZ pszParent, FILEFINDBUF4* pFindBuffer )
{
   PUSERRECFILES pRecFiles;
   PCHAR pChar;

   pRecFiles = WinSendMsg(hwndContFiles,
                          CM_ALLOCRECORD,
                          MPFROMLONG(cbRecordDataFiles), MPFROMSHORT(1));

   memset(pRecFiles,0,cbRecordDataFiles);

   memset(pRecFiles->szFullName,0,sizeof(pRecFiles->szFullName));
   sprintf(pRecFiles->szFullName,"%s\\%s",pszParent,pFindBuffer->achName);

   //pRecFiles->hptr=WinLoadFileIcon(pRecFiles->szFullName,FALSE);
   pRecFiles->hptr=hptrSysFile;

   memset(pRecFiles->szNomeFile,0,sizeof(pRecFiles->szNomeFile));
   strcpy(pRecFiles->szNomeFile,pFindBuffer->achName);

   pRecFiles->ulattr = pFindBuffer->attrFile;
   if (pFindBuffer->cbList>4) {
     listEAs(pRecFiles->szFullName, ".LONGNAME", pRecFiles->szNomeFile);
     if (listEAs(pRecFiles->szFullName, ".ICON", NULL)) pRecFiles->ulattr |= FILE_DIR_HAVE_ICON;
   }

   CaseName(pRecFiles->szNomeFile);
   pRecFiles->pszNomeFile=pRecFiles->szNomeFile;

   memset(pRecFiles->type,0,sizeof(pRecFiles->type));
   if (!(pFindBuffer->attrFile & FILE_DIRECTORY)) {
     pChar=strchr(pFindBuffer->achName,'.');
     if (pChar && strlen(pChar)<=SIZE_EXT) {
       strcpy(pRecFiles->type,CaseName(pChar+1));
     }
   }
   pRecFiles->ptype=pRecFiles->type;

   pRecFiles->date.day     = pFindBuffer->fdateLastWrite.day  ;
   pRecFiles->date.month   = pFindBuffer->fdateLastWrite.month;
   pRecFiles->date.year    = pFindBuffer->fdateLastWrite.year + 1980;
   pRecFiles->time.seconds = pFindBuffer->ftimeLastWrite.twosecs;
   pRecFiles->time.minutes = pFindBuffer->ftimeLastWrite.minutes;
   pRecFiles->time.hours   = pFindBuffer->ftimeLastWrite.hours  ;
   pRecFiles->size         = pFindBuffer->cbFile;

   memset(pRecFiles->attr,0,sizeof(pRecFiles->attr));
   if (pRecFiles->ulattr & FILE_SYSTEM)   strcat(pRecFiles->attr,"S");
   if (pRecFiles->ulattr & FILE_ARCHIVED) strcat(pRecFiles->attr,"A");
   if (pRecFiles->ulattr & FILE_READONLY) strcat(pRecFiles->attr,"R");
   if (pRecFiles->ulattr & FILE_HIDDEN)   strcat(pRecFiles->attr,"H");
   pRecFiles->pattr=pRecFiles->attr;
   pRecFiles->vuoto=0;
   pRecFiles->pvuoto=&pRecFiles->vuoto;

   pRecFiles->recordCore.cb = sizeof(RECORDCORE);
   pRecFiles->recordCore.flRecordAttr = CRA_DROPONABLE;
   pRecFiles->recordCore.pszIcon = pRecFiles->szNomeFile;
   pRecFiles->recordCore.pszText = pRecFiles->szNomeFile;
   pRecFiles->recordCore.pszName = pRecFiles->szNomeFile;
   pRecFiles->recordCore.pszTree = pRecFiles->szNomeFile;
   pRecFiles->recordCore.preccNextRecord=NULLHANDLE;

   pRecFiles->recordCore.hptrIcon  = pRecFiles->hptr;

   recordInsert.cb = sizeof(RECORDINSERT);
   recordInsert.pRecordParent     = NULL;
   recordInsert.pRecordOrder      = (PRECORDCORE)CMA_END;
   recordInsert.zOrder            = CMA_TOP;
   recordInsert.cRecordsInsert    = 1;
   recordInsert.fInvalidateRecord = FALSE;
   WinSendMsg(hwndContFiles,
              CM_INSERTRECORD,
              (PRECORDCORE)pRecFiles, &recordInsert);

   if (!primavolta) return;
   primavolta=FALSE;

   {
      PFIELDINFO pFieldInfo, firstFieldInfo;
      FIELDINFOINSERT fieldInfoInsert;
      pFieldInfo = WinSendMsg(hwndContFiles,
                              CM_ALLOCDETAILFIELDINFO,
                              MPFROMLONG(9), NULL);

      firstFieldInfo = pFieldInfo;

      pFieldInfo->cb = sizeof(FIELDINFO);
      //pFieldInfo->flData = CFA_BITMAPORICON | CFA_LEFT | CFA_VCENTER | CFA_FIREADONLY;
      pFieldInfo->flData = CFA_OWNER | CFA_CENTER | CFA_VCENTER | CFA_FIREADONLY;
      pFieldInfo->flTitle = CFA_CENTER | CFA_BOTTOM;
      pFieldInfo->pTitleData = "";
      pFieldInfo->offStruct = FIELDOFFSET(USERRECFILES,hptr);
      pFieldInfo = pFieldInfo->pNextFieldInfo;

      pFieldInfoText=pFieldInfo;
      pFieldInfo->cb = sizeof(FIELDINFO);
      pFieldInfo->flData = CFA_STRING | CFA_LEFT | CFA_VCENTER;
      pFieldInfo->flTitle = CFA_LEFT | CFA_BOTTOM;
      pFieldInfo->pTitleData = "Nome";
      pFieldInfo->offStruct = FIELDOFFSET(USERRECFILES,pszNomeFile);
      pFieldInfo = pFieldInfo->pNextFieldInfo;

      pFieldInfo->cb = sizeof(FIELDINFO);
      pFieldInfo->flData = CFA_ULONG | CFA_RIGHT | CFA_VCENTER | CFA_FIREADONLY;
      pFieldInfo->flTitle = CFA_RIGHT | CFA_BOTTOM;
      pFieldInfo->pTitleData = "Dimensione";
      pFieldInfo->offStruct = FIELDOFFSET(USERRECFILES,size);
      pFieldInfo = pFieldInfo->pNextFieldInfo;

      pFieldInfo->cb = sizeof(FIELDINFO);
      pFieldInfo->flData = CFA_STRING | CFA_LEFT | CFA_VCENTER | CFA_FIREADONLY;
      pFieldInfo->flTitle = CFA_LEFT | CFA_BOTTOM;
      pFieldInfo->pTitleData = "Tipo";
      pFieldInfo->offStruct = FIELDOFFSET(USERRECFILES,ptype);
      pFieldInfo = pFieldInfo->pNextFieldInfo;

      pFieldInfo->cb = sizeof(FIELDINFO);
      pFieldInfo->flData = CFA_DATE | CFA_RIGHT | CFA_VCENTER | CFA_FIREADONLY;
      pFieldInfo->flTitle = CFA_RIGHT | CFA_BOTTOM;
      pFieldInfo->pTitleData =  "Data";
      pFieldInfo->offStruct = FIELDOFFSET(USERRECFILES,date);
      pFieldInfo = pFieldInfo->pNextFieldInfo;

      pFieldInfo->cb = sizeof(FIELDINFO);
      pFieldInfo->flData = CFA_TIME | CFA_RIGHT | CFA_VCENTER | CFA_FIREADONLY;
      pFieldInfo->flTitle = CFA_RIGHT | CFA_BOTTOM;
      pFieldInfo->pTitleData = (PVOID) "Ora";
      pFieldInfo->offStruct = FIELDOFFSET(USERRECFILES,time);
      pFieldInfo = pFieldInfo->pNextFieldInfo;

      pFieldInfo->cb = sizeof(FIELDINFO);
      pFieldInfo->flData = CFA_STRING | CFA_CENTER | CFA_VCENTER | CFA_FIREADONLY;
      pFieldInfo->flTitle = CFA_CENTER | CFA_BOTTOM;
      pFieldInfo->pTitleData = (PVOID) "Attributi";
      pFieldInfo->offStruct = FIELDOFFSET(USERRECFILES,pattr);
      pFieldInfo = pFieldInfo->pNextFieldInfo;

      pFieldInfo->cb = sizeof(FIELDINFO);
      pFieldInfo->flData = CFA_STRING | CFA_CENTER | CFA_VCENTER | CFA_FIREADONLY;
      pFieldInfo->flTitle = CFA_CENTER | CFA_BOTTOM;
      pFieldInfo->pTitleData = (PVOID)"";
      pFieldInfo->offStruct = FIELDOFFSET(USERRECFILES,pvuoto);
      pFieldInfo = pFieldInfo->pNextFieldInfo;

      pFieldInfo->pNextFieldInfo = NULLHANDLE;

      fieldInfoInsert.cb = (ULONG)(sizeof(FIELDINFOINSERT));
      fieldInfoInsert.pFieldInfoOrder = (PFIELDINFO)CMA_FIRST;
      fieldInfoInsert.cFieldInfoInsert = 8;
      fieldInfoInsert.fInvalidateFieldInfo = FALSE;

      WinSendMsg(hwndContFiles,
                 CM_INSERTDETAILFIELDINFO,
                 MPFROMP(firstFieldInfo),
                 MPFROMP(&fieldInfoInsert));

      WinSendMsg(hwndContFiles,
                 CM_INVALIDATEDETAILFIELDINFO,
                 0L, 0L );
   }
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

VOID ImpostazioniSalva( )
{
  ULONG AttrFound=0;
  ULONG ColorValue=0;
  CHAR  fontNameSize[FACESIZE];
  SWP   swp;
  HINI  hini;
  CSOJ2I0H_POS pos;

  hini=PrfOpenProfile(hab,PRF_HINIFILE);

  // hwndFrame
  WinQueryWindowPos(hwndFrame,&swp);
  PrfWriteProfileData(hini, PRF_APPL, PRF_FRAME_POS, &swp, sizeof(swp));

  // hwndContTree
  memset(fontNameSize,0,sizeof(fontNameSize));
  if (WinQueryPresParam(hwndContTree, PP_FONTNAMESIZE, 0, &AttrFound, sizeof(fontNameSize), &fontNameSize, QPF_NOINHERIT) > 0)
     PrfWriteProfileString(hini, PRF_APPL, PRF_TREE_FONT, fontNameSize);
  if (WinQueryPresParam(hwndContTree, PP_FOREGROUNDCOLOR, 0, &AttrFound, sizeof(ColorValue), &ColorValue, QPF_PURERGBCOLOR | QPF_NOINHERIT) > 0)
     PrfWriteProfileData(hini, PRF_APPL, PRF_TREE_COLORFORE, &ColorValue, sizeof(ColorValue));
  if (WinQueryPresParam(hwndContTree, PP_BACKGROUNDCOLOR, 0, &AttrFound, sizeof(ColorValue), &ColorValue, QPF_PURERGBCOLOR | QPF_NOINHERIT) > 0)
     PrfWriteProfileData(hini, PRF_APPL, PRF_TREE_COLORBACK, &ColorValue, sizeof(ColorValue));

  // hwndContFile
  pos.hini=hini;
  pos.appl=PRF_APPL;
  pos.key=PRF_BTN_POS;
  WinSendMsg(hwndContFiles,WM_CSOJ2I0H_STOREPOS,MPFROMP(&pos),0L);

  memset(fontNameSize,0,sizeof(fontNameSize));
  if (WinQueryPresParam(hwndContFiles, PP_FONTNAMESIZE, 0, &AttrFound, sizeof(fontNameSize), &fontNameSize, QPF_NOINHERIT) > 0)
     PrfWriteProfileString(hini, PRF_APPL, PRF_FILE_FONT, fontNameSize);
  if (WinQueryPresParam(hwndContFiles, PP_FOREGROUNDCOLOR, 0, &AttrFound, sizeof(ColorValue), &ColorValue, QPF_PURERGBCOLOR | QPF_NOINHERIT) > 0)
     PrfWriteProfileData(hini, PRF_APPL, PRF_FILE_COLORFORE, &ColorValue, sizeof(ColorValue));
  if (WinQueryPresParam(hwndContFiles, PP_BACKGROUNDCOLOR, 0, &AttrFound, sizeof(ColorValue), &ColorValue, QPF_PURERGBCOLOR | QPF_NOINHERIT) > 0)
     PrfWriteProfileData(hini, PRF_APPL, PRF_FILE_COLORBACK, &ColorValue, sizeof(ColorValue));

  memset(fontNameSize,0,sizeof(fontNameSize));
  if (WinQueryPresParam(hwndPreviewTxt, PP_FONTNAMESIZE, 0, &AttrFound, sizeof(fontNameSize), &fontNameSize, QPF_NOINHERIT) > 0)
     PrfWriteProfileString(hini, PRF_APPL, PRF_PREV_FONT, fontNameSize);
  if (WinQueryPresParam(hwndPreviewTxt, PP_FOREGROUNDCOLOR, 0, &AttrFound, sizeof(ColorValue), &ColorValue, QPF_PURERGBCOLOR | QPF_NOINHERIT) > 0)
     PrfWriteProfileData(hini, PRF_APPL, PRF_PREV_COLORFORE, &ColorValue, sizeof(ColorValue));
  if (WinQueryPresParam(hwndPreviewTxt, PP_BACKGROUNDCOLOR, 0, &AttrFound, sizeof(ColorValue), &ColorValue, QPF_PURERGBCOLOR | QPF_NOINHERIT) > 0)
     PrfWriteProfileData(hini, PRF_APPL, PRF_PREV_COLORBACK, &ColorValue, sizeof(ColorValue));

  // hwndBarraSepX
  WinQueryWindowPos(hwndBarraSepX,&swp);
  PrfWriteProfileData(hini, PRF_APPL, PRF_POSSEPX, &swp.x, sizeof(swp.x));

  // hwndBarraSepY
  WinQueryWindowPos(hwndBarraSepY,&swp);
  PrfWriteProfileData(hini, PRF_APPL, PRF_POSSEPY, &swp.y, sizeof(swp.y));
  PrfWriteProfileData(hini, PRF_APPL, PRF_FLAGSEPY, &fSepY, sizeof(fSepY));

  // hwndDefaultView
  idDefaultView=idViewSchiacciato;
  PrfWriteProfileData(hini, PRF_APPL, PRF_DEF_VIEW, &idDefaultView, sizeof(idDefaultView));
  PrfWriteProfileData(hini, PRF_APPL, PRF_TIPOPREVIEW, &tipoPreviewBmp, sizeof(tipoPreviewBmp));

  PrfCloseProfile(hini);
  return;
}

VOID ImpostazioniLeggi( )
{
  ULONG sizeData=0;
  HINI  hini;

  hini=PrfOpenProfile(hab,PRF_HINIFILE);

  sizeData=sizeof(posSepX);
  if (!PrfQueryProfileData(hini, PRF_APPL, PRF_POSSEPX, &posSepX, &sizeData ))
     posSepX=170;

  sizeData=sizeof(posSepY);
  if (!PrfQueryProfileData(hini, PRF_APPL, PRF_POSSEPY, &posSepY, &sizeData ))
     posSepY=150;

  sizeData=sizeof(fSepY);
  if (!PrfQueryProfileData(hini, PRF_APPL, PRF_FLAGSEPY, &fSepY, &sizeData ))
     fSepY=TRUE;

  sizeData=sizeof(tipoPreviewBmp);
  if (!PrfQueryProfileData(hini, PRF_APPL, PRF_TIPOPREVIEW, &tipoPreviewBmp, &sizeData ))
     tipoPreviewBmp=WS_CSOJ2I0A_THUMBNAIL;

  sizeData=sizeof(filemask);
  if (!PrfQueryProfileData(hini, PRF_APPL, PRF_FILE_MASK, &filemask, &sizeData ))
     filemask=FILE_ARCHIVED | FILE_ARCHIVED | FILE_READONLY | FILE_HIDDEN | FILE_SYSTEM;

  sizeData=sizeof(idDefaultView);
  if (!PrfQueryProfileData(hini, PRF_APPL, PRF_DEF_VIEW, &idDefaultView, &sizeData ))
     idDefaultView=ID_DETAIL;

  PrfCloseProfile(hini);
  return;
}

VOID ImpostazioniRipristina( )
{
  ULONG ColorValue=0;
  ULONG sizeColorValue=0;
  CHAR  fontNameSize[FACESIZE];
  SWP   swp;
  HINI  hini;
  BOOL  fSuccess;
  CSOJ2I0H_POS pos;

  hini=PrfOpenProfile(hab,PRF_HINIFILE);

  // hwndFrame
  sizeColorValue=sizeof(swp);
  fSuccess=PrfQueryProfileData(hini, PRF_APPL, PRF_FRAME_POS, &swp, &sizeColorValue);
  if (fSuccess) {
    if (swp.fl & SWP_MAXIMIZE) {
      WinSetWindowPos(hwndFrame, NULLHANDLE, 0L, 0L, 0L, 0L, SWP_MAXIMIZE | SWP_SHOW );
    } else {
      if (swp.fl & SWP_MINIMIZE) {
        WinSetWindowPos(hwndFrame, NULLHANDLE, 0L, 0L, 0L, 0L, SWP_SHOW );
      } else {
        WinSetWindowPos(hwndFrame, NULLHANDLE, swp.x, swp.y, swp.cx, swp.cy, SWP_SHOW | SWP_SIZE | SWP_MOVE );
      }
    }
  } else {
    WinSetWindowPos(hwndFrame, NULLHANDLE, 0L, 0L, 0L, 0L, SWP_SHOW );
  }

  // hwndContTree
  memset(fontNameSize,0,sizeof(fontNameSize));
  if (PrfQueryProfileString(hini, PRF_APPL, PRF_TREE_FONT, NULL, fontNameSize, sizeof(fontNameSize) ))
     WinSetPresParam(hwndContTree, PP_FONTNAMESIZE, sizeof(fontNameSize), fontNameSize);
  sizeColorValue=sizeof(ColorValue);
  if (PrfQueryProfileData(hini, PRF_APPL, PRF_TREE_COLORFORE, &ColorValue, &sizeColorValue ))
     WinSetPresParam(hwndContTree, PP_FOREGROUNDCOLOR, sizeof(ColorValue), &ColorValue);
  sizeColorValue=sizeof(ColorValue);
  if (PrfQueryProfileData(hini, PRF_APPL, PRF_TREE_COLORBACK, &ColorValue, &sizeColorValue ))
     WinSetPresParam(hwndContTree, PP_BACKGROUNDCOLOR, sizeof(ColorValue), &ColorValue);

  // hwndContFile
  pos.hini=hini;
  pos.appl=PRF_APPL;
  pos.key=PRF_BTN_POS;
  WinSendMsg(hwndContFiles,WM_CSOJ2I0H_RESTOREPOS,MPFROMP(&pos),0L);

  memset(fontNameSize,0,sizeof(fontNameSize));
  if (PrfQueryProfileString(hini, PRF_APPL, PRF_FILE_FONT, NULL, fontNameSize, sizeof(fontNameSize) ))
     WinSetPresParam(hwndContFiles, PP_FONTNAMESIZE, sizeof(fontNameSize), fontNameSize);
  sizeColorValue=sizeof(ColorValue);
  if (PrfQueryProfileData(hini, PRF_APPL, PRF_FILE_COLORFORE, &ColorValue, &sizeColorValue ))
     WinSetPresParam(hwndContFiles, PP_FOREGROUNDCOLOR, sizeof(ColorValue), &ColorValue);
  sizeColorValue=sizeof(ColorValue);
  if (PrfQueryProfileData(hini, PRF_APPL, PRF_FILE_COLORBACK, &ColorValue, &sizeColorValue ))
     WinSetPresParam(hwndContFiles, PP_BACKGROUNDCOLOR, sizeof(ColorValue), &ColorValue);

  // hwndPreviewBmp
  memset(fontNameSize,0,sizeof(fontNameSize));
  if (PrfQueryProfileString(hini, PRF_APPL, PRF_PREV_FONT, NULL, fontNameSize, sizeof(fontNameSize) )) {
     WinSetPresParam(hwndPreviewBmp, PP_FONTNAMESIZE, sizeof(fontNameSize), fontNameSize);
     WinSetPresParam(hwndPreviewTxt, PP_FONTNAMESIZE, sizeof(fontNameSize), fontNameSize);
  }
  sizeColorValue=sizeof(ColorValue);
  if (PrfQueryProfileData(hini, PRF_APPL, PRF_PREV_COLORFORE, &ColorValue, &sizeColorValue )) {
     WinSetPresParam(hwndPreviewBmp, PP_FOREGROUNDCOLOR, sizeof(ColorValue), &ColorValue);
     WinSetPresParam(hwndPreviewTxt, PP_FOREGROUNDCOLOR, sizeof(ColorValue), &ColorValue);
  }
  sizeColorValue=sizeof(ColorValue);
  if (PrfQueryProfileData(hini, PRF_APPL, PRF_PREV_COLORBACK, &ColorValue, &sizeColorValue )) {
     WinSetPresParam(hwndPreviewBmp, PP_BACKGROUNDCOLOR, sizeof(ColorValue), &ColorValue);
     WinSetPresParam(hwndPreviewTxt, PP_BACKGROUNDCOLOR, sizeof(ColorValue), &ColorValue);
  }
  PrfCloseProfile(hini);
  return;
}

BOOL VaiADirectory( PSZ pszDir, BOOL fPosiziona )
{
   CHAR  chDisk;
   PUSERRECTREE pRec;

   WinSetPointer(HWND_DESKTOP,WinQuerySysPointer(HWND_DESKTOP, SPTR_WAIT, FALSE));

   chDisk=WinUpperChar(hab,0,0,*pszDir)-65;

   if (chDisk<0 || chDisk>MAX_DISK) goto stop;
   if (pRecDisk[chDisk]==NULLHANDLE) goto stop;

   pRec=SearchDirectory(pszDir,pRecDisk[chDisk]);
   WinSendMsg(hwndContTree,
              CM_SETRECORDEMPHASIS,
              MPFROMP( pRec ),
              MPFROM2SHORT(TRUE,CRA_SELECTED));

   if (fPosiziona) {
     QUERYRECORDRECT queryRecordRect;
     RECTL rectlRec,rectlCont;
     LONG spostamento=0;
     WinQueryWindowRect(hwndContTree,&rectlCont);
     memset(&queryRecordRect,0,sizeof(queryRecordRect));
     queryRecordRect.cb=sizeof(queryRecordRect);
     queryRecordRect.pRecord=(PRECORDCORE)pRec;
     queryRecordRect.fsExtent=CMA_TEXT;
     WinSendMsg(hwndContTree,
                CM_QUERYRECORDRECT,
                MPFROMP( &rectlRec ), MPFROMP( &queryRecordRect ));
     spostamento = (LONG) - ( (rectlRec.yTop + 15) - (rectlCont.yTop - rectlCont.yBottom));
     WinSendMsg(hwndContTree,
                CM_SCROLLWINDOW,
                MPFROMSHORT( CMA_VERTICAL ), MPFROMLONG( spostamento ));
   }

stop:
   WinSetPointer(HWND_DESKTOP,WinQuerySysPointer(HWND_DESKTOP, SPTR_ARROW, FALSE));
   return(TRUE);
}

PUSERRECTREE SearchDirectory( PSZ baseDir, PUSERRECTREE pRecPadre )
{
   PUSERRECTREE pChild;
   WinSendMsg(hwndContTree, CM_EXPANDTREE, MPFROMP( pRecPadre ), 0L);
   pChild=(PUSERRECTREE)WinSendMsg(hwndContTree,
                                   CM_QUERYRECORD,
                                   MPFROMP(pRecPadre),
                                   MPFROM2SHORT( CMA_FIRSTCHILD , CMA_ITEMORDER ));
   while (pChild) {
      if (stricmp(baseDir,pChild->szFullName)==0) return(pChild);
      if ((strnicmp(baseDir,pChild->szFullName,strlen(pChild->szFullName))==0) &&
          (*(baseDir+strlen(pChild->szFullName))=='\\') ) {
         return(SearchDirectory(baseDir,pChild));
      }
      pChild=(PUSERRECTREE)WinSendMsg(hwndContTree,
                                      CM_QUERYRECORD,
                                      MPFROMP(pChild),
                                      MPFROM2SHORT( CMA_NEXT, CMA_ITEMORDER ));
   }
   return(pRecPadre);
}

SHORT pfnCompareTree( PRECORDCORE pTrg, PRECORDCORE pSrc, PVOID pStorage )
{
  return( strcmp( (PCHAR)((PUSERRECTREE)pTrg)->szTitle,
                  (PCHAR)((PUSERRECTREE)pSrc)->szTitle ) );
  return(0);
}

SHORT pfnCompareFiles( PRECORDCORE pTrg, PRECORDCORE pSrc, PVOID pStorage )
{
  if (   (((PUSERRECFILES)pSrc)->ulattr & FILE_DIRECTORY) &&
       ! (((PUSERRECFILES)pTrg)->ulattr & FILE_DIRECTORY) ) return 1;
  if ( ! (((PUSERRECFILES)pSrc)->ulattr & FILE_DIRECTORY) &&
         (((PUSERRECFILES)pTrg)->ulattr & FILE_DIRECTORY) ) return -1;
  return( strcmp( (PCHAR)((PUSERRECFILES)pTrg)->szNomeFile,
                  (PCHAR)((PUSERRECFILES)pSrc)->szNomeFile ) );
  return(0);
}

MRESULT EXPENTRY BarraMenuProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
   switch(msg)
   {
      case WM_COMMAND:
         return(ProcessCommand(hwnd,SHORT1FROMMP(mp1)));
      break;

      case WM_CONTROL:
         switch ( SHORT2FROMMP( mp1 ) ) {
            case BN_PAINT:
               DrawButton( MPFROMP(mp2) );
            break;

            case CBN_ENTER:
               if (SHORT1FROMMP(mp1)==ID_COMBO_DRIVE) {
                  CHAR szString[3];
                  SHORT index;
                  index=(SHORT)WinSendMsg(hwndComboDrive,LM_QUERYSELECTION,MPFROMSHORT(LIT_FIRST),0L);
                  memset(szString,0,sizeof(szString));
                  sprintf(szString,"%c:",pComboTab[index]);
                  VaiADirectory(szString,TRUE);
               }
            break;
         }
      break;

      case WM_MEASUREITEM:
         {
            HPS   hpsTest;
            SHORT usSizeText;
            ULONG ulSizeMiniIcon;
            RECTL rc;
            PCHAR pChar;

            usSizeText=(SHORT)WinSendMsg(hwndComboDrive, LM_QUERYITEMTEXTLENGTH, mp2, 0L);
            pChar=malloc(usSizeText+1);
            memset(pChar,0,usSizeText+1);
            WinSendMsg(hwndComboDrive,
                       LM_QUERYITEMTEXT,
                       MPFROM2SHORT(SHORT1FROMMP(mp2),usSizeText),
                       MPFROMP(pChar));

            hpsTest=WinGetPS(hwndComboDrive);
            rc.xLeft=0;
            rc.xRight=1;
            rc.yBottom=0;
            rc.yTop=1;
            WinDrawText(hpsTest,-1L, pChar, &rc, 0L,0L, DT_LEFT | DT_TOP | DT_QUERYEXTENT );
            WinReleasePS(hpsTest);
            free(pChar);

            ulSizeMiniIcon = WinQuerySysValue( HWND_DESKTOP, SV_CXICON ) / 2;
            return MRFROM2SHORT( ulSizeMiniIcon+2, ulSizeMiniIcon+(rc.xRight-rc.xLeft)+6 );
         }
      break;

      case WM_DRAWITEM:
         {
            POWNERITEM poiItem;
            SHORT usWidthText;
            static SHORT usBaseText=0;
            PCHAR pChar;

            poiItem=(POWNERITEM)MPFROMP(mp2);
            poiItem->rclItem.xLeft--;
            poiItem->rclItem.xRight++;

            WinFillRect(poiItem->hps, &poiItem->rclItem,
                        poiItem->fsState ? CLR_DARKGRAY : CLR_BACKGROUND );

            if (poiItem->idItem!=LIT_NONE) {
               usWidthText=(SHORT)WinSendMsg(poiItem->hwnd,
                          LM_QUERYITEMTEXTLENGTH,
                          MPFROMSHORT(poiItem->idItem),
                          0L);

               pChar=malloc(usWidthText+1);
               memset(pChar,0,usWidthText+1);

               WinSendMsg(poiItem->hwnd,
                          LM_QUERYITEMTEXT,
                          MPFROM2SHORT(poiItem->idItem,usWidthText),
                          MPFROMP(pChar));

               poiItem->rclItem.xLeft+=4;
               poiItem->rclItem.yBottom++;
               poiItem->rclItem.xRight-=2;
               WinDrawPointer( poiItem->hps, poiItem->rclItem.xLeft, poiItem->rclItem.yBottom,
                               pRecDisk[pComboTab[poiItem->idItem]-'A']->recordCore.hptrIcon, DP_MINI );
               poiItem->rclItem.xLeft+=(WinQuerySysValue(HWND_DESKTOP, SV_CXICON)/2)+2;

               if (usBaseText==0) {
                  RECTL rc;
                  rc.xLeft=0;
                  rc.xRight=1;
                  rc.yBottom=0;
                  rc.yTop=1;
                  WinDrawText(poiItem->hps, -1L, pChar, &rc, 0L,0L, DT_LEFT | DT_TOP | DT_QUERYEXTENT );
                  usBaseText = ((poiItem->rclItem.yTop - poiItem->rclItem.yBottom) - (rc.yTop - rc.yBottom)) / 2;
               }

               poiItem->rclItem.yBottom+=usBaseText;

               WinDrawText(poiItem->hps,
                           -1,
                           pChar,
                           &poiItem->rclItem,
                           poiItem->fsState ? CLR_WHITE    : CLR_BLACK,
                           poiItem->fsState ? CLR_DARKGRAY : CLR_BACKGROUND,
                           DT_LEFT | DT_BOTTOM);

               free(pChar);
            }
            poiItem->fsState=FALSE;
            poiItem->fsStateOld=FALSE;

            return MRFROMSHORT(TRUE);
         }
      break;
   }
   return(WinDefDlgProc(hwnd, msg, mp1, mp2));
}

MRESULT EXPENTRY BarraStatoProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
   return(WinDefDlgProc(hwnd, msg, mp1, mp2));
}

MRESULT EXPENTRY ClientProc( HWND hwnd, ULONG  msg, MPARAM mp1, MPARAM mp2 )
{
   switch( msg )
   {
      case WM_DRAWITEM:
         switch (SHORT1FROMMP(mp1)) {
            case ID_CONT_TREE:
               {
                  POWNERITEM pItem;
                  PCNRDRAWITEMINFO pInfo;
                  pItem=PVOIDFROMMP(mp2);
                  switch (pItem->idItem) {
                    case CMA_ICON:
                       pInfo=(PCNRDRAWITEMINFO)pItem->hItem;
                       if ( (pInfo->pRecord->flRecordAttr & CRA_SELECTED) &&
                          ( ((PUSERRECTREE)pInfo->pRecord)->ulattr & FILE_DIRECTORY) &&
                          !(((PUSERRECTREE)pInfo->pRecord)->ulattr & FILE_DIR_HAVE_ICON) ) {
                         WinDrawPointer( pItem->hps , pItem->rclItem.xLeft-4, pItem->rclItem.yBottom-1, hptrFolderO, DP_MINI);
                       } else {
                         WinDrawPointer( pItem->hps , pItem->rclItem.xLeft-4, pItem->rclItem.yBottom-1, pInfo->pRecord->hptrIcon, DP_MINI);
                       }
                       return(MRESULT)TRUE;
                    break;

                    case CMA_TREEICON:
                       {
                          POINTL ptl;
                          static ULONG oldBackColor=0;
                          ULONG  BackColor=0;
                          ULONG  ulPresParams;

                          pInfo=(PCNRDRAWITEMINFO)pItem->hItem;

                          if ( (WinQueryPresParam(hwndContTree, PP_BACKGROUNDCOLOR, 0, &ulPresParams, (ULONG)sizeof(BackColor), (PVOID)&BackColor, QPF_NOINHERIT) > 0)
                               && BackColor!=oldBackColor ) {
                             WinSetPresParam(hwndComboDrive, PP_BACKGROUNDCOLOR,
                                            (ULONG)sizeof(BackColor),
                                            (PVOID)&BackColor);
                             WinInvalidateRect(hwndComboDrive,NULL,TRUE);
                          }

                          pItem->rclItem.xLeft+=2;
                          pItem->rclItem.xRight-=2;
                          pItem->rclItem.yBottom+=2;
                          pItem->rclItem.yTop-=2;

                          GpiSetColor( pItem->hps, CLR_BLACK );

                          WinDrawBorder(pItem->hps, &pItem->rclItem,  1, 1, CLR_DARKGRAY, 0L, DB_PATCOPY  );
                          ptl.x=pItem->rclItem.xLeft+2;
                          ptl.y=pItem->rclItem.yBottom+4;
                          GpiMove( pItem->hps, &ptl );
                          ptl.x+=4;
                          GpiLine( pItem->hps, &ptl );

                          if (pItem->fsAttribute & CRA_COLLAPSED) {
                             ptl.x=pItem->rclItem.xLeft+4;
                             ptl.y=pItem->rclItem.yBottom+2;
                             GpiMove( pItem->hps, &ptl );
                             ptl.y+=4;
                             GpiLine( pItem->hps, &ptl );
                          }
                          return(MRESULT)TRUE;
                       }
                    break;
                    default:
                       return(MRESULT)FALSE;
                  }
               }
            break;

            case ID_CONT_FILES:
               {
                  POWNERITEM pItem;
                  PCNRDRAWITEMINFO pInfo;
                  PUSERRECFILES pRec;
                  pItem =PVOIDFROMMP(mp2);
                  pInfo =(PCNRDRAWITEMINFO)pItem->hItem;
                  pRec  =(PUSERRECFILES)pInfo->pRecord;
                  switch (pItem->idItem) {
                    case 0:
                    case CMA_ICON:
                       if (pRec->hptr==hptrSysFile) {
                         pRec->hptr=WinLoadFileIcon(pRec->szFullName,FALSE);
                         pInfo->pRecord->hptrIcon=pRec->hptr;
                       }
                    break;
                  }
                  switch (pItem->idItem) {
                    case 0:
                       if (pInfo->pRecord->flRecordAttr & CRA_SELECTED)
                          WinFillRect( pItem->hps , &pItem->rclItem, SYSCLR_HILITEBACKGROUND );
                       WinDrawPointer( pItem->hps , pItem->rclItem.xLeft, pItem->rclItem.yBottom, pInfo->pRecord->hptrIcon, DP_MINI);
                       return(MRESULT)TRUE;
                    break;
                    default:
                       return(MRESULT)FALSE;
                  }
               }
            break;
         }
      break;

      case WM_COMMAND:
         return(ProcessCommand(hwnd,SHORT1FROMMP(mp1)));
      break;

      case WM_CHAR:
         if (CHARMSG(&msg)->fs & KC_KEYUP) {
             switch (CHARMSG(&msg)->vkey) {
               case VK_BACKSPACE: ProcessCommand(hwnd,ID_PARENT); break;
               case VK_DELETE: ProcessCommand(hwnd,ID_MN_CANCELLARE); break;
             }
         }
      break;

      case WM_CONTROL:
         switch (SHORT1FROMMP(mp1)) {
            case ID_CONT_TREE:
               switch (SHORT2FROMMP(mp1)) {
                  case CN_DRAGAFTER:
                  case CN_DRAGLEAVE:
                  case CN_DRAGOVER:
                  case CN_DROP:
                  case CN_DROPNOTIFY:
                  case CN_DROPHELP:
                      DosBeep(100,100);
                  break;

                  case CN_SETFOCUS:
                     hwndLastCont=hwndContTree;
                  break;

                  case CN_EMPHASIS:
                     {
                       PNOTIFYRECORDEMPHASIS pEmph;
                       PUSERRECTREE          pRecSelected;
                       CHAR                  szDisk[3];
                       static PRECORDCORE    pOldRecord=NULLHANDLE;
                       pEmph=MPFROMP(mp2);
                       pRecSelected=(PUSERRECTREE)pEmph->pRecord;
                       if ( (pEmph->fEmphasisMask & CRA_SELECTED == CRA_SELECTED)
                             &&
                             pEmph->pRecord!=pOldRecord ) {
                          strncpy(szDisk,pRecSelected->szFullName,sizeof(szDisk));
                          InfoDisk( szDisk );
                          CaricaLista( pRecSelected->szFullName );
                       }
                       strcpy(szCurrDir,pRecSelected->szFullName);
                       WinSetWindowText( hwndTreeTop, szCurrDir );
                       pOldRecord=pEmph->pRecord;
                     }
                  break;

                  case CN_EXPANDTREE:
                     CaricaRamo( (PUSERRECTREE)MPFROMP(mp2) );
                  break;

                  case CN_ENTER:
                     {
                       PNOTIFYRECORDENTER pEnt;
                       PUSERRECTREE       pRecEnter;
                       pEnt=MPFROMP(mp2);
                       pRecEnter=(PUSERRECTREE)pEnt->pRecord;
                       if (!pRecEnter) break;
                       CSPCSTA4(hab,hwndFrame,pRecEnter->szFullName,PRF_APPL,PRF_GRAP_POS);
                     }
                  break;

                  case CN_REALLOCPSZ:
                     if (((PCNREDITDATA)mp2)->cbText>CCHMAXPATH) return(MRESULT)FALSE;
                                                            else return(MRESULT)TRUE;
                  break;

                  case CN_ENDEDIT:
                     {
                       PCNREDITDATA   pEdit;
                       PUSERRECTREE   pRecEdit;
                       CHAR           szNewName[CCHMAXPATH];
                       PCHAR          pChar;
                       pEdit=MPFROMP(mp2);
                       pRecEdit=(PUSERRECTREE)pEdit->pRecord;
                       if (!pRecEdit) break;
                       if (strlen(*pEdit->ppszText)==0) break;
                       strcpy(szNewName,pRecEdit->szFullName);
                       pChar=strrchr(szNewName,'\\');
                       if (pChar) *pChar=0;
                             else memset(szNewName,0,sizeof(szNewName));
                       strcat(szNewName,"\\");
                       strcat(szNewName,*pEdit->ppszText);
                       if (!RinominaFile(pRecEdit->szFullName,szNewName))
                          MakeTitleFromFullName( pRecEdit );
                       return(MRESULT)0;
                     }
                  break;

                  case CN_CONTEXTMENU:
                     {
                        POINTL point;
                        HWND   hwndContMenu;
                        WinQueryPointerPos(HWND_DESKTOP,&point);
                        if (PVOIDFROMMP(mp2)==NULL) {
                           hwndContMenu=hwndMenuContNew;
                        } else {
                           if (((PUSERRECFILES)mp2)->ulattr & FILE_DRIVE) {
                             hwndContMenu=hwndMenuContDrives;
                           } else {
                             hwndContMenu=hwndMenuContTrees;
                           }
                        }
                        WinPopupMenu(HWND_DESKTOP,
                                     hwnd,
                                     hwndContMenu,
                                     point.x, point.y,
                                     0, PU_HCONSTRAIN | PU_VCONSTRAIN | PU_NONE | PU_MOUSEBUTTON1 | PU_MOUSEBUTTON2 | PU_KEYBOARD);
                     }
                  break;
               }
            break;

            case ID_CONT_FILES:
               switch (SHORT2FROMMP(mp1)) {
                  case CN_DRAGAFTER:
                  case CN_DRAGLEAVE:
                  case CN_DRAGOVER:
                  case CN_DROP:
                  case CN_DROPNOTIFY:
                  case CN_DROPHELP:
                      DosBeep(100,100);
                  break;

                  case CN_SETFOCUS:
                     hwndLastCont=hwndContFiles;
                  break;

                  case CN_ENTER:
                     {
                       PNOTIFYRECORDENTER pEnt;
                       PUSERRECFILES      pRecEnter;
                       pEnt=MPFROMP(mp2);
                       pRecEnter=(PUSERRECFILES)pEnt->pRecord;
                       if (!pRecEnter) break;
                       if (pRecEnter->ulattr & FILE_DIRECTORY) {
                          strcpy(szCurrDir,pRecEnter->szFullName);
                          VaiADirectory( pRecEnter->szFullName,FALSE);
                          //WinOpenObject(WinQueryObject(pRecEnter->szFullName),OPEN_DETAILS,TRUE);
                       } else {
                          WinSetFocus(HWND_DESKTOP,HWND_DESKTOP);
                          WinOpenObject(WinQueryObject(pRecEnter->szFullName),OPEN_DEFAULT,FALSE);
                       }
                     }
                  break;

                  case CN_EMPHASIS:
                     {
                        PNOTIFYRECORDEMPHASIS pEmph;
                        PUSERRECFILES         pRecSelected;
                        pEmph=MPFROMP(mp2);
                        pRecSelected=(PUSERRECFILES)pEmph->pRecord;
                        InfoFile();
                        if ( !fSepY || posSepY<=BORDER_MIACLIENT ) break;
                        if ( ((pEmph->fEmphasisMask & CRA_SELECTED)==CRA_SELECTED)
                             &&
                             (pRecSelected==RECEMPHASIS(hwndContFiles,CMA_FIRST)) ) {
                         //if (!(pRecSelected->ulattr & FILE_DIRECTORY))
                         //   WinPostMsg(hwndPreviewBmp,WM_CSOJ2I0A_SET_FOTO,pRecSelected->szFullName,0L);
                         //else
                         //   WinPostMsg(hwndPreviewBmp,WM_CSOJ2I0A_SET_FOTO,MPFROMP(""),0L);
                         //if (!(pRecSelected->ulattr & FILE_DIRECTORY)) {
                              PreviewFile( pRecSelected, PREVIEW_FILEFROMEXT );
                         //}
                        }
                     }
                  break;

                  case CN_REALLOCPSZ:
                     if (((PCNREDITDATA)mp2)->cbText<CCHMAXPATH) return(MRESULT)TRUE;
                                                            else return(MRESULT)FALSE;
                  break;

                  case CN_ENDEDIT:
                     {
                       PCNREDITDATA   pEdit;
                       PUSERRECFILES  pRecEdit;
                       CHAR           szNewName[CCHMAXPATH];
                       PCHAR          pChar;
                       pEdit=MPFROMP(mp2);
                       pRecEdit=(PUSERRECFILES)pEdit->pRecord;
                       if (!pRecEdit) break;
                       if (strlen(*pEdit->ppszText)==0) break;
                       strcpy(szNewName,pRecEdit->szFullName);
                       pChar=strrchr(szNewName,'\\');
                       if (pChar) *pChar=0;
                             else memset(szNewName,0,sizeof(szNewName));
                       strcat(szNewName,"\\");
                       strcat(szNewName,*pEdit->ppszText);
                       if (!RinominaFile(pRecEdit->szFullName,szNewName))
                          MakeTitleFromFullName( pRecEdit );
                       return(MRESULT)0;
                     }
                  break;

                  case CN_CONTEXTMENU:
                     {
                        POINTL point;
                        HWND   hwndContMenu;
                        WinQueryPointerPos(HWND_DESKTOP,&point);
                        if (PVOIDFROMMP(mp2)==NULL) {
                           hwndContMenu=hwndMenuContNew;
                        } else {
                           if (((PUSERRECFILES)mp2)->ulattr & FILE_DIRECTORY) {
                              hwndContMenu=hwndMenuContTrees;
                           } else {
                              hwndContMenu=hwndMenuContFiles;
                           }
                        }
                        WinPopupMenu(HWND_DESKTOP,
                                     hwnd,
                                     hwndContMenu,
                                     point.x, point.y,
                                     0, PU_HCONSTRAIN | PU_VCONSTRAIN | PU_NONE | PU_MOUSEBUTTON1 | PU_MOUSEBUTTON2 | PU_KEYBOARD);
                     }
                  break;
               }
            break;

            case ID_BARRA_SEPX:
               {
                  LONG delta;
                  SWP swp,swpContTree;
                  delta=LONGFROMMP(mp2);
                  posSepX+=delta;

                  WinQueryWindowPos(hwndContTree,&swpContTree);
                  WinSetWindowPos(hwndContTree,NULLHANDLE,0L,0L,swpContTree.cx+delta,swpContTree.cy,SWP_SIZE);

                  WinQueryWindowPos(hwndContFiles,&swp);
                  WinSetWindowPos(hwndContFiles,NULLHANDLE,swp.x+delta,swp.y,swp.cx-delta,swp.cy,SWP_MOVE|SWP_SIZE);

                  WinQueryWindowPos(hwndBarraSepY,&swp);
                  WinSetWindowPos(hwndBarraSepY,NULLHANDLE,swp.x+delta,swp.y,swp.cx-delta,swp.cy,SWP_MOVE|SWP_SIZE);

                  WinSetWindowPos(hwndPreviewBmp,NULLHANDLE, swp.x+delta, BORDER_MIACLIENT,
                                                             swp.cx-delta, swp.y-BORDER_MIACLIENT, SWP_MOVE|SWP_SIZE);
                  WinSetWindowPos(hwndPreviewTxt,NULLHANDLE, swp.x+delta, BORDER_MIACLIENT,
                                                             swp.cx-delta, swp.y-BORDER_MIACLIENT, SWP_MOVE|SWP_SIZE);

                  WinQueryWindowPos(hwndTreeTop,&swp);
                  WinSetWindowPos(hwndTreeTop,NULLHANDLE,0L,0L,swp.cx+delta,swp.cy,SWP_SIZE);

                  WinQueryWindowPos(hwndFilesTop,&swp);
                  WinSetWindowPos(hwndFilesTop,NULLHANDLE,swp.x+delta,swp.y,swp.cx-delta,swp.cy,SWP_MOVE|SWP_SIZE);

                  WinQueryWindowPos(hwndTreeBottom,&swp);
                  WinSetWindowPos(hwndTreeBottom,NULLHANDLE,0L,0L,swp.cx+delta,swp.cy,SWP_SIZE);

                  WinQueryWindowPos(hwndFilesBottom,&swp);
                  WinSetWindowPos(hwndFilesBottom,NULLHANDLE,swp.x+delta,swp.y,swp.cx-delta,swp.cy,SWP_MOVE|SWP_SIZE);
               }
            break;

            case ID_BARRA_SEPY:
               {
                  LONG delta;
                  SWP swp;
                  delta=LONGFROMMP(mp2);
                  posSepY+=delta;

                  memset(&swp,0,sizeof(swp));
                  WinQueryWindowPos(hwndContFiles,&swp);
                  WinSetWindowPos(hwndContFiles,NULLHANDLE,swp.x,swp.y+delta,swp.cx,swp.cy-delta, SWP_MOVE | SWP_SIZE );
                  WinSetWindowPos(hwndPreviewBmp,NULLHANDLE,swp.x,0L,swp.cx,posSepY-BORDER_MIACLIENT,SWP_SIZE);
                  WinSetWindowPos(hwndPreviewTxt,NULLHANDLE,swp.x,0L,swp.cx,posSepY-BORDER_MIACLIENT,SWP_SIZE);
               }
            break;

            case ID_PREVIEW_TXT:
            case ID_PREVIEW_BMP:
               {
                  if (SHORT2FROMMP(mp1)==WN_CSOJ2I0K_PRESPARAM ||
                      SHORT2FROMMP(mp1)==WN_CSOJ2I0A_PRESPARAM) {
                     static BOOL fRecurse=FALSE;
                     PCSOJ2I0K_PRESPARAM pPresParam;
                     if (fRecurse) return(MRESULT)0;
                     fRecurse=TRUE;
                     pPresParam=PVOIDFROMMP(mp2);
                     WinSetPresParam( hwndPreviewBmp,
                                      pPresParam->type,
                                      pPresParam->cbBuff,
                                      pPresParam->pBuff );
                     WinSetPresParam( hwndPreviewTxt,
                                      pPresParam->type,
                                      pPresParam->cbBuff,
                                      pPresParam->pBuff );
                     fRecurse=FALSE;
                  }
               }
            break;
         }
     break;

     case WM_PAINT:
        {
           HPS hps;
           RECTL rcWin;
           POINTL ptl;
           hps = WinBeginPaint( hwnd, (HPS)NULL, &rcWin );
           //WinFillRect( hps, &rcWin, CLR_PALEGRAY );
           WinQueryWindowRect( hwnd , &rcWin );

           rcWin.yTop--;
           rcWin.xRight--;

           GpiSetColor(hps,CLR_DARKGRAY);
           ptl.x = rcWin.xLeft;
           ptl.y = rcWin.yBottom;
           GpiMove(hps, &ptl);
           ptl.y = rcWin.yTop;
           GpiLine(hps, &ptl);
           ptl.x = rcWin.xRight;
           GpiLine(hps, &ptl);
           GpiSetColor(hps,CLR_PALEGRAY);
           ptl.y = rcWin.yBottom;
           GpiLine(hps, &ptl);
           ptl.x = rcWin.xLeft;
           GpiLine(hps, &ptl);

           rcWin.yTop--;
           rcWin.yBottom++;
           rcWin.xRight--;
           rcWin.xLeft++;

           GpiSetColor(hps,CLR_BLACK);
           ptl.x = rcWin.xLeft;
           ptl.y = rcWin.yBottom;
           GpiMove(hps, &ptl);
           ptl.y = rcWin.yTop;
           GpiLine(hps, &ptl);
           ptl.x = rcWin.xRight;
           GpiLine(hps, &ptl);
           GpiSetColor(hps,CLR_WHITE);
           ptl.y = rcWin.yBottom;
           GpiLine(hps, &ptl);
           ptl.x = rcWin.xLeft;
           GpiLine(hps, &ptl);

           WinEndPaint( hps );
           return(MRESULT)0;
        }
     break;
   }
   return(WinDefWindowProc(hwnd, msg, mp1, mp2));
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
   BITMAPINFOHEADER BmpInfo;

   WinQueryWindowRect( pBtn->hwnd , &rcWin );
   if ((SHORT)WinSendMsg(pBtn->hwnd, BM_QUERYCHECK, 0L, 0L)==1)
      pBtn->fsState |= BDS_HILITED;

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
      hbm = GpiLoadBitmap(pBtn->hps,NULLHANDLE,idBmp,0,0);
      WinSetWindowULong(pBtn->hwnd,QWL_USER,(ULONG)hbm);
   }
   GpiQueryBitmapParameters(hbm,&BmpInfo);

   if (pBtn->fsState & BDS_HILITED) {
     rcBmp.yTop--;
     rcBmp.xRight++;
     rcBmp.yBottom--;
     rcBmp.xLeft++;
   }
   rcBmp.xLeft=rcBmp.xLeft+(rcBmp.xRight-rcBmp.xLeft-BmpInfo.cx)/2;
   rcBmp.yBottom=rcBmp.yBottom+(rcBmp.yTop-rcBmp.yBottom-BmpInfo.cy)/2;

   if (pBtn->fsState & BDS_DISABLED) {
      GpiSetLineType( pBtn->hps, LINETYPE_DOT );
      WinDrawBitmap(pBtn->hps, hbm, (PRECTL)NULL, (PPOINTL)&rcBmp, 0L, 0L, DBM_IMAGEATTRS | DBM_HALFTONE );
   } else {
      WinDrawBitmap(pBtn->hps, hbm, (PRECTL)NULL, (PPOINTL)&rcBmp, 0L, 0L, DBM_NORMAL );
   }

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

/* From EDM2 *****************************************************************/
const char* printAsciiValue(const char* pBase, char* buffout)
{
   USHORT length = *(USHORT*)pBase;
   static char buffer[1024];
   strncpy(buffer, pBase+sizeof(USHORT), length);
   buffer[length] = 0;
   strncpy(buffout,buffer,length);
   return pBase+length+sizeof(USHORT);
}

const char* printMVMTValue(const char* pBase, char* buffout)
{
   USHORT length = *(USHORT*)pBase+1;
   const char* pp;
   ULONG count;
   pp = pBase + 2*sizeof(USHORT);
   for (count=0; pp && count < length; ++count) {
     pp = printEA(pp,buffout);
   }
   return pp;
}

const char* printEA(const char* pBase, char* buffout)
{
  USHORT* pType = (USHORT*)(pBase);
  switch (*pType) {
  case EAT_ASCII:
     return printAsciiValue(pBase + sizeof(USHORT),buffout);
     break;
  case EAT_ICON:
     break;
  case EAT_BITMAP:
     break;
  case EAT_METAFILE:
     break;
  case EAT_MVMT:
     return printMVMTValue(pBase + sizeof(USHORT),buffout);
     break;
  case EAT_MVST:
     break;
  case EAT_BINARY:
     break;
  case EAT_EA:
     break;
  case EAT_ASN1:
     break;
  default:
          ;
  }
  return 0;
}

BOOL listEAs(const char* name, const char* type, char* longname)
{
   const unsigned size = 2000; // Incidently, if you make size > 65364
                               // bytes, and the file does not exist,
                               // a *LARGE* portion of your memory,
                               // starting at the buffer pointer,
                               // will be zeroed, destroying a lot
                               // of your data. Try it!!!
   char* pBuffer;
   APIRET rc;
   ULONG count = -1; // means, fill as many names as there is room for
   PDENA2 pDena;
   ULONG offset = 0;
   const ULONG eaop2size = 65000;
   PGEA2 pGea2;
   ULONG length;
   PEAOP2 pEAOP2;

   DosAllocMem((PVOID)&pBuffer, size, PAG_COMMIT | PAG_READ | PAG_WRITE);

   rc = DosEnumAttribute(ENUMEA_REFTYPE_PATH,
                         (char*)name,
                         1, // ordinal index of first EA to read,
                         (char*)pBuffer,
                         size,
                         &count,
                         ENUMEA_LEVEL_NO_VALUE); // only legal value!!
  if (rc) {
     DosFreeMem(pBuffer);
     return(FALSE);
  }
  pDena = (PDENA2)pBuffer;
  DosAllocMem((PVOID)&pEAOP2, eaop2size, PAG_COMMIT | PAG_READ | PAG_WRITE);
  pEAOP2->fpGEA2List = (PGEA2LIST)((CHAR*)pEAOP2 + sizeof(EAOP2));
                                                    // let's hope it's allocated
  pGea2 = (&(pEAOP2->fpGEA2List->list[0]));            // on a double word address!!
  if (count != 0) {
    do {
       pDena = (PDENA2)((CHAR*)(pDena) + offset);
       strcpy(pGea2->szName, pDena->szName);
       pGea2->cbName = pDena->cbName;
       offset = pDena->oNextEntryOffset;
       length = pGea2->cbName + sizeof(pGea2->cbName) + sizeof(pGea2->oNextEntryOffset);
       if (length % 4) {
          length+= 4-(length%4); // calc double word alignment
       }
       pGea2->oNextEntryOffset = offset ? length : 0;
       pGea2 = (PGEA2)((CHAR*)pGea2 + length);
    } while ( pDena->oNextEntryOffset != 0 ); /* enddo */
    pEAOP2->fpGEA2List->cbList = ((char*)pGea2 - (char*)(pEAOP2->fpGEA2List));
    pEAOP2->fpFEA2List = (PFEA2LIST)((CHAR*)pEAOP2->fpGEA2List + pEAOP2->fpGEA2List->cbList);
    pEAOP2->fpFEA2List->cbList = eaop2size - ((CHAR*)pEAOP2->fpFEA2List - (CHAR*)pEAOP2);
    rc = DosQueryPathInfo((PVOID)name,
                          FIL_QUERYEASFROMLIST,
                          (PVOID)pEAOP2,
                          sizeof(EAOP2));
    if (rc) {
       DosFreeMem(pEAOP2);
       DosFreeMem(pBuffer);
       return(FALSE);
    }
    if (pEAOP2->fpFEA2List->cbList) {
       ULONG offset = 0;
       char* pEABase;
       PFEA2 pFEA2 = pEAOP2->fpFEA2List->list;
       do {
         pFEA2 = (PFEA2)((CHAR*)pFEA2 + offset);
         pEABase = (CHAR*)pFEA2->szName + pFEA2->cbName+1;
         if (strcmp(type,(CHAR*)pFEA2->szName)==0) {
            if (longname!=NULL) printEA(pEABase,longname);
            DosFreeMem(pEAOP2);
            DosFreeMem(pBuffer);
            return(TRUE);
         }
         offset = pFEA2->oNextEntryOffset;
       } while ( offset );
    }
  }
  DosFreeMem(pEAOP2);
  DosFreeMem(pBuffer);
  return(FALSE);
}
/*****************************************************************************/

VOID timedebug( PSZ stringa, BOOL flag )
{
  CPUUTIL CPUUtil={0};
  double cpu1,cpu2;
  static double t1=0,t2=0;
  FILE *fp;

  DosPerfSysCall(CMD_KI_RDCNT,(ULONG) &CPUUtil,0,0);
  cpu1 = CPUUtil.ulTimeHigh*4294967296.0;
  cpu2 = CPUUtil.ulTimeLow;
  t2 = cpu1 + cpu2;
  fp=fopen("debug.log","a");
  if (flag) {
    fprintf(fp,"%s inizio conteggio\n",stringa);
  } else {
    fprintf(fp,"%s %12.0f\n",stringa,(double)((t2-t1)/1000.0));
  }
  fclose(fp);
  t1 = t2;
  return;
}

MRESULT ProcessCommand( HWND hwnd, USHORT command )
{
  switch ( command ) {

    case ID_EXIT:
       WinSendMsg(hwnd,WM_CLOSE,0L,0L);
    break;

    case ID_PARENT:
       {
         CHAR  *chr;
         chr=strrchr(szCurrDir,'\\');
         if (chr) {
            *chr=0;
            VaiADirectory( szCurrDir, FALSE );
         }
       }
    break;

    case ID_OPTIONS:
       {
          COMMAREA cm;
          memset(&cm,0,sizeof(COMMAREA));
          cm.hab=hab;
          cm.hwndParent=hwnd;
          cm.hwndContTree=hwndContTree;
          cm.hwndContFiles=hwndContFiles;
          cm.hwndPreviewBmp=hwndPreviewBmp;
          NFMSETT( &cm );
       }
     //WinSendDlgItemMsg(hwndBarraMenu,ID_OPTIONS,BM_SETHILITE,MPFROMSHORT(TRUE),0L);
    break;

    case ID_SEL_ALL:
       SelectContainer( hwndContFiles, SELECT_ALL );
    break;

    case ID_SEL_INVERT:
       SelectContainer( hwndContFiles, SELECT_INVERT );
    break;

    case ID_VAIA:
       WinDlgBox( HWND_DESKTOP, hwnd, (PFNWP)VaiAProc, (HMODULE)0, ID_VAIA_DIAG, NULL);
    break;

    case ID_PREVIEW:
       if (fSepY) {
          SWP swp;
          fSepY=FALSE;
          WinSendDlgItemMsg(hwndBarraMenu,ID_PREVIEW,BM_SETHILITE,MPFROMSHORT(FALSE),0L);
          WinShowWindow( hwndPreviewBmp, FALSE );
          WinShowWindow( hwndPreviewTxt, FALSE );
          WinShowWindow( hwndBarraSepY, FALSE );
          WinQueryWindowPos(hwndContFiles,&swp);
          WinSetWindowPos(hwndContFiles,NULLHANDLE,swp.x,BORDER_MIACLIENT,swp.cx,swpMiaClient.cy-(BORDER_MIACLIENT*2),SWP_MOVE|SWP_SIZE);
          WinSendMsg(hwndMenu, MM_SETITEMATTR, MPFROM2SHORT(ID_PREVIEW, TRUE), MPFROM2SHORT(MIA_CHECKED, FALSE));
          WinSendMsg(hwndMenuContNew, MM_SETITEMATTR, MPFROM2SHORT(ID_PREVIEW, TRUE), MPFROM2SHORT(MIA_CHECKED, FALSE));
       } else {
          SWP swp;
          PUSERRECFILES pRecSelected;
          fSepY=TRUE;
          WinSendDlgItemMsg(hwndBarraMenu,ID_PREVIEW,BM_SETHILITE,MPFROMSHORT(TRUE),0L);
          WinShowWindow( hwndBarraSepY, TRUE );
          WinQueryWindowPos(hwndContFiles,&swp);
          WinSetWindowPos(hwndContFiles,NULLHANDLE,swp.x,posSepY+SIZE_BARRA_SEP,swp.cx,swpMiaClient.cy-posSepY-SIZE_BARRA_SEP-BORDER_MIACLIENT,SWP_MOVE|SWP_SIZE);
          pRecSelected=RECEMPHASIS(hwndContFiles,CMA_FIRST);
          if (pRecSelected) PreviewFile( pRecSelected, PREVIEW_FORCEFILEFROMEXT );
                       else PreviewFile( NULL, PREVIEW_FORCENULLTEXT );
          WinSendMsg(hwndMenu, MM_SETITEMATTR, MPFROM2SHORT(ID_PREVIEW, TRUE), MPFROM2SHORT(MIA_CHECKED, MIA_CHECKED));
          WinSendMsg(hwndMenuContNew, MM_SETITEMATTR, MPFROM2SHORT(ID_PREVIEW, TRUE), MPFROM2SHORT(MIA_CHECKED, MIA_CHECKED));
       }
    break;

    case ID_TIPO_PREVIEW:
       switch (fView) {
       case 1:
            switch (tipoPreviewBmp) {
              case WS_CSOJ2I0A_ORIGINAL :
                 WinSendMsg(hwndPreviewBmp,WM_COMMAND,MPFROMSHORT(ID_CSOJ2I0A_BTN_ORIGINALE),0L);
                 tipoPreviewBmp=WS_CSOJ2I0A_THUMBNAIL;
              break;
              case WS_CSOJ2I0A_STRETCH  :
                 WinSendMsg(hwndPreviewBmp,WM_COMMAND,MPFROMSHORT(ID_CSOJ2I0A_BTN_STRETCH),0L);
                 tipoPreviewBmp=WS_CSOJ2I0A_ORIGINAL;
              break;
              default:
                 WinSendMsg(hwndPreviewBmp,WM_COMMAND,MPFROMSHORT(ID_CSOJ2I0A_BTN_THUMBNAIL),0L);
                 tipoPreviewBmp=WS_CSOJ2I0A_STRETCH;
            }
          break;

       case 2:
            switch (tipoPreviewTxt) {
              case TRUE:
                 WinSendMsg(hwndPreviewTxt,WM_CSOJ2I0K_SET_VIS,MPFROMSHORT(FALSE),0L);
                 tipoPreviewTxt=FALSE;
              break;
              case FALSE:
                 WinSendMsg(hwndPreviewTxt,WM_CSOJ2I0K_SET_VIS,MPFROMSHORT(TRUE),0L);
                 tipoPreviewTxt=TRUE;
            }
          break;
       }
    break;

    case ID_ICON:
    case ID_LIST:
    case ID_CONTENT:
    case ID_DETAIL:
       {
          if (command==idViewSchiacciato)
             return(MRESULT)0;

          //WinSendDlgItemMsg(hwndBarraMenu,command,BM_SETHILITE,MPFROMSHORT(TRUE),0L);
          WinSendDlgItemMsg(hwndBarraMenu,command,BM_SETCHECK,MPFROMSHORT(1),0L);
          WinSendMsg(hwndMenu, MM_SETITEMATTR, MPFROM2SHORT(command, TRUE), MPFROM2SHORT(MIA_CHECKED, MIA_CHECKED));
          WinSendMsg(hwndMenuContNew, MM_SETITEMATTR, MPFROM2SHORT(command, TRUE), MPFROM2SHORT(MIA_CHECKED, MIA_CHECKED));
          //WinSendDlgItemMsg(hwndBarraMenu,idViewSchiacciato,BM_SETHILITE,MPFROMSHORT(FALSE),0L);
          WinSendDlgItemMsg(hwndBarraMenu,idViewSchiacciato,BM_SETCHECK,MPFROMSHORT(0),0L);
          WinSendMsg(hwndMenu, MM_SETITEMATTR, MPFROM2SHORT(idViewSchiacciato, TRUE), MPFROM2SHORT(MIA_CHECKED, FALSE));
          WinSendMsg(hwndMenuContNew, MM_SETITEMATTR, MPFROM2SHORT(idViewSchiacciato, TRUE), MPFROM2SHORT(MIA_CHECKED, FALSE));
          idViewSchiacciato=command;

          switch ( command ) {
            case ID_ICON:
               SetContFiles( hwndContFiles, CV_NAME | CV_FLOW );
               //WinSendMsg(hwndContFiles,CM_ARRANGE,MPFROMLONG(CMA_ARRANGEGRID),MPFROMLONG(CMA_TOP));
               //WinSendMsg(hwndContFiles,CM_ARRANGE,0L,0L);
            break;

            case ID_LIST:
               SetContFiles( hwndContFiles, CV_NAME | CV_FLOW | CV_MINI );
               //WinSendMsg(hwndContFiles,CM_ARRANGE,MPFROMLONG(CMA_ARRANGEGRID),MPFROMLONG(CMA_TOP));
               //WinSendMsg(hwndContFiles,CM_ARRANGE,0L,0L);
            break;

            case ID_CONTENT:
               SetContFiles( hwndContFiles, CV_TEXT | CV_FLOW );
               //WinSendMsg(hwndContFiles,CM_ARRANGE,MPFROMLONG(CMA_ARRANGEGRID),MPFROMLONG(CMA_TOP));
               //WinSendMsg(hwndContFiles,CM_ARRANGE,0L,0L);
            break;

            case ID_DETAIL:
               SetContFiles( hwndContFiles, CV_DETAIL | CV_MINI | CA_DRAWICON );
            break;
          }
       }
    break;

    case ID_RELOAD:
       ;
    break;

    case ID_MN_MODIFICARE:
       {
          PRECORDCORE pRecSelected;
          pRecSelected=RECEMPHASIS(hwndLastCont,CMA_FIRST);
          while (pRecSelected) {
             WinSetFocus(HWND_DESKTOP,HWND_DESKTOP);
             ExecExtPgm( "EPM.EXE", ((PUSERRECTREE)pRecSelected)->szFullName);
             pRecSelected=RECEMPHASIS(hwndLastCont,pRecSelected);
          }
       }
    break;

    case ID_MN_APRI:
    case ID_MN_IMPOSTAZIONI:
    case ID_MN_APRIICONE:
    case ID_MN_APRITREE:
    case ID_MN_APRIDETT:
       {
          PRECORDCORE pRecSelected;
          ULONG       open;
          switch (command) {
             case ID_MN_APRI        : open=OPEN_DEFAULT;   break;
             case ID_MN_IMPOSTAZIONI: open=OPEN_SETTINGS;  break;
             case ID_MN_APRIICONE   : open=OPEN_CONTENTS;  break;
             case ID_MN_APRITREE    : open=OPEN_TREE;      break;
             case ID_MN_APRIDETT    : open=OPEN_DETAILS;   break;
          }
          pRecSelected=RECEMPHASIS(hwndLastCont,CMA_FIRST);
          while (pRecSelected) {
             WinSetFocus(HWND_DESKTOP,HWND_DESKTOP);
             WinOpenObject(WinQueryObject(((PUSERRECTREE)pRecSelected)->szFullName),open,TRUE);
             pRecSelected=RECEMPHASIS(hwndLastCont,pRecSelected);
          }
       }
    break;

    case ID_MN_CANCELLARE:
       {
          PRECORDCORE  pRecSelected,pLastRecSelected;
          PRECORDCORE  *ppRecSelected,*ppLastRecSelected,*ppFirstRecSelected;
          ULONG        numEl=0,i;
          CHAR         szString[CCHMAXPATH+255];
          BOOL         fDirectory=FALSE;
          HWND         hwndMsg;

          pRecSelected=RECEMPHASIS(hwndLastCont,CMA_FIRST);
          pLastRecSelected=pRecSelected;
          while (pRecSelected) {
             numEl++;
             pLastRecSelected=pRecSelected;
             pRecSelected=RECEMPHASIS(hwndLastCont,pRecSelected);
          }
          if (numEl==0) return(MRESULT)0;
          if (numEl>1) {
             sprintf(szString,"Si conferma la cancellazione dei %d oggetti selezionati ?",numEl);
          } else {
             sprintf(szString,"Si conferma la cancellazione di \"%s\" ?",((PUSERRECTREE)pLastRecSelected)->szTitle);
          }
          if (WinMessageBox(HWND_DESKTOP,hwndFrame,
                            szString, "Nfm",
                            0L, MB_APPLMODAL | MB_MOVEABLE | MB_QUERY | MB_YESNO)!=MBID_YES)
              return(MRESULT)0;

          DosAllocMem((PVOID)&ppFirstRecSelected,sizeof(PRECORDCORE)*numEl,PAG_COMMIT | PAG_READ | PAG_WRITE);
          ppRecSelected=ppLastRecSelected=ppFirstRecSelected;
          *ppRecSelected=RECEMPHASIS(hwndLastCont,CMA_FIRST);
          while (*ppRecSelected) {
            ppLastRecSelected=ppRecSelected;
            ppRecSelected++;
            *ppRecSelected=RECEMPHASIS(hwndLastCont,*ppLastRecSelected);
          }
          ppRecSelected=ppFirstRecSelected;

          hwndMsg = WinLoadDlg( HWND_DESKTOP, hwndFrame, (PFNWP)MsgProc, (HMODULE)0, ID_OPER, NULL);
          for (i=0;i<numEl;i++) {
            if (CancellaFile(((PUSERRECTREE)*ppRecSelected)->szFullName,WinWindowFromID(hwndMsg,ID_OPER_MSG))==0) {
               if (((PUSERRECTREE)*ppRecSelected)->ulattr & FILE_DIRECTORY) fDirectory=TRUE;
               WinSendMsg(hwndLastCont, CM_REMOVERECORD, MPFROMP(ppRecSelected), MPFROM2SHORT(1, CMA_FREE ));
            } else {
               break;
            }
            ppRecSelected++;
          }
          WinDismissDlg( hwndMsg, TRUE );

          DosFreeMem(ppFirstRecSelected);
          WinSendMsg(hwndLastCont, CM_INVALIDATERECORD, 0L, MPFROM2SHORT(0, CMA_REPOSITION ));

          if (hwndLastCont==hwndContFiles && fDirectory) {
             pRecSelected=RECEMPHASIS(hwndContTree,CMA_FIRST);
             CaricaRamo((PUSERRECTREE)pRecSelected);
          }
       }
    break;

    case ID_MN_RINOMINARE:
       {
          PRECORDCORE  pRecSelected;
          CNREDITDATA  Edit;
          PSZ          pszString;
          pRecSelected=RECEMPHASIS(hwndLastCont,CMA_FIRST);
          if (pRecSelected) {
             memset(&Edit,0,sizeof(CNREDITDATA));
             Edit.cb        = sizeof(CNREDITDATA);
             Edit.hwndCnr   = hwndLastCont;
             Edit.pRecord   = pRecSelected;
             pszString=((PUSERRECTREE)pRecSelected)->szTitle;
             Edit.ppszText  = &pszString;
             Edit.cbText    = strlen(*Edit.ppszText);
             if (hwndLastCont==hwndContFiles) {
                if (idViewSchiacciato==ID_DETAIL) {
                   Edit.pFieldInfo = pFieldInfoText;
                   Edit.id         = CID_LEFTDVWND;
                } else {
                   Edit.pFieldInfo = NULL;
                   Edit.id         = ID_CONT_FILES;
                }
             } else {
                Edit.pFieldInfo = NULL;
                Edit.id         = ID_CONT_TREE;
             }
             WinSendMsg(hwndLastCont, CM_OPENEDIT, MPFROMP(&Edit), 0L);
          }
       }
    break;

    case ID_MN_CONTROLLARE:
    case ID_MN_FORMATTARE:
    case ID_MN_PARTIZIONARE:
       {
          PUSERRECTREE pRecSelected;
          pRecSelected=RECEMPHASIS(hwndContTree,CMA_FIRST);
          switch ( command ) {
            case ID_MN_CONTROLLARE:  ExecExtPgm( "PMCHKDSK.EXE", pRecSelected->szFullName ); break;
            case ID_MN_FORMATTARE:   ExecExtPgm( "PMFORMAT.EXE", pRecSelected->szFullName ); break;
            case ID_MN_PARTIZIONARE: ExecExtPgm( "FDISKPM.EXE",  pRecSelected->szFullName ); break;
          }
       }
    break;

    case ID_MN_VISGRAFICA:
       CSPCSTA4(hab,hwndFrame,szCurrDir,PRF_APPL,PRF_GRAP_POS);
    break;

    case ID_SW_BARRA_MENU:
       GestBarraMenu( swpBarraMenu.cx>0 ? FALSE : TRUE );
    break;

    case ID_SW_BARRA_STATO:
       GestBarraStato( swpBarraStato.cx>0 ? FALSE : TRUE );
    break;

    case ID_MN_NEWCOLL:
       {
           FILEDLG fdg;
           memset(&fdg, 0, sizeof(FILEDLG));
           fdg.cbSize = sizeof(FILEDLG);

           fdg.pszTitle = "Nuovo collegamento";
           fdg.pszOKButton = "Crea";
           fdg.ulUser = 0L;
           fdg.fl = FDS_CENTER | FDS_INCLUDE_EAS | FDS_FILTERUNION |
                    FDS_OPEN_DIALOG;
           strcpy( fdg.szFullFile, "C:\\" );
           if ( WinFileDlg ( HWND_DESKTOP,
                             hwndFrame,
                             (PFILEDLG)&fdg )) {
              if (fdg.lReturn == DID_OK) {
                 if (WinCreateShadow( WinQueryObject( fdg.szFullFile ),
                                      WinQueryObject( szCurrDir ), 0 )==NULLHANDLE) {
                    ;
                 }
              }
           }
       }
    break;

    case ID_MN_SPOSTARE:
    case ID_MN_COPIARE:
    case ID_MN_COPIACOLLEGATA:
    case ID_MN_NEWFOLD:
    case ID_MN_ANNULLA:
    case ID_MN_TAGLIA:
    case ID_MN_COPIA:
    case ID_MN_INCOLLA:
       WinMessageBox(HWND_DESKTOP,hwndFrame,"Funzione non implementata","Errore",0L,MB_APPLMODAL | MB_MOVEABLE | MB_ERROR | MB_OK);
    break;

  }
  return(MRESULT)0;
}

APIRET CancellaFile( PSZ szOrFile, HWND hwndMsg )
{
  HDIR          hdirFindHandle = HDIR_CREATE;
  FILEFINDBUF3  FindBuffer     = {0};
  ULONG         ulFindCount;
  APIRET        rc             = 0;
  CHAR          szPath[CCHMAXPATH];
  CHAR          szFile[CCHMAXPATH];
  PCHAR         pChar=NULL;

  memset(&FindBuffer,0,sizeof(FILEFINDBUF3));
  ulFindCount = 1;

  memset(szPath,0,CCHMAXPATH);
  strcpy(szPath,szOrFile);
  pChar=strrchr(szPath,'\\');
  if (pChar) *pChar=0;

  rc = DosFindFirst( szOrFile,
                     &hdirFindHandle,
                     FILE_DIRECTORY | FILE_ARCHIVED | FILE_READONLY | FILE_HIDDEN | FILE_SYSTEM,
                     &FindBuffer,
                     sizeof(FILEFINDBUF3),
                     &ulFindCount,
                     FIL_STANDARD);
  while (rc==0) {
     if (strcmp(FindBuffer.achName,".") !=0 &&
         strcmp(FindBuffer.achName,"..") !=0 ) {

        memset(szFile,0,CCHMAXPATH);
        sprintf(szFile,"%s\\%s",szPath,FindBuffer.achName);

        if (FindBuffer.attrFile & FILE_DIRECTORY) rc=DosDeleteDir(szFile);
                                             else rc=DosDelete   (szFile);
        if (rc == ERROR_ACCESS_DENIED) {
           rc = TogliAttrFile( szFile );
           if (rc == NO_ERROR) {
              if (FindBuffer.attrFile & FILE_DIRECTORY) rc=DosDeleteDir(szFile);
                                                   else rc=DosDelete(szFile);
           }
        }

        if (rc == ERROR_ACCESS_DENIED &&
           (FindBuffer.attrFile & FILE_DIRECTORY)) {
           PCHAR pszNewPath=NULL;
           DosAllocMem((PVOID)&pszNewPath, CCHMAXPATH, PAG_COMMIT | PAG_READ | PAG_WRITE);
           memset(pszNewPath,0,CCHMAXPATH);
           sprintf(pszNewPath,"%s\\%s\\*.*",szPath,FindBuffer.achName);
           rc = CancellaFile( pszNewPath, hwndMsg );
           DosFreeMem(pszNewPath);
           if (rc != NO_ERROR) {
              DosFindClose(hdirFindHandle);
              return( rc );
           }
           rc=DosDeleteDir(szFile);
        }
        if (rc != NO_ERROR) {
            ApiretMsg( rc, szFile );
            DosFindClose(hdirFindHandle);
            return( rc );
        }
        WinSetWindowText(hwndMsg,szFile);
     }
     ulFindCount = 1;
     rc = DosFindNext(hdirFindHandle,
                      &FindBuffer,
                      sizeof(FILEFINDBUF3),
                      &ulFindCount);
  }
  DosFindClose(hdirFindHandle);
  return( NO_ERROR );
}

BOOL RinominaFile( PSZ szFile, PSZ szFileNew )
{
  APIRET rc;
  if (strcmp(szFile,szFileNew)==0) return(TRUE);
  if (strlen(szFileNew)==0) return(FALSE);
  rc=DosMove( szFile, szFileNew );
  if (rc) {
    ApiretMsg( rc , szFile );
    return(FALSE);
  } else {
    return(TRUE);
  }
//return(WinMoveObject(WinQueryObject(szFile),WinQueryObject(szFileNew),0L));
}

CHAR * FormatNum( ULONG num, ULONG ulsz )
{
  static CHAR szString1[30];
  static CHAR szString2[30];
  static CHAR szString3[30];
  char * pchar;
  ULONG miliardi,milioni,migliaia,unita;

  switch (ulsz) {
     case 1: pchar=szString1; break;
     case 2: pchar=szString2; break;
     case 3: pchar=szString3; break;
  }

  miliardi=num/1000000000;
  num=num-miliardi*1000000000;
  milioni=num/1000000;
  num=num-milioni*1000000;
  migliaia=num/1000;
  num=num-migliaia*1000;
  unita=num;
       if (miliardi) sprintf(pchar,"%d%s%03d%s%03d%s%03d",miliardi,CtryInfo.szThousandsSeparator,milioni,CtryInfo.szThousandsSeparator,migliaia,CtryInfo.szThousandsSeparator,unita);
  else if (milioni)  sprintf(pchar,"%d%s%03d%s%03d",milioni,CtryInfo.szThousandsSeparator,migliaia,CtryInfo.szThousandsSeparator,unita);
  else if (migliaia) sprintf(pchar,"%d%s%03d",migliaia,CtryInfo.szThousandsSeparator,unita);
  else if (unita)    sprintf(pchar,"%d",unita);
  else sprintf(pchar,"0");
  return(pchar);
}

VOID InfoDisk( PSZ szDir )
{
  FSALLOCATE  aFSAlloc;
  FSINFO      aFSInfo;
  CHAR        newdisk=0;
  static CHAR olddisk=0;
  char        szString[50];
  ULONG       ulDriveNumber;
  APIRET      rc;
  SHORT       iDrive;

  newdisk=WinUpperChar(hab,0,0,*szDir);
  if (newdisk==olddisk) return;
  olddisk=newdisk;

  ulDriveNumber=*szDir-'A';

  memset(&aFSAlloc,0,sizeof(aFSAlloc));
  rc=DosQueryFSInfo(ulDriveNumber+1, FSIL_ALLOC, (PVOID)&aFSAlloc, sizeof(aFSAlloc));
  memset(&aFSInfo,0,sizeof(aFSInfo));
  // Se il drive non Š pronto Š inutile appesantire con inutili letture
  if (rc==0) {
    DosQueryFSInfo(ulDriveNumber+1, FSIL_VOLSER, (PVOID)&aFSInfo, sizeof(aFSInfo));
  }

  if (pRecDisk[ulDriveNumber]) {
     memset(pRecDisk[ulDriveNumber]->szTitle   ,0,sizeof(pRecDisk[ulDriveNumber]->szTitle   ));
     sprintf(pRecDisk[ulDriveNumber]->szTitle,"[%c:] %s",(char)*szDir,CaseName(aFSInfo.vol.szVolLabel));
  }

  memset(szString,0,sizeof(szString));

  sprintf(szString,"Unit…  \"%c:\"  %.0f Mb  (%.0f liberi)",
          newdisk,
          (double)((aFSAlloc.cUnit * aFSAlloc.cSectorUnit * aFSAlloc.cbSector)/1048576),
          (double)((aFSAlloc.cUnitAvail * aFSAlloc.cSectorUnit * aFSAlloc.cbSector)/1048576));

  WinSetWindowText(hwndTreeBottom,szString);

  for (iDrive=0;iDrive<MAX_DISK;iDrive++) {
     if (pComboTab[iDrive]==newdisk) {
        WinSendMsg(hwndComboDrive,LM_SELECTITEM,MPFROMSHORT(iDrive),MPFROMLONG(TRUE));
        break;
     }
  }

  return;
}

VOID InfoFile( VOID )
{
  char szString[50];
  ULONG sel=0,bytes=0;
  PUSERRECFILES pRecSelected;
  pRecSelected=RECEMPHASIS(hwndContFiles,CMA_FIRST);
  while (pRecSelected) {
     sel++;
     bytes=bytes+pRecSelected->size;
     pRecSelected=RECEMPHASIS(hwndContFiles,pRecSelected);
  }
  sprintf(szString,"Selezionati %s (%s bytes)", FormatNum(sel,1), FormatNum(bytes,2));
  WinSetWindowText(hwndFilesBottom,szString);
  return;
}

VOID ExecExtPgm( PSZ pgm, PSZ param )
{
  PSZ  Envs = NULL;
  RESULTCODES ChildRC = {0};
  CHAR Args[255];
  memset(Args,0,sizeof(Args));
  strcpy(Args,pgm);
  strcpy(Args+strlen(Args)+1,param);
  DosExecPgm("", 0l, EXEC_ASYNC, Args, Envs, &ChildRC, pgm);
  return;
}

VOID ApiretMsg( APIRET rc, PSZ szMsg )
{
  PCHAR pMsg={0};
  ULONG cbMsg=256;
  PCHAR pBuffer={0};
  ULONG cbBuffer=CCHMAXPATH+cbMsg;
  PCHAR pChar;
  DosAllocMem((PVOID)&pMsg, cbMsg, PAG_COMMIT | PAG_READ | PAG_WRITE);
  DosAllocMem((PVOID)&pBuffer, cbBuffer, PAG_COMMIT | PAG_READ | PAG_WRITE);
  DosGetMessage(NULL,0,pMsg,cbMsg,rc,"OSO001.MSG",&cbMsg);
  while (TRUE) {
     pChar=strstr(pMsg,"\x0d\x0a");
     if (pChar) memcpy(pChar,"  ",2);
           else break;
  }
  memset(pBuffer,0,cbBuffer);
  sprintf(pBuffer,"\"%s\" %s",szMsg,pMsg+9);
  WinMessageBox(HWND_DESKTOP,hwndFrame,pBuffer,"Errore",0L,MB_APPLMODAL | MB_MOVEABLE | MB_ERROR | MB_OK);
  DosFreeMem(pBuffer);
  DosFreeMem(pMsg);
  return;
}

VOID MakeTitleFromFullName( PVOID pRec )
{
  PCHAR pChar;
  pChar=strrchr(((PUSERRECTREE)pRec)->szFullName,'\\');
  if (pChar) {
    strcpy(((PUSERRECTREE)pRec)->szTitle,pChar+1);
  } else {
    strcpy(((PUSERRECTREE)pRec)->szTitle,((PUSERRECTREE)pRec)->szFullName);
  }
  if (((PUSERRECTREE)pRec)->ulattr & FILE_DRIVE_CASENOTSENS) {
     CaseName(((PUSERRECTREE)pRec)->szTitle);
  }
  return;
}

MRESULT EXPENTRY  VaiAProc( HWND hwnd, ULONG  msg, MPARAM mp1, MPARAM mp2 )
{
   switch(msg)
   {
      case WM_INITDLG:
         WinSendDlgItemMsg( hwnd, ID_VAIA_EF, EM_SETTEXTLIMIT, MPFROMSHORT(CCHMAXPATH), 0L);
      break;

      case WM_COMMAND:
         switch (SHORT1FROMMP(mp1)) {
           case ID_VAIA_OK:
              {
                 CHAR szDir[CCHMAXPATH];
                 memset(szDir,0,sizeof(szDir));
                 WinQueryDlgItemText(hwnd,ID_VAIA_EF,sizeof(szDir),szDir);
                 WinDismissDlg(hwnd,TRUE);
                 VaiADirectory(szDir,TRUE);
              }
           break;
           case ID_VAIA_CANCEL:
              WinDismissDlg(hwnd,TRUE);
           break;
         }
      break;
   }
   return(WinDefDlgProc(hwnd, msg, mp1, mp2));
}

MRESULT EXPENTRY MsgProc( HWND hwnd, ULONG  msg, MPARAM mp1, MPARAM mp2 )
{
   return(WinDefDlgProc(hwnd, msg, mp1, mp2));
}

VOID SelectContainer( HWND hwndContainer, ULONG op )
{
   PRECORDCORE pRec;
   BOOL        flag;
   pRec=(PRECORDCORE)WinSendMsg(hwndContainer,
                                CM_QUERYRECORD,
                                0L,
                                MPFROM2SHORT( CMA_FIRST , CMA_ITEMORDER ));
   while (pRec) {
      switch (op) {
        case SELECT_ALL:    flag=TRUE; break;
        case SELECT_NONE:   flag=FALSE; break;
        case SELECT_INVERT: flag=!(pRec->flRecordAttr & CRA_SELECTED); break;
      }
      WinSendMsg(hwndContainer,
                 CM_SETRECORDEMPHASIS,
                 MPFROMP(pRec),
                 MPFROM2SHORT( flag, CRA_SELECTED));
      pRec=(PRECORDCORE)WinSendMsg(hwndContainer,
                                   CM_QUERYRECORD,
                                   MPFROMP(pRec),
                                   MPFROM2SHORT( CMA_NEXT, CMA_ITEMORDER ));
   }
   return;
}

APIRET TogliAttrFile( PSZ szFile )
{
   APIRET rc=0;
   FILESTATUS3 fsts3PathInfo = {{0}};
   rc = DosQueryPathInfo(szFile, FIL_STANDARD,
                        &fsts3PathInfo, sizeof(fsts3PathInfo));
   if (rc != NO_ERROR) return( rc );

   fsts3PathInfo.attrFile = FILE_NORMAL;
   rc = DosSetPathInfo(szFile,
                       FIL_STANDARD,
                       &fsts3PathInfo,
                       sizeof(fsts3PathInfo),
                       DSPI_WRTTHRU );
   if (rc != NO_ERROR) return( rc );
   return( 0L );
}

VOID PreviewFile( PUSERRECFILES pRec, ULONG flag )
{
  switch (flag) {
    case PREVIEW_FILEFROMEXT:
      if (WinSendMsg(hwndPreviewBmp,WM_CSOJ2I0A_TEST_EXT,MPFROMP(pRec->type),0L)) {
         WinPostMsg(hwndPreviewBmp,WM_CSOJ2I0A_SET_FOTO,MPFROMP(pRec->szFullName),0L);
         if (fView!=1) {
            fView=1;
            WinShowWindow(hwndPreviewTxt,FALSE);
            WinShowWindow(hwndPreviewBmp,TRUE);
         }
      } else {
         WinSendMsg(hwndPreviewTxt,WM_CSOJ2I0K_SET_TEXT,MPFROMP(pRec->szFullName),0L);
         if (fView!=2) {
            fView=2;
            WinShowWindow(hwndPreviewBmp,FALSE);
            WinShowWindow(hwndPreviewTxt,TRUE);
         }
      }
    break;

    case PREVIEW_FORCEFILEFROMEXT:
      fView=0;
      if (WinSendMsg(hwndPreviewBmp,WM_CSOJ2I0A_TEST_EXT,MPFROMP(pRec->type),0L)) {
         WinPostMsg(hwndPreviewBmp,WM_CSOJ2I0A_SET_FOTO,MPFROMP(pRec->szFullName),0L);
         if (fView!=1) {
            fView=1;
            WinShowWindow(hwndPreviewTxt,FALSE);
            WinShowWindow(hwndPreviewBmp,TRUE);
         }
      } else {
         WinSendMsg(hwndPreviewTxt,WM_CSOJ2I0K_SET_TEXT,MPFROMP(pRec->szFullName),0L);
         if (fView!=2) {
            fView=2;
            WinShowWindow(hwndPreviewBmp,FALSE);
            WinShowWindow(hwndPreviewTxt,TRUE);
         }
      }
    break;

    case PREVIEW_FORCENULLGRAPH:
      fView=1;
      WinPostMsg(hwndPreviewBmp,WM_CSOJ2I0A_SET_FOTO,MPFROMP(""),0L);
      WinShowWindow(hwndPreviewTxt,FALSE);
      WinShowWindow(hwndPreviewBmp,TRUE);
    break;

    case PREVIEW_FORCENULLTEXT:
      fView=2;
      WinSendMsg(hwndPreviewTxt,WM_CSOJ2I0K_SET_TEXT,MPFROMP(""),0L);
      WinShowWindow(hwndPreviewBmp,FALSE);
      WinShowWindow(hwndPreviewTxt,TRUE);
    break;
  }
  return;
}

VOID GestBarraMenu( BOOL opt )
{
   if (opt) {
     WinQueryWindowPos(hwndBarraMenu,&swpBarraMenu);
     WinSetWindowPos(hwndBarraMenu, NULLHANDLE,
                     0L, swpClient.cy-swpBarraMenu.cy,
                     swpClient.cx+2, swpBarraMenu.cy,
                     SWP_MOVE | SWP_SIZE | SWP_SHOW );
     WinSendMsg(hwndMenu, MM_SETITEMATTR, MPFROM2SHORT(ID_SW_BARRA_MENU, TRUE), MPFROM2SHORT(MIA_CHECKED, MIA_CHECKED));
     WinSendMsg(hwndMenuContNew, MM_SETITEMATTR, MPFROM2SHORT(ID_SW_BARRA_MENU, TRUE), MPFROM2SHORT(MIA_CHECKED, MIA_CHECKED));
   } else {
     memset(&swpBarraMenu,0,sizeof(swpBarraMenu));
     WinShowWindow( hwndBarraMenu, FALSE );
     WinSendMsg(hwndMenu, MM_SETITEMATTR, MPFROM2SHORT(ID_SW_BARRA_MENU, TRUE), MPFROM2SHORT(MIA_CHECKED, FALSE));
     WinSendMsg(hwndMenuContNew, MM_SETITEMATTR, MPFROM2SHORT(ID_SW_BARRA_MENU, TRUE), MPFROM2SHORT(MIA_CHECKED, FALSE));
   }
   WinSendMsg( hwndFrame, WM_SIZE, 0L, 0L);
   return;
}

VOID GestBarraStato( BOOL opt )
{
   if (opt) {
     WinQueryWindowPos(hwndBarraStato,&swpBarraStato);
     WinSetWindowPos(hwndBarraStato, NULLHANDLE,
                     0L, swpClient.cy-swpBarraStato.cy,
                     swpClient.cx+2, swpBarraStato.cy,
                     SWP_MOVE | SWP_SIZE | SWP_SHOW );
     WinSendMsg(hwndMenu, MM_SETITEMATTR, MPFROM2SHORT(ID_SW_BARRA_STATO, TRUE), MPFROM2SHORT(MIA_CHECKED, MIA_CHECKED));
     WinSendMsg(hwndMenuContNew, MM_SETITEMATTR, MPFROM2SHORT(ID_SW_BARRA_STATO, TRUE), MPFROM2SHORT(MIA_CHECKED, MIA_CHECKED));
   } else {
     memset(&swpBarraStato,0,sizeof(swpBarraStato));
     WinShowWindow( hwndBarraStato, FALSE );
     WinSendMsg(hwndMenu, MM_SETITEMATTR, MPFROM2SHORT(ID_SW_BARRA_STATO, TRUE), MPFROM2SHORT(MIA_CHECKED, FALSE));
     WinSendMsg(hwndMenuContNew, MM_SETITEMATTR, MPFROM2SHORT(ID_SW_BARRA_STATO, TRUE), MPFROM2SHORT(MIA_CHECKED, FALSE));
   }
   WinSendMsg( hwndFrame, WM_SIZE, 0L, 0L);
   return;
}
