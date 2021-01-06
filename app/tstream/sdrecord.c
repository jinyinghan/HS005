#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <fcntl.h>
#include <sys/statfs.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <math.h>
#include <unistd.h>

#include <arpa/inet.h>

#include "common_env.h"
#include "common_func.h"
#include "sdrecord.h"
#include "memory_debug.h"

typedef struct _NaluUnit
{
	int type;
	int size;
	unsigned char *data;
}NaluUnit;
void extract_spspps(uint8_t *data , int size );
static const char sps[23] = {0x67,0x64,0x0,0x28,0xac,0x3b,0x50,0x3c,0x1,0x13,0xf2,0xc2,0x0,0x0,0x3,0x0,0x2,0x0,0x0,0x3,0x0,0x3d,0x8};
static const char pps[4] = {0x68,0xee,0x3c,0x80};
//static const char confBUF[2] = {0x15,0x88};   //for 8K
static const char confBUF[2] = {0x14,0x8};  //for 16k

extern long long start_timestamp;

static pthread_mutex_t record_Mutex;//sd卡录像 锁
static pthread_mutex_t record_indexfile_Mutex;//sd卡录像 索引文件锁
static pthread_mutex_t md_record_Mutex;//移动侦测录像 锁

int sd_flag = 0;
static MP4FileHandle mp4_handle = NULL,md_record_handle;
static MP4TrackId video_tk =MP4_INVALID_TRACK_ID ,audio_tk=MP4_INVALID_TRACK_ID;
static MP4TrackId md_video_tk =MP4_INVALID_TRACK_ID ,md_audio_tk=MP4_INVALID_TRACK_ID;
static char video_record_path_cur[128] = {0};
static char video_del_path_cur[128] = {0};

struct tm curr_time={0};
char tmp_filename[128] = {0};
char dst_filename[128] = {0};
char dst_short_filename[32] = {0};

void SD_Record_Init()//没有被初始化， 目前看暂时也没有什么问题
{
	pthread_mutex_init(&record_Mutex,NULL);
	pthread_mutex_init(&record_indexfile_Mutex,NULL);
	pthread_mutex_init(&md_record_Mutex,NULL);
	
	return ;
}

int update_video_record_path(char *path, int p_len)
{
	if(path == NULL)
		return -1;
	if(strcmp(video_record_path_cur, path)){
		CLEAR(video_record_path_cur);
		memcpy(video_record_path_cur, path, p_len);
		return 0;
	}
	return 1;
}

int video_record_path_cur_del_cmp(char *path)
{
	if(path == NULL)
		return -1;
	if(!strcmp(video_record_path_cur, path)){
		return 0;
	}
	return 1;
}

int update_video_del_path(char *path, int p_len)
{
	if(path == NULL)
		return -1;
	if(strcmp(video_del_path_cur, path)){
		CLEAR(video_del_path_cur);
		memcpy(video_del_path_cur, path, p_len);
		return 0;
	}
	return 1;
}

int video_del_path_cur_del_cmp(char *path)
{
	if(path == NULL)
		return -1;
	if(!strcmp(video_del_path_cur, path)){
		return 0;
	}
	return 1;
}



static int err_cnt = 0;
struct timeval lst_vid = {0,0};


int write_video_frame(int keyframe,uint8_t *buf, int len ,const struct timeval *tv)
{
	int ret = 0;
	pthread_mutex_lock(&record_Mutex);
	//if(keyframe)
	//	printf("keyfame %d \n",len);
	
	if(mp4_handle != NULL)
	{
		if(len >= 4)
		{
			uint32_t cov_len  = htonl(len -4);
			int i = 0;
			for(;i<4;i++)
				buf[i] = (cov_len >> (8*i)) & 0xff;

		}

		MP4Duration tmp = 0;

		struct timeval tv;
		gettimeofday(&tv,NULL);
		uint64_t t_diff ;

		t_diff = abs((tv.tv_sec-lst_vid.tv_sec)*1000000+tv.tv_usec-lst_vid.tv_usec);

		if(lst_vid.tv_sec <= 0 )
			tmp = 90000*1/H264_FRAME_RATE;
		else if(t_diff>=(1000000 * 1))
		{
			tmp = 90000*1/10;
		}
		else
			tmp = 90*t_diff/1000 ;//   90000 / 100000 = 90 / 1000

		//		printf("vid duration = %ld\n",tmp);
		memcpy(&lst_vid,&tv,sizeof(struct timeval));
		if( !MP4WriteSample(mp4_handle, video_tk, buf, len,tmp , 0, 1) )
		{
			LOG("Error while writing video frame \n");
			if(err_cnt++ > 10 )
				ret = -1;
		}
	}
	pthread_mutex_unlock(&record_Mutex);
	return ret;
}

int write_audio_frame(uint8_t *buf, int len)
{
	int ret = 0;
	pthread_mutex_lock(&record_Mutex);
	if(mp4_handle != NULL)
	{
		if( ! MP4WriteSample(mp4_handle, audio_tk, buf, len , MP4_INVALID_DURATION, 0, 1))
		{
			LOG("Error while writing audio frame \n");
			if(err_cnt++ > 10 )
				ret = -1;
		}
	}
	pthread_mutex_unlock(&record_Mutex);
	return ret;
}


int mp4_record_init(const char *filename)
{
	if(NULL == filename){
		LOG("file name is NULL !\n");
		return -1;
	}

	mp4_handle = MP4CreateEx(filename,  0, 1, 1, 0, 0, 0, 0);
	if (mp4_handle == MP4_INVALID_FILE_HANDLE)
	{
		LOG("open file fialed.\n");
		return -1;
	}
	MP4SetTimeScale(mp4_handle, 90000);

	video_tk = MP4AddH264VideoTrack(mp4_handle, 90000, 90000 / 15, MAIN_WIDTH, MAIN_HEIGHT,
			0x64, //sps[1] AVCProfileIndication
			0x00, //sps[2] profile_compat
			0x1f, //sps[3] AVCLevelIndication
			3); // 4 bytes length before each NAL unit

	if (video_tk == MP4_INVALID_TRACK_ID)
	{
		LOG("add video track fialed.\n");
		return -1;
	}
	MP4SetVideoProfileLevel(mp4_handle, 0x7F);

	int audio_time_scale = AUDIO_SAMPLERATE_YSX;  // samplerate
	audio_tk = MP4AddAudioTrack(mp4_handle, audio_time_scale, 1024, MP4_MPEG4_AUDIO_TYPE);
	if (audio_tk == MP4_INVALID_TRACK_ID)
	{
		LOG("add audio track fialed.\n");
		return -1;
	}
	MP4SetAudioProfileLevel(mp4_handle, 0x2);

	memset(&lst_vid,0,sizeof(struct timeval));

//	EMGCY_LOG("Record init ok! \n");
	return 0;
}


int mp4_record_close()
{
//	EMGCY_LOG("close mp4 record ...\n");
	if (mp4_handle) {
		MP4Close(mp4_handle,0);
		mp4_handle = NULL;
	}
	return 0;
}

int SdRecord_Stop()
{
//	EMGCY_LOG("SD record stop #\n");
	pthread_mutex_lock(&record_Mutex);
	if (mp4_handle) {
		mp4_record_close();
	}
	sd_flag = 0;
	pthread_mutex_unlock(&record_Mutex);
	return 0;
}

static int create_multi_dir(const char *path)
{
	int i, len;

	len = strlen(path);
	char dir_path[len+1];
	dir_path[len] = '\0';

	strncpy(dir_path, path, len);

	for (i=0; i<len; i++) {
		if (dir_path[i] == '/' && i > 0) {
			dir_path[i]='\0';
			if (access(dir_path, F_OK) != 0) {
				if (mkdir(dir_path, 0755) < 0) {
					LOG("mkdir=%s:msg=%s\n", dir_path, strerror(errno));
					return -1;
				}
			}
			dir_path[i]='/';
		}
	}

	return 0;
}

void dump_index(RIndex temp)
{
	printf("iy = %d ",temp.iy);
	printf("im = %d ",temp.im);
	printf("id = %d ",temp.id);
	printf("ih = %d ",temp.ih);
	printf("imi = %d ",temp.imi);
	printf("timelen = %d ",temp.timelen);
	printf("\n");
	return;
}


int gen_record_name(char *sdpath,char *filename)
{
	time_t timep;
	char dir_path[128] = {0};
	struct tm *p = NULL;
	int i_ret = -1;
	if(NULL == filename){
		LOG("file name is NULL\n");
		return -1;
	}

	time(&timep);
	struct tm *t= localtime(&timep); //?????????
	memcpy(&curr_time,t,sizeof(struct tm));
	p = t;

	CLEAR(dir_path);
	//sprintf(dir_path,"%s/VAVACAM/%04d-%02d-%02d/%02d/",sdpath,(1900+p->tm_year) , (1+p->tm_mon), p->tm_mday,p->tm_hour);
	sprintf(dir_path,"%s/VAVACAM/%04d%02d%02d/%02d/",sdpath,(1900+p->tm_year) , (1+p->tm_mon), p->tm_mday,p->tm_hour);

	create_multi_dir(dir_path);
	i_ret = update_video_record_path(dir_path, sizeof(dir_path));
	pr_dbg("in i_ret %d\n", i_ret);
	/*2016_12_14_10_22_49.mp4*/
	//sprintf(filename,"%stmp-%04d_%02d_%02d_%02d_%02d_00.mp4",dir_path,(1900+p->tm_year),
	//		(1+p->tm_mon), p->tm_mday, p->tm_hour,p->tm_min);
	sprintf(filename,"%stmp-%02d%02d00_%d.mp4",dir_path,p->tm_hour,p->tm_min,VAVA_REC_TYPE_NORMAL);//普通录像 或者 定时录像

	sprintf(tmp_filename,"%stmp-%02d%02d00_%d.mp4",dir_path,p->tm_hour,p->tm_min,VAVA_REC_TYPE_NORMAL);//用于rename
	sprintf(dst_filename,"%s%02d%02d00_%d.mp4",dir_path,p->tm_hour,p->tm_min,VAVA_REC_TYPE_NORMAL);//用于rename
	sprintf(dst_short_filename,"%02d%02d00_%d.mp4",p->tm_hour,p->tm_min,VAVA_REC_TYPE_NORMAL);//用于rename
	
	return 0;
}

int remove_tmp_file(char *path)
{
	char sys_cmd[256];
	CLEAR(sys_cmd);
	snprintf(sys_cmd,sizeof(sys_cmd),"rm %stmp-* 2>/dev/null",path);
	//	LOG("delet tmp --%s--\n",sys_cmd);
	AMCSystemCmd(sys_cmd);

	return 0;
}

/*
return:
2 --- empty
0 -- dir error
>2 -- not empty
*/

enum {
	DIR_STAT_ERR		   = 0,
	DIR_STAT_EMPTY		   = 2,
	DIR_STAT_ONE_LEFT	   = 3,
	DIR_STAT_NUMS,
};

int is_empty_dir(const char * dirpath)
{
	DIR * dirp = NULL;
	int num=0;

	dirp = opendir(dirpath);
	while(dirp) 
	{
		if (readdir(dirp) != NULL) {
			++num;
			if (num > 3) { //减少遍历时间
				break;
			}
		}
		else {
			break;
		}
	}

	if (dirp) {
		closedir(dirp);
	}

	return num;
}


int remove_dir(const char * path)
{
	int ret = 0;
	if (access(path, F_OK) == 0) {
		ret = rmdir(path);
		if (ret < 0) {	
			LOG("rmdir: %s failed with ret: %s\n", path, strerror(ret));
		}
	}

	return ret;
}

//整个函数需要改造 VAVACAM
int del_oldest_record(const char *path)
{
	//EMGCY_LOG("##### DEL RECORD FILE ##### \n");
	int ret = 0;
	struct dirent **namelist,*ptr;
	struct stat s_buf;
	int n,i;
	char nameTemp[128];// /tmp/mmcblkp01/VAVACAM/20201212
	char vavacamPath[128];  ///tmp/mmcblkp01/VAVACAM/
	int find_flag = 0;//确定有无找小时目录

	memset(vavacamPath,0,sizeof(vavacamPath));
	snprintf(vavacamPath,sizeof(vavacamPath)-1, "%s/VAVACAM/",path);
	
	//扫描VAVACAM目录
	n = scandir(vavacamPath, &namelist, NULL, alphasort);
	if (n < 0)
	{
		LOG("scan dir error:%s",strerror(errno));
		return -1;
	}

	//遍历所有目录
    for(i=0;i<n;i++)
    {
        ptr = namelist[i];
        if(strcmp(ptr->d_name,".")==0 || strcmp(ptr->d_name,"..")==0)     ///current dir OR parrent dir
                continue;

		if(find_flag==1)
		{
			free(ptr); 
			continue;
		}
		
        memset(nameTemp,0,sizeof(nameTemp));
        strncpy(nameTemp,vavacamPath,sizeof(nameTemp)-1);
        strcat(nameTemp,"/");
        strcat(nameTemp,ptr->d_name);

		stat(nameTemp,&s_buf);
		if(S_ISDIR(s_buf.st_mode))//得到第一个文件夹
		{
			int j;
			for(j=0;j<24;j++)//遍历24小时中间的一个 如果找到 find_flag 设置为 1
			{
				char dir_name[128]={0};// /tmp/mmcblkp01/VAVACAM/20201212/01
				snprintf(dir_name,sizeof(dir_name)-1, "%s/%02d",nameTemp,j);
				if( 0 == access(dir_name,F_OK))
				{
					char cmdbuf[128]={0};
					snprintf(cmdbuf,sizeof(cmdbuf)-1, "rm -r %s",dir_name);
					system(cmdbuf);
					LOG("### %s\n",cmdbuf);
					find_flag = 1;					
					break;
				}
			}
		}
		else if(S_ISREG(s_buf.st_mode))
		{
			//如果是一个用户文件文件 要不要删除
			//remove(nameTemp);
		}else{}
		
		free(ptr);
    }
	free(namelist);

	//到这里还没有删除 小时文件则可以删除整个目录
	if(find_flag==0)
	{
		LOG("rm all dir %s",nameTemp);
		rmdir(nameTemp);//如果不保留 用户文件可以调用系统函数rm -r
		//重新扫描索引文件 这里应该是要做进去
		//fix me！
		
	}
	
	
	//EMGCY_LOG("remove file %s\n",buf);
	return ret;
}

int readonly_check()
{
	char buffer[256];
	CLEAR(buffer);
	pid_t pid;

	FILE *read_fp = ysx_popen("mount | grep mmc | grep \"(ro\"","r",&pid);
	if (read_fp != NULL)
	{
		fread(buffer, sizeof(char), 50, read_fp);
		if (strlen(buffer) != 0)
		{
			ysx_pclose(read_fp,pid);
			LOG("## SD Readonly ## , Disable mp4 record\n");
			LOG("## SD Readonly ## , Disable mp4 record\n");
			LOG("## SD Readonly ## , Disable mp4 record\n");

			remove("/tmp/sd_ok");
			return 1;
		}
	}
	else{
		sprintf(buffer, "ysx_popen sdcard readonly error:%s\n", strerror(errno));
		LOG(buffer);
	}

	ysx_pclose(read_fp,pid);
	return 0;
}


int capacity_check(char *mount_point)
{
	struct statfs diskInfo; //系统stat的结构体
	int check_cnt = 0;


	while(1)
	{
		if (readonly_check()) {
			return -1;
		}
		
		if (statfs(mount_point, &diskInfo) == -1){   //获取分区的状态
			LOG("stat %s failed with: %s\n", mount_point, strerror(errno));
			return -1;
		}
		
		if ((check_cnt++) > 20) {
			LOG("No Space For SD record !\n");
			check_cnt = 0;
			return -1;
		}

		unsigned long long blocksize = diskInfo.f_bsize;    //每个block里包含的字节数
		unsigned long long totalsize = diskInfo.f_blocks * blocksize >> 20;
		unsigned long long freesize  = diskInfo.f_bfree  * blocksize >> 20; //剩余空间的大小

		g_enviro_struct.t_sdtotal = totalsize;
		g_enviro_struct.t_sdfree  = freesize;

		if (freesize > 100) { //100MB
			check_cnt = 0;
			break;
		}

		LOG("Total_size = %llu MB , free_size = %llu MB \n",   totalsize , freesize);
		del_oldest_record(mount_point); //删除VAVACAM和VAVACAM_ALARM下面的最老小时目录

	}

	return 0;
}

void extract_spspps(uint8_t *data , int size )
{
	NaluUnit naluUnit = {0};
	int offset = 0;
	int ret;
	while(ret = ReadOneNaluFromBuf_record(&naluUnit,data+offset,size-offset))
	{
		//printf("Nal type -> %d\n",naluUnit.type);
		if(naluUnit.type == 7)  // 7-> pps , 8->sps
		{
			MP4AddH264PictureParameterSet(mp4_handle,video_tk,naluUnit.data,naluUnit.size);
			//hex_dump(naluUnit.data,naluUnit.size);
		}else if(naluUnit.type == 8){
			//hex_dump(naluUnit.data,naluUnit.size);
			MP4AddH264SequenceParameterSet(mp4_handle,video_tk,naluUnit.data,naluUnit.size);
		}
		if(naluUnit.data)
			mem_free(naluUnit.data);

		memset(&naluUnit,0,sizeof(NaluUnit));
		offset += ret;
		if(offset >= size)
			break;
	}

	return ;
}

int get_sd_path(char *path, int path_len)
{
	if((path == NULL) || (path_len < SD_PATH_LEN)) {
		pr_error("in %d\n", path_len);
		return -1;
	}
	FILE *fp = fopen("/tmp/sd_ok", "r");
	if(!fp){

		return -1;
	}
	CLEAR(path);
	int ret=fread(path, 1, SD_PATH_LEN, fp);
	path[ret-1]='\0';
	fclose(fp);
	return 0;
}

void record_indexfile_Mutex_lock(void)
{
	pthread_mutex_lock(&record_indexfile_Mutex);
	return ;
}

void record_indexfile_Mutex_unlock(void)
{
	pthread_mutex_unlock(&record_indexfile_Mutex);
	return ;
}

int index_file_is_completed(char *file_name)
{
	FILE *fp;
	char line[64];
	fp = fopen(file_name, "r");

	if(fp==NULL)
		return -1;
	
	while (fgets(line, sizeof(line) , fp) != NULL)
	{
		if(strncmp(RECORD_INDEX_END_MARK,line,strlen(RECORD_INDEX_END_MARK))==0)
		{
			return 0;
		}
	}

	return -1;
}

void update_index_file(struct tm *t, char *filename,int duration)
{
	char index_file[128] = {0};
	char line_context[128]={0};
	char end_mark[sizeof(RECORD_INDEX_END_MARK)]={0};
	struct tm *p = NULL;
	FILE *fp = NULL;
	
	p = t;
	CLEAR(index_file);

	snprintf(index_file,sizeof(index_file)-1, "%s/VAVACAM/%04d%02d%02d/index.txt",
				g_enviro_struct.t_sdpath,(1900+p->tm_year) , (1+p->tm_mon), p->tm_mday);

	
	record_indexfile_Mutex_lock();
				
	if(access(index_file, F_OK)!=0)//文件不存在
	{
		fp = fopen(index_file, "w+");
		if(fp==NULL)
		{
			LOG("fp error\n");
			record_indexfile_Mutex_unlock();
			return ;
		}

		sprintf(line_context,"%s(%d)\n",filename,duration);//文件名增加换行符号
		fwrite(line_context,1,strlen(line_context),fp);
		fwrite(RECORD_INDEX_END_MARK,1,strlen(RECORD_INDEX_END_MARK),fp);
		fclose(fp);
	}
	else
	{
		FILE *fp_out;
		char path_tmp[128];
		char backup_file[128];
		char line[64];
		sprintf(path_tmp, "%s.tmp",index_file);
		sprintf(backup_file, "%s.backup",index_file);
		 
		fp = fopen(index_file, "r");
		if(fp==NULL)
		{
			LOG("fp error\n");
			record_indexfile_Mutex_unlock();
			return ;
		}

		fp_out = fopen(path_tmp, "w+");
		if(fp==NULL)
		{
			LOG("fp error\n");
			fclose(fp);
			record_indexfile_Mutex_unlock();
			return ;
		}

		while (fgets(line, sizeof(line) , fp) != NULL)
		{
			if(strncmp(RECORD_INDEX_END_MARK,line,strlen(RECORD_INDEX_END_MARK))==0)
			{
				sprintf(line_context,"%s(%d)\n",filename,duration);//文件名增加换行符号
				fwrite(line_context,1,strlen(line_context),fp_out);
				fwrite(RECORD_INDEX_END_MARK,1,strlen(RECORD_INDEX_END_MARK),fp_out);
				break;
			}
			fwrite(line, 1, strlen(line), fp_out);
			memset(line, 0 , sizeof(line)  );
		}

		fclose(fp_out);
		fclose(fp);

		//如果index_file不存在或者没有结束标志 需要从backup文件和tmp文件中去做恢复 先判断tmp文件是否有结束标志 在判断backup文件
		rename(index_file,backup_file);//保留次新文件
		sync();
		rename(path_tmp, index_file);
		sync();
		remove(backup_file);
	}

	record_indexfile_Mutex_unlock();

	return ;
}


//update_index_file 中可能存在index文件丢失 这里实现恢复
int sync_record_index(void)
{
	int ret = 0;
	struct dirent **namelist,*ptr;
	struct stat s_buf;
	int n,i;
	char nameTemp[128];// /tmp/mmcblkp01/VAVACAM/20201212
	char vavacamPath[128];  ///tmp/mmcblkp01/VAVACAM/
	int find_flag = 0;

	record_indexfile_Mutex_lock();	
	
	memset(vavacamPath,0,sizeof(vavacamPath));
	snprintf(vavacamPath,sizeof(vavacamPath)-1, "%s/VAVACAM/",g_enviro_struct.t_sdpath);
	
	//扫描VAVACAM目录
	n = scandir(vavacamPath, &namelist, NULL, alphasort);
	if (n < 0)
	{
		LOG("scan dir error:%s",strerror(errno));
		record_indexfile_Mutex_unlock();
		return -1;
	}

	//遍历所有目录
    for(i=0; i<n; i++)
    {
        ptr = namelist[i];
        if(strcmp(ptr->d_name,".")==0 || strcmp(ptr->d_name,"..")==0)     ///current dir OR parrent dir
                continue;
	
        memset(nameTemp,0,sizeof(nameTemp));
        strncpy(nameTemp,vavacamPath,sizeof(nameTemp)-1);
        strcat(nameTemp,"/");
        strcat(nameTemp,ptr->d_name);

		stat(nameTemp,&s_buf);
		if(S_ISDIR(s_buf.st_mode))//得到第一个文件夹
		{
			char index_file[128];
			char tmp_file[128];
			char backup_file[128];
			sprintf(index_file, "%s/index.txt",nameTemp);
			sprintf(tmp_file, "%s/index.txt.tmp",nameTemp);
			sprintf(backup_file, "%s/index.txt.backup",nameTemp);
			
			if(access(index_file ,F_OK) == 0)//index.txt存在
			{
				if( 0 == index_file_is_completed(index_file) )
				{
					remove(tmp_file);
					remove(backup_file);
					LOG("%s is OK 1\n",index_file);
				}
				else
				{
					if( 0 == index_file_is_completed(tmp_file) )
					{
						remove(index_file);
						rename(tmp_file, index_file);
						remove(backup_file);
						LOG("%s is OK 2\n",index_file);
					}
					else if( 0 == index_file_is_completed(backup_file) )
					{
						remove(index_file);
						rename(backup_file, index_file);
						remove(tmp_file);
						LOG("%s is OK 3\n",index_file);
					}
					else{
						LOG("%s is ERR 1\n",index_file);
						/*index文件将丢失 是否需要重新扫描？？*/
					}
				}
			}
			else
			{
				if( 0 == index_file_is_completed(tmp_file) )
				{
					remove(index_file);
					rename(tmp_file, index_file);
					remove(backup_file);
					LOG("%s is OK 5\n",index_file);
				}
				else if( 0 == index_file_is_completed(backup_file) )
				{
					remove(index_file);
					rename(backup_file, index_file);
					remove(tmp_file);
					LOG("%s is OK 6\n",index_file);
				}
				else{
					LOG("%s is ERR 2\n",index_file);
					/*index文件将丢失 是否需要重新扫描？？*/
				}
			}
		}
		else if(S_ISREG(s_buf.st_mode))
		{
			//如果是一个用户文件文件 要不要删除
			//remove(nameTemp);
		}else{}
		
		free(ptr);
    }
	free(namelist);
	
	record_indexfile_Mutex_unlock();
	return 0;
}


int SdRecord_Start(int keyframe , struct timeval tv,uint8_t *buf , int len)
{
	static char file[128]={'\0'};
	static char iframe_cnt = 0;
	static time_t lst_rcd_time = 0;

	pthread_mutex_lock(&record_Mutex);

	time_t current_time = tv.tv_sec;
	struct tm * diff_tm;  
	diff_tm = gmtime((const time_t *)&lst_rcd_time);  //减小等待I帧累计的时差

	/*not sync time*/
	if (access("/tmp/sync_time", X_OK) != 0) {
		pthread_mutex_unlock(&record_Mutex);
		return -1;
	}
	/*sd exit check*/
	if ( access("/tmp/sd_ok",F_OK) == 0 ) {
		if (sd_flag == 0) {//初始化状态  sd_flag 0
			get_sd_path(g_enviro_struct.t_sdpath, sizeof(g_enviro_struct.t_sdpath));
			if (mp4_handle) 
			{
				mp4_record_close();
			}
			sync_record_index();
		}

		if ((current_time%60 == 0) && sd_flag != 2) // 整分开始录像 sd_flag 2
		{
			sd_flag  = 2;
		}

		if (sd_flag != 2) { // sd_flag 0 状态变为 1
			sd_flag = 1;
		}
	}
	else {
		sd_flag = 0;
		if (mp4_handle) {
			mp4_record_close();
		}
		pthread_mutex_unlock(&record_Mutex);
		
		return -1;
	}

	if (sd_flag == 2 && abs( current_time - lst_rcd_time ) > RECORD_DURATION - diff_tm->tm_sec)
	{
		if(mp4_handle)
		{
			iframe_cnt = 0;
			mp4_record_close();
			rename(tmp_filename, dst_filename);//重命名  之后有 定时录像的问题 需要确认录制完毕才进入定时不录像模式
			//更新索引文件
			update_index_file(&curr_time,dst_short_filename, abs( current_time - lst_rcd_time ));
		}
		if(!keyframe)
		{
			pthread_mutex_unlock(&record_Mutex);
			if((iframe_cnt%10) == 0)
			{
				//QCamVideoInput_SetIFrame(MAIN_CHN);
				QCamVideoInput_SetIFrame(SECOND_CHN); //720
			}
			iframe_cnt ++;
			//			LOG("Wait for key frame %d..\n", iframe_cnt);
			return -1;
		}

		lst_rcd_time = current_time;
		if(capacity_check(g_enviro_struct.t_sdpath) == 0)
		{
			memset(file,0,sizeof(file));
			gen_record_name(g_enviro_struct.t_sdpath,file);
///			EMGCY_LOG("### new record ...%s\n",file);
			mp4_record_init(file);
			extract_spspps(buf,len);
		}

	}
	pthread_mutex_unlock(&record_Mutex);

	return 0;
}


int ReadOneNaluFromBuf_record(NaluUnit *nalu,unsigned char *buf, unsigned int size)
{
	  unsigned int	nalhead_pos = 0;
	
	  int nal_offset=nalhead_pos;
	  nalu->size = 0;
	  int one_nalu = 1; //本地读取buf是否包含多个nal
	  if(size < 4)
	  {
		printf("buf size is too small %d\n",size);
		return 0;
	  }
	
	  while(nalhead_pos + 4 <size)
	  {
		//search for nal header ，NALU 单元的开始, 必须是 "00 00 00 01" 或 "00 00 01",
			// find next nal header 00 00 00 01 to calu the length of last nal
		if(buf[nalhead_pos++] == 0x00 &&
		  buf[nalhead_pos++] == 0x00 &&
			buf[nalhead_pos++] == 0x00 &&
			  buf[nalhead_pos++] == 0x01)
		{
		  goto gotnal_head;
		}
		else
		{
		  continue;
		}
		//search for nal tail which is also the head of next nal
	gotnal_head:
		nal_offset = nalhead_pos;
		while (nal_offset + 4 < size)
		{
		  // find next nal header 00 00 00 01 to calu the length of last nal
		  if(buf[nal_offset++] == 0x00 &&
			buf[nal_offset++] == 0x00 &&
			  buf[nal_offset++] == 0x00 &&
				buf[nal_offset++] == 0x01)
		  {
			nalu->size = (nal_offset-4)-nalhead_pos; //nal头4个字节
			one_nalu = 0;
			break;
		  }
		  else
			continue;
	
		}
		if(one_nalu)
		{
		  nal_offset = size;
		  nalu->size = nal_offset-nalhead_pos; //nal头4个字节
		}
		nalu->type = buf[nalhead_pos]&0x1f;   // 7-> pps , 8->sps
		if(nalu->size)
		{
		  nalu->data= (unsigned char *)malloc(nalu->size);
		  if(nalu->data)
			memcpy(nalu->data,buf+nalhead_pos,nalu->size);
		  else
			printf("malloc for nal data err %s\n",strerror(errno));
		}
	
		nalhead_pos =(one_nalu==1) ? nal_offset:(nal_offset-4);
		break;
	  }
	
	  return nalhead_pos;
}



int wait_keyframe = 0;
struct timeval md_lst_vid = {0,0};
struct timeval md_lst_aud = {0,0};
static int md_err_cnt = 0;

int md_record_init(const char *filename)
{
	if(NULL == filename){
		LOG("file name is NULL !\n");
		return -1;
	}

	md_record_handle = MP4CreateEx(filename,  0, 1, 1, 0, 0, 0, 0);
	if (md_record_handle == MP4_INVALID_FILE_HANDLE)
	{
		LOG("open file fialed.\n");
		return -1;
	}
	MP4SetTimeScale(md_record_handle, 90000);

	md_video_tk = MP4AddH264VideoTrack(md_record_handle, 90000, 90000 / 15, MAIN_WIDTH, MAIN_HEIGHT,
			sps[1], //sps[1] AVCProfileIndication
			sps[2], //sps[2] profile_compat
			sps[3], //sps[3] AVCLevelIndication
			3); // 4 bytes length before each NAL unit

	if (md_video_tk == MP4_INVALID_TRACK_ID)
	{
		LOG("add video track fialed.\n");
		return -1;
	}
	MP4SetVideoProfileLevel(md_record_handle, 0x7F);

	int audio_time_scale = AUDIO_SAMPLERATE_YSX;  // samplerate
	md_audio_tk = MP4AddAudioTrack(md_record_handle, audio_time_scale, 1024, MP4_MPEG4_AUDIO_TYPE);
	if (md_audio_tk == MP4_INVALID_TRACK_ID)
	{
		LOG("add audio track fialed.\n");
		return -1;
	}

	MP4SetAudioProfileLevel(md_record_handle, 0x2);
	MP4SetTrackESConfiguration(md_record_handle,md_audio_tk,confBUF,2);

	MP4AddH264SequenceParameterSet(md_record_handle,md_video_tk,sps,23);
	MP4AddH264PictureParameterSet(md_record_handle,md_video_tk,pps,4);

	memset(&md_lst_vid,0,sizeof(struct timeval));
	memset(&md_lst_aud,0,sizeof(struct timeval));
	wait_keyframe = 1;
	start_timestamp = 0;
	return 0;
}

int md_write_video_frame(int keyframe,uint8_t *buf, int len ,const struct timeval *tv)
{
	int ret = 0;
	NaluUnit naluUnit;
	int offset = 0;
	int size = 0;
	pthread_mutex_lock(&md_record_Mutex);
	if(wait_keyframe == 1){
		QCamVideoInput_SetIFrame(MAIN_CHN);	
		if(keyframe == 0)
		{
			pthread_mutex_unlock(&md_record_Mutex);
			return 0;
		}
		else {	
			start_timestamp = tv->tv_sec*1000 + tv->tv_usec/1000;
			wait_keyframe = 0;
		}
	}
	if(md_record_handle != NULL)
	{
		MP4Duration tmp = 0;

		//		struct timeval tv;
		//		gettimeofday(&tv,NULL);
		uint64_t t_diff;

		t_diff = abs((tv->tv_sec-md_lst_vid.tv_sec)*1000000+tv->tv_usec-md_lst_vid.tv_usec);

		if(md_lst_vid.tv_sec <= 0 )
			tmp = 90000*1/H264_FRAME_RATE;
		else if(t_diff>=(1000000 * 1))
		{
			tmp = 90000*1/10;
		}
		else
			tmp = 90*t_diff/1000 ;//   90000 / 100000 = 90 / 1000*/

		//		printf("vid duration = %ld\n",tmp);
		memcpy(&md_lst_vid,tv,sizeof(struct timeval));

		while(size = ReadOneNaluFromBuf_record(&naluUnit,buf+offset,len-offset))
		{
			int data_len = naluUnit.size + 4;
			unsigned char * data = (unsigned char *)mem_malloc(data_len);
			data[0] = naluUnit.size>>24;
			data[1] = naluUnit.size>>16;
			data[2] = naluUnit.size>>8;
			data[3] = naluUnit.size&0xff;
			memcpy(data+4,naluUnit.data,naluUnit.size);

			if( !MP4WriteSample(md_record_handle, md_video_tk,data,data_len,tmp , 0, 1) )
			{
				LOG("Error while writing video frame \n");
				if(md_err_cnt++ > 10 )
				{
					md_err_cnt = 0;
					ret = -1;
				}

			}

			if(naluUnit.data)
				mem_free(naluUnit.data);

			if(data)
				mem_free(data);

			memset(&naluUnit,0,sizeof(NaluUnit));

			offset += size;
			if(offset >= len)
				break;
		}

	}
	else
	{
		ret = -1;
	}

	pthread_mutex_unlock(&md_record_Mutex);
	return ret;
}

int md_write_audio_frame(uint8_t *buf, int len,const struct timeval *tv)
{
	int ret = 0;
	pthread_mutex_lock(&md_record_Mutex);
	if(md_record_handle != NULL)
	{
		MP4Duration tmp = 0;

		uint64_t t_diff;

		t_diff = abs((tv->tv_sec-md_lst_aud.tv_sec)*1000000+tv->tv_usec-md_lst_aud.tv_usec);
		if(md_lst_aud.tv_sec <= 0 )
			tmp = 16000/1024;
		else if(t_diff>=(1000000 * 1))
		{
			tmp = 16000/1024;
		}
		else
			tmp = 16*t_diff/1000 ; 

		//		printf("vid duration = %ld\n",tmp);
		memcpy(&md_lst_aud,tv,sizeof(struct timeval));
		if( ! MP4WriteSample(md_record_handle, md_audio_tk,&buf[7],len-7,tmp, 0, 1))
		{
			LOG("Error while writing audio frame \n");

			if(md_err_cnt++ > 10 )
			{
				md_err_cnt = 0;
				ret = -1;
			}
		}
	}
	else
		ret = -1;

	pthread_mutex_unlock(&md_record_Mutex);
	return ret;
}

int md_record_close()
{
	//	LOG("close mp4 record ...\n");
	if(md_record_handle)
	{
		MP4Close(md_record_handle,0);
		md_record_handle = NULL;
	}
	md_audio_tk = MP4_INVALID_TRACK_ID;
	return 0;
}

