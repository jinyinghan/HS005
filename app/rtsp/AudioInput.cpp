/*
 * Ingenic IMP RTSPServer AudioInput
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 * Author: Zoro <yakun.li@ingenic.com>
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <linux/soundcard.h>
#include <sys/ioctl.h>

#include <imp/imp_log.h>
#include <imp/imp_common.h>
#include <imp/imp_system.h>
#include <imp/imp_utils.h>
#include <imp/imp_framesource.h>
#include <imp/imp_audio.h>


#include "Options.hh"
#include "AudioInput.hh"
#include "AudioStreamSource.hh"

#define TAG 						"sample-RTSPServer"
unsigned fuSecsPerFrame;

#ifdef SPEECH_TEST
extern int speech_stop;
int StreamIsOn= 1;
void *pData = NULL;
#endif

/* Audio param */
struct audio_list {
	struct audio_list *next;
	char * buffer;
	char * buffer_backup;
	int count;
	int status;
	int size;
};
struct audio_list *head = NULL;
#define AUDIOBUFFERNUM		160

Boolean AudioInput::fHaveInitialized = False;//audio
int AudioInput::chnID = 0;
int AudioInput::devID = 1;
int AudioInput::devIDr = 0;
int AudioInput::ptfunc = 0;

static int audio_init(void)
{
	int ret;

	IMPAudioIOAttr attr;
	attr.samplerate = AUDIO_SAMPLE_RATE_8000;
	attr.bitwidth = AUDIO_BIT_WIDTH_16;
	attr.soundmode = AUDIO_SOUND_MODE_MONO;
	attr.frmNum = 40;
	attr.numPerFrm = 320;
	attr.chnCnt = 1;
	ret = IMP_AI_SetPubAttr(AudioInput::devID, &attr);
	if(ret != 0) {
		IMP_LOG_ERR(TAG, "set ai %d attr err: %d\n", AudioInput::devID, ret);
		return -1;
	}
	return 0;
}

static inline int close_stream_file(int fd)
{
	return close(fd);
}

AudioInput* AudioInput::createNew(UsageEnvironment& env, int streamNum) {
	if (!fHaveInitialized) {
		if (!initialize(env)) return NULL;
		fHaveInitialized = True;
	}

	AudioInput *audioInput = new AudioInput(env, streamNum);

	return audioInput;
}

AudioInput::AudioInput(UsageEnvironment& env, int streamNum)
	: Medium(env), fAudioSource(NULL), streamNum(streamNum),
	scheduleTid_a(-1) {
}

AudioInput::~AudioInput() {
}

int AudioInput::pollingStream(void)
{
		/*sleep();*/
	int ret;
	ret = IMP_AI_PollingFrame(devID, chnID, 1000);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "Audio Polling stream timeout\n");
		return -1;
	}

	return 0;
}

bool AudioInput::initialize(UsageEnvironment& env) {
	int ret;

	ret = audio_init();
	if (ret < 0)
		return false;
	fuSecsPerFrame = 1000000/audioSamplingFrequency_t;/*samples-per-second*/;  

	return true;
}

FramedSource* AudioInput::audioSource() {
	if (fAudioSource == NULL) {
		fAudioSource = new AudioStreamSource(envir(), *this);
	}
	return fAudioSource;
}

int AudioInput::getStream(void* to, unsigned int* len, struct timeval* timestamp, unsigned fMaxSize) {

	int ret;
	IMPAudioFrame frm,ref;

	ret = IMP_AI_GetFrameAndRef(devID, chnID, &frm, &ref,BLOCK);
	if(ret != 0) {
		printf("Audio Get Frame Data error\n");
		while(1);
		return -1;
	}
	memmove(to, frm.virAddr, frm.len);
	*len = frm.len;
//	printf("###RTSP Audio len = %d\n", *len);
	ret = IMP_AI_ReleaseFrame(devID, chnID, &frm);
	if(ret != 0) {
		IMP_LOG_ERR(TAG, "Audio release frame data error\n");
		return -1;
	}


	gettimeofday(timestamp, NULL);



#if 1
	fuSecsPerFrame = frm.len * 1000000/8000/2;
	if(timestamp->tv_sec ==0 && timestamp->tv_usec == 0)
		gettimeofday(timestamp, NULL);
	else{
		unsigned uSeconds = timestamp->tv_usec + fuSecsPerFrame; 
		timestamp->tv_sec += uSeconds/1000000;  
		timestamp->tv_usec = uSeconds%1000000;  

	}  //40000

#else
	int timeinc;
	gettimeofday(timestamp, NULL);

	/* PR#2665 fix from Robin
	 * Assuming audio format = AFMT_S16_LE
	 * Get the current time
	 * Substract the time increment of the audio oss buffer, which is equal to
	 * buffer_size / channel_number / sample_rate / sample_size ==> 400+ millisec
	 */
	timeinc = frm.len * 1000 / audioNumChannels_t / (audioSamplingFrequency_t/1000) / 2;
	printf("frm_len = %d ,timeinc = %d\n",frm.len,timeinc);
	while (timestamp->tv_usec < timeinc)
	{
		timestamp->tv_sec -= 1;
		timeinc -= 1000000;
	} timestamp->tv_usec -= timeinc;
#endif
	return 0;
}

int AudioInput::streamOn(void)
{
	int ret = -1;

	ret = IMP_AI_Enable(devID);
	if(ret != 0) {
		IMP_LOG_ERR(TAG, "enable ai %d err\n", devID);
		return -1;
	}

	IMPAudioIChnParam chnParam;
	chnParam.usrFrmDepth = 40;
	ret = IMP_AI_SetChnParam(devID, chnID, &chnParam);
	if(ret != 0) {
		IMP_LOG_ERR(TAG, "set ai %d channel %d attr err: %d\n", devID, chnID, ret);
		return -1;
	}

	/* Step 5: Set audio channel volume. */
	int chnVol = 90;
	ret = IMP_AI_SetVol(devID, chnID, chnVol);
	if(ret != 0) {
		IMP_LOG_ERR(TAG, "Audio Record set volume failed\n");
		return -1;
	}

	int chnGain = 23;
	ret = IMP_AI_SetGain(devID, chnID, chnGain);
	if(ret != 0) {
		IMP_LOG_ERR(TAG, "Audio Record set gain failed\n");
		return -1;
	}
	ret = IMP_AI_EnableChn(devID, chnID);
	if(ret != 0) {
		IMP_LOG_ERR(TAG, "Audio Record enable channel failed\n");
		return -1;
	}
	ret = IMP_AI_EnableAecRefFrame(devID, chnID, 0, 0);
	if(ret != 0) {
		IMP_LOG_ERR(TAG, "IMP_AI_EnableAecRefFrame dev%d channel%d failed\n",devID,chnID);
		return -1;
	} 
	printf("AI Stream On !\n");
	return 0;
}

int AudioInput::streamOff(void)
{
	int ret;

	ret = IMP_AI_DisableChn(devID, chnID);
	if(ret != 0) {
		IMP_LOG_ERR(TAG, "Audio channel disable error\n");
		return -1;
	}

	ret = IMP_AI_Disable(devID);
	if(ret != 0) {
		IMP_LOG_ERR(TAG, "Audio device disable error\n");
		return -1;
	}

#ifdef SPEECH_TEST
	if (pData)
		free(pData);
#endif

	return 0;
}

int init_list(int list_num)
{
	int i = 0;
	struct audio_list *prve_list = NULL;
	for (i = 0; i < list_num; i++) {
		struct audio_list *list = (struct audio_list *)malloc(sizeof(struct audio_list));
		if (list == NULL) {
			printf("malloc list error, num = %d\n", i);
			return -1;
		}
		list->buffer = (char *)malloc(sizeof(char) * 800);
		if (list->buffer == NULL) {
			printf("malloc list->buffer error, num = %d\n", i);
			return -1;
		}
		memset(list->buffer, 0, 800);
		list->buffer_backup = list->buffer;
		list->count = i;
		list->size = 0;
		list->status = 0;
		if (i == 0) {
			head = list;
			list->next = NULL;
		} else {
			prve_list->next = list;
			list->next = NULL;
		}
		if (i == (list_num - 1)) {
			list->next = head;
		}
		prve_list = list;
	}

	return 0;
}

void deinit_list(int list_num)
{
	struct audio_list * list = head;
	struct audio_list * list_n;
	int i = 0;
	for (i = 0; i < list_num; i++) {
		list_n = list->next;
		free(list->buffer);
		free(list);
		list = list_n;
	}
}

struct audio_list * find_free_list(int num)
{
	int i = 0;
	struct audio_list * list = head;
	for (i = 0; i < AUDIOBUFFERNUM; i++) {
		/*if (list->status == 0)*/
		if (list->count == num)
			break;
		list = list->next;
	}

	if (i == AUDIOBUFFERNUM) {
		printf("num=%d > max num!!!!\n", num);
		return NULL;
	}

	return list;
}

struct audio_list * find_used_list(int num)
{
	int i = 0;
	struct audio_list * list = head;
	for (i = 0; i < AUDIOBUFFERNUM; i++) {
		if (list->count == num)
			break;
		list = list->next;
	}
	if (i == AUDIOBUFFERNUM) {
		printf("audio list all free!!!\n");
		return NULL;
	}

	return list;
}

#if 0
void *audio_write_func(void * arg)
{
	//int size = -1;
	int ret = -1;
	static int found_used_list = 0;
	static int num_used_list = 0;
	struct audio_list *list  = NULL;
	struct audio_list *rlist = NULL;

	usleep(1000 * 1000 * 2);
	IMPAudioFrame frm;
	while(AudioInput::ptfunc) {
		if (!found_used_list) {
			list = find_used_list(num_used_list);
			if (list == NULL) {
				printf("can not find used list!!!\n");
			}
			if (list->status == 1) {
				found_used_list = 1;
				rlist = list;
			}
		}
		if (found_used_list == 1) {
			printf("P: node count = %d\n", rlist->count);
			frm.virAddr = (uint32_t *)rlist->buffer;
			frm.len = rlist->size;
			ret = IMP_AO_SendFrame(AudioInput::devIDr, AudioInput::chnID, &frm, BLOCK);
			if(ret != 0) {
				IMP_LOG_ERR(TAG, "send Frame Data error\n");
				return NULL;
			}
			rlist->status = 0;
			rlist->size   = 0;
			found_used_list = 0;
			num_used_list++;
			if (num_used_list == AUDIOBUFFERNUM)
				num_used_list = 0;
		}
	}
}
#endif

int sock;
void close_sock()
{
	close(sock);
}

void AudioInput::streamRecv(void)
{
	struct sockaddr_in toAddr;
	struct sockaddr_in fromAddr;
	//int recvLen;
	unsigned int addrLen;
	char recvBuffer[800];

	int ret = -1;
	//static int found_free_list = 0;
	/*
	 *ret = init_list(AUDIOBUFFERNUM);
	 *if (ret != 0) {
	 *    printf("init list error\n");
	 *    return;
	 *}
	 */

	sock = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
	if(sock < 0)
	{
		printf("Socket create error.\r\n");
		exit(0);
	}
	memset(&fromAddr,0,sizeof(fromAddr));

	fromAddr.sin_family=AF_INET;
	fromAddr.sin_addr.s_addr=htonl(INADDR_ANY);
	fromAddr.sin_port = htons(40000);

	int opt = 1;
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const void *)&opt, sizeof(opt));

	struct timeval tv_out;

	tv_out.tv_sec = 1;//wait 1''
	tv_out.tv_usec = 0;
	setsockopt(sock,SOL_SOCKET,SO_RCVTIMEO,&tv_out, sizeof(tv_out));

	if(bind(sock,(struct sockaddr*)&fromAddr,sizeof(fromAddr))<0)
	{
		printf("bind error.\r\n");
		close_sock();
		return;
	}

	IMPAudioIOAttr attr;
	attr.samplerate = AUDIO_SAMPLE_RATE_8000;
	attr.bitwidth = AUDIO_BIT_WIDTH_16;
	attr.soundmode = AUDIO_SOUND_MODE_MONO;
	attr.frmNum = 40;
	attr.numPerFrm = 320;
	attr.chnCnt = 1;
	ret = IMP_AO_SetPubAttr(devIDr, &attr);
	if(ret != 0) {
		IMP_LOG_ERR(TAG, "set ao %d attr err: %d\n", devIDr, ret);
		return;
	}

	ret = IMP_AO_Enable(devIDr);
	if(ret != 0) {
		IMP_LOG_ERR(TAG, "enable ao %d err\n", devIDr);
		return;
	}

	int chnID = 0;
	ret = IMP_AO_EnableChn(devIDr, chnID);
	if(ret != 0) {
		IMP_LOG_ERR(TAG, "Audio play enable channel failed\n");
		return;
	}

	/*
	 *pthread_t thread_id;
	 *if (pthread_create(&thread_id, NULL, audio_write_func, NULL) != 0) {
	 *    printf("Create thread error!\n");
	 *    exit(1);
	 *}
	 */

	ptfunc = 1;
	ret = IMP_AI_EnableAec(devID, chnID, 0, 0);
	if(ret != 0) {
		IMP_LOG_ERR(TAG, "Audio enable aec failed\n");
		return;
	}
	do {
		addrLen = sizeof(toAddr);
		int ss = 0;
recv_again:
		if (!AudioInput::ptfunc)
			break;

		if((ss = recvfrom(sock,recvBuffer, attr.numPerFrm * 2, 0, (struct sockaddr*)&toAddr,&addrLen))<0) {
			printf("recvfrom error.\r\n");
			if (!AudioInput::ptfunc)
				break;
			goto recv_again;
		}

		IMPAudioFrame frm;
		frm.virAddr = (uint32_t *)recvBuffer;
		frm.len = ss;
		ret = IMP_AO_SendFrame(AudioInput::devIDr, AudioInput::chnID, &frm, BLOCK);
		if(ret != 0) {
			printf("\n IMP_AO_SendFrame size = %d, ss = %d\n", frm.len, ss);
			IMP_LOG_ERR(TAG, "send Frame Data error\n");
			return;
		}
	} while(AudioInput::ptfunc);

	ret = IMP_AI_DisableAec(devID, chnID);
	if(ret != 0) {
		IMP_LOG_ERR(TAG, "Audio disable aec failed\n");
		return;
	}

	ret = IMP_AO_DisableChn(devIDr, chnID);
	if(ret != 0) {
		IMP_LOG_ERR(TAG, "Audio channel disable error\n");
		return;
	}

	ret = IMP_AO_Disable(devIDr);
	if(ret != 0) {
		IMP_LOG_ERR(TAG, "Audio device disable error\n");
		return;
	}
	close_sock();
	/*deinit_list(AUDIOBUFFERNUM);*/

	return;
}
