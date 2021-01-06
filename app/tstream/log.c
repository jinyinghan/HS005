#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "log.h"

#define LOG_FLAG   		"/etc/SNIP39/log.conf"

static char ysx_log_path[128]={'\0'};
static int log_level = LOG_OFF;
static void log_size_check(char *src )
{
	FILE *fp = fopen(src,"r");
	if(fp)
	{
		fseek(fp,0,SEEK_END);
		int src_size = ftell(fp);
		if(src_size >= LOG_SIZE )
		{
			fseek(fp,LOG_DSIZE,SEEK_SET);
			int ret;
			char buf[1024];
			FILE *tmp = fopen("/var/log/log_tmp.txt","w");
			while(1){
				ret = fread(buf,1,sizeof(buf),fp);
				if(ret <= 0)
					break;
				fwrite(buf,1,sizeof(buf),tmp);
			}
			fclose(tmp);
			remove(src);
			rename("/var/log/log_tmp.txt",src);
			fprintf(stdout,"TRUNCATE File ## %d\n",LOG_DSIZE);
		}
		fclose(fp);
	}
}

void YSXLOG(LOGLevel level, const char *func, const int line, const char *fmt,...)
{
	char szBuffer [4096];
	szBuffer[4095] = 0;

	va_list ap;
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

	char szOut[4096];
	snprintf(szOut, sizeof(szOut), "[%04d/%02d/%02d %02d:%02d:%02d.%03u] %s:%d %s",
		now.tm_year + 1900,
		now.tm_mon + 1, 
		now.tm_mday, 
		now.tm_hour, 
		now.tm_min, 
		now.tm_sec, 
		ms,
		func, 
		line, 
		szBuffer);

	if(strlen(ysx_log_path)&&(level != LOG_LZF))
	{
		log_size_check(ysx_log_path);

		FILE *fp = fopen(ysx_log_path,"a");
		fprintf(fp,"%s", szOut);
		fclose(fp);
	}
	else
		fprintf(stdout,"%s", szOut);

}


void LOG_INIT(char *logfile)
{
	do{\
		strcpy(ysx_log_path,logfile);\
		printf("ysx_log_path[%s]\n",ysx_log_path);\
	}while(0);
}

int GetLogLevel(LOGLevel type){
	int i_ret = 0;
	switch (type){
		case LOG_DEBUG:
		{
			if(access("/tmp/app_dbg",F_OK) == 0)
				i_ret = 1;
		}break;
		case LOG_INFO:
		{
			if(access("/tmp/app_info",F_OK) == 0)
				i_ret = 1;
		}break;
		case LOG_ERROR:
		{
			if(access("/tmp/app_err",F_OK) == 0)
				i_ret = 1;
		}break;
		default:
			break;
	}
	return i_ret;
}

