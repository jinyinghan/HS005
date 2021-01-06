#ifndef __IVS_INF_FAKE_H__
#define __IVS_INF_FAKE_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */

#include "ivs_common.h"

#define MAX_SIZE 5
/*
 * fake detector
 */
typedef struct {
  IVSLine line;
  int fake_text;
}IVSFakeDetector;

/*
 * fake 算法的输入结构体
 */
typedef struct {
  IVSFakeDetector fakeDetector[MAX_SIZE];
  int cntFake;
} fake_param_input_t;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
#endif /* __IVS_INF_FAKE_H__ */
