typedef struct {
   HAB  hab;
   HWND hwndParent;
   HWND hwndContTree;
   HWND hwndContFiles;
   HWND hwndPreviewBmp;
} COMMAREA;
typedef COMMAREA *PCOMMAREA;

BOOL EXPENTRY NFMSETT ( PCOMMAREA );
BOOL EXPENTRY SetColor( HAB, HWND, ULONG );

#define BKS_NEWCONT          0x00000800L
#define FCF_CLOSE            0x04000000L

#define ID_SETTINGS                 100
#define ID_NOTEBOOK                 101
#define ID_SETTINGS_A               102

#define ID_SPINR                    201
#define ID_SPING                    202
#define ID_SPINB                    203
#define ID_COL                      204
#define ID_SETTINGS_COLORFONT       200
#define ID_SETTINGS_FILTER          300
#define ID_FILE_A                   301
#define ID_FILE_H                   302
#define ID_FILE_S                   303
#define ID_FILE_R                   304
#define ID_DEFAULT                  305
