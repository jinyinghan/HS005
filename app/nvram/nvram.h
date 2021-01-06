#ifndef _NVRAM_H_
#define _NVRAM_H_

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define DEVICE "/dev/mtdblock6"

typedef struct {
	int uid_len;
	char uid[512];
	int mac_len;
	char mac[20];
    char account[1024];
    int account_len;
    char key[1024];
    int  key_len;
	int x_max_steps;
	int y_max_steps;
}NVRAM_T;

enum  {
	OPT_UID,
	OPT_FID,
	OPT_MAC,
	OPT_X_STEPS,
	OPT_Y_STEPS,
	OPT_ACCOUNT,
	OPT_KEY,
};

#define SET 0
#define GET 1

extern const char *opt_str;

void print_usage(char flag);
int flash_erase(int offset, int size);
int flash_write(int offset,const char *buf,int size);
int set_uid(const char *buf);
int flash_read(int offset, char *buf,int size);
int get_uid();



#endif /* _NVRAM_H_ */

