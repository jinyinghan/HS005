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


#include "qcam.h"
#include "qcam_audio_input.h"
#include "qcam_audio_output.h"
#include "qcam_log.h"
#include "qcam_motion_detect.h"
#include "qcam_sys.h"
#include "qcam_video_input.h"



int main(int argc,char **argv)
{
	int ret = -1;
	int record_num = 0;
    struct timeval tv;

	if(argv == NULL) {
		QLOG(FATAL, "Please input the record file name.\n");
		return -1;
	}

	FILE *record_file = fopen(argv[1], "wb");
	if(record_file == NULL) {
		QLOG(FATAL, "fopen %s failed\n", argv);
		return -1;
	}

	/* Step 1: set public attribute of AI device. */
	int devID = 1;
	IMPAudioIOAttr attr;
	attr.samplerate = AUDIO_SAMPLE_RATE_16000;
	attr.bitwidth = AUDIO_BIT_WIDTH_16;
	attr.soundmode = AUDIO_SOUND_MODE_MOMO;
	attr.frmNum = 3;
	attr.numPerFrm = 160;
	attr.chnCnt = 1;
	ret = IMP_AI_SetPubAttr(devID, &attr);
	if(ret != 0) {
		QLOG(FATAL, "set ai %d attr err: %d\n", devID, ret);
		return -1;
	}

	memset(&attr, 0x0, sizeof(attr));
	ret = IMP_AI_GetPubAttr(devID, &attr);
	if(ret != 0) {
		QLOG(FATAL, "get ai %d attr err: %d\n", devID, ret);
		return -1;
	}

	QLOG(FATAL, "Audio In GetPubAttr samplerate : %d\n", attr.samplerate);
	QLOG(FATAL, "Audio In GetPubAttr   bitwidth : %d\n", attr.bitwidth);
	QLOG(FATAL, "Audio In GetPubAttr  soundmode : %d\n", attr.soundmode);
	QLOG(FATAL, "Audio In GetPubAttr     frmNum : %d\n", attr.frmNum);
	QLOG(FATAL, "Audio In GetPubAttr  numPerFrm : %d\n", attr.numPerFrm);
	QLOG(FATAL, "Audio In GetPubAttr     chnCnt : %d\n", attr.chnCnt);

	/* Step 2: enable AI device. */
	ret = IMP_AI_Enable(devID);
	if(ret != 0) {
		QLOG(FATAL, "enable ai %d err\n", devID);
		return -1;
	}

	/* Step 3: set audio channel attribute of AI device. */
	int chnID = 0;
	IMPAudioIChnParam chnParam;
	chnParam.usrFrmDepth = 2;
	ret = IMP_AI_SetChnParam(devID, chnID, &chnParam);
	if(ret != 0) {
		QLOG(FATAL, "set ai %d channel %d attr err: %d\n", devID, chnID, ret);
		return -1;
	}

	memset(&chnParam, 0x0, sizeof(chnParam));
	ret = IMP_AI_GetChnParam(devID, chnID, &chnParam);
	if(ret != 0) {
		QLOG(FATAL, "get ai %d channel %d attr err: %d\n", devID, chnID, ret);
		return -1;
	}

	QLOG(FATAL, "Audio In GetChnParam usrFrmDepth : %d\n", chnParam.usrFrmDepth);

	/* Step 4: enable AI channel. */
	ret = IMP_AI_EnableChn(devID, chnID);
	if(ret != 0) {
		QLOG(FATAL, "Audio Record enable channel failed\n");
		return -1;
	}

	/* Step 5: Set audio channel volume. */
	int chnVol = 100;
	ret = IMP_AI_SetVol(devID, chnID, chnVol);
	if(ret != 0) {
		QLOG(FATAL, "Audio Record set volume failed\n");
		return -1;
	}

	ret = IMP_AI_GetVol(devID, chnID, &chnVol);
	if(ret != 0) {
		QLOG(FATAL, "Audio Record get volume failed\n");
		return -1;
	}
	QLOG(FATAL, "Audio In GetVol    vol : %d\n", chnVol);
    gettimeofday(&tv,NULL);
    printf("start time = %d\n",tv.tv_sec);
	while(1) {
		/* Step 6: get audio record frame. */

		ret = IMP_AI_PollingFrame(devID, chnID, 1000);
		if (ret != 0 ) {
			QLOG(FATAL, "Audio Polling Frame Data error\n");
		}
		IMPAudioFrame frm;
		ret = IMP_AI_GetFrame(devID, chnID, &frm, BLOCK);
		if(ret != 0) {
			QLOG(FATAL, "Audio Get Frame Data error\n");
			return -1;
		}

		/* Step 7: Save the recording data to a file. */
		fwrite(frm.virAddr, 1, frm.len, record_file);

		/* Step 8: release the audio record frame. */
		ret = IMP_AI_ReleaseFrame(devID, chnID, &frm);
		if(ret != 0) {
			QLOG(FATAL, "Audio release frame data error\n");
			return -1;
		}

		if(++record_num >= 200)
			break;
		if ((record_num % 200) == 0) {
			chnVol += 10;
			ret = IMP_AI_SetVol(devID, chnID, chnVol);
			if(ret != 0) {
				QLOG(FATAL, "Audio Record set volume failed\n");
				return -1;
			}

			ret = IMP_AI_GetVol(devID, chnID, &chnVol);
			if(ret != 0) {
				QLOG(FATAL, "Audio Record get volume failed\n");
				return -1;
			}
			QLOG(FATAL, "Audio In GetVol    vol : %d\n", chnVol);
		}
	}
    gettimeofday(&tv,NULL);
    printf("start time = %d\n",tv.tv_sec);
	/* Step 9: disable the audio channel. */
	ret = IMP_AI_DisableChn(devID, chnID);
	if(ret != 0) {
		QLOG(FATAL, "Audio channel disable error\n");
		return -1;
	}

	/* Step 10: disable the audio devices. */
	ret = IMP_AI_Disable(devID);
	if(ret != 0) {
		QLOG(FATAL, "Audio device disable error\n");
		return -1;
	}

	fclose(record_file);
	exit(0);
}

