#ifndef __LOG_H_
#define __LOG_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <memory.h>
#include <stdarg.h>

#define LOG_SIZE (1024*1024*1)		//1M
#define LOG_DSIZE (64*1024)		// the size of delete every time

typedef enum
{
	LOG_FATAL,
    LOG_WARN,
    LOG_TRACE,
    LOG_DETAIL,
    LOG_FORCE,
	LOG_DEBUG = 6,		//DEBUG Level指出细粒度信息事件对调试应用程序是非常有帮助的。
	LOG_INFO,		//INFO level表明 消息在粗粒度级别上突出强调应用程序的运行过程。
	LOG_ERROR,		//ERROR level指出虽然发生错误事件，但仍然不影响系统的继续运行。
	LOG_OFF,		//OFF Level是最高等级的，用于关闭所有日志记录。
	LOG_LZF,
    LOG_LEVEL_MAX,
}LOGLevel;

void LOG_INIT(char *logfile);
void YSXLOG(LOGLevel level, const char *func, const int line, const char *fmt,...);
int GetLogLevel(LOGLevel type);

#define LOG(fmt...)	\
	do{\
		YSXLOG(LOG_DETAIL,__FUNCTION__,__LINE__,##fmt);\
	}while(0)
#define LOG_LZF(fmt...)	\
			do{\
				YSXLOG(LOG_LZF,__FUNCTION__,__LINE__,##fmt);\
}while(0)

#define avSendIOCtrl_pr_error(type, ret)	\
	if(ret < 0) {	\
		printf("[avSendIOCtrl_ERROR]");		\
		printf("FILE:%s FUNC:%s LINE:%d ",__FILE__,__FUNCTION__,__LINE__);	\
		printf("type, ret:(%d, %d)\n", type, ret);	\
	}

#define pr_error(fmt,args...)	\
		do{\
			if ((GetLogLevel(LOG_ERROR) == 1) ||	\
				(GetLogLevel(LOG_INFO) == 1) ||	\
				(GetLogLevel(LOG_DEBUG) == 1)){	\
				printf("[LOG_ERROR]");		\
				printf("FILE:%s FUNC:%s LINE:%d ",__FILE__,__FUNCTION__,__LINE__);	\
				printf(fmt, ##args);\
			}	\
			else{	\
				LOG(fmt,##args);\
			}	\
		}while(0)
#define pr_info(fmt,args...)	\
		do{\
			if ((GetLogLevel(LOG_INFO) == 1) ||	\
				(GetLogLevel(LOG_DEBUG) == 1)){ \
				printf("[LOG_INFO]");			\
				printf("FILE:%s FUNC:%s LINE:%d ",__FILE__,__FUNCTION__,__LINE__);	\
				printf(fmt, ##args);\
			}						\
		}while(0)
#define pr_dbg(fmt,args...)	\
		do{\
			if (GetLogLevel(LOG_DEBUG) == 1){	\
				printf("[LOG_DEBUG]");			\
				printf("FILE:%s FUNC:%s LINE:%d ",__FILE__,__FUNCTION__,__LINE__);	\
				printf(fmt, ##args);\
			}						\
		}while(0)




//#define EMERGENCY_DEBUG
#if defined  EMERGENCY_DEBUG
#define RED            "\033[0;32;31m"
#define LIGHT_RED      "\033[1;31m"
#define NONE           "\033[m"
#define WHITE          "\033[1;37m"

#define EMGCY_LOG(fmt...)   \
        do {\
            printf(LIGHT_RED"[%s] [%d]: "NONE, __FUNCTION__, __LINE__);\
            printf(fmt);\
        }while(0)  
#else
#define EMGCY_LOG(...)   
#endif 




		
#endif
