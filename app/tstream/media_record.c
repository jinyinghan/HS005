/*******************************************************************************
* write at SZ, 2019-03-14 17:56:58, by yuanhao
* media_record.c
* mp4v2 used
* mp4(audio, video) record
* mp4(audio, video) play
* rindex used
*******************************************************************************/
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/statfs.h>
#include <dirent.h>
#include <sys/time.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <math.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <unistd.h>  
#include <pthread.h>  
#include "include/common_env.h"
#include "include/common_func.h"
#include "media_record.h"
#include "ysx_data_buf.h"


long long t_sdtotal = 0;
long long t_sdfree = 0;
struct tm curr_time={0};

typedef struct media_record_attr_s
{
	s_record_info record_info;
	av_buffer *video_pool;
	av_buffer *audio_pool;
    pthread_t audio_pid;
	pthread_t video_pid;
    pthread_mutex_t audio_mutex;
	pthread_mutex_t video_mutex;
	MP4FileHandle mp4_handle;
	FILE *aac_fp;
	MP4TrackId video_tk;
	MP4TrackId audio_tk;
	struct timeval lst_vid;
	struct timeval lst_aud;
	unsigned long int file_start_time;
	unsigned long int record_start_time;
	int err_cnt;
	char filename[128];
    unsigned char running;
	unsigned char start;
	unsigned char wait_keyframe;
	unsigned char cur_record_type;
	unsigned char lst_record_type;
}
MEDIA_RECORD_ATTR_S;
static MEDIA_RECORD_ATTR_S g_record_args[RECORD_CHANNEL3];

#define EMERGENCY_DEBUG
#ifdef  EMERGENCY_DEBUG
#define RED            "\033[0;32;31m"
#define LIGHT_RED      "\033[1;31m"
#define NONE           "\033[m"
#define WHITE          "\033[1;37m"

#define RECORD_LOG(fmt...)   \
	do {\
	    printf(LIGHT_RED"[%s] [%s] [%d]: "NONE,  \
        __FILE__, \
        __FUNCTION__,  \
        __LINE__);\
        printf(fmt);\
	}while(0)  
#endif 

#define STR_SIZE sizeof("rwxrwxrwx")
static int Check_Record_FilePerm(char *filename, e_record_type itype)
{
	struct stat file_stat;
	struct tm tb;
	char file_name[128] = {'\0'};
	char file_path[32] = {'\0'};

	if(itype == RECORD_TYPE_MP4)
		strcpy(file_path, SD_RECORD_PATH);
	else if(itype == RECORD_TYPE_AAC)
		strcpy(file_path, FLASH_RECORD_PATH);
	
	const char fmt[] = "%Y_%m_%d_%H_%M_%S";  
	if (strptime(filename, fmt, &tb)) {
		sprintf(file_name, "%s/YsxCam/%04d-%02d-%02d/%02d/%s",
							file_path, 1900+tb.tm_year, tb.tm_mon+1, 
							tb.tm_mday, tb.tm_hour, filename);
	}

	stat(file_name, &file_stat);

	static char str[STR_SIZE];
    snprintf(str, STR_SIZE, "%c%c%c%c%c%c%c%c%c",
        (file_stat.st_mode & S_IRUSR) ? 'r' : '-', (file_stat.st_mode & S_IWUSR) ? 'w' : '-',
        (file_stat.st_mode & S_IXUSR) ? 'x' : '-',
       
        (file_stat.st_mode& S_IRGRP) ? 'r' : '-', (file_stat.st_mode & S_IWGRP) ? 'w' : '-',
        (file_stat.st_mode & S_IXGRP) ? 'x' : '-',
    
        (file_stat.st_mode & S_IROTH) ? 'r' : '-', (file_stat.st_mode & S_IWOTH) ? 'w' : '-',
        (file_stat.st_mode & S_IXOTH) ? 'x' : '-');

	if(strncmp("r-xr-xr-x", str, STR_SIZE) == 0)
		return 1;
	else
		return 0;
	
}

//for rindex start
/*按时间顺序索引文件*/
int Create_Record_Index(char *basePath, FILE *fIndex, e_record_type itype)
{
    struct dirent **namelist,*ptr;
    struct stat s_buf;
    int n;
    char nameTemp[128];
    RECORD_LOG("%s\n", basePath);
    n = scandir(basePath, &namelist, NULL, alphasort);
    if (n < 0) {
        RECORD_LOG("scan dir error:%s\n",strerror(errno));
        return -1;
    }

    int i;
    for(i=0;i<n;i++) {
        ptr = namelist[i];
        if(strcmp(ptr->d_name,".")==0 || strcmp(ptr->d_name,"..")==0)     ///current dir OR parrent dir
            continue;

        memset(nameTemp,0,sizeof(nameTemp));
        strcpy(nameTemp,basePath);
        strcat(nameTemp,"/");
        strcat(nameTemp,ptr->d_name);

        stat(nameTemp,&s_buf);
        if(S_ISDIR(s_buf.st_mode)) {
            Create_Record_Index(nameTemp, fIndex, itype);
        }
        else if(S_ISREG(s_buf.st_mode)) {
//			RECORD_LOG("###file_name:%s###\n", ptr->d_name);
			if(Check_Record_FilePerm(ptr->d_name, itype))
			{
            	RIndex rtmp;
            	struct tm tb;
				
				RECORD_LOG("###file_name:%s###\n", ptr->d_name);
            	const char fmt[] = "%Y_%m_%d_%H_%M_%S";                 //"2000_02_01_00_09_33.mp4";
            	if (strptime(ptr->d_name, fmt, &tb)) {
                    rtmp.itype = itype;
                    rtmp.iy = 1900+tb.tm_year;
                    rtmp.im = tb.tm_mon+1;
                    rtmp.id = tb.tm_mday;
                    rtmp.ih = tb.tm_hour;
                    rtmp.imi = tb.tm_min;
                    rtmp.isec = tb.tm_sec;
                    fwrite(&rtmp, sizeof(RIndex), 1, fIndex);
            	}
			}
        }

        free(ptr);
    }
    free(namelist);
    return 0;
}

int Sync_Record_Index(char *path, e_record_type itype, int index)
{
    RECORD_LOG("Creat New record index\n");
    g_record_args[index].err_cnt = 0;
    char tmp[128];
    FILE *fp = NULL;
    int i_ret = -1;

    fp = fopen(path,"wb");
    if(NULL == fp) {
        RECORD_LOG("#Create Record Index File error#\n");
		RECORD_LOG("#Record Path %s#\n", path);
        return -1;
    }

    CLEAR(tmp);
    sprintf(tmp,"%s/YsxCam/", SD_RECORD_PATH);
	if(itype == RECORD_TYPE_AAC){
		CLEAR(tmp);
    	sprintf(tmp,"%s/YsxCam/", FLASH_RECORD_PATH);
	}
		
    i_ret = access(tmp, F_OK);
    if(i_ret == 0) {
        Create_Record_Index(tmp, fp, itype) ;
    }
    fclose(fp);

    return 0;
}

void Printf_Record_Index(RIndex temp)
{
    RECORD_LOG("iy = %d ",temp.itype);
    RECORD_LOG("iy = %d ",temp.iy);
    RECORD_LOG("im = %d ",temp.im);
    RECORD_LOG("id = %d ",temp.id);
    RECORD_LOG("ih = %d ",temp.ih);
    RECORD_LOG("imi = %d ",temp.imi);
    RECORD_LOG("timelen = %d ",temp.isec);
    RECORD_LOG("\n");
    return;
}

int Save_Record_Index(e_record_type itype, int index)
{
    FILE *fp = NULL;
    char filename[256];

    if(strlen(g_record_args[index].filename) == 0)
        return -1;

	chmod(g_record_args[index].filename, S_IRUSR|S_IXUSR|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH);

    /*sync record*/
    struct tm *p = &curr_time;
    RIndex tmp;
    tmp.itype  = itype;
    tmp.iy  = (1900+p->tm_year);
    tmp.im  = (1+p->tm_mon);
    tmp.id  = p->tm_mday;
    tmp.ih  = p->tm_hour;
    tmp.imi = p->tm_min;
    tmp.isec = p->tm_sec;

    CLEAR(filename);
    if(itype == RECORD_TYPE_MP4)
        sprintf(filename,"%s",RECORD_INDEX_SD);
    else if(itype == RECORD_TYPE_AAC)
        sprintf(filename,"%s",RECORD_INDEX_FLASH);
	
    fp = fopen(filename,"ab+");
    if(fp){
        fwrite(&tmp,sizeof(RIndex),1,fp);
        fclose(fp);
    }
	RECORD_LOG("### Save Record %s ###\n", g_record_args[index].filename);
    return 0;
}

int Get_Record_FilePath(int file_type, RIndex *file_time, char *file_name)
{
    int ret = 0;
    char record_path[32] = {'\0'};

    if(file_type == RECORD_TYPE_MP4){
		strcpy(record_path, SD_RECORD_PATH);
        sprintf(file_name, "%s/YsxCam/%04d-%02d-%02d/%02d/%04d_%02d_%02d_%02d_%02d_%02d.mp4",
            record_path,file_time->iy,file_time->im,file_time->id,
            file_time->ih,
            file_time->iy,file_time->im,file_time->id,
            file_time->ih,file_time->imi,file_time->isec);

        if(access(file_name,F_OK) != 0){
            RECORD_LOG("file %s no exist #\n",file_name);
            ret = -1;
        }
    }else if(file_type == RECORD_TYPE_AAC){
		strcpy(record_path, FLASH_RECORD_PATH);
        sprintf(file_name, "%s/YsxCam/%04d-%02d-%02d/%02d/%04d_%02d_%02d_%02d_%02d_%02d.aac",
            record_path,file_time->iy,file_time->im,file_time->id,
            file_time->ih,
            file_time->iy,file_time->im,file_time->id,
            file_time->ih,file_time->imi,file_time->isec);

        if(access(file_name,F_OK) != 0){
            RECORD_LOG("file %s no exist #\n",file_name);
            ret = -1;
        }
    }else{
        RECORD_LOG("file_type = %d is err \n",file_type);
        ret = -1;
    }

    return ret;
}

int Media_Record_Stop(void)
{
	int index;
	for(index = 0; index < RECORD_CHANNEL3; index++)
		Media_Record_UnInit(RECORD_UNINIT_PRESERVE, index);

	return 0;
}

int Check_SDCard_Exist()
{
	if(access("/tmp/sd_ok",F_OK) == 0)
		return 1;
	else
		return 0;
}

int Record_AAC_Start(int index)
{
	if(NULL == g_record_args[index].filename){
        RECORD_LOG("file name is NULL !\n");
        return -1;
    }

    if(g_record_args[index].aac_fp){
        fclose(g_record_args[index].aac_fp);
        g_record_args[index].aac_fp = NULL;
    }

    g_record_args[index].aac_fp = fopen(g_record_args[index].filename, "wb");
    if(g_record_args[index].aac_fp == NULL){
        RECORD_LOG("fopen %s err !\n", g_record_args[index].filename);
    }

    return 0;
}

void Record_AAC_Stop(int index)
{
    pthread_mutex_lock(&g_record_args[index].audio_mutex);
    if(g_record_args[index].aac_fp){
		fclose(g_record_args[index].aac_fp);	
		g_record_args[index].aac_fp = NULL;	
		g_record_args[index].start = 0;
	}
    pthread_mutex_unlock(&g_record_args[index].audio_mutex);
}

int	RecordAAC_Write_Audio(int index, char *buf, int buf_len)
{
	int ret = 0;

    pthread_mutex_lock(&g_record_args[index].audio_mutex);
    if(g_record_args[index].aac_fp){
        fwrite(buf, buf_len, 1, g_record_args[index].aac_fp);
        fflush(g_record_args[index].aac_fp);
        ret = 0;
    }else{
//        YSX_LOG(LOG_APP_ERROR, "pfile is NULL !\n");
        ret = -1;
    }
    pthread_mutex_unlock(&g_record_args[index].audio_mutex);

	return ret;
}

int ReadOneNaluFromBuf(NaluUnit *nalu, unsigned char *buf, unsigned int size)
{
	unsigned int  nalhead_pos = 0;

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
		nalu->type = buf[nalhead_pos]&0x1f; 	// 7-> pps , 8->sps
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

int Check_Record_SavePath(char *record_path)
{
	if(access(record_path, F_OK) == 0 ) {
		return 0;
    }

	return 1;
}

int Check_Record_Readonly()
{
	char buffer[64] = {'\0'};

	FILE *read_fp = popen("mount | grep mmc | grep \"(ro\"","r");
	if (read_fp != NULL)
	{
		fread(buffer, sizeof(char), 64, read_fp);
		if (strlen(buffer) != 0)
		{
			pclose(read_fp);
			RECORD_LOG("## SD Readonly ## , Disable mp4 record\n");
			RECORD_LOG("## SD Readonly ## , Disable mp4 record\n");
			RECORD_LOG("## SD Readonly ## , Disable mp4 record\n");

			remove("/tmp/sd_ok");
			return 1;
		}
	}
	pclose(read_fp);
	return 0;
}

int Record_Delete_OldFile(const char *path, e_record_type itype)
{
	int ret;
	char index_name[64],buf[128],index_tmp[64];
	FILE *fp , *fp_tmp;
	RIndex tm_info;
	RECORD_LOG("##### DEL RECORD FILE ##### \n");

    CLEAR(index_name);
    CLEAR(index_tmp);
    if(itype == RECORD_TYPE_MP4){
        snprintf(index_name,sizeof(index_name),"%s", RECORD_INDEX_SD);
        snprintf(index_tmp,sizeof(index_tmp),"%s", RECORD_INDEX_SD_TMP);
    }
    else{
        snprintf(index_name,sizeof(index_name),"%s", RECORD_INDEX_FLASH);
        snprintf(index_tmp,sizeof(index_tmp),"%s", RECORD_INDEX_FLASH_TMP);
    }
	
	fseek(fp,0,SEEK_SET);
	ret = fread(&tm_info, sizeof(RIndex), 1, fp);
	if(ret <= 0){
		RECORD_LOG("Empty index file\n");
		return -1;
	};

	char temp[512] = {'\0'};
	while(1){
		ret = fread(temp,1,512,fp);
		if(ret <= 0 )
			break;
		else
			fwrite(temp,ret,1,fp_tmp);
	}

	fclose(fp);
	fclose(fp_tmp);

	remove(index_name);
	rename(index_tmp,index_name);

	CLEAR(buf);
	snprintf(buf,sizeof(buf),"%s/YsxCam/%04d-%02d-%02d/%02d/%04d_%02d_%02d_%02d_%02d_00.mp4",
			path,tm_info.iy,tm_info.im,tm_info.id,tm_info.ih,tm_info.iy,tm_info.im,tm_info.id,tm_info.ih,tm_info.imi);
	RECORD_LOG("remove file %s\n",buf);
	if(remove(buf) != 0)
		perror("remove file\n");

	return 0;
}

int Check_Record_RemainSize(char *record_path, e_record_type itype)
{
	struct statfs diskInfo; 												//系统stat的结构体
	static int check_cnt = 0;
	
	while(1)
	{
		if(itype == RECORD_TYPE_MP4){
			if(Check_Record_Readonly())
				return RECORD_READONLY;
		}
		
		if (statfs(record_path, &diskInfo) == -1){   						//获取分区的状态
			RECORD_LOG("stafs failed: %s\n",strerror(errno));
			RECORD_LOG("statfs failed for path->[%s]\n", record_path);
			return -1;
		}
		if((check_cnt++) > 5)
		{
			RECORD_LOG("No Space For Record !\n");
			check_cnt = 0;
			return -1;
		}

	    unsigned long long blocksize = diskInfo.f_bsize;    				//每个block里包含的字节数
	    unsigned long long totalsize = diskInfo.f_blocks * blocksize >> 20;
	    unsigned long long freesize  = diskInfo.f_bfree  * blocksize >> 20; //剩余空间的大小

		t_sdtotal = totalsize;
        t_sdfree  = freesize;
//		g_enviro_struct.t_sdtotal = totalsize;
//		g_enviro_struct.t_sdfree  = freesize;

		if(freesize > 50)
		{
			check_cnt = 0;
			break;
		}

	    RECORD_LOG("Total_size = %llu MB , free_size = %llu MB \n",   totalsize , freesize);
		Record_Delete_OldFile(record_path, itype);

	}

	return 0;
}

static int Create_Multi_Dir(const char *path)
{
    int i, len;

    len = strlen(path);
    char dir_path[len+1];
    dir_path[len] = '\0';

    strncpy(dir_path, path, len);

    for (i=0; i<len; i++) {
        if (dir_path[i] == '/' && i > 0) {
            dir_path[i]='\0';
            if(access(dir_path, F_OK) != 0) {
                    if (mkdir(dir_path, 0755) < 0) {
                            RECORD_LOG("mkdir=%s:msg=%s\n", dir_path, strerror(errno));
                            return -1;
                    }
            }
            dir_path[i]='/';
        }
    }

    return 0;
}

int Get_Record_FileName(char *path, char *filename, e_record_type file_type)
{
    time_t timep;
    char dir_path[128];
    struct tm *p;

    if(NULL == filename){
        RECORD_LOG("file name is NULL\n");
        return -1;
    }

    time(&timep);
    struct tm *t= localtime(&timep); //?????????
    memcpy(&curr_time,t,sizeof(struct tm));
    p = t;

    CLEAR(dir_path);
    sprintf(dir_path,"%s/YsxCam/%04d-%02d-%02d/%02d/", path,(1900+p->tm_year) , (1+p->tm_mon), p->tm_mday,p->tm_hour);
    Create_Multi_Dir(dir_path);

    /*2016_12_14_10_22_49.mp4*/
	if(file_type == RECORD_TYPE_MP4){
    	sprintf(filename,"%s%04d_%02d_%02d_%02d_%02d_%02d.mp4",dir_path,(1900+p->tm_year),
                    (1+p->tm_mon), p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);
	}else if(file_type == RECORD_TYPE_AAC){
		sprintf(filename,"%s%04d_%02d_%02d_%02d_%02d_%02d.aac",dir_path,(1900+p->tm_year),
        			(1+p->tm_mon), p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);
	}
		
    return 0;
}

char *Get_Record_Dpi(e_video_dpi dpi, unsigned int *dpi_w, unsigned int *dpi_h)
{
    char *sps_dpi = NULL;
    switch(dpi) {
        case RECORD_DPI_1080P: {
            sps_dpi = (char *)sps_1080;
            *dpi_w = CAM_RES_INFO_1080P_W;
            *dpi_h = CAM_RES_INFO_1080P_H;
        }break;
        case RECORD_DPI_720P: {
            sps_dpi = (char *)sps_720;
            *dpi_w = CAM_RES_INFO_720P_W;
            *dpi_h = CAM_RES_INFO_720P_H;
        }break;
        case RECORD_DPI_480P: {
            sps_dpi = (char *)sps_480;
            *dpi_w = CAM_RES_INFO_480P_W;
            *dpi_h = CAM_RES_INFO_480P_H;
        }break;
//        case RECORD_DPI_360: {
//            sps_dpi = (char *)sps_360;
//            *dpi_w = CAM_RES_INFO_360P_W;
//            *dpi_h = CAM_RES_INFO_360P_H;
//        }break;
        default:
            RECORD_LOG("snap dpi not support, set to default, %s\n", dpi);
    }
    return sps_dpi;
}

int Record_MP4_Start(int index)
{
    unsigned dpi_w = 0, dpi_h = 0;
    char *sps_tmp = Get_Record_Dpi(g_record_args[index].record_info.dpi, &dpi_w, &dpi_h);

    if((NULL == g_record_args[index].filename) || (NULL == sps)){
        RECORD_LOG("file name is null\n");
        return -1;
    }
	
	pthread_mutex_lock(&g_record_args[index].video_mutex);
	g_record_args[index].mp4_handle = MP4CreateEx(g_record_args[index].filename, 0, 1, 1, 0, 0, 0, 0);
    if (g_record_args[index].mp4_handle == MP4_INVALID_FILE_HANDLE) {
        RECORD_LOG("open file fialed.\n");
        return -1;
    }
    MP4SetTimeScale(g_record_args[index].mp4_handle, 90000);
    if(RECORD_DPI_720P == g_record_args[index].record_info.dpi) {
#ifdef NALUUNIT //for naluunit work, by echo 2018-11-03 11:21:57
              g_record_args[index].video_tk = MP4AddH264VideoTrack(g_record_args[index].mp4_handle, 90000, 90000 / 15, dpi_w, dpi_h,
                                                      sps_720[1],//0x64, //sps[1],// AVCProfileIndication
                                                      sps_720[2],//0x00, //sps[2],// profile_compat
                                                      sps_720[3],//0x1f, //sps[3],// AVCLevelIndication
                                                      3); // 4 bytes length before each NAL unit
              MP4SetVideoProfileLevel(g_record_args[index].mp4_handle, sps_720[1]);
#else
              g_record_args[index].video_tk = MP4AddH264VideoTrack(g_record_args[index].mp4_handle, 90000, 90000 / 15, dpi_w, dpi_h,
                                                      0x64, //sps[1],// AVCProfileIndication
                                                      0x00, //sps[2],// profile_compat
                                                      0x1f, //sps[3],// AVCLevelIndication
                                                      3); // 4 bytes length before each NAL unit
              MP4SetVideoProfileLevel(g_record_args[index].mp4_handle, 0x7F);
#endif
              if (g_record_args[index].video_tk == MP4_INVALID_TRACK_ID) {
                    RECORD_LOG("add video track fialed.\n");
                    return -1;
              }

              int audio_time_scale = AUDIO_SAMPLERATE_YSX;  // samplerate
              g_record_args[index].audio_tk = MP4AddAudioTrack(g_record_args[index].mp4_handle, audio_time_scale, (SAMPLE_RATE_K/8)*512, MP4_MPEG4_AUDIO_TYPE);
              if (g_record_args[index].audio_tk == MP4_INVALID_TRACK_ID) {
                  RECORD_LOG("add audio track fialed.\n");
                  return -1;
              }
              MP4SetAudioProfileLevel(g_record_args[index].mp4_handle, 0x2);
#ifdef NALUUNIT //for naluunit work, by echo 2018-11-03 11:21:57
              MP4SetTrackESConfiguration(g_record_args[index].mp4_handle, g_record_args[index].audio_tk, confBUF,2);

              MP4AddH264SequenceParameterSet(g_record_args[index].mp4_handle, g_record_args[index].video_tk, sps_720, SPS_LEN);
              MP4AddH264PictureParameterSet(g_record_args[index].mp4_handle, g_record_args[index].video_tk, pps, PPS_LEN);
#endif
      } else {
#ifdef NALUUNIT //for naluunit work, by echo 2018-11-03 11:21:57
        g_record_args[index].video_tk = MP4AddH264VideoTrack(g_record_args[index].mp4_handle, 90000, 90000 / 15, dpi_w, dpi_h,
                                                sps_480[1],//0x64, //sps[1],// AVCProfileIndication
                                                sps_480[2],//0x00, //sps[2],// profile_compat
                                                sps_480[3],//0x1f, //sps[3],// AVCLevelIndication
                                                3); // 4 bytes length before each NAL unit
        MP4SetVideoProfileLevel(g_record_args[index].mp4_handle, sps_480[1]);
#else
        g_record_args[index].video_tk = MP4AddH264VideoTrack(g_record_args[index].mp4_handle, 90000, 90000 / 15, dpi_w, dpi_h,
                                                0x64, //sps[1],// AVCProfileIndication
                                                0x00, //sps[2],// profile_compat
                                                0x1f, //sps[3],// AVCLevelIndication
                                                3); // 4 bytes length before each NAL unit
        MP4SetVideoProfileLevel(g_record_args[index].mp4_handle, 0x7F);
#endif
        if (g_record_args[index].video_tk == MP4_INVALID_TRACK_ID) {
              RECORD_LOG("add video track fialed.\n");
              return -1;
        }

        int audio_time_scale = AUDIO_SAMPLERATE_YSX;  // samplerate
        g_record_args[index].audio_tk = MP4AddAudioTrack(g_record_args[index].mp4_handle, audio_time_scale, (SAMPLE_RATE_K/8)*512, MP4_MPEG4_AUDIO_TYPE);
        if (g_record_args[index].audio_tk == MP4_INVALID_TRACK_ID) {
            RECORD_LOG("add audio track fialed.\n");
            return -1;
        }
        MP4SetAudioProfileLevel(g_record_args[index].mp4_handle, 0x2);
#ifdef NALUUNIT //for naluunit work, by echo 2018-11-03 11:21:57
        MP4SetTrackESConfiguration(g_record_args[index].mp4_handle, g_record_args[index].audio_tk, confBUF, 2);

        MP4AddH264SequenceParameterSet(g_record_args[index].mp4_handle, g_record_args[index].video_tk, sps_480, SPS_LEN);
        MP4AddH264PictureParameterSet(g_record_args[index].mp4_handle, g_record_args[index].video_tk, pps, PPS_LEN);
#endif
      }
	  
    memset(&g_record_args[index].lst_vid, 0, sizeof(struct timeval));
    memset(&g_record_args[index].lst_aud, 0, sizeof(struct timeval));
    g_record_args[index].wait_keyframe = 1;
	pthread_mutex_unlock(&g_record_args[index].video_mutex);
	
    return 0;
}

void Record_MP4_Stop(int index)
{
    pthread_mutex_lock(&g_record_args[index].video_mutex);
    if(g_record_args[index].mp4_handle){
		MP4Close(g_record_args[index].mp4_handle, 0);
		g_record_args[index].mp4_handle = NULL;	
		g_record_args[index].start = 0;
	}
    pthread_mutex_unlock(&g_record_args[index].video_mutex);
}

int RecordMP4_Write_Video(const int index, const int keyframe, 
			const uint8_t *vid_data, const int vid_len, const struct timeval *tv)
{
    int ret = 0;
    uint8_t *buf = (uint8_t *)vid_data;
    static unsigned int invalid_frame_cnt = 0;

    MP4Duration tmp = 0;
    long l_tmp = -1, len = vid_len;
    long long t_diff = -1;
    struct timeval *s_tv = NULL;
    s_tv = (struct timeval *)tv;

    pthread_mutex_lock(&g_record_args[index].video_mutex);
    if(keyframe == 1) {
    //I frame
        if(g_record_args[index].wait_keyframe == 1) {
            g_record_args[index].wait_keyframe = 0;
            invalid_frame_cnt = 0;
        }
    } else {
    //p frame
        if(g_record_args[index].wait_keyframe == 1) {
            pthread_mutex_unlock(&g_record_args[index].video_mutex);
            invalid_frame_cnt++ ;
//            RECORD_LOG("no I frame, cnt %d\n",invalid_frame_cnt);
            return 1;
        }
    }
	
    if(g_record_args[index].mp4_handle != NULL) {
        t_diff = s_tv->tv_sec-g_record_args[index].lst_vid.tv_sec;
        if(t_diff <= 1) {
            t_diff = t_diff*1000000 + s_tv->tv_usec - g_record_args[index].lst_vid.tv_usec;
        } else {
            t_diff = 2*1000000;
        }
//        YSX_LOG(LOG_APP_DEBUG, "%d, %u\n", offset, t_diff);
        memcpy(&g_record_args[index].lst_vid,s_tv,sizeof(struct timeval));
        if(t_diff <= 0 ) {
            l_tmp = 90000*1/H264_FRAME_RATE;
        } else if(t_diff>=(1000000 * 1)) {
            l_tmp = 90000*1/10;
        } else {
            l_tmp = 90*t_diff/1000 ;//   90000 / 100000 = 90 / 1000
        }
//        YSX_LOG(LOG_APP_DEBUG, "vid duration = %ld\n", l_tmp);
        if(len >= 4) {
            uint32_t cov_len  = htonl(len -4);
            int i = 0;
            for(i=0; i<4; i++) {
                buf[i] = (cov_len >> (8*i)) & 0xff;
            }
        }
        if( !MP4WriteSample(g_record_args[index].mp4_handle, g_record_args[index].video_tk, buf, len, l_tmp, 0, 1) ) {
            RECORD_LOG("Error while writing video frame \n");
            if(g_record_args[index].err_cnt++ > 10 )
                ret = -1;
        }
    }
    else {
        ret = -2;
    }
	
    pthread_mutex_unlock(&g_record_args[index].video_mutex);
    return ret;
}

int RecordMP4_Write_Audio(const int index, const uint8_t *buf, const int len, const struct timeval *tv)
{
	int ret = 0;
	struct timeval *s_tv = NULL;
	s_tv = (struct timeval *)tv;
	
	pthread_mutex_lock(&g_record_args[index].audio_mutex);
	if(g_record_args[index].mp4_handle != NULL) {
		if(!MP4WriteSample(g_record_args[index].mp4_handle, g_record_args[index].audio_tk, buf, len, MP4_INVALID_DURATION, 0, 1)) {
			RECORD_LOG( "Error while writing audio frame \n");
			if(g_record_args[index].err_cnt++ > 10 )
				ret = -1;
		}
	}
	else
		ret = -2;
	pthread_mutex_unlock(&g_record_args[index].audio_mutex);
	return ret;
}

void *Audio_Record_Proc(void *args)
{
	int ret;
	int index;
	index = (int) (*((int*)args));

	char pr_name[64];
    memset(pr_name,0,sizeof(pr_name));
    sprintf(pr_name,"Audio_Record_Proc[%d]", index);
    prctl(PR_SET_NAME,pr_name);

	char *tmp_data = NULL;
	int tmp_size = 0;
	int tmp_keyframe = 0;
	struct timeval tmp_tv;
	unsigned char tmp_frame_num = 0;

	clean_buffer_data(g_record_args[index].audio_pool);

	while(g_record_args[index].running){
		pthread_mutex_lock(&g_record_args[index].audio_mutex);
		read_buffer_data(g_record_args[index].audio_pool, &tmp_data, &tmp_size, &tmp_keyframe, &tmp_tv, &tmp_frame_num);
		pthread_mutex_unlock(&g_record_args[index].audio_mutex);
		
		if(!Check_SDCard_Exist()){
			if(tmp_size) {
				if(tmp_data) {
					if(!g_record_args[index].record_start_time)
						g_record_args[index].record_start_time = tmp_tv.tv_sec;	
				
					if(g_record_args[index].cur_record_type != RECORD_TYPE_AAC){
						g_record_args[index].cur_record_type = RECORD_TYPE_AAC;
						Record_MP4_Stop(index);
					}
					
					if(abs(tmp_tv.tv_sec - g_record_args[index].record_start_time) > (g_record_args[index].record_info.total_len-1)){
						RECORD_LOG("### tmp_time=%ld ###\n", tmp_tv.tv_sec);
						RECORD_LOG("### record_time=%ld ###\n",  g_record_args[index].record_start_time);
						RECORD_LOG("### record end ###\n");
						Record_AAC_Stop(index);
						ret = Save_Record_Index(RECORD_TYPE_AAC, index);
						g_record_args[index].record_info.status_cb(RECORD_END);
						break;
					}
				
					if(!g_record_args[index].start){
						g_record_args[index].start = 1;
						if(!Check_Record_RemainSize(FLASH_RECORD_PATH, RECORD_TYPE_AAC))
						{								
							memset(g_record_args[index].filename, 0, 128);
							g_record_args[index].file_start_time = tmp_tv.tv_sec;
							Get_Record_FileName(FLASH_RECORD_PATH, g_record_args[index].filename, RECORD_TYPE_AAC);
        					RECORD_LOG("### new record %s ###\n",g_record_args[index].filename);
        					if(!Record_AAC_Start(index)){
								ret = RecordAAC_Write_Audio(index, tmp_data, tmp_size);
								if(ret < 0){
									g_record_args[index].record_info.status_cb(RECORD_WRITE_ERR);
									Record_AAC_Stop(index);
								}
        					}
						}
						else
							g_record_args[index].start = 0;
					}
					else{
						if(abs(tmp_tv.tv_sec - g_record_args[index].file_start_time) > (g_record_args[index].record_info.file_len-1)){
							Record_AAC_Stop(index);
							ret = Save_Record_Index(RECORD_TYPE_AAC, index);
							if(!ret){
								RECORD_LOG("### tmp_time=%ld ###\n", tmp_tv.tv_sec);
								RECORD_LOG("### file_time=%ld ###\n",  g_record_args[index].file_start_time);
							}
						}else{
							ret = RecordAAC_Write_Audio(index, tmp_data, tmp_size);
//							RECORD_LOG("### Record Audio_len %d ###\n", tmp_size);
							if(ret < 0)
								g_record_args[index].record_info.status_cb(RECORD_WRITE_ERR);
						}
					}
				}
			}
			else
				usleep(50*1000);
		}
		else{
			if(tmp_size) {
				if(tmp_data) {
					if(g_record_args[index].cur_record_type != RECORD_TYPE_MP4){
						g_record_args[index].cur_record_type = RECORD_TYPE_MP4;
						Record_AAC_Stop(index);
					}
				
					ret = RecordMP4_Write_Audio(index, tmp_data, tmp_size, &tmp_tv);
//					RECORD_LOG("### Record Audio_len %d ###\n", tmp_size);
					if(ret < 0){
						Record_MP4_Stop(index);
						g_record_args[index].record_info.status_cb(RECORD_WRITE_ERR);
					}
				}
			}
			else
				usleep(50*1000);
		}
	}

	RECORD_LOG("Audio_Record_Proc[%d] exit\n", index);
	pthread_exit(0);
}

void *Video_Record_Proc(void *args)
{
	int ret;
	int index;
	index = (int) (*((int*)args));

	char pr_name[64];
    memset(pr_name,0,sizeof(pr_name));
    sprintf(pr_name,"Video_Record_Proc[%d]", index);
    prctl(PR_SET_NAME,pr_name);

//	if(Check_Record_SavePath(g_record_args[index].record_info.record_path))
	
	char *tmp_data = NULL;
	int tmp_size = 0;
	int tmp_keyframe = 0;
	struct timeval tmp_tv;
	unsigned char tmp_frame_num = 0;

	g_record_args[index].mp4_handle = NULL;
	g_record_args[index].aac_fp = NULL;
	g_record_args[index].video_tk = MP4_INVALID_TRACK_ID;
	g_record_args[index].audio_tk = MP4_INVALID_TRACK_ID;

	clean_buffer_data(g_record_args[index].video_pool);

	while(g_record_args[index].running){
		if(access("/tmp/index_sync",F_OK) == 0){
        	remove("/tmp/index_sync");
			Sync_Record_Index(RECORD_INDEX_SD, RECORD_TYPE_MP4, index);
    	}

		if(!Check_SDCard_Exist()){
			usleep(50*1000);
			continue;
		}

		pthread_mutex_lock(&g_record_args[index].video_mutex);
		read_buffer_data(g_record_args[index].video_pool , &tmp_data, &tmp_size, &tmp_keyframe, &tmp_tv, &tmp_frame_num);
		pthread_mutex_unlock(&g_record_args[index].video_mutex);
		
		if(tmp_size) {
			if(tmp_data) {
				if(!g_record_args[index].record_start_time)
					g_record_args[index].record_start_time = tmp_tv.tv_sec;
			
				if(abs(tmp_tv.tv_sec - g_record_args[index].record_start_time) > (g_record_args[index].record_info.total_len-1)){
					RECORD_LOG("### tmp_time=%ld ###\n", tmp_tv.tv_sec);
					RECORD_LOG("### record_time=%ld ###\n",  g_record_args[index].record_start_time);
					RECORD_LOG("### record end ###\n");
					Record_MP4_Stop(index);
					ret = Save_Record_Index(RECORD_TYPE_MP4, index);
					g_record_args[index].record_info.status_cb(RECORD_END);
					break;
				}
				
				if(!g_record_args[index].start){
					g_record_args[index].start = 1;
					ret = Check_Record_RemainSize(SD_RECORD_PATH, RECORD_TYPE_MP4);
					if(!ret){
						memset(g_record_args[index].filename, 0, 128);
						g_record_args[index].file_start_time = tmp_tv.tv_sec;
						Get_Record_FileName(SD_RECORD_PATH, g_record_args[index].filename, RECORD_TYPE_MP4);
        				RECORD_LOG("### new record %s ###\n", g_record_args[index].filename);
        				if(!Record_MP4_Start(index)){
							ret = RecordMP4_Write_Video(index, tmp_keyframe, tmp_data, tmp_size, &tmp_tv);
							if(ret < 0){
								g_record_args[index].record_info.status_cb(RECORD_WRITE_ERR);
								Record_MP4_Stop(index);
							}
        				}
					}
					else{
						g_record_args[index].start = 0;
						g_record_args[index].record_info.status_cb(RECORD_READONLY);
					}
				}
				else{
					if((tmp_tv.tv_sec - g_record_args[index].file_start_time) > (g_record_args[index].record_info.file_len-1)){
						Record_MP4_Stop(index);
						ret = Save_Record_Index(RECORD_TYPE_MP4, index);
						if(!ret){
							RECORD_LOG("### tmp_time=%ld ###\n", tmp_tv.tv_sec);
							RECORD_LOG("### file_time=%ld ###\n",  g_record_args[index].file_start_time);
						}
					}else{
						ret = RecordMP4_Write_Video(index, tmp_keyframe, tmp_data, tmp_size, &tmp_tv);
						if(ret < 0){
							Record_MP4_Stop(index);
							g_record_args[index].record_info.status_cb(RECORD_WRITE_ERR);
						}
					}
				}
			}
		}
		else{
//			RECORD_LOG("###Record NO Video###\n");
			usleep(50*1000);
		}
	}

	RECORD_LOG("Video_Record_Proc[%d] exit\n", index);
	pthread_exit(0);
}

int Media_Record_Init(s_record_info info, av_buffer *video_handle, av_buffer *audio_handle)
{
	int ret;

	if (MEDIA_RECORD_DBG){
		RECORD_LOG("record.total_len = %ld\n", info.total_len);
		RECORD_LOG("record.file_len = %ld\n", info.file_len);
		RECORD_LOG("record.dpi = %d\n", info.dpi);
	}
	
	memset(&g_record_args[info.channel], 0, sizeof(MEDIA_RECORD_ATTR_S));
	g_record_args[info.channel].audio_pool = audio_handle;
	g_record_args[info.channel].video_pool = video_handle;
	memcpy(&g_record_args[info.channel].record_info, &info, sizeof(s_record_info));

	g_record_args[info.channel].running = 1;
	g_record_args[info.channel].cur_record_type = RECORD_TYPE_MIN;
	pthread_mutex_init(&g_record_args[info.channel].audio_mutex, NULL);
	pthread_mutex_init(&g_record_args[info.channel].video_mutex, NULL);

	Sync_Record_Index(RECORD_INDEX_FLASH, RECORD_TYPE_AAC, info.channel);
	
	ret = pthread_create(&g_record_args[info.channel].video_pid, NULL, Video_Record_Proc, &g_record_args[info.channel].record_info.channel);
	if(ret < 0){
		RECORD_LOG("Creat Video_record_proc failed\n");
		return -1;
	}

	ret = pthread_create(&g_record_args[info.channel].audio_pid, NULL, Audio_Record_Proc, &g_record_args[info.channel].record_info.channel);
	if(ret < 0){
		RECORD_LOG("Creat Audio_record_proc failed\n");
		return -1;
	}

	return 0;
}

int Media_Record_UnInit(e_record_uninit_type type, e_record_channel index)
{
	g_record_args[index].running = 0;
	pthread_join(g_record_args[index].audio_pid, NULL);
	pthread_join(g_record_args[index].video_pid, NULL);

	Record_MP4_Stop(index);
	Record_AAC_Stop(index);

	pthread_mutex_destroy(&g_record_args[index].audio_mutex);
	pthread_mutex_destroy(&g_record_args[index].video_mutex);	

	memset(&g_record_args[index], 0, sizeof(MEDIA_RECORD_ATTR_S));
}

