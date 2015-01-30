BOOL EXPENTRY CSOJ2I0A_REGISTRA( HAB );

/*****************************************************************************/
/* Window Styles                                                             */
/*****************************************************************************/
#define  WS_CSOJ2I0A_ORIGINAL      0X00000001  //Default su tipo visualizzazione
#define  WS_CSOJ2I0A_THUMBNAIL     0x00000002
#define  WS_CSOJ2I0A_STRETCH       0x00000003
#define  WS_CSOJ2I0A_BOTTONI       0x00000010  //Se OFF bottoni assenti
#define  WS_CSOJ2I0A_MODIFICABILE  0x00000020  //Se OFF bottone open... assente

/*****************************************************************************/
/* Messaggi privati                                                          */
/*****************************************************************************/
#define WM_CSOJ2I0A_SET_FOTO      WM_USER + 1   //Imposta l'immagine di mp1
#define WM_CSOJ2I0A_BOTTONI       WM_USER + 2   //mp1=TRUE mette i bottoni
                                                //mp1=FALSE toglie i bottoni
#define WM_CSOJ2I0A_STAMPA        WM_USER + 3   //mp1=PPRINT_PARAMETERS
#define WM_CSOJ2I0A_GET_HBM       WM_USER + 4   //mresult=hBmp
#define WM_CSOJ2I0A_SET_COEFF     WM_USER + 5   //mp1=(ULONG)newCoeff (100=100%)
#define WM_CSOJ2I0A_TEST_EXT      WM_USER + 6   //mp1=Estensione es "BMP"

/*****************************************************************************/
/* Notifiche                                                                 */
/*****************************************************************************/
#define WN_CSOJ2I0A_MODIFIED     0x0001         //In mp2 la nuova immagine

#define WN_CSOJ2I0A_PRESPARAM    0x0002         //In mp2 il puntatore a
                                                //CSOJ2I0A_PRESPARAM

typedef struct {
  ULONG   type;
  ULONG   cbBuff;
  PVOID   pBuff;
} CSOJ2I0A_PRESPARAM;
typedef CSOJ2I0A_PRESPARAM *PCSOJ2I0A_PRESPARAM;

/*****************************************************************************/
/* ID Interni alla classe                                                    */
/*****************************************************************************/
#define ID_CSOJ2I0A_BITMAP        102
#define ID_CSOJ2I0A_BTN_ORIGINALE 103
#define ID_CSOJ2I0A_BTN_THUMBNAIL 104
#define ID_CSOJ2I0A_BTN_STRETCH   105
#define ID_CSOJ2I0A_SPIN          106
#define ID_CSOJ2I0A_TESTO         107
#define ID_CSOJ2I0A_BTN_FOLDER    108
#define ID_CSOJ2I0A_INDIC         109

typedef struct {
  char    string[256];
  char    doc[256];
  BOOL    fScritta;
  ULONG   border;
  ULONG   x1Src;   // Source bottom
  ULONG   y1Src;   //        left
  ULONG   x2Src;   //        top
  ULONG   y2Src;   //        left
  ULONG   x1Trg;   // Target bottom
  ULONG   y1Trg;   //        left
  ULONG   x2Trg;   //        top
  ULONG   y2Trg;   //        left
} CSOJ2I0A_PRINT_PARAMETERS;
typedef CSOJ2I0A_PRINT_PARAMETERS *PCSOJ2I0A_PRINT_PARAMETERS;

