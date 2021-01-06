#ifndef _YSX_UTILS_H_
#define _YSX_UTILS_H_


//int AMCSystemCmd (const char *format, ...);

FILE* ysx_popen(const char *pCommand, const char *pMode, pid_t *pid);

int ysx_pclose(FILE *fp, pid_t pid);

void ysx_setTimer(int seconds, int mseconds);

#endif

