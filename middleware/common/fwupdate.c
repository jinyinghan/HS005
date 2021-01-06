#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
//#include <qcam_sys.h>

#define CLEAR(x) (memset(x,0,sizeof(x)))
#define FILE_UBOOT  "/tmp/uboot.bin"
#define FILE_KERNEL "/tmp/kernel.bin"
#define FILE_ROOT   "/tmp/root.bin"
#define FILE_USER   "/tmp/user.bin"
#define FILE_MTD   "/tmp/mtd.bin"

#if 0
int check_cmd(const char *cmd)
{
    char buf1[128],buf2[256],tmp[256];
    char *env;
    int ret = -1;
    char file[128];
    
    CLEAR(buf1);
    CLEAR(buf2);
    strcpy(buf2,getenv("PATH"));
    do{
        CLEAR(tmp); 
        sscanf(buf2,"%[^:]:%s",buf1,tmp);
        CLEAR(buf2);       
        strcpy(buf2,tmp);
        sprintf(file,"%s/%s",buf1,cmd);
        if(access(file,F_OK) == 0)
        {
            ret = 0;
            break;
        }
    }while(strlen(buf2));

    return ret;
}

int QCamFlashBurn(const char *firmwarePath)
{
    FILE *fp = NULL;
    uint8_t sys_cmd[128];
	uint8_t f_flag = 0;
    uint32_t count=0,offset=0;
	struct stat st;
    
    if(stat(firmwarePath,&st) == -1)
    {
        fprintf(stderr,"file %s not exit !\n",firmwarePath);
        return -1;
    }
	
	printf("file size = %d\n",st.st_size/1024);
	if((st.st_size/1024) > 16000)
		f_flag = 1;
    
	
    if(check_cmd("flash_erase") != 0)
    {
        fprintf(stderr,"cmd flash_erase not find !\n");
        return -1;
    }
    if(check_cmd("flashcp") != 0)
    {
        fprintf(stderr,"cmd flashcp not find !\n");
        return -1;
    }    
 
    /*step 1 . extract fw into kernel rootfs user*/
	if(f_flag == 1){
		
		/*step 2 . update uboot */
		CLEAR(sys_cmd);
		count  = 256;
		offset = 0;
		snprintf(sys_cmd,128,"dd if=%s of=%s bs=1024 count=%d skip=%d",firmwarePath,FILE_UBOOT,count,offset);
		system(sys_cmd);
		
		snprintf(sys_cmd,128,"flashcp %s /dev/mtd0",FILE_UBOOT);  // update uboot 
		system(sys_cmd);
		remove(FILE_UBOOT);
	}
	
	/*step 2 . update kernel */
    CLEAR(sys_cmd);
	offset += count;
    count  = 2048;
    snprintf(sys_cmd,128,"dd if=%s of=%s bs=1024 count=%d skip=%d",firmwarePath,FILE_KERNEL,count,offset);
    system(sys_cmd);
    system("flash_erase /dev/mtd1 0x1F8000 1");     // erase 'WORK' flag
    CLEAR(sys_cmd);
    snprintf(sys_cmd,128,"flashcp %s /dev/mtd1",FILE_KERNEL);  // update mtd 
    system(sys_cmd);
	remove(FILE_KERNEL);
	
	/*step 3 . update rootfs */
    CLEAR(sys_cmd);
	offset += count;	
    count  = 1792;
    snprintf(sys_cmd,128,"dd if=%s of=%s bs=1024 count=%d skip=%d",firmwarePath,FILE_ROOT,count,offset);
    system(sys_cmd);
    system("flash_erase /dev/mtd2 0x1B8000 1");     // erase 'WORK' flag
    CLEAR(sys_cmd);
    snprintf(sys_cmd,128,"flashcp %s /dev/mtd2",FILE_ROOT);  // update mtd 
    system(sys_cmd);
	remove(FILE_ROOT);
	
    /*step 4 . update usr */
    CLEAR(sys_cmd);
	offset += count;	
    count  = 3584;
    snprintf(sys_cmd,128,"dd if=%s of=%s bs=1024 count=%d skip=%d",firmwarePath,FILE_USER,count,offset);
    system(sys_cmd);
	
    system("flash_erase /dev/mtd3 0x378000 1");     // erase 'WORK' flag
    CLEAR(sys_cmd);
    snprintf(sys_cmd,128,"flashcp %s /dev/mtd3",FILE_USER);  // update mtd 
    system(sys_cmd);
	remove(FILE_USER);
	
	if(f_flag == 1){
		/*step 5 . update kernel1 */
		CLEAR(sys_cmd);
		offset += count;
		count  = 2048;
		snprintf(sys_cmd,128,"dd if=%s of=%s bs=1024 count=%d skip=%d",firmwarePath,FILE_KERNEL,count,offset);
		system(sys_cmd);
		system("flash_erase /dev/mtd4 0x1F8000 1");     // erase 'WORK' flag
		CLEAR(sys_cmd);
		snprintf(sys_cmd,128,"flashcp %s /dev/mtd4",FILE_KERNEL);  // update mtd 
		system(sys_cmd);
		remove(FILE_KERNEL);
		
		/*step 6 . update rootfs1 */
		CLEAR(sys_cmd);
		offset += count;	
		count  = 1792;
		snprintf(sys_cmd,128,"dd if=%s of=%s bs=1024 count=%d skip=%d",firmwarePath,FILE_ROOT,count,offset);
		system(sys_cmd);
		system("flash_erase /dev/mtd5 0x1B8000 1");     // erase 'WORK' flag
		CLEAR(sys_cmd);
		snprintf(sys_cmd,128,"flashcp %s /dev/mtd5",FILE_ROOT);  // update mtd 
		system(sys_cmd);
		remove(FILE_ROOT);
		
		/*step 7 . update usr1 */
		CLEAR(sys_cmd);
		offset += count;	
		count  = 3584;
		snprintf(sys_cmd,128,"dd if=%s of=%s bs=1024 count=%d skip=%d",firmwarePath,FILE_USER,count,offset);
		system(sys_cmd);
		
		system("flash_erase /dev/mtd6 0x378000 1");     // erase 'WORK' flag
		CLEAR(sys_cmd);
		snprintf(sys_cmd,128,"flashcp %s /dev/mtd6",FILE_USER);  // update mtd 
		system(sys_cmd);
		remove(FILE_USER);		
		
		/*step 7 . update mtd */
		CLEAR(sys_cmd);
		offset += count;	
		count  = 1024;
		snprintf(sys_cmd,128,"dd if=%s of=%s bs=1024 count=%d skip=%d",firmwarePath,FILE_MTD,count,offset);
		system(sys_cmd);
		
		system("flash_erase /dev/mtd7 0xf8000 1");     // erase 'WORK' flag
		CLEAR(sys_cmd);
		snprintf(sys_cmd,128,"flashcp %s /dev/mtd7",FILE_MTD);  // update mtd 
		system(sys_cmd);
		remove(FILE_MTD);				
	}
	
    return 0;
}

#endif 
int main(int argc , char **argv)
{
    if(argc < 2){
        fprintf(stderr,"usage: %s <filename> \n",argv[0]);
        return -1;
    }

    if(QCamFlashBurn(argv[1]) != 0)
	{
		return -1;
	}

    printf("-------------------------\n");
    printf("------FW UPDATE DONE-----\n");
    printf("-------------------------\n");

    return 0;
}
