/* Funzione di registrazione *************************************************/
BOOL    EXPENTRY CSOJ2I0I_REGISTRA   ( HAB lochab );

/* Servizi *******************************************************************/
#define WM_CSOJ2I0I_SETSIZEBORDER   WM_USER + 1
// Imposta la distanza dal bordo per TrackRect
//    mp1=ULONG size
//    mp2=NULL

/* Notifiche *****************************************************************/
#define WN_CSOJ2I0I_MOVED          0x0001
//    mp2=LONG delta

/* ID Esportati **************************************************************/

