#include <stdio.h>
#include "nvram.h"

void dump_info(NVRAM_T tmp)
{
	// if(tmp.uid_len)
		printf("uid_len=%d,uid:%s\n",tmp.uid_len,tmp.uid);
	// if(tmp.mac_len)
		printf("mac_len=%d,mac:%s\n",tmp.mac_len,tmp.mac);
}

void print_get_usage()
{
	printf("usage:get data from facotry partion,\n");
	printf("    nvram_get type -- type can be M_ACCOUNT UID, FID,KEY, MAC, X_STEPS , Y_STEPS\n");	
	exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
    int  size ;
    int offset;
	NVRAM_T info;
	int option = -1;
	
    if(argc != 2){
        print_get_usage();       
    }
	
	size = sizeof(NVRAM_T);
	memset(&info,0,sizeof(NVRAM_T));
	if (0==strcmp(argv[1], "UID"))
		option = OPT_UID;
	else if (0==strcmp(argv[1], "MAC"))
		option = OPT_MAC;
	else if (0==strcmp(argv[1], "X_STEPS"))
		option = OPT_X_STEPS;
	else if (0==strcmp(argv[1], "Y_STEPS"))
		option = OPT_Y_STEPS;
    else if (0==strcmp(argv[1], "M_ACCOUNT"))
        option = OPT_ACCOUNT;
    else if (0==strcmp(argv[1], "KEY"))
        option = OPT_KEY;
	else
		print_get_usage();
	
	int fd ;
	fd = open(DEVICE,O_SYNC | O_RDONLY);
	if(fd < 0){
		perror("open mtd8 error");
		return -1;
	}
	
	offset = 0;
	if (offset != lseek(fd, offset, SEEK_SET)) {
		perror("lseek()");
		return -1;
	}	
	if( size != read(fd,&info,sizeof(NVRAM_T)) ){
		perror("read()");
		return -1;
	}
	
	if(access("/tmp/nvram_bug",F_OK)==0)
		dump_info(info);
	
	switch(option){
		case OPT_UID:
			if(info.uid_len < 0)
				return 0;
			info.uid[info.uid_len]='\0';
			fprintf(stdout,"%s",info.uid);
			break;
		case OPT_MAC:
			if(info.mac_len < 0)
				return 0;
			info.mac[info.mac_len]='\0';
			fprintf(stdout,"%s",info.mac);
			break;
		case OPT_X_STEPS:
			fprintf(stdout,"%d",info.x_max_steps);
			break;
		case OPT_Y_STEPS:
			fprintf(stdout,"%d",info.y_max_steps);
			break;
		case OPT_ACCOUNT:
			if(info.account_len < 0)
				return 0;
			info.account[info.account_len]='\0';
			fprintf(stdout,"%s",info.account);
			break;
		case OPT_KEY:
			if(info.key_len < 0)
				return 0;
			info.key[info.key_len]='\0';
			fprintf(stdout,"%s",info.key);
			break;
		default:
			print_get_usage();
	}
 
	close(fd);

    return 0;

}
