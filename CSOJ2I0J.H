#define ID_CSOJ2I0J_BMP_DOWN     201
#define ID_CSOJ2I0J_BMP_RIGHT    202

/* Funzione di registrazione *************************************************/
BOOL    EXPENTRY CSOJ2I0J_REGISTRA( HAB );

/* Servizi *******************************************************************/
#define WM_CSOJ2I0J_INSERTITEM      WM_USER + 1
// Inserisce un nuovo record come da struttura allegata
//   mp1=Indirizzo di INSERTITEM
//   mp2=NULL
//   mresult=(LONG)Index Item insert (0=il primo ...)

#define WM_CSOJ2I0J_SETITEM         WM_USER + 2
// Imposta i dati con quelli passati nella struttura
//   mp1=Indirizzo di INSERTITEM
//   mp2=(LONG)Index Item
//   mresult=NULL

typedef struct {
   CHAR    szTitle[256];
   CHAR    szContent[1024];
   HBITMAP hbmp;
} CSOJ2I0J_INSERTITEM;
typedef CSOJ2I0J_INSERTITEM *PCSOJ2I0J_INSERTITEM;

/* Notifiche *****************************************************************/


/* ID Esportati **************************************************************/
#define ID_CSOJ2I0J_VSB          101
#define ID_CSOJ2I0J_AREA         102
