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
#define  N_TASKS                         2       /* Number of identical tasks                          */

#define  TASKSET1_SELECT

/*
*********************************************************************************************************
*                                               VARIABLES
*********************************************************************************************************
*/

OS_STK        TaskStk[N_TASKS][TASK_STK_SIZE];        /* Tasks stacks                                  */
OS_STK        TaskStartStk[TASK_STK_SIZE];
char          TaskData[N_TASKS];                      /* Parameters to pass to each task               */

INT32U        GlobalStartTime;                                /* Global tasks start time               */
// unsigned int  TaskSet1[2][2] = {{1, 3}, {3, 5}};              /* Task set 1 */
// unsigned int  TaskSet2[3][2] = {{1, 4}, {2, 5}, {2, 10}};      /* Task set 2 (Lab 2) */
unsigned int  TaskSet1[2][2] = {{1, 3}, {3, 6}};              /* Task set 1 (Lab 1) */
unsigned int  TaskSet2[3][2] = {{1, 3}, {3, 6}, {4, 9}};      /* Task set 2 (Lab 1) */

#define  DISPLAY_HIGH                   24
#define  COMPLETE_EVENT                  1
#define  PREEMPT_EVENT                   2
#define  DEADLINE_EVENT                  3

/* Defined for ISR to print message */
extern char           MessageBuffer[DISPLAY_HIGH][80];              /* User message MessageBuffer */
extern unsigned int   RowCount;                               /* ISR use RowCount to store message to OutputBuffer*/

/*
*********************************************************************************************************
*                                           FUNCTION PROTOTYPES
*********************************************************************************************************
*/

        // void  Task(void *data);                       /* Function prototypes of tasks                  */
        // void  TaskStart(void *data);                  /* Function prototypes of Startup task           */
static  void  TaskStartCreateTasks(void);
static  void  TaskStartDisp(void);
        void  PeriodicTask(void *data);

/*$PAGE*/
/*
*********************************************************************************************************
*                                                MAIN
*********************************************************************************************************
*/

void  main (void)
{
    PC_DispClrScr(DISP_FGND_WHITE + DISP_BGND_BLACK);      /* Clear the screen                         */

    OSInit();                                              /* Initialize uC/OS-II                      */

    PC_DOSSaveReturn();                                    /* Save environment to return to DOS        */
    PC_VectSet(uCOS, OSCtxSw);                             /* Install uC/OS-II's context switch vector */

    PC_ElapsedInit();                                      /* Initialized elapsed time measurement     */

    // OSTaskCreate(TaskStart, (void *)0, &TaskStartStk[TASK_STK_SIZE - 1], 0);
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
    INT8U  i;
    INT8U  nr_task = 2;

    for (i = 0; i < nr_task; i++) {                        /* Create N_TASKS identical tasks           */
        OSTaskCreate(PeriodicTask, (void *)&TaskSet1[i], &TaskStk[i][TASK_STK_SIZE - 1], i + 1);
    }
#else
    INT8U  i;
    INT8U  nr_task = 3;

    for (i = 0; i < nr_task; i++) {                        /* Create N_TASKS identical tasks           */
        OSTaskCreate(PeriodicTask, (void *)&TaskSet2[i], &TaskStk[i][TASK_STK_SIZE - 1], i + 1);
    }
#endif
}

/*
*********************************************************************************************************
*                                                  TASKS
*********************************************************************************************************
*/

void  PeriodicTask (void *pdata)
{
    int end, toDelay;
    int c = ((int *) pdata)[0]; /* The first parameter in pdata*/
    INT16S     key;

    pdata = pdata;                                         /* Prevent compiler warning                 */
    
    /* Let one task to set uC/OS-II clock */
    if (OSTCBCur->OSTCBPrio == 1) {
        OS_ENTER_CRITICAL();
        PC_VectSet(0x08, OSTickISR);                           /* Install uC/OS-II's clock tick ISR        */
        PC_SetTickRate(OS_TICKS_PER_SEC);                      /* Reprogram tick rate                      */
        OS_EXIT_CRITICAL();
    }

    /* Update the global start time */
    OSTCBCur->start = GlobalStartTime;

    while (1) {
        if (PC_GetKey(&key) == TRUE) {                     /* See if key has been pressed              */
            if (key == 0x1B) {                             /* Yes, see if it's the ESCAPE key          */
                PC_DOSReturn();                            /* Return to DOS                            */
            }
        }
        while (OSTCBCur->compTime > 0) {
            /* Check the deadline. (The tasks in Lab2 won't miss the deadline.) */
            if ((OSTimeGet() > (OSTCBCur->start + OSTCBCur->period)) && RowCount < DISPLAY_HIGH) {
                // OutputBuffer[RowCount][0] = (OSTCBCur->start + OSTCBCur->period);
                // OutputBuffer[RowCount][1] = DEADLINE_EVENT;
                // OutputBuffer[RowCount][2] = (INT16U) OSPrioCur;
                // OutputBuffer[RowCount][3] = (INT16U) OSPrioCur;
                sprintf(MessageBuffer[RowCount], "%3d Deadline %3d %3d\n", (OSTCBCur->start + OSTCBCur->period), OSPrioCur, OSPrioCur);
                RowCount++;
            }
        }

        /* Let one task to print message */
        if (OSTCBCur->OSTCBPrio == 1)
            TaskStartDisp();

        end = OSTimeGet();
        toDelay = OSTCBCur->period - (end - OSTCBCur->start);
        OSTCBCur->start += OSTCBCur->period;
        OSTCBCur->compTime = c;
 
        OSTimeDly(toDelay);
    }
}