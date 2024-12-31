#include "DataTypes.h"

#ifndef TUNES_H
#define TUNES_H


//CPU1&2 UART
#define CPU1_RX             12
#define CPU1_TX             13
#define CPU2_RX             14
#define CPU2_TX             15


//APP TUNES
#define ACTIVE_NAME_TIMEOUT  15000

//THREAD CORE ASSIGNMENTS
#define WEB_SOCKET_TX_TASK_DELAY 20
#define WEB_SOCKET_TX_TASK_PRIORITY THREAD_PRIORITY_HIGH



#endif
