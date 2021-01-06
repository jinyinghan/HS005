/*
 *   Author:xuc@yingshixun.com
 *   Date:2016.7
 *   Function: an example for h264 capture stream data
 */

#ifndef __COMMON_H__
#define __COMMON_H__


#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/queue.h>
#include <sys/prctl.h>

#include "imp/imp_common.h"
#include "imp/imp_system.h"
#include "imp/imp_framesource.h"
#include "imp/imp_encoder.h"
#include "imp/imp_isp.h"
#include "imp/imp_ivs.h"
#include "imp/imp_osd.h"
#include "sysutils/su_adc.h"

#include "qcam.h"
#include "qcam_audio_input.h"
#include "qcam_audio_output.h"
#include "qcam_log.h"
#include "qcam_motion_detect.h"
#include "qcam_sys.h"
#include "qcam_video_input.h"

#define MAX_FS 3

typedef enum {
    FS_MAIN = 0,
    FS_SECOND = 1,    
}FS_STREAM;




#endif  /*#ifndef __COMMON_H__*/
