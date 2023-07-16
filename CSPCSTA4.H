#define _QUOTE(A)     #A
#define _MAKESTR(A)   _QUOTE(A)

BOOL CSPCSTA4( HAB, HWND, PSZ, PSZ, PSZ );
/*
 Passare:
   HAB   hab
   HWND  handParent
   PSZ   path da scandire (eventualmente C: per intero disco)
   PSZ   application per save/restore pos eventualmente NULL)
   PSZ   key per save/restore pos eventualmente NULL)
*/

#define ID_CSPCSTA4_FRAME                    100
#define ID_CSPCSTA4_TITOLI                   102
#define ID_CSPCSTA4_PERCENTUALE              103
#define ID_CSPCSTA4_GRAFICO                  104
#define ID_CSPCSTA4_DETAILS                  105
#define ID_CSPCSTA4_PERC                     106

#define ID_CSPCSTA4_BARRE                    120
#define ID_CSPCSTA4_TORTA                    121
#define ID_CSPCSTA4_LINEE                    122

#define ID_CSPCSTA4_BMP_GRAFICO1             201
#define ID_CSPCSTA4_BMP_GRAFICO2             202
#define ID_CSPCSTA4_BMP_GRAFICO3             203
