#include <stdio.h>
#include <stdlib.h>
#include "aiengine.h"
#include "cJSON.h"

#include "record_interface.h"

typedef enum aiengine_cmd {
	NOCMD = 0,
	PAUSE_VIDEO,
	RECOVERY_VIDEO,
	HELP
}aiengine_cmd;


extern int (*aiengine_delete_f)(struct aiengine *engine);
extern int (*aiengine_start_f)(struct aiengine *engine, const char *param,
		char id[64], aiengine_callback callback, const void *usrdata);
extern int (*aiengine_feed_f)(struct aiengine *engine, const void *data,
		int size);
extern int (*aiengine_stop_f)(struct aiengine *engine);

static char *native_asr_param = "\
	{\
		\"coreProvideType\": \"native\",\
		\"vadEnable\": 1,\
		\"audio\": {\
			\"audioType\": \"wav\",\
			\"channel\": 1,\
			\"sampleBytes\": 2,\
			\"sampleRate\": 16000\
		},\
		\"app\": {\
			\"userId\": \"test\"\
		},\
		\"request\": {\
			\"coreType\": \"cn.asr.rec\",\
		}\
	}";

static int g_vad_end = 0;
static int g_result = 0;
char g_input_buf[1024] = {0};
aiengine_cmd aicmd  = NOCMD;
int _native_asr_callback(const void *user_data, const char *id,  int type, const void *msg, int len)
{
	//printf("resp data: %.*s\n", len, (char *)msg);
	cJSON *out = cJSON_Parse((char*)msg);
	if (strstr(msg, "vad_status")) {
		cJSON *vad_status = cJSON_GetObjectItem(out, "vad_status");
		if (vad_status->valueint == 2) {
			g_vad_end = 1;
			printf("*****************vad end**********************\n");
		}
	} else {
		//printf("resp data: %.*s\n", len, (char *)msg);
		cJSON *result = cJSON_GetObjectItem(out, "result");
		if (result) {
			cJSON *input = cJSON_GetObjectItem(result, "rec");
			if(input){
				printf("\nrec:%s\n", input->valuestring);
				memset(g_input_buf, 0x0, sizeof(g_input_buf));

				if(strcmp(input->valuestring, "暂 停 录 像") == 0){
					strcpy(g_input_buf, "pause_video");
					aicmd = PAUSE_VIDEO;
				}
				else if(strcmp(input->valuestring, "恢 复 录 像") == 0){
					strcpy(g_input_buf, "recovery_video");
					aicmd = RECOVERY_VIDEO;
				}
				else if(strcmp(input->valuestring, "小 智 救 命") == 0){
					strcpy(g_input_buf, "help!!!");
					aicmd = HELP;
				}
				printf("[Action]: %s \n",g_input_buf);
			}
		}
		g_result = 1;
	}

	if (out)
		cJSON_Delete(out);

	return 0;
}

int call_MotionDetectionSwitch(struct IVSControllerImpl* p, int i);
int call_FaceDetectionSwitch(struct IVSControllerImpl* p, int i);

void jz_process_asr()
{
	struct IVSControllerImpl *p;

	audio_play("/data/speech/src/wait.mp3");

	if (aicmd == PAUSE_VIDEO) {
		audio_play("/data/speech/src/pause.mp3");
	} else if (aicmd == RECOVERY_VIDEO) {
		audio_play("/data/speech/src/recovery.mp3");
	} else if (aicmd == HELP) {
		audio_play("/data/speech/src/help.mp3");
	} else {
		printf("error unsupport cmd...");
	}
}

extern int speech_flag;
int test_native_asr(struct aiengine *agn) {
	char uuid[64] = {0};
	int ret = -1;
	int loop = 0;

	aiengine_start(agn, native_asr_param, uuid, _native_asr_callback, NULL);

	/* complete record_info struct */
	dmic_record record_info;
	memset(&record_info, 0, sizeof(record_info));

	record_info.bits = 16; // 16bit
	record_info.channels = 1; // mono-channel
	record_info.sample_rate = 16000; //sample rate
	record_info.record_volume = 200; //record volume

	/* count buf size according to rate, time and bits. */
	/* buf size = sample_rate * bits * channels * record_time (bytes)*/
	record_info.data_size = READ_LEN; //record_info.sample_rate * time * (16 >> 3) * 2;
	record_info.data_buf = (char *)malloc(record_info.data_size);	// record data buf
	if (!record_info.data_buf) {
		printf("data buf malloc fail!");
		return -1;
	}

	/* init cound card */
	ret = mozart_sound_card_init(&record_info);
	if (ret < 0) {
		printf("mozart_sound_card_init failed\n");
		return -1;
	}

	printf("Please Speak...\n");

	/* begin record */
	int size = 0;
	char *pbuf = NULL;

	size = record_info.data_size;
	pbuf = record_info.data_buf;
	while (g_vad_end != 1) {
		if (!speech_flag)
			break;
		ret = mozart_record_one_buf(&record_info);
		if (ret < 0) {
			printf("mozart_record failed\n");
			free(record_info.data_buf);
			mozart_sound_card_uninit(&record_info);
			return -1;
		}
		if (size > 0) {
			ret=aiengine_feed(agn, pbuf, size);
		} else {
			break;
		}

		if (ret < 0) {
			printf("agn feed failed.\n");
			break;
		}
	}
	g_vad_end = 0;
	aiengine_stop(agn);
	//关闭录音机
	free(record_info.data_buf);

	/* uninit sound card */
	mozart_sound_card_uninit(&record_info);

	printf("Please wait result...");
	loop = 0;
    while(!g_result){
        usleep(10000);
         loop++;
         if(loop>3000)
         {
             printf("No result found. Time out.\n");
             break;
         }
     }
	g_result = 0;

	if (aicmd != NOCMD)
		jz_process_asr();

	aicmd = NOCMD;
	/*aiengine_delete(agn);*/

	return 0;
}
