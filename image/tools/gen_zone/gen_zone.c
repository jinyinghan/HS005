/*
 * A simple program to write flag to the binary.
 */

#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>

#define ZONE_NUM 	3
#define ZONE_F_NUM  8

#define UBOOT_OFF 	0x00000000
#define ROOTFS_OFF  0x00040000
#define KERNEL_OFF  0x00240000
#define USER_OFF    0x00400000
//#define KERNEL2_OFF 0x00800000
//#define USER2_OFF   0x009c0000
//#define MTD_OFF	    0x00ec0000
#define KERNEL2_OFF 0x008c0000
#define USER2_OFF   0x00a80000
#define MTD_OFF	    0x00f40000

struct zone_info{
	uint32_t id;
	uint32_t len;
	uint32_t flash_off;
	uint32_t reserve1;
};

int choose_offset(int num, int id)
{	
	uint32_t ret = 0;
	if(ZONE_NUM == num)
	{
		switch(id){
			case 1: ret = KERNEL_OFF;break;			
			case 2:	ret = USER_OFF ; break;
			default:
				printf("bad id (%d)\n",id); ret = -1;
		}
			
	}else if(ZONE_F_NUM == num)
	{
		switch(id){
			case 1: ret = UBOOT_OFF ; break;			
			case 2: ret = ROOTFS_OFF;break;
			case 3: ret = KERNEL_OFF;break;			
			case 4:	ret = USER_OFF ; break;
			case 5: ret = KERNEL2_OFF ; break;
			case 6: ret = USER2_OFF;break;
			case 7:	ret = MTD_OFF ; break;			
			default:
				printf("bad id (%d)\n",id); ret = -1;
		}
		
	}
		
	return ret;	
}

int main(int argc, char *argv[])
{
	if (argc < 4) {
		printf("usage: gen_zone [file] num kernel_size root_size user_size\n");
		_exit(1);
	}

	int fd;
	int id = 1;
	struct zone_info info;
	uint32_t kernel,root,user,num,off=0;
	char *file_path = argv[id++];
	num  	= atoi(argv[id++]);
	if(num != ZONE_NUM && num != ZONE_F_NUM)
	{
		printf("num must be 3 or 8\n");
		_exit(1);
	}
	
	printf("file: %s, num:%d, \n", file_path, num);

	fd = open(file_path, O_RDWR);
	if (fd < 0) {
		perror("Input file open error");
		_exit(1);
	}

	int i;
	ssize_t ret = write(fd, &num, 4);
	if (ret != 4) {
			printf("write info error\n");
			_exit(1);
		}	
		
	for(i=1;i<=num;i++)
	{
		memset(&info,0,sizeof(struct zone_info));
		info.id = i;
		info.len = atoi(argv[i+2]);		
		info.flash_off = choose_offset(num,info.id);
		
		printf("wirte id:%d -- size:%d\n",info.id,info.len);
		ssize_t ret = write(fd, &info, sizeof(struct zone_info));
		if (ret != sizeof(struct zone_info)) {
			printf("write info error\n");
			_exit(1);
		}			
	}


	close(fd);
	printf("write info finished\n");

    return 0;
}
