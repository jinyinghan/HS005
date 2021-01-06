#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <time.h>

#include "imp/imp_common.h"
#include "imp/imp_system.h"

#include "imp/imp_audio.h"


#define AEC_SAMPLE_RATE 8000
#define AEC_SAMPLE_TIME 10

#define IMP_AUDIO_BUF_SIZE (5 * (AEC_SAMPLE_RATE * sizeof(short) * AEC_SAMPLE_TIME / 1000))
#include <imp/imp_log.h>

#define TAG "test"


int main(int argc , char **argv)
{
	unsigned char *buf = NULL;
	int size = 0;
	int ret = -1;


	
	IMP_Audio_Mode_Set(1);
	IMP_Audio_Client_Init();
	
	if(argv == NULL) {
		IMP_LOG_ERR(TAG, "[ERROR] %s: Please input the play file name.\n", __func__);
		return NULL;
	}

	
	

	FILE *play_file = fopen(argv[1], "rb");
	if(play_file == NULL) {
		IMP_LOG_ERR(TAG, "[ERROR] %s: fopen %s failed\n", __func__, argv[1]);
		return NULL;
	}

	/* Step 1: set public attribute of AO device. */
	int devID = 0;
	IMPAudioIOAttr attr;
	attr.samplerate = AUDIO_SAMPLE_RATE_8000;
	attr.bitwidth = AUDIO_BIT_WIDTH_16;
	attr.soundmode = AUDIO_SOUND_MODE_MOMO;
	attr.frmNum = 100;
	attr.numPerFrm = (10*attr.samplerate / 1000);
	attr.chnCnt = 1;
    int frm_size = 2*10*attr.samplerate * attr.bitwidth / 8 / 1000;
    buf = (unsigned char *)malloc(frm_size);
    if(buf == NULL) {
		IMP_LOG_ERR(TAG, "[ERROR] %s: malloc audio buf error\n", __func__);
		return NULL;
	}	
    
	ret = IMP_AO_SetPubAttr(devID, &attr);
	if(ret != 0) {
		IMP_LOG_ERR(TAG, "set ao %d attr err: %d\n", devID, ret);
		return NULL;
	}

	memset(&attr, 0x0, sizeof(attr));
	ret = IMP_AO_GetPubAttr(devID, &attr);
	if(ret != 0) {
		IMP_LOG_ERR(TAG, "get ao %d attr err: %d\n", devID, ret);
		return NULL;
	}

	IMP_LOG_INFO(TAG, "Audio Out GetPubAttr samplerate:%d\n", attr.samplerate);
	IMP_LOG_INFO(TAG, "Audio Out GetPubAttr   bitwidth:%d\n", attr.bitwidth);
	IMP_LOG_INFO(TAG, "Audio Out GetPubAttr  soundmode:%d\n", attr.soundmode);
	IMP_LOG_INFO(TAG, "Audio Out GetPubAttr     frmNum:%d\n", attr.frmNum);
	IMP_LOG_INFO(TAG, "Audio Out GetPubAttr  numPerFrm:%d\n", attr.numPerFrm);
	IMP_LOG_INFO(TAG, "Audio Out GetPubAttr     chnCnt:%d\n", attr.chnCnt);

	/* Step 2: enable AO device. */
	ret = IMP_AO_Enable(devID);
	if(ret != 0) {
		IMP_LOG_ERR(TAG, "enable ao %d err\n", devID);
		return NULL;
	}
    printf("buf size = %d\n",frm_size);
	/* Step 3: enable AI channel. */
	int chnID = 0;
	ret = IMP_AO_EnableChn(devID, chnID);
	if(ret != 0) {
		IMP_LOG_ERR(TAG, "Audio play enable channel failed\n");
		return NULL;
	}

	/* Step 4: Set audio channel volume. */
	int chnVol = 78;

	int i = 0;
	while(1) {
		size = fread(buf, 1, frm_size, play_file);
		if(size < frm_size)
			break;

		/* Step 5: send frame data. */
		IMPAudioFrame frm;
		frm.virAddr = (uint32_t *)buf;
		frm.len = size;
		ret = IMP_AO_SendFrame(devID, chnID, &frm, BLOCK);
		if(ret != 0) {
			IMP_LOG_ERR(TAG, "send Frame Data error\n");
			return NULL;
		}

		IMPAudioOChnState play_status;
		ret = IMP_AO_QueryChnStat(devID, chnID, &play_status);
		if(ret != 0) {
			IMP_LOG_ERR(TAG, "IMP_AO_QueryChnStat error\n");
			return NULL;
		}

		IMP_LOG_INFO(TAG, "Play: TotalNum %d, FreeNum %d, BusyNum %d\n",
				play_status.chnTotalNum, play_status.chnFreeNum, play_status.chnBusyNum);

/*
		if(++i == 100) {
			ret = IMP_AO_PauseChn(devID, chnID);
			if(ret != 0) {
				IMP_LOG_ERR(TAG, "IMP_AO_PauseChn error\n");
				return NULL;
			}

			printf("[INFO] Test : Audio Play Pause test.\n");
			printf("[INFO]      : Please input any key to continue.\n");
			getchar();

			ret = IMP_AO_ClearChnBuf(devID, chnID);
			if(ret != 0) {
				IMP_LOG_ERR(TAG, "IMP_AO_ClearChnBuf error\n");
				return NULL;
			}

			ret = IMP_AO_ResumeChn(devID, chnID);
			if(ret != 0) {
				IMP_LOG_ERR(TAG, "IMP_AO_ResumeChn error\n");
				return NULL;
			}
		}
		*/
	}
	sleep(50);
	/* Step 6: disable the audio channel. */
	ret = IMP_AO_DisableChn(devID, chnID);
	if(ret != 0) {
		IMP_LOG_ERR(TAG, "Audio channel disable error\n");
		return NULL;
	}

	/* Step 7: disable the audio devices. */
	ret = IMP_AO_Disable(devID);
	if(ret != 0) {
		IMP_LOG_ERR(TAG, "Audio device disable error\n");
		return NULL;
	}

	fclose(play_file);
	free(buf);
	pthread_exit(0);
}

