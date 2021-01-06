#ifndef _YSX_AUDIO_H_
#define _YSX_AUDIO_H_

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <time.h>
#include <sys/prctl.h>

#include "imp/imp_common.h"
#include "imp/imp_system.h"

#include <imp/imp_audio.h>


#include "qcam.h"
#include "qcam_audio_input.h"
#include "qcam_audio_output.h"
#include "qcam_log.h"
#include "qcam_motion_detect.h"
#include "qcam_sys.h"
#include "qcam_video_input.h"

#define AUDIO_VERSION "a-0.0.3"

#define SPK_EN_GPIO  63

struct ysx_audio_record{

    uint8_t enable;
    uint8_t start;
    uint8_t DevId;
    uint8_t ChnId;
    int mic_gain;
    QCamAudioInputAttr attr;
    QCamAudioInputCallback cb;
};

struct ysx_audio_pb{
   
    uint8_t DevId;
    uint8_t ChnId;
    uint32_t buf_size;
    uint32_t has_left;
    uint8_t *buf;
    int spk_gain;
    QCamAudioOutputAttribute attr;
    
};

typedef enum spk_en_e
{
	SPK_EN_INIT  = -1,
 	SPK_EN_CLOSE = 0,
	SPK_EN_OPEN  = 1,	
}SPK_EN_E;


#endif /*_YSX_AUDIO_H_*/
