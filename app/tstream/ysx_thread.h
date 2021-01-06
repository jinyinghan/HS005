/*******************************************************************************
* write at SZ, 2018-10-11 11:08:16, by echo
*******************************************************************************/

#if !defined(YSX_THREAD_H)
#define YSX_THREAD_H

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <math.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/prctl.h>
#include <time.h>
#include <pthread.h>
#include <errno.h>

#include "include/common_env.h"
#include "include/common_func.h"

//#define LED_LIGHT                   1
#ifndef pr_dbg
#define pr_dbg(fmt,args...) \
    do{\
        printf("[LOG_DEBUG]");          \
        printf("FILE:%s FUNC:%s LINE:%d ",__FILE__,__FUNCTION__,__LINE__);  \
        printf(fmt, ##args);\
    }while(0)
#endif
#ifndef pr_error
#define pr_error(fmt,args...) \
    do{\
        printf("[LOG_ERROR]");          \
        printf("FILE:%s FUNC:%s LINE:%d ",__FILE__,__FUNCTION__,__LINE__);  \
        printf(fmt, ##args);\
    }while(0)
#endif
#ifndef pr_info
#define pr_info(fmt,args...) \
    do{\
        printf("[LOG_INFO]");          \
        printf("FILE:%s FUNC:%s LINE:%d ",__FILE__,__FUNCTION__,__LINE__);  \
        printf(fmt, ##args);\
    }while(0)
#endif


typedef struct {
    int p_led_ctrl_en;
}ysx_pthread_flag_s;

typedef struct {
    pthread_t id_led_ctrl;
}ysx_pthread_id_s;

int Initialization_Pthread_Led_Ctrl(void *arg);
void Uninitialization_Pthread_Led_Ctrl(void);
int internet_is_ok(void);


#endif
