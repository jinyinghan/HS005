//-----------------------------------------------------
// 360Cam 硬件抽象层
// author : zhaohuaqiang@360.cn
//-----------------------------------------------------

#ifndef _QCAM_H_
#define _QCAM_H_

#define QCAM_OK		(0)
#define QCAM_FAIL	(-1)

// 依据QCAM为c实现还是C++实现，在makefile定义不同的值

#if (defined QCAM_AV_C_API)  && (defined __cplusplus)
	extern "C" {
#endif
	#include "qcam_audio_input.h"
	#include "qcam_audio_output.h"
	#include "qcam_video_input.h"
	#include "qcam_motion_detect.h"
#if (defined QCAM_AV_C_API)  && (defined __cplusplus)
	}
#endif


#if (defined QCAM_SYS_C_API)  && (defined __cplusplus)
	extern "C" {
#endif
	#include "qcam_sys.h"
#if (defined QCAM_SYS_C_API)  && (defined __cplusplus)
	}
#endif

#endif
