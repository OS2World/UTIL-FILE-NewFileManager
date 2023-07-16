BOOL EXPENTRY CSOJ2I0K_REGISTRA( HAB );

BOOL EXPENTRY CSOJ2I0K_CheckPgm( PSZ );

/*****************************************************************************/
/* Window Styles                                                             */
/*****************************************************************************/

/*****************************************************************************/
/* Messaggi privati                                                          */
/*****************************************************************************/
#define WM_CSOJ2I0K_ADD_EXT       WM_USER + 1   //mp1=ext
                                                //mp1=riga comandi
#define WM_CSOJ2I0K_SET_TEXT      WM_USER + 2   //Imposta il file di mp1
#define WM_CSOJ2I0K_SET_VIS       WM_USER + 3   //mp1=BOOL True=Hex False=Dec
#define WM_CSOJ2I0K_RELOAD        WM_USER + 4   //mp1=mp2=NULL

/*****************************************************************************/
/* Notifiche                                                                 */
/*****************************************************************************/
#define WN_CSOJ2I0K_PRESPARAM     0x0001        //In mp2 il puntatore a
                                                //CSOJ2I0K_PRESPARAM

typedef struct {
  ULONG   type;
  ULONG   cbBuff;
  PVOID   pBuff;
} CSOJ2I0K_PRESPARAM;
typedef CSOJ2I0K_PRESPARAM *PCSOJ2I0K_PRESPARAM;

/*****************************************************************************/
/* ID Interni alla classe                                                    */
/*****************************************************************************/
#define ID_CSOJ2I0K_TEXT          101
#define ID_CSOJ2I0K_INDIC         102
