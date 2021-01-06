#ifndef __JZ_MXU_VECTOR_H__
#define __JZ_MXU_VECTOR_H__

#include "mxu_types_c.h"

#include "mxu_internal.hpp"
#if JZ_MXU
enum
{
	CVT16_8 = 0 ,
	CVT32_16= 1 ,
	CVT8_16 = 2 ,
	CVT16_32= 3 ,
	SHIFT_1 = 4 ,
	SHIFT_2 = 5 ,
	SHIFT_3 = 6 ,
	SHIFT_4 = 7
};
enum
{
	HIGH = 0 ,
	LOW = 1
};

namespace mxu
{

MXU_EXPORTS_W void fillvector(v16u8& dst,unsigned char type,unsigned char save_type);

}

MXUAPI(void) mxuFillVector(v16u8& dst,unsigned char type,unsigned char save_type);
#endif

#endif
