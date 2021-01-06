
#ifndef __IPC_IRCUT_H__
#define __IPC_IRCUT_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif

#include "qcam_video_input.h"

#define NIGHT_THRESHOLD		80
#define DAY_THRESHOLD		1000


int ipc_ircut_init();

int ipc_ircut_exit();

int ipc_ircut_mode_set(QCAM_IR_MODE mode);

QCAM_IR_MODE ipc_ircut_mode_get();

QCAM_IR_STATUS ipc_ircut_status_get();

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif


#endif

