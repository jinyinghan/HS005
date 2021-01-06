
#ifndef __JZ_MXU_MERGE_HPP__
#define __JZ_MXU_MERGE_HPP__

#include "./../core/mxu_core.hpp"
#include "mxu_types_c.h"

#ifdef __cplusplus

/*! \namespace mxu
 Namespace where all the C++ JzMxu functionality resides
 */
namespace mxu
{

	enum { OPEN_MIN=MXU_OPEN_MIN, OPEN_MAX=MXU_OPEN_MAX, CLOSE_MIN=MXU_CLOSE_MIN, CLOSE_MAX=MXU_CLOSE_MAX, SINGLE_OPEN=MXU_SINGLE_OPEN, SINGLE_CLOSE=MXU_SINGLE_CLOSE };

MXU_EXPORTS_W void mergemorphologyEx( InputArray src, OutputArray dst, int op,
                       InputArray kernel, Point anchor);

MXU_EXPORTS_W void mergeline( InputArray src1, InputArray src2, OutputArray dst1, OutputArray dst2,
			int threshold_val, int max_val ,int threshold_type, uchar timestamp,uchar duration, int op,InputArray kernel, Point anchor);

//MXU_EXPORTS_W void mergemove( InputArray src1, OutputArray dst, int threshold_val, int max_val ,int threshold_type, int op,InputArray kernel, Point anchor);

MXU_EXPORTS_W void mergemove( InputArray src1, InputArray src2, OutputArray dst, int threshold_val,
		int max_val ,int threshold_type, int op,InputArray kernel, Point anchor);

MXU_EXPORTS_W void mergemovecopy( InputArray src1, InputArray src2, OutputArray dst1, OutputArray dst2, int threshold_val,
		int max_val ,int threshold_type, int op,InputArray kernel, Point anchor);

MXU_EXPORTS_W int simple_contours(Mat _src,vector< Rect >& contours,int  mode,int  method, Point offset,Size minsize);

}

#endif /* __cplusplus */

#endif

/* End of file. */
