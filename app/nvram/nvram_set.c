#include <stdio.h>
#include "nvram.h"

void dump_info(NVRAM_T tmp)
{
	if(tmp.uid_len)
		printf("uid_len=%d,uid:%s\n",tmp.uid_len,tmp.uid);
	if(tmp.mac_len)
		printf("mac_len=%d,mac:%s\n",tmp.mac_len,tmp.mac);
}

void print_set_usage()
{
	printf("usage:set data to facotry partion,\n");
	printf("    nvram_set type data  -- type can be M_ACCOUNT UID,FID,KEY, MAC, X_STEPS , Y_STEPS\n");	
	printf("example:  nvram_set MAC 12:12:12:12:12:12\n");
	exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
    int len , size ;
    int offset;
	NVRAM_T info;
	int option = -1;
    FILE*f_did = NULL;
	
    if(argc != 3){
        print_set_usage();       
    }
	
	size = sizeof(NVRAM_T);
	memset(&info,0,sizeof(NVRAM_T));
	if (0==strcmp(argv[1], "UID"))
		option = OPT_UID;
    else if(0==strcmp(argv[1], "FID"))
		option = OPT_FID;
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
		print_set_usage();
	
	int fd ;
	fd = open(DEVICE,O_SYNC | O_RDWR);
	if(fd < 0){
		perror("open mtd8 error");
		return -1;
	}
    if(option == OPT_FID){
     f_did = fopen(argv[2],"r"); 
     if(!f_did){
		perror("open did file error");
        close(fd);
		return -1;
     
     }
    
    
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
	/*reset fd to first*/
	offset = 0;
	lseek(fd, offset, SEEK_SET);
	len = strlen(argv[2]);
	if( len > 1024)
	{
		printf("string uid is too big = %d!\n",len);
		close(fd);
		return -1;
	}
            char buf[512] = {0};
	
	switch(option){
		case OPT_FID:
            memset(buf,0,sizeof(buf));
            int fread_len = fread(buf,1,512,f_did);
            printf("uid len: %d",fread_len);
            if(fread_len > 0)
			info.uid_len = fread_len;		
            else
			info.uid_len = 0;		

			strncpy(info.uid,buf,info.uid_len); 
            fclose(f_did);
			break;
		case OPT_UID:
			info.uid_len = len;		
			strcpy(info.uid,argv[2]);	
			break;
		case OPT_MAC:
			info.mac_len = len;		
			strcpy(info.mac,argv[2]);	
			break;
		case OPT_X_STEPS:
			info.x_max_steps = atoi(argv[2]);
			break;
		case OPT_Y_STEPS:
			info.y_max_steps = atoi(argv[2]);
			break;
		case OPT_ACCOUNT:
            info.account_len = len;
			strcpy(info.account,argv[2]);
			break;
		case OPT_KEY:
            info.key_len = len;
			strcpy(info.key,argv[2]);
			break;
		default:
			print_set_usage();
	}
	
	if(access("/tmp/nvram_bug",F_OK)==0)
		dump_info(info);
	
	int err = write(fd,(uint8_t *)(&info),sizeof(NVRAM_T));
	if( err < 0 ){
		perror("write info to flash:");
		return -1;
	}else
		printf("Set %s success!\n",argv[1]);
	
	close(fd);
	
    return 0;

}
