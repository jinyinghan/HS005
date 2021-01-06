#ifndef __OTA_H
#define __OTA_H

#include "include/common_env.h"
#include "include/common_func.h"
#include "curl/curl.h"
#include "vava_cloud.h"
#include "circular_buf.h"
#include "http_curl.h"


#include <fcntl.h>
#include <sys/reboot.h>
#include <semaphore.h>

typedef struct {
        uint32_t id;
        uint32_t len;
        uint32_t flash_off;
        uint32_t reserve1;
}zone_info;

typedef struct
{  
		const char *file;
		const char *device;
		uint32_t size;
}flash_part;

#define MAX_PARTS 7
#define FILE_UBOOT  "/tmp/uboot.bin"
#define FILE_KERNEL "/tmp/kernel.bin"
#define FILE_ROOT   "/tmp/root.bin"
#define FILE_USER   "/tmp/user.bin"
#define FILE_MTD   "/tmp/mtd.bin"
#define IH_MAGIC    0x27051956  // Image Magic Number       
#define IH_ARCH_MIPS        5   // MIPS     
#define IH_TYPE_FIRMWARE    5   // Firmware Image    

#define OTA_FLAG	"/tmp/ota_flag"
#define FW_HEAD_PATH "/tmp/fw_head.bin"
#define OTA_BUFFER_SIZE 512*1024
#define OTA_WRITE_SIZE	384*1024 


pthread_t fw_OTA_id;


int test_ota_flag();
int GenerateHead(char* url, int offset);
//int  set_mtd_func(int arg, char* url, void* cbuf);
void *thread_OTA_fw(void *arg);
int compare_fw_version(char*fw_ver);
void update_status_init();
void get_update_status(int status, int loaddata);
void set_update_status(int status, int loaddata);
long check_crc_append(const char *filepath, unsigned long crc_in, int totalSize);
int http_download_file_part(char *url, void *fp,  int idx, int offset, int restlen);
//int http_download_file_test(CURL* curlhandle, void *fp,  int idx, int offset);

//void* write_mtd_thread(void *arg);
int write_mtd_func(void *arg, char* filepath,  int partlenth);
size_t download_process_ota(void *buffer,size_t size,size_t nmemb,void *user);










#endif
