#include <pthread.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <stdio.h>
#include <sys/time.h>
#include "aiengine.h"
#include "audio.h"

static char *_cfg =
		"\
{\
	\"luaPath\": \"/data/speech/lua/luabin.lub\",\
    \"appKey\": \"14327742440003c5\",\
    \"secretKey\": \"59db7351b3790ec75c776f6881b35d7e\",\
    \"provision\": \"/data/speech/bin/prov-jz.file\",\
    \"prof\": {\
		\"enable\": 0,\
		\"output\": \"a.log\"\
	},\
    \"vad\":{\
		\"enable\": 1,\
		\"res\": \"/data/speech/bin/vad.aifar.0.0.2.bin\",\
		\"speechLowSeek\": 60,\
		\"sampleRate\": 16000,\
		\"strip\": 2\
	},\
    \"cloud\": {\
		\"server\": \"ws://s.api.aispeech.com, ws://s.api.aispeech.com:1028\"\
	},\
    \"native\": {\
		\"cn.asr.rec\":{\
			 \"netBinPath\":\"/data/speech/bin/local.net.help.bin\",\
			 \"resBinPath\":\"/data/speech/bin/ebnfr.aifar.0.0.1.bin\"\
		}\
	}\
}";

int start_engiene = 1;
int speech_flag;

extern void test_native_asr(struct aiengine *agn);

static int aiengine_run = 0;
static pthread_t aiengine_start_thread;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

static inline void AIENGINE_Start_Wait()
{
	pthread_mutex_lock(&mutex);
	pthread_cond_wait(&cond, &mutex);
	pthread_mutex_unlock(&mutex);
}

static inline void AIENGINE_Start_Post()
{
	pthread_mutex_lock(&mutex);
	pthread_cond_signal(&cond);
	pthread_mutex_unlock(&mutex);
}

static void *AIENGINE_Start_Thread(void * argv)
{
	struct aiengine *agn;
	char cfg[2048] = {0};
	int len = 0;

	printf("aiengine_startup thread..\n");
	while (aiengine_run) {
		agn = aiengine_new(_cfg);
		if (agn == NULL) {
			printf("init aiengine error.\n");
			break ;
		}

		test_native_asr(agn);

		aiengine_delete(agn);

	}
	if(aiengine_run)
		AIENGINE_Start_Wait();

	pthread_exit(0);
}
int aiengine_server_startup()
{
	int ret = -1;

	speech_flag = 1;

	set_volume(0xc4);
	audio_play("/data/speech/src/start.mp3");
	if(aiengine_run == 1)
		return 0;

	aiengine_run = 1;
	ret = pthread_create(&aiengine_start_thread, NULL, AIENGINE_Start_Thread, NULL);
	if(ret != 0) {
		printf("RCF Server thread cretate error\n");
		return ret;
	}

	return 0;
}

int aiengine_server_stop()
{
	if(aiengine_run == 0)
		return 0;

	speech_flag = 0;


	aiengine_run = 0;
	AIENGINE_Start_Post();
	pthread_join(aiengine_start_thread, NULL);
	audio_play("/data/speech/src/stop.mp3");
	return 0;
}
