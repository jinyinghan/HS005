#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>   
#include <sys/stat.h>   
#include <fcntl.h>
#include <ctype.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>
#include <stdarg.h>


typedef struct _recordindex
{
	unsigned int iy;
	char im;
	char id;
	char ih;
	char imi;
	unsigned int timelen;
}RIndex;
#define RECORD_INDEX   	  		"/tmp/rindex"		// "./CameraVideo" //
#define RECORD_INDEX_TMP  		"/tmp/rindex_tmp"


//#include "Common_func.h"
#define CLEAR(x) memset(x,0,sizeof(x))

FILE *stdout_fp;
#define _CMD_LEN    (256)

#if 0
static void _close_all_fds (void)
{
    int i;
    for (i = 0; i < sysconf(_SC_OPEN_MAX); i++) {
      if (i != STDIN_FILENO && i != STDOUT_FILENO && i != STDERR_FILENO)
        close(i);
    }
}
extern int __libc_fork (void);
static int _system (char *command)
{
    int pid = 0;
    int status = 0;
    char *argv[4];
    extern char **environ;

    if (NULL == command) {
        return -1;
    }

    pid = __libc_fork();        /* vfork() also works */
    if (pid < 0) {
        return -1;
    }
    if (0 == pid) {             /* child process */
        _close_all_fds();       /* è¿™æ˜¯æˆ‘è‡ªå·±å†™çš„ä¸€ä¸ªå‡½æ•°ï¼Œç”¨æ¥å…³é—­æ‰€æœ‰ç»§æ‰¿çš„æ–‡ä»¶æè¿°ç¬¦ã€‚å¯ä¸ç”¨ */
        argv[0] = "sh";
        argv[1] = "-c";
        argv[2] = command;
        argv[3] = NULL;

        execve ("/bin/sh", argv, environ);    /* execve() also an implementation of exec() */
        _exit (127);
    }

    // else
    /* wait for child process to start */
    while (waitpid(pid, &status, 0) < 0)
        if (errno != EINTR) {
            status = -1; /* error other than EINTR from waitpid() */
            break;
        }

    return (status);
}

int AMCSystemCmd (const char *format, ...)
{
    char cmdBuff[_CMD_LEN];
    va_list vaList;
	int i_ret = 0;
	int i_dbg = 0;	
    va_start (vaList, format);
    vsnprintf ((char *)cmdBuff, sizeof(cmdBuff), format, vaList);
    va_end (vaList);
	
    i_ret = _system ((char *)cmdBuff);
    if ((i_ret) && (i_dbg == 1))
       printf("err %s, cmd: %s\n",strerror(errno), cmdBuff);
	
    return i_ret;
}
#endif
int remove_tmp_file(char *path)
{
	char sys_cmd[256];
	CLEAR(sys_cmd);
	snprintf(sys_cmd,sizeof(sys_cmd),"rm %stmp-* 2>/dev/null",path);
//	LOG("delet tmp --%s--\n",sys_cmd);
	AMCSystemCmd(sys_cmd);

	return 0;
}
int empty_dir(char *dirpath)
{
	DIR *dirp;
	int num=0;
	printf("dirpath:%s\n",dirpath);
	dirp = opendir(dirpath);
	while(dirp) {
		if (readdir(dirp) != NULL)
			++num;
		else
		{
			closedir(dirp);
			break;
		}
	}
	printf("num:%d\n",num);
	return num;
}

void std2file()
{       
   if(access("/tmp/sd_ok",F_OK) == 0 )
    {
        printf("### Change LOG destination\n");
        char buf[128];
        FILE *fp = fopen("/tmp/sd_ok", "r");
        CLEAR(buf);
        int ret = fread(buf, 1, sizeof(buf), fp);
        buf[ret-1]='\0';
        fclose(fp);
      
        strcat(buf,"/camera_log.txt");
        stdout_fp = freopen(buf, "a+", stdout);  
		setvbuf(stdout_fp,NULL,_IOLBF ,1024);           
    }
}
void file2std()
{
 
	stdout_fp = freopen("/dev/console", "w", stdout);
  
}
int del_oldest_record(const char *path)
{
	int ret;
	char index_name[64],buf[128],index_tmp[64];
	FILE *fp , *fp_tmp;
	RIndex tm_info;
	printf("##### DEL RECORD FILE ##### \n");

	CLEAR(index_name);
	snprintf(index_name,sizeof(index_name),"%s",RECORD_INDEX);
	fp = fopen(RECORD_INDEX,"rb");
	if(NULL == fp)
	{
		//LOG("Failed to open %s\n",index_name);
		return -1;
	}

	fseek(fp,0,SEEK_SET);
	ret = fread(&tm_info, sizeof(RIndex), 1, fp);
	if(ret <= 0){
		//LOG("Empty index file\n");
		return -1;
	}
//	dump_index(tm_info);

	CLEAR(index_tmp);
	snprintf(index_tmp,sizeof(index_tmp),"%s",RECORD_INDEX_TMP);
	fp_tmp = fopen(index_tmp,"wb");
	if(NULL == fp_tmp)
	{
		//LOG("Failed to open %s\n",index_tmp);
		return -1;
	}

	char temp[512] = {'\0'};
	if(NULL == temp)
	{
		//LOG("malloc for temp error %s\n",strerror(errno));
		return -1;
	}

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

	chmod(buf, S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IWGRP|S_IXGRP|S_IROTH|S_IWOTH|S_IXOTH);
	printf("%s\n",buf);
	//LOG("remove file %s\n",buf);
	if(remove(buf) != 0)
		perror("remove file\n");
	char *start = rindex(buf,'/');
	printf("start:%s\n",start);//     /1970_01_01_08_02_00.mp4
	int len = strlen(start)-1;
	memset(start+1,0,len);
	remove_tmp_file(buf);	/*å…ˆåˆ é™¤ä¸´æ—¶æ–‡ä»¶*/
	if(empty_dir(buf) == 2)  //ÎªÊ²Ã´ÊÇ2¾ÍÉ¾³ýÒòÎªÓÐ.ºÍ..2¸öÎÄ¼þ
		rmdir(buf);

	start = rindex(buf,'/');
	len = strlen(start)-1;
	memset(start+1,0,len);
	if(empty_dir(buf) == 2)//Õâ¸öÊÇÒÆ³ý08 ,09ÉÏÒ»²ãÄ¿Â¼
		rmdir(buf);

	return 0;
}

int main()
{
    
 del_oldest_record("/tmp/mmcblk0p1");

}
