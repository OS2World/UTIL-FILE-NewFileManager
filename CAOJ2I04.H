BOOL EXPENTRY CAOJ2I04_REGISTRA( HAB );

/*****************************************************************************/
/* Window Styles                                                             */
/*****************************************************************************/
#define  WS_CSOJ2I0A_ORIZZONTALE   0X00000000
#define  WS_CSOJ2I0A_VERTICALE     0x00000001
#define  WS_CSOJ2I0A_VIS_PERC      0x00000002 /* Inserisce la scritta nn % */

/*****************************************************************************/
/* Messaggi privati                                                          */
/*****************************************************************************/
#define WM_SETPERC        WM_USER + 1   /* Setta la percentuale               */
                                        /* mp1(ULONG)=dividendo;              */
                                        /* mp2(ULONG)=divisore, se omesso 100 */
