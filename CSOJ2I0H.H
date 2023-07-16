/* Funzione di registrazione *************************************************/
BOOL    EXPENTRY CSOJ2I0H_REGISTRA( HAB );

/* Servizi *******************************************************************/
#define WM_CSOJ2I0H_STOREPOS     WM_USER + 1
// Salva le posizioni e dimensioni correnti in Appl Key dell'OS2.INI
//   mp1=Struct CSOJ2I0H_POS
//   mp2=NULL

#define WM_CSOJ2I0H_RESTOREPOS   WM_USER + 2
// Ripristina le posizioni e dimensioni correnti prese da APPL Key dell'OS2.INI
//   mp1=Struct CSOJ2I0H_POS
//   mp2=NULL
//   mresult=TRUE=ok FALSE=ko

typedef struct {
   HINI hini;
   PSZ  appl;
   PSZ  key;
} CSOJ2I0H_POS, *PCSOJ2I0H_POS;

#define WM_CSOJ2I0H_SORT         WM_USER + 3
// Sorta secondo la colonna indicata (0=prima colonna  1=seconda colonna...)
//   mp1=(ULONG)Numero colonna
//   mp2=(BOOL)TRUE=Descending FALSE=Ascending

#define WM_CSOJ2I0H_RESIZE       WM_USER + 4
// Dimensiona la colonna indicata (0=ALL);
//   mp1=(ULONG)Numero colonna
//   mp2=NULL

/* Notifiche *****************************************************************/


/* ID Esportati **************************************************************/
#define ID_BMP_DOWN     100
#define ID_BMP_UP       101
#define ID_BMP_RELOAD   102
#define ID_BMP_RELOAD_R 103
