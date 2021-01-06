#ifndef __JZ_MXU_MERGE_C_H__
#define __JZ_MXU_MERGE_C_H__

#include "./../core/mxu_core_c.h"
#include "mxu_types_c.h"
#include <stdlib.h>

using namespace std;

#ifdef __cplusplus
extern "C" {
#endif



MXUAPI(void) mxuMergeMorphologyEx( const void* srcarr, void* dstarr,
                IftConvKernel* element, int op);

MXUAPI(void) mxuMergeLine( const void* srcarr1, const void* srcarr2, void* dstarr1, void* dstarr2,
		int threshold_val, int max_val ,int threshold_type,uchar timestamp,uchar duration,
                IftConvKernel* element, int op);

MXUAPI(void) mxuMergeMove( const void* srcarr1, const void* srcarr2, void* dstarr,int threshold_val,
		int max_val ,int threshold_type,IftConvKernel* element, int op);

MXUAPI(void) mxuMergeMoveCopy( const void* srcarr1, const void* srcarr2, void* dstarr1, void* dstarr2,int threshold_val,
		int max_val ,int threshold_type,IftConvKernel* element, int op);


#ifdef __cplusplus
}
#endif

#endif
