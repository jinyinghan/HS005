//-----------------------------------------------------
// 360Cam ????????
// ????????? QCAM ???????
// author : zhaohuaqiang@360.cn
//-----------------------------------------------------


#ifndef _QCAM_LOG_H
#define _QCAM_LOG_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <memory.h>
#include <stdarg.h>

typedef enum
{
    FATAL,
    WARN,    
    TRACE,    
    DETAIL,  
    FORCE,
    LEVEL_MAX,
}LogLevel;

static void QCamLog(LogLevel level, const char*file, const int line, const char *fmt,...)
{
	char szBuffer [4096];
	szBuffer[4095] = 0;

	va_list	ap;
	va_start(ap, fmt);
	vsnprintf( szBuffer, sizeof (szBuffer) - 1, fmt, ap) ;
	va_end(ap);

	struct timeval tv;
	struct tm now;
	int ms = 0;
	memset(&tv, 0, sizeof(tv));
	gettimeofday(&tv, NULL);
	ms = (int)(tv.tv_usec/1000);

	memset(&now, 0, sizeof(now));
	localtime_r(&tv.tv_sec, &now);

	char szOut [4096+512];
	snprintf(szOut, sizeof(szOut), "[QCAM] %02d-%02d %02d:%02d:%02d.%03u %s:%d %s\n", 
		now.tm_mon + 1, now.tm_mday, now.tm_hour, now.tm_min, now.tm_sec, ms,
		file, line, szBuffer);

	printf("%s", szOut);
}


#define QLOG(level,fmt...)	\
	do{\
		QCamLog(level,__FUNCTION__,__LINE__,##fmt);\
	}while(0)

#define HI_PRT(fmt...)  do {printf("[%s:%d]: ", __FUNCTION__, __LINE__);printf(fmt);}while(0)

#endif
