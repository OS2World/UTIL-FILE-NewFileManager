/* Funzione di registrazione *************************************************/
BOOL    EXPENTRY CSPCSTA3_REGISTRA( HAB );

/* Costanti ******************************************************************/
#define CSPCSTA3_DRAW_TORTA_3D        0
#define CSPCSTA3_DRAW_BARRE_3D        1
#define CSPCSTA3_DRAW_LINEE_3D        2

/* Window Styles *************************************************************/
#define  WS_CSPCSTA3_TORTA_3D   0X00000000  //Tipo grafico (default)
#define  WS_CSPCSTA3_BARRE_3D   0X00000001
#define  WS_CSPCSTA3_LINEE_3D   0X00000002

/* Servizi *******************************************************************/
#define WM_CSPCSTA3_INSERTITEM      WM_USER + 1
// Inserisce un nuovo record come da struttura allegata
//   mp1=Indirizzo di INSERTITEM
//   mp2=NULL
//   mresult=(ULONG)id

typedef struct {
   double  dblPerc;                 /* Percentuale da inserire
                                       il totale non pu• superare 100.00 */
   CHAR    szLabel[CCHMAXPATH];     /* Label, eventualmente NULL */
} CSPCSTA3_INSERTITEM;
typedef CSPCSTA3_INSERTITEM *PCSPCSTA3_INSERTITEM;

#define WM_CSPCSTA3_AZZERA          WM_USER + 2
// Azzera la tabella

#define WM_CSPCSTA3_SETDRAW         WM_USER + 3
// Cambia il tipo di visualizzazione
//   mp1=(ULONG)Tipo di visualizzazione    CSPCSTA3_DRAW_TORTA_3D
//                                         CSPCSTA3_DRAW_BARRE_3D
//                                         CSPCSTA3_DRAW_LINEE_3D

#define WM_CSPCSTA3_QUERYLABEL      WM_USER + 4
// Chiede l'etichetta dell'ID richiesto (simile a LM_QUERYITEMTEXT)
//   mp1=(USHORT)idItem:in (USHORT)cbBuff:in
//   mp2=(PSZ)pBuff:out

/* Notifiche *****************************************************************/
#define WN_CSPCSTA3_SELECT          1
// mp2=(ULONG)id selezionato

/* ID Esportati **************************************************************/

