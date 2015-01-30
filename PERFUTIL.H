#ifdef __cplusplus
  extern "C" {
#endif

#ifndef  PERFCALL_INCLUDED
#define  PERFCALL_INCLUDED
#endif

/*
   DosPerfSysCall Function Prototype
*/

/* The  ordinal for DosPerfSysCall (in BSEORD.H) */
/* is defined as ORD_DOS32PERFSYSCALL         */

APIRET APIENTRY DosPerfSysCall(ULONG ulCommand, ULONG ulParm1, ULONG ulParm2, ULONG ulParm3);

/***
 *
 * Software Tracing
 * ----------------
 *
 **/

#define   CMD_SOFTTRACE_LOG (0x14)

 typedef struct _HookData {
    ULONG ulLength;
    PBYTE pData;
 } HOOKDATA;
 typedef HOOKDATA * PHOOKDATA;


/***
 *
 * CPU Utilization
 * ---------------
 *
 **/

#define   CMD_KI_RDCNT    (0x63)

 typedef struct _CPUUTIL {
   ULONG ulTimeLow;     /* Low 32 bits of time stamp      */
   ULONG ulTimeHigh;    /* High 32 bits of time stamp     */
   ULONG ulIdleLow;     /* Low 32 bits of idle time       */
   ULONG ulIdleHigh;    /* High 32 bits of idle time      */
   ULONG ulBusyLow;     /* Low 32 bits of busy time       */
   ULONG ulBusyHigh;    /* High 32 bits of busy time      */
   ULONG ulIntrLow;     /* Low 32 bits of interrupt time  */
   ULONG ulIntrHigh;    /* High 32 bits of interrupt time */
  } CPUUTIL;

 typedef CPUUTIL *PCPUUTIL;


#ifdef __cplusplus
  }
#endif
