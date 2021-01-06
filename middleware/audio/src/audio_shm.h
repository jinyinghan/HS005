#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/prctl.h>
#include <semaphore.h>
#include <pthread.h>
#include <stdbool.h>

#define SHM_NAME	"ysx_audio_deamon_shm"
#define SHM_OPEN_FLAG 	(O_RDWR|O_CREAT)
#define SHM_OPEN_MODE 	(0777)


typedef struct audio_deamon_fmt {
	int status;
	int use_cnt;
	int play_done;
	time_t lst_time;
}audio_deamon_fmt;

#define SHM_SIZE 		(sizeof(audio_deamon_fmt)+16)


#define SEM_GLOBAL_NAME "imp_audio_client_global_sem"

#if 0
#define DP(...) { printf("%s,%d", __func__, __LINE__); printf(__VA_ARGS__); }
#else
#define DP(...) 
#endif

enum {
	AUDIO_OUT_OPEN = 0,
	AUDIO_OUT_CLOSE,
};

#define WAIT_CNT 6

