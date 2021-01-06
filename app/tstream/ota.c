#include "ota.h"

char vava_otn_bsup_url[128] = {0};
char vava_otn_bsup_upver[12] = {0};
char vava_otn_bsup_timestart[12] = {0};
char vava_otn_bsup_timeend[12] = {0};

zone_info part_info[7] = {0};

int ota_status_info = -1;
int ota_loaddata =-1;
int g_appOtaSession = -1;
long crc_out = 0;
int total_writeNumb = 0;//写入字节的计数+1024==头64字节中的ih_size值 
int upid = 0;
bool g_appOtaFlag = false;
bool downloadend_flag = false;
bool downloaderr_flag = false;
bool backmtdclean_flag = false;

extern ota_info_t *ota_cmd;
extern  char url_JsonHead[] ;

//pthread_mutex_t seekNumb_mutex;
pthread_cond_t  ota_cond_test;
pthread_mutex_t ota_mutex_test;
pthread_mutex_t ota_status_mutex;

sem_t Download_sem;
sem_t Writeend_sem;


flash_part pi[MAX_PARTS] = {

	{FILE_UBOOT, "/dev/mtd0", 256}, 
	{FILE_KERNEL,"/dev/mtd1",1792}, 
	{FILE_ROOT,  "/dev/mtd2",1792}, 
	{FILE_USER,  "/dev/mtd3",4288}, 
	{FILE_KERNEL,"/dev/mtd4",1792}, 
	{FILE_ROOT,  "/dev/mtd5",1792}, 
	{FILE_USER,  "/dev/mtd6",4288}, 
	{FILE_MTD,	 "/dev/mtd7", 256}
};

int test_ota_flag(){
  if(access(OTA_FLAG, F_OK) == 0)
  	return 1;
  return 0;
}

bool check_mtd_ff(const char* path, int lenth)
{
	 int blocksize = 1024/4;
     int retval = 0;
	 int i,j;

	 return ture;

	 unsigned int *data = (unsigned int *)malloc(blocksize);
	 if(NULL == data)
	 {
		 LOG("malloc buffer %d failed !\n", blocksize);
		 return false;
	 }

     int fp = open(path,O_RDONLY);
     if(fp < 0)
     {
         LOG("open file %s error !\n",path);
		 free(data);
		 return false;
     }

	 lseek(fp, 0, SEEK_SET);
	 retval = read(fp, data, blocksize);
	 if(retval != blocksize )	
	 {
		LOG("read data len = %d is error , it should be %d!\n", retval, blocksize);
		free(data);
		close(fp);
		return false;
	 }

	 for(i = 0; i < blocksize-1; i++) //测试前1k是否清0xff
	 {
	 	LOG("(*data+%d)= %#x , ^ = %d \n",i,(*(data+i)), ((*(data+i)) ^ 0xffffffff));
		if( (*(data+i)) ^ 0xffffffff)
		{
			LOG("%s is not clean!\n",path);
//			free(data);
//			close(fp);
//			return false;
		}
	 }
	
	for(i = 0; i < lenth; i++) //lenth单位是kb
	{
		lseek(fp, i*blocksize, SEEK_SET);
		retval = read(fp, data, blocksize);
		if(retval != blocksize )	
		{
			LOG("read data len = %d is error , it should be %d!\n", retval, blocksize);
			free(data);
			close(fp);
			return false;
		}

		for(j = 0; j < blocksize-1; j++) //测试每1k是否清0xff
	 	{
			if( (*(data+j)) ^ 0xffffffff)
			{
				LOG("%s is not clean!\n",path);
				free(data);
				close(fp);
				return false;
			}
		 }
	}
	
	free(data);
	close(fp);  
	
	return true;	
}

int check_mtd_clean(int part_num)
{
	int id,idx = 0;
	int i;
	bool res = false;
	bool ret = false;
	char sys_cmd[64] = {0};
	int back_num = which_zone_to_write();
	if(back_num < 0)
	{
		fprintf(stderr,"cmdline read error !\n");
		return -1;
	}
	if(back_num == 1)
	{
        id = 1;//擦除主分区
    }
	else
	{
		id = 4;//擦除备份分区
	}	
	idx = id;

	for(i= 0;i < part_num;i++,idx++)
	{
		res = check_mtd_ff(pi[idx].device, pi[idx].size);
		if(res == false)
		{	
			snprintf(sys_cmd,256,"flash_erase %s 0 0", pi[idx].device);
			AMCSystemCmd(sys_cmd);//擦除对应分区
			LOG("AMCSystemCmd(%s)\n",sys_cmd);
			ret = check_mtd_ff(pi[idx].device, pi[idx].size);
			if(ret == false)
			{
				LOG("%s is broken!please quit ota!!!\n", pi[idx].device);
				backmtdclean_flag == false;
				return -1;
			}
		}
	}
	
	backmtdclean_flag == true;
	
	LOG("mtd already clean!\n");

	return 0;//-1 err,0 success
}

long check_crc_append(const char *filepath, unsigned long crc_in, int totalSize)
{
	 int blocksize = 1024;
     int retval = 0;
	 int i;
     unsigned long crc_dat_out = 0;
     unsigned long crc_dat_in = crc_in;

     int fp = open(filepath,O_RDONLY);
     if(fp < 0)
     {
         LOG("open file %s error !\n",filepath);
     }

	unsigned char *data = (unsigned char *)malloc(blocksize);
	if(NULL == data)
	{
		LOG("malloc buffer %d failed !\n", blocksize);
		close(fp);
		return -1;
	}
	
	for(i=0; i<totalSize; i++) //totalSize单位是kb
	{
		lseek(fp, i*blocksize, SEEK_SET);
		retval = read(fp, data, blocksize);
		if(retval != blocksize )	
		{
			LOG("read data len = %d is error , it should be %d!\n", retval, blocksize);
			free(data);
			return -1;
		}
		crc_dat_out = crc32(crc_dat_in, (unsigned char const *)data, retval);
		crc_dat_in = crc_dat_out;
	 }
	
	free(data);
	close(fp);  
	return crc_dat_out;
}

int compare_fw_version(char*fw_ver)
{
	return 1;
}

void init_update_status()
{
	ota_status_info = 0;
	ota_loaddata = 0;
}

void get_update_status(int status, int loaddata)
{
//    pthread_mutex_lock(&ota_status_mutex);
	status = ota_status_info;
	loaddata = ota_loaddata;
//	pthread_mutex_unlock(&ota_status_mutex);
}

void set_update_status(int status, int loaddata)
{
//    pthread_mutex_lock(&ota_status_mutex);
	ota_status_info = status;
	ota_loaddata = loaddata;
	//608 VAVA_CMD_UPGRATE_STATUS 主动上报升级状态	
	ppcs_proto_send_upgrate_status(g_appOtaSession, ota_status_info, VAVA_UPDATE_TYPE_CAMERA, ota_loaddata);
//    pthread_mutex_lock(&ota_status_mutex);
}

void* write_mtd_thread(void *arg)
{
	cbuf_handle_t read_cbuf = (cbuf_handle_t)arg;
	if(read_cbuf == NULL || read_cbuf->buffer == NULL)
	{
		LOG("cbuf_handle is null !\n");
		sem_post(&Writeend_sem);
		pthread_exit(0);
	}
	int size, fp, ret, return_size;
	int idx = 0;
	char devpath[64] = {0};
	int seekNumb = 0 ;
	total_writeNumb = 0;
	int upidx = upid;
	sprintf(devpath, pi[upidx].device);	
	
	while(g_enviro_struct._loop)
	{
		usleep(500);
		size = circular_buf_size(read_cbuf);
		if(size > OTA_WRITE_SIZE || downloadend_flag == true ||downloaderr_flag == true)
		{
			if(downloaderr_flag == true)
			{			
				LOG("downloaderr write_mtd_thread exit !\n");
				sem_post(&Writeend_sem);
				downloaderr_flag = false;
				pthread_exit(0);
			}
			
			if(read_cbuf == NULL || read_cbuf->buffer == NULL)
			{
				LOG("cbuf_handle is null !\n");
				sem_post(&Writeend_sem);
				pthread_exit(0);
			}

			if(total_writeNumb == 0)
			{
				retreat_pointer(read_cbuf, 1088);//前1088个字节不写入mtd
				size = size - 1088;
				LOG("throw first 1088 size = %d\n",size);
			}
		
			if(seekNumb + size > part_info[idx].len*1024)//mtd需要分成两端copy
			{
				size = part_info[idx].len*1024 - seekNumb;
				LOG("last part size = %d\n",size);
			}

			fp = open(devpath, O_RDWR); 
			if(fp < 0)
			{
				LOG("open file %s error !\n",devpath);
				sem_post(&Writeend_sem);
				pthread_exit(0);
			}
			
			ret = lseek(fp, seekNumb, SEEK_SET);//seek 应该与 ret相等
			if(ret != seekNumb)
			{
				LOG("lseek is %d,should be %d\n",ret, seekNumb);
				close(fp);
				sem_post(&Writeend_sem);
				pthread_exit(0);
			}
			
			if((read_cbuf->tail + size) > read_cbuf->max)//缓冲区需要分成两段copy
			{
				int len1 = read_cbuf->max - read_cbuf->tail;
				int len2 = size - len1;
				int seekb = seekNumb + len1;
				
				 
				int return_size1 = write (fp, (read_cbuf->buffer + read_cbuf->tail), len1);//第一段
				if(return_size1 != len1)
				{
					LOG("write is %d,should be %d\n",return_size1, len1);
				}
				//memset((read_cbuf->buffer + read_cbuf->tail), 0, len1);
				
				ret = lseek(fp, seekb, SEEK_SET);//seek 应该与 ret相等		
				if(ret != seekb)
				{
					LOG("lseek is %d,should be %d\n",ret, seekb);
				}
				
				int return_size2 = write (fp, read_cbuf->buffer, len2);//第二段，绕到整个存储区的开头
				if(return_size2 != len2)
				{
					LOG("write is %d,should be %d\n",return_size2, len2);
				}	
				//memset(read_cbuf->buffer, 0, len2);
				
//				LOG("[%d]+++len1 = %d write = %d seek1 = %d,len2 = %d write = %d seek2 = %d to dev = %s\n ",size,len1,return_size1,seekNumb,len2,return_size2,seekb,devpath);
				read_cbuf->full = false;
				read_cbuf->tail = len2;//更新读出位置
				return_size = return_size1 + return_size2;
				
			}
			else
			{
				return_size = write (fp, (read_cbuf->buffer + read_cbuf->tail), size);
				if(return_size != size)
				{
					LOG("write is %d,should be %d\n",return_size, size);
					close(fp);
					sem_post(&Writeend_sem);
					pthread_exit(0);
				}				
				retreat_pointer(read_cbuf, size);
			}
	
			sync();
			close(fp);
			
			total_writeNumb += return_size;
			int percent = total_writeNumb * 100/(1024*(part_info[0].len + part_info[1].len + part_info[2].len));
			if(percent%2==0)
				set_update_status(VAVA_UPDATE_LOADING, percent);//升级包下载中，上报进度

//			LOG("+++[%s]+write_size = %d,seek = %d, total_writeNumb = %d\n",devpath, return_size, seekNumb, total_writeNumb);

			if(downloadend_flag == true)
			{
				LOG("write mtd end!\n");
				sem_post(&Writeend_sem);
				downloadend_flag = false;
				pthread_exit(0);	
			}
			
			if(size == part_info[idx].len*1024 - seekNumb)
			{
				upidx++;idx++;
				sprintf(devpath, pi[upidx].device);
				seekNumb = 0;
				if(idx >2)
				{
					LOG("write mtd finish![%d]\n",idx);
					sem_post(&Writeend_sem);
					pthread_exit(0);
				}
				LOG("will change to next :[%s][%d] \n",pi[upidx].device, part_info[idx].len);
			}
			else
			{
				seekNumb += size;
			}

			size = 0;
			sem_post(&Download_sem);
//			LOG("sem_post(&Download_sem)\n");

		}

	}

}

size_t download_process_ota(void *buffer,size_t size,size_t nmemb,void *user)
{
	size_t return_size;
	FILE *fp = (FILE *)user;
	return_size = fwrite(buffer,size,nmemb,fp);
	//printf("write %s\n",buffer);
	return return_size;	
}

int download_progress_up(void* clientp, double dltotal, double dlnow, double ultotal, double ulnow)
{
	int *ret = (int *)clientp;
	*ret = (int)(100*dlnow/dltotal);
	if((*ret)%100 == 0)
	{	
		set_update_status(VAVA_UPDATE_LOADING, *ret);//升级包下载中，上报进度		
//		printf("progress : %d",*ret);
	}

//	上报百分比，一秒一次，只有0 / 100。
	return 0;
}

size_t download_firmware_process(void *buffer,size_t size,size_t nmemb,void *user)
{
	size_t return_size = size * nmemb;	
	
	int res = circular_buf_put(user, buffer, return_size);
	if(res == -1)
	{
		LOG("circular_buf_put2 return %d error!\n",res);
		return -1;
	}
	size_t len = circular_buf_size(user);
		
	if(len > OTA_WRITE_SIZE || circular_buf_full(user))
	{	
		LOG("len - %d stop download to wait buffer read!\n",len);
		sem_wait(&Download_sem);
	}
	return return_size;	
}

int http_download_file_ota(char *url, int progress, void *fp,  int verbose)
{
        CURL *handle;
        CURLcode ret = CURLE_OK;
        struct curl_slist * slist_urlHttpHead =  NULL;
        unsigned short second = 180; //60*60 =  1 hour

        LOG("URL:%s\n", url);

        slist_urlHttpHead = curl_slist_append(slist_urlHttpHead, url_JsonHead);

        //Initialize the globle env
//        ret = curl_global_init(CURL_GLOBAL_ALL);
        //    curl_dbg(ret);

        //get the handle of curl
        handle = curl_easy_init();
        if (handle) {
                // the attribute for curl
                ret = curl_easy_setopt(handle, CURLOPT_URL, url);
                curl_dbg(ret);

                ret = curl_easy_setopt(handle, CURLOPT_TIMEOUT, second);
                curl_dbg(ret);

                if (verbose) {
                        ret = curl_easy_setopt (handle, CURLOPT_VERBOSE, 1L);
                        curl_dbg(ret);
                }

                ret = curl_easy_setopt (handle, CURLOPT_HTTPHEADER, slist_urlHttpHead);
                curl_dbg(ret);

                ret = curl_easy_setopt(handle, CURLOPT_SSL_VERIFYPEER, 0);
                curl_dbg(ret);

                ret = curl_easy_setopt(handle, CURLOPT_SSL_VERIFYHOST, 0);
                curl_dbg(ret);

                ret = curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, &download_firmware_process);
                curl_dbg(ret);

                ret = curl_easy_setopt(handle, CURLOPT_WRITEDATA, fp);
                curl_dbg(ret);

                if(progress)
                {
                    /*download progress configure*/
                    ret = curl_easy_setopt(handle, CURLOPT_NOPROGRESS, 0L);
                    curl_dbg(ret);

                    ret = curl_easy_setopt(handle, CURLOPT_PROGRESSFUNCTION, &download_progress_up);
                    curl_dbg(ret);

                    ret = curl_easy_setopt(handle, CURLOPT_PROGRESSDATA, progress);
                    curl_dbg(ret);

                   // create_report_ota_progress_pthread();
                }

                ret = curl_easy_perform(handle);
                if(ret == CURLE_OK)
                        printf("curl perform success [%d]!\n",ret);
                else
                        curl_dbg(ret);


		}

//		curl_global_cleanup();
        curl_slist_free_all(slist_urlHttpHead);	
        return ret;
}


int http_download_file_part(char *url, void *fp,  int idx, int offset, int restlen)
{
        CURL *handle;
        CURLcode ret = CURLE_OK;
        struct curl_slist * slist_urlHttpHead =  NULL;
        unsigned short second = 10; 
		unsigned int startPos, endPos = 0;

		char szRange[128] ={};

		if(offset == 0)//下载前128字节
		{
			startPos = 0;
			endPos = 128;
		}
		else if(offset == 64)
		{
			startPos = offset;
			endPos = offset + 1024 - 1;
		}	
		
		sprintf( szRange, "%d-%d", startPos, endPos);
		
		slist_urlHttpHead = curl_slist_append(slist_urlHttpHead, url_JsonHead);//TextHead);

        //Initialize the globle env
//        ret = curl_global_init(CURL_GLOBAL_ALL);
        //    curl_dbg(ret);

        //get the handle of curl
        handle = curl_easy_init();
        if (handle)
		{
                // the attribute for curl
                ret = curl_easy_setopt(handle, CURLOPT_URL, url);
                curl_dbg(ret);

                ret = curl_easy_setopt(handle, CURLOPT_TIMEOUT, second);
                curl_dbg(ret);

                ret = curl_easy_setopt (handle, CURLOPT_VERBOSE, 0);//1L);
                curl_dbg(ret);

                ret = curl_easy_setopt (handle, CURLOPT_HTTPHEADER, slist_urlHttpHead);
                curl_dbg(ret);

                ret = curl_easy_setopt(handle, CURLOPT_SSL_VERIFYPEER, 0);
                curl_dbg(ret);

                ret = curl_easy_setopt(handle, CURLOPT_SSL_VERIFYHOST, 0);
                curl_dbg(ret);

               	ret = curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, &download_process_ota);//头校验前128字节下载回调函数
               	curl_dbg(ret);		
				
                ret = curl_easy_setopt(handle, CURLOPT_WRITEDATA, fp);
                curl_dbg(ret);
				ret = curl_easy_setopt(handle, CURLOPT_RANGE, szRange);//"0-199");
				curl_dbg(ret);
				
                ret = curl_easy_perform(handle);
                if(ret == CURLE_OK)
                {
//                        printf("curl perform success [%d]!\n",ret);
                }
				else
				{
					curl_dbg(ret);
				}			
				
                curl_easy_cleanup(handle);
        }
//        curl_global_cleanup();
        curl_slist_free_all(slist_urlHttpHead);	

        return ret;
}

int GenerateHead(char* url, int offset)
{	
	 FILE *fp = fopen(FW_HEAD_PATH, "wb");
	 if (fp == NULL)
	 {
	 	LOG("______fopen /tmp/mmcblk0p1/fw_head.bin for write is failed!!!\n");
	 }

	int i_ret = http_download_file_part(url,fp, 0, offset, 0);
	 
	 fclose(fp);
	 return i_ret;
}

void *thread_OTA_fw(void *arg)
{
	time_t timep;
	struct tm *sync_start_time;
	struct timespec out_time;
	struct timeval now_t;

	int ret =-1;
	int i,idx;
	int res = -1;
	char downloadURL[128] = {0};
	char sys_cmd[256] = {0};
	bool progressing = false;
	int part_num=0;
	int update_num = 0;
	unsigned int checkCrc = 0;
	ota_info_t ota ;
	
	snprintf(sys_cmd,256,"rm -rf %s",FW_HEAD_PATH);
	
	pthread_mutex_init(&ota_mutex_test, NULL);
	pthread_cond_init(&ota_cond_test, NULL);
	pthread_mutex_init(&ota_status_mutex,NULL);	

	while(g_enviro_struct._loop)
	{
		gettimeofday(&now_t, NULL);
		out_time.tv_sec = now_t.tv_sec + 5;//30;
		out_time.tv_nsec = now_t.tv_usec * 1000;
		pthread_mutex_lock(&ota_mutex_test);	
		pthread_cond_timedwait(&ota_cond_test, &ota_mutex_test, &out_time);
		pthread_mutex_unlock(&ota_mutex_test);		

		time(&timep);
		time(NULL);
		sync_start_time = localtime(&timep);
//		ota_starttime->tm_hour = atoi(vava_otn_bsup_timestart);
		
//		LOG("%d %d\n",sync_start_time->tm_hour,sync_start_time->tm_min);
//		LOG("==============OTA Test======t_online = %d===progressing = %d =====\n",g_enviro_struct.t_online, progressing);
//			if(g_enviro_struct.t_online == 1 && progressing == false && ((sync_start_time->tm_hour == 2 && sync_start_time->tm_min == 10 ) || g_appOtaFlag == true || test_ota_flag() == 1))

		if(backmtdclean_flag == false)
		{
			res = check_mtd_clean(3);
			if(res < 0)
			{
				LOG("mtd err!quit ota thread! %d \n",res);				
				pthread_exit(0);					
			}
		}
		
		if(g_enviro_struct.t_online == 1 && progressing == false && ( g_appOtaFlag == true || test_ota_flag() == 1))
		{	
			progressing = true;				
			init_update_status();
	
			if(g_appOtaFlag == true)//app触发升级不对版本号进行校验
			{
				g_appOtaFlag = false; 
				sprintf(ota.url, ota_cmd->url);
				ota.type = ota_cmd->type;
				sprintf(ota.otaver,ota_cmd->otaver);
			}
			else//在静默升级时间段内，先请求一次刷新升级信息
			{
				vava_get_ota_fwbin();//
				sprintf(ota.url, vava_otn_bsup_url);
				ota.type = atoi(vava_otn_bsup_upver) ;
				sprintf(ota.otaver,vava_otn_bsup_upver);
				if(compare_fw_version(ota.otaver) == 1)
				{						
					LOG("compare_fw_version success !\n");
				}
				else
				{
					LOG("compare_fw_version error !\n");
					progressing = false;
					continue;
				}
			}
			
			set_update_status(VAVA_UPDATE_START,0); //进入升级模式,主动上报
			
			ret = GenerateHead(ota.url, 0);//下载包头128字节
			if(ret != CURLE_OK)
			{						
				LOG("GenerateHead download first 128 error!\n");
				set_update_status(VAVA_UPDATE_LOAD_ERR,0); //下载升级包失败 ,主动上报
				progressing = false;
				continue;						
			}

			update_num = FwHeadConfig(FW_HEAD_PATH, &part_info, &part_num,&checkCrc);//校验包头 此函数定义在middleware\common\ysx_sys.c
				
			AMCSystemCmd(sys_cmd);
			LOG("AMCSystemCmd (%s) [0-128]\n",sys_cmd);//校验完成删除"/tmp/fw_head.bin"

			if(update_num < 0 || (part_num != 3 && part_num != 8))
			{
				LOG("FwHeadConfig error update_num = %d!\n",update_num);
				set_update_status(VAVA_UPDATE_CHECK_FAIL,0); //设备端校验升级包失败,主动上报
				progressing = false;
				continue;
			}
			ret = GenerateHead(ota.url, 64);//下载包头64字节后的1024字节
			crc_out = 0;
			crc_out = check_crc_append(FW_HEAD_PATH, 0, 1);//最后一个参数size单位是kb

			AMCSystemCmd(sys_cmd);
			LOG("AMCSystemCmd (%s) [64-1087]\n",sys_cmd);//校验完成删除"/tmp/fw_head.bin"
			if(ret != CURLE_OK || crc_out <= 0 )
			{
				LOG("download 1024 head %d or check crc return %x error\n",ret,crc_out);
				progressing = false;
				continue;
			}
					
			LOG("checkCrc = %x, update_num = %d, part_num = %d\n",checkCrc,update_num,part_num);
			if(update_num == 1)
			{
	            upid = 1;//升级主分区
	        }
			else
			{
				upid = 4;//升级备份分区
			}

			if(backmtdclean_flag == false)
			{
				res = check_mtd_clean(3);
				if(res < 0)
				{
					LOG("mtd err!quit ota thread! %d \n",res);				
					pthread_exit(0);					
				}
			}

			sem_init(&Download_sem, 0, 0);//初始化信号量
			sem_init(&Writeend_sem, 0, 0);//初始化信号量
			
			uint8_t * ota_buffer  = malloc(OTA_BUFFER_SIZE * sizeof(uint8_t));
			if(ota_buffer == NULL)
			{
				LOG("cannot get memory for buffer in init_ota_buffer\n");
				set_update_status(VAVA_UPDATE_LOAD_ERR,0); //下载升级包失败 ,主动上报
				progressing = false;
				continue;
			}
			memset(ota_buffer, 0, sizeof(ota_buffer));

			cbuf_handle_t fw_cbuf = circular_buf_init(ota_buffer, OTA_BUFFER_SIZE);
			if(fw_cbuf == NULL || fw_cbuf->buffer == NULL)
			{
				LOG("cbuf_handle_t init error!\n");
				set_update_status(VAVA_UPDATE_LOAD_ERR,0); //下载升级包失败 ,主动上报
				progressing = false;
				free(ota_buffer);
				continue;
			}
				
			pthread_t set_OTA_pid;
			int rest = pthread_create(&set_OTA_pid, NULL, &write_mtd_thread, (void*)fw_cbuf);
			if ( rest < 0)
			{
				LOG("write_mtd_thread Thread create fail, ret=[%d]\n", rest);
				set_update_status(VAVA_UPDATE_LOAD_ERR, 0);//设备端升级超时，上报进度
				circular_buf_free(fw_cbuf);
				progressing = false;
				free(ota_buffer);
				continue;
			}
			pthread_detach(set_OTA_pid);

			int i_ret = http_download_file_ota(ota.url, 0, fw_cbuf, 1);
			if(i_ret != CURLE_OK)//下载写入失败的处理
			{
				LOG("Timeout or connect error\n");				
				set_update_status(VAVA_UPDATE_TIMEOUT, 0);//设备端升级超时，上报进度
				circular_buf_free(fw_cbuf);
				progressing = false;
				free(ota_buffer);
				downloaderr_flag = true;
				continue;
			}

			downloadend_flag = true;

			sem_wait(&Writeend_sem);
			
			circular_buf_free(fw_cbuf);
			free(ota_buffer);
				

			set_update_status(VAVA_UPDATE_LOAD_FINISH,0); //升级包下载完成 ,主动上报							

			idx = upid;
			for(i=0; i <part_num; i++,idx++)
			{
				LOG("idx = %d,i = %d\n",idx,i);
				
				crc_out = check_crc_append(pi[idx].device, crc_out, part_info[i].len);
				if(crc_out <= 0)
				{
					LOG("check_crc_append return %x error \n",crc_out);
					set_update_status(VAVA_UPDATE_CHECK_FAIL,0); //设备端校验升级包失败 ,主动上报
					progressing = false;
					continue;
				}
				LOG("write work flag to %s at %d\n",pi[idx].device,pi[idx].size*1024);
				wr_flag(pi[idx].device, pi[idx].size*1024);
			}
			
		 	LOG("=================checkCrc =  %x=====crc_out final = %x \n",checkCrc,crc_out);
			if(checkCrc != crc_out)
			{
				LOG("crc check error!\n");
				set_update_status(VAVA_UPDATE_CHECK_FAIL,0); //设备端校验升级包失败 ,主动上报
				progressing = false;
				continue;								
			}
		 	LOG("flash mtd8\n");
		    AMCSystemCmd("flash_erase /dev/mtd8 0x18000 1");
			
		    if(update_num == 2)
			{
		   		wr_rescure_flag("/dev/mtd8",128*1024,"BACK_UP");  
				LOG("BACK_UP to mtd8\n");
			}
			else
			{
		        wr_rescure_flag("/dev/mtd8",128*1024,"MAIN_UP");
				LOG("MAIN_UP to mtd8\n");
			}
			LOG("ota end ! will reboot\n");							
			set_update_status(VAVA_UPDATE_UPGRADING,0); //正在升级中 ,主动上报							
			progressing = false;
			g_appOtaFlag = false; 
			pthread_mutex_destroy(&ota_status_mutex);
//				sem_destroy(&WriteMtd_sem);
			sem_destroy(&Download_sem);
			sem_destroy(&Writeend_sem);
			
//上报 即将重启
			system("sync");
			sleep(1);
			reboot(RB_AUTOBOOT);
			pthread_exit(0);
			
		}		
	}
	LOG("quit thread\n");
	pthread_exit(0); 	
}

void init_ota_thread(void)
{
	int tmp = pthread_create(&fw_OTA_id, NULL, &thread_OTA_fw, NULL);
	if ( tmp < 0)
	{
		LOG("thread_OTA_fw Thread create fail, ret=[%d]\n", tmp);
	}
	pthread_detach(fw_OTA_id); 
}

void uninit_ota_thread(void)
{
	
}


