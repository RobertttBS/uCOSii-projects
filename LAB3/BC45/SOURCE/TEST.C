/*
*********************************************************************************************************
*                                                uC/OS-II
*                                          The Real-Time Kernel
*
*                           (c) Copyright 1992-2002, Jean J. Labrosse, Weston, FL
*                                           All Rights Reserved
*
*                                               EXAMPLE #1
*********************************************************************************************************
*/

#include "includes.h"

/*
*********************************************************************************************************
*                                               CONSTANTS
*********************************************************************************************************
*/

#define  TASK_STK_SIZE                 512       /* Size of each task's stacks (# of WORDs)            */
#define  N_TASKS                         3       /* Number of identical tasks                          */

// #define  TASKSET1_SELECT

/*
*********************************************************************************************************
*                                               VARIABLES
*********************************************************************************************************
*/

OS_STK        TaskStk[N_TASKS][TASK_STK_SIZE];        /* Tasks stacks                                  */
OS_STK        TaskStartStk[TASK_STK_SIZE];
char          TaskData[N_TASKS];                      /* Parameters to pass to each task               */
OS_EVENT     *RandomSem;

INT32U        GlobalStartTime;                                /* Global tasks start time               */

#define  DISPLAY_HIGH                   24

/* Defined for ISR to print message */
extern char           MessageBuffer[DISPLAY_HIGH][80];              /* User message MessageBuffer */
extern unsigned int   RowCount;                               /* ISR use RowCount to store message to OutputBuffer*/

OS_EVENT *R1;
OS_EVENT *R2;


/*
*********************************************************************************************************
*                                           FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static  void  TaskStartCreateTasks(void);
static  void  TaskStartDisp(void);
        void  Task1(void *data);
        void  Task2(void *data);
        void  Task3(void *data);

/*$PAGE*/
/*
*********************************************************************************************************
*                                                MAIN
*********************************************************************************************************
*/

void  main (void)
{
    INT8U err;

    PC_DispClrScr(DISP_FGND_WHITE + DISP_BGND_BLACK);      /* Clear the screen                         */
    OSInit();                                              /* Initialize uC/OS-II                      */
    PC_DOSSaveReturn();                                    /* Save environment to return to DOS        */
    PC_VectSet(uCOS, OSCtxSw);                             /* Install uC/OS-II's context switch vector */

    PC_ElapsedInit();                                      /* Initialized elapsed time measurement.    */

    /* Create resource. */
    R1 = OSMutexCreate(1, &err);
    R2 = OSMutexCreate(2, &err);

    TaskStartCreateTasks();

    OSStart();                                             /* Start multitasking                       */
}

/* To display the output */
static  void  TaskStartDisp (void)
{
    char s[80];
    static unsigned int i;

    for (i = 0; i < DISPLAY_HIGH; i++) {
        printf("%s", MessageBuffer[i]);
    }
}

/*$PAGE*/
/*
*********************************************************************************************************
*                                             CREATE TASKS
*********************************************************************************************************
*/

static  void  TaskStartCreateTasks (void)
{
#ifdef TASKSET1_SELECT
    OSTaskCreate(Task1, (void *)0, &TaskStk[0][TASK_STK_SIZE - 1], (INT8U) 3);
    OSTaskCreate(Task2, (void *)0, &TaskStk[1][TASK_STK_SIZE - 1], (INT8U) 4);
    OSTaskCreate(Task3, (void *)0, &TaskStk[2][TASK_STK_SIZE - 1], (INT8U) 5);
#else
    OSTaskCreate(Task1, (void *)0, &TaskStk[0][TASK_STK_SIZE - 1], (INT8U) 3);
    OSTaskCreate(Task2, (void *)0, &TaskStk[1][TASK_STK_SIZE - 1], (INT8U) 4);
#endif
}

/*
*********************************************************************************************************
*                                                  TASKS
*********************************************************************************************************
*/

#ifdef TASKSET1_SELECT

void  Task1 (void *pdata)
{
    INT16S     key;
    INT8U      error;

    pdata = pdata;                                         /* Prevent compiler warning                 */
    
    /* Let Task1 to set uC/OS-II clock */
    OS_ENTER_CRITICAL();
    PC_VectSet(0x08, OSTickISR);                           /* Install uC/OS-II's clock tick ISR        */
    PC_SetTickRate(OS_TICKS_PER_SEC);                      /* Reprogram tick rate                      */
    OS_EXIT_CRITICAL();

    while (1) {
        /* Delay 8 ticks at first. */
        OSTimeDly(8);

        /* Compute 2 ticks. */
        OSTCBCur->compTime = 2;
        while (OSTCBCur->compTime > 0);

        /* Acquire R1 for 2 ticks */
        OSMutexPend(R1, 0, &error);
        OSTCBCur->compTime = 2;
        while (OSTCBCur->compTime > 0);

        /* Acquire R2 for 2 ticks */
        OSMutexPend(R2, 0, &error);
        OSTCBCur->compTime = 2;
        while (OSTCBCur->compTime > 0);
        OSMutexPost(R2);
        OSMutexPost(R1);

        if (PC_GetKey(&key) == TRUE) {                     /* See if key has been pressed              */
            if (key == 0x1B) {                             /* Yes, see if it's the ESCAPE key          */
                PC_DOSReturn();                            /* Return to DOS                            */
            }
        }
 
        OSTimeDly(1000);
    }
}

void  Task2 (void *pdata)
{
    INT16S     key;
    INT8U      error;

    pdata = pdata;                                         /* Prevent compiler warning                 */

    while (1) {
        /* Delay 4 ticks at first. */
        OSTimeDly(4);

        /* Compute 2 ticks */
        OSTCBCur->compTime = 2;
        while (OSTCBCur->compTime > 0);

        /* Acquire R2 for 4 ticks */
        OSMutexPend(R2, 0, &error);
        OSTCBCur->compTime = 4;
        while (OSTCBCur->compTime > 0);
        OSMutexPost(R2);

        if (PC_GetKey(&key) == TRUE) {                     /* See if key has been pressed              */
            if (key == 0x1B) {                             /* Yes, see if it's the ESCAPE key          */
                PC_DOSReturn();                            /* Return to DOS                            */
            }
        }
 
        OSTimeDly(1000);
    }
}

void  Task3 (void *pdata)
{
    INT16S     key;
    INT8U      error;

    pdata = pdata;                                         /* Prevent compiler warning                 */

    while (1) {
        /* Compute 2 ticks */
        OSTCBCur->compTime = 2;
        while (OSTCBCur->compTime > 0);

        /* Acquire R1 for 7 ticks */
        OSMutexPend(R1, 0, &error);
        OSTCBCur->compTime = 7;
        while (OSTCBCur->compTime > 0);
        OSMutexPost(R1);

        if (PC_GetKey(&key) == TRUE) {                     /* See if key has been pressed              */
            if (key == 0x1B) {                             /* Yes, see if it's the ESCAPE key          */
                PC_DOSReturn();                            /* Return to DOS                            */
            }
        }
 
        OSTimeDly(1000);
    }
}

#else

void  Task1 (void *pdata)
{
    INT16S     key;
    INT8U      error;

    pdata = pdata;                                         /* Prevent compiler warning                 */
    
    /* Let one task to set uC/OS-II clock */
    OS_ENTER_CRITICAL();
    PC_VectSet(0x08, OSTickISR);                           /* Install uC/OS-II's clock tick ISR        */
    PC_SetTickRate(OS_TICKS_PER_SEC);                      /* Reprogram tick rate                      */
    OS_EXIT_CRITICAL();

    while (1) {
        /* Delay 5 ticks */
        OSTimeDly(5);

        /* Compute 2 ticks */
        OSTCBCur->compTime = 2;
        while (OSTCBCur->compTime > 0);

        /* Get R2 for 3 ticks */
        OSMutexPend(R2, 0, &error);
        OSTCBCur->compTime = 3;
        while (OSTCBCur->compTime > 0);
        
        /* Get R1 for 3 ticks */
        OSMutexPend(R1, 0, &error);
        OSTCBCur->compTime = 3;
        while (OSTCBCur->compTime > 0);
        OSMutexPost(R1);

        OSTCBCur->compTime = 3;
        while (OSTCBCur->compTime > 0);
        OSMutexPost(R2);

        if (PC_GetKey(&key) == TRUE) {                     /* See if key has been pressed              */
            if (key == 0x1B) {                             /* Yes, see if it's the ESCAPE key          */
                PC_DOSReturn();                            /* Return to DOS                            */
            }
        }
 
        OSTimeDly(1000);
    }
}

void  Task2 (void *pdata)
{
    INT16S     key;
    INT8U      error;

    pdata = pdata;                                         /* Prevent compiler warning                 */

    while (1) {
        /* Compute for 2 ticks */
        OSTCBCur->compTime = 2;
        while (OSTCBCur->compTime > 0);

        /* Get R1 for 6 ticks */
        OSMutexPend(R1, 0, &error);
        OSTCBCur->compTime = 6;
        while (OSTCBCur->compTime > 0);

        /* Get R2 for 2 ticks */
        OSMutexPend(R2, 0, &error);
        OSTCBCur->compTime = 2;
        while (OSTCBCur->compTime > 0);
        OSMutexPost(R2);

        OSTCBCur->compTime = 2;
        while (OSTCBCur->compTime > 0);
        OSMutexPost(R1);

        if (PC_GetKey(&key) == TRUE) {                     /* See if key has been pressed              */
            if (key == 0x1B) {                             /* Yes, see if it's the ESCAPE key          */
                PC_DOSReturn();                            /* Return to DOS                            */
            }
        }
 
        OSTimeDly(1000);
    }
}

#endif

