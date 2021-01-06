#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/prctl.h>
#include <semaphore.h>
#include <pthread.h>
#include <stdbool.h>



#include "audio_shm.h"

int get_sig_flag = 0;
int deamon_shm_fd = 0;
void *deamon_shm_buffer = NULL;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  cond = PTHREAD_COND_INITIALIZER;

sem_t *deamon_sem_global;

int deamon_shm_init(void)
{
	int ret = -1;
	/* init shm */
	deamon_shm_fd = shm_open(SHM_NAME, SHM_OPEN_FLAG, SHM_OPEN_MODE);
	if (-1 == deamon_shm_fd) {
		DP("err: shm_open err %s, %s\n", strerror(errno), SHM_NAME);
		goto err_shm_open;
	}
	ret = ftruncate(deamon_shm_fd, SHM_SIZE);
	if (-1 == ret) {
		DP("err: ftruncate err %s\n", strerror(errno));
		goto err_ftruncate;
	}
	deamon_shm_buffer = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, deamon_shm_fd, 0);
	if (MAP_FAILED == deamon_shm_buffer) {
		DP("err: mmap err %s\n", strerror(errno));
		goto err_mmap;
	}

	/* init sem */	
	deamon_sem_global = sem_open(SEM_GLOBAL_NAME, O_CREAT | O_EXCL, 0666, 0);
	if (SEM_FAILED == deamon_sem_global) {

		if (EEXIST == errno) {
			sem_unlink(SEM_GLOBAL_NAME);
			DP("info: sem_unlink %s\n", SEM_GLOBAL_NAME);
			deamon_sem_global = sem_open(SEM_GLOBAL_NAME, O_CREAT | O_EXCL, 0666, 0);
			if (SEM_FAILED == deamon_sem_global) {
				DP("err: sem_open global err %s\n", strerror(errno));
				goto err_sem_open_global;
			}
		} else {
			DP("err: sem_open global err %s\n", strerror(errno));
			goto err_sem_open_global;
		}
	}

	return 0;
err_sem_open_global:
err_mmap:
err_ftruncate:
err_shm_open:
	return -1;
}

int deamon_shm_deinit(void)
{
	int ret = -1;
	/* deinit shm */
	if (NULL != deamon_shm_buffer) {
		ret = munmap(deamon_shm_buffer, SHM_SIZE);
		if (-1 == ret) {
			DP("err: munmap err %s\n", strerror(errno));
			goto err_munmap;
		}
	}
	ret = shm_unlink(SHM_NAME);
	if (-1 == ret) {
		DP("err: shm_unlink err %s\n", strerror(errno));
		goto err_shm_unlink;
	}

	/* deinit sem */
	if (0 != sem_close(deamon_sem_global)) {
		DP("err: sem_close global err %s\n", strerror(errno));
		goto err_sem_global_close;
	}
	if (0 != sem_unlink(SEM_GLOBAL_NAME)) {
		DP("err: sem_unlink toc err %s\n", strerror(errno));
		goto err_sem_global_unlink;
	}

	return 0;
err_sem_global_unlink:
err_sem_global_close:
err_shm_unlink:
err_munmap:
	return -1;
}

void *thr_ysx_pa_control()
{
	int ret;
	audio_deamon_fmt *shm = (audio_deamon_fmt *)deamon_shm_buffer;
	
	while(1)
	{
		sleep(1);
		pthread_mutex_lock(&lock);
		if(get_sig_flag == 1 && shm->status == AUDIO_OUT_OPEN && abs(time(NULL)-shm->lst_time) > 3)
		{		
			DP("close .....PA \n");	
			get_sig_flag = 0;
			shm->status = AUDIO_OUT_CLOSE;
		}
		pthread_mutex_unlock(&lock);		
	}
}

int main(int argc, char **argv)
{
	int ret = -1;
	audio_deamon_fmt *deamon_fmt;
	int done_cnt=0;
	int aout_driver_mode = 2;
	int ch,i;
	time_t old_time= 0;
	pthread_t tid;
	opterr = 0;
	while ((ch = getopt(argc,argv,"m:"))!=-1)
	{
		switch(ch)
		{
		case 'm':
			printf("option a:'%s'\n",optarg);
			aout_driver_mode = atoi(optarg);
			if ((aout_driver_mode < 1) || (aout_driver_mode > 4)) {
				printf("err: should set mode 1-4\n");
				return -1;
			}
			break;
		default:
			printf("err: option not support :%c\n",ch);
			break;
		}
	}
	printf("info: aout driver mode %d\n", aout_driver_mode);

	
	ret = deamon_shm_init();
	if (0 != ret) {
		DP("err: deamon_shm_init\n");
		return -1;
	}
	
	if (NULL == deamon_shm_buffer) {
		DP("err: deamon_shm_buffer = NULL\n");
		DP("err: deamon exit\n");
		goto err;
	}
	deamon_fmt = (audio_deamon_fmt *)deamon_shm_buffer;
	deamon_fmt->use_cnt = 0;
	deamon_fmt->status  =AUDIO_OUT_CLOSE;
	printf("-------- ysx deamon start --------\n\n");

	while(1) {
		ret = sem_wait(deamon_sem_global);
		if (0 != ret) {
			DP("err: sem_wait %s\n", strerror(errno));
		}
		old_time = deamon_fmt->lst_time;
		for(i=0;i<5;i++)
		{
			sleep(1);
			if(old_time != deamon_fmt->lst_time)
				break;
			DP("####lst time = %d , shm time =%d\n",old_time,deamon_fmt->lst_time);
		}
		if(i != 5)
			continue;
		
//		if(deamon_fmt->use_cnt <= 0)
		{
			deamon_fmt->use_cnt = 0;
			deamon_fmt->play_done = 1;
			deamon_fmt->status = AUDIO_OUT_CLOSE ;		
			system("echo 0 > /proc/jz/aw8733a/mode");		
			DP("CLOSE AW8733 !\n");
		}
	}

	deamon_shm_deinit();
	return 0;
err:
	deamon_shm_deinit();	
	return -1;
}
