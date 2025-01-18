#include "DataTypes.h"

#ifndef TUNES_H
#define TUNES_H

//CPU1&2 UART
#define CPU1_RX             12
#define CPU1_TX             13
#define CPU2_RX             14
#define CPU2_TX             15

//THREAD CORE ASSIGNMENTS
#define DATALINK_TASK_PRIORITY          THREAD_PRIORITY_HIGH

#define WEB_SOCKET_QUEUE_SIZE           50
#define WEB_SOCKET_TX_TASK_PRIORITY     THREAD_PRIORITY_HIGH
#define WEB_SOCKET_TX_TASK_DELAY        10
#define NULL_POINTER_THREAD_DELAY       100

#endif
