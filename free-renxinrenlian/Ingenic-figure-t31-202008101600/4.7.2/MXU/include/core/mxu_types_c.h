
#ifndef __JZ_MXU_CORE_TYPES_C_H__
#define __JZ_MXU_CORE_TYPES_C_H__

#if !defined _CRT_SECURE_NO_DEPRECATE && defined _MSC_VER
#  if _MSC_VER > 1300
#    define _CRT_SECURE_NO_DEPRECATE /* to avoid multiple Visual Studio 2005 warnings */
#  endif
#endif


#ifndef SKIP_INCLUDES

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>

#if !defined _MSC_VER && !defined __BORLANDC__
#  include <stdint.h>
#endif

#if defined __ICL
#  define MXU_ICC   __ICL
#elif defined __ICC
#  define MXU_ICC   __ICC
#elif defined __ECL
#  define MXU_ICC   __ECL
#elif defined __ECC
#  define MXU_ICC   __ECC
#elif defined __INTEL_COMPILER
#  define MXU_ICC   __INTEL_COMPILER
#endif

#if defined MXU_ICC && !defined MXU_ENABLE_UNROLLED
#  define MXU_ENABLE_UNROLLED 0
#else
#  define MXU_ENABLE_UNROLLED 1
#endif

#if (defined _M_X64 && defined _MSC_VER && _MSC_VER >= 1400) || (__GNUC__ >= 4 && defined __x86_64__)
#  if defined WIN32
#    include <intrin.h>
#  endif
#  if defined __SSE2__ || !defined __GNUC__
#    include <emmintrin.h>
#  endif
#endif

#if defined __BORLANDC__
#  include <fastmath.h>
#else
#  include <math.h>
#endif

#endif // SKIP_INCLUDES

#if defined WIN32 || defined _WIN32
#  define MXU_CDECL __cdecl
#  define MXU_STDCALL __stdcall
#else
#  define MXU_CDECL
#  define MXU_STDCALL
#endif

#ifndef MXU_EXTERN_C
#  ifdef __cplusplus
#    define MXU_EXTERN_C extern "C"
#    define MXU_DEFAULT(val) = val
#  else
#    define MXU_EXTERN_C
#    define MXU_DEFAULT(val)
#  endif
#endif

#ifndef MXU_EXTERN_C_FUNCPTR
#  ifdef __cplusplus
#    define MXU_EXTERN_C_FUNCPTR(x) extern "C" { typedef x; }
#  else
#    define MXU_EXTERN_C_FUNCPTR(x) typedef x
#  endif
#endif

#ifndef MXU_INLINE
#  if defined __cplusplus
#    define MXU_INLINE inline
#  elif defined _MSC_VER
#    define MXU_INLINE __inline
#  else
#    define MXU_INLINE static
#  endif
#endif /* MXU_INLINE */

#if (defined WIN32 || defined _WIN32 || defined WINCE) && defined MXUAPI_EXPORTS
#  define MXU_EXPORTS __declspec(dllexport)
#else
#  define MXU_EXPORTS
#endif

#ifndef MXUAPI
#  define MXUAPI(rettype) MXU_EXTERN_C MXU_EXPORTS rettype MXU_CDECL
#endif

#if defined _MSC_VER || defined __BORLANDC__
   typedef __int64 int64;
   typedef unsigned __int64 uint64;
#  define MXU_BIG_INT(n)   n##I64
#  define MXU_BIG_UINT(n)  n##UI64
#else
   typedef int64_t int64;
   typedef uint64_t uint64;
#  define MXU_BIG_INT(n)   n##LL
#  define MXU_BIG_UINT(n)  n##ULL
#endif

typedef signed char schar;
typedef unsigned char uchar;

/* special informative macros for wrapper generators */
#define MXU_CARRAY(counter)
#define MXU_CUSTOM_CARRAY(args)
#define MXU_EXPORTS_W MXU_EXPORTS
#define MXU_EXPORTS_W_SIMPLE MXU_EXPORTS
#define MXU_EXPORTS_AS(synonym) MXU_EXPORTS
#define MXU_EXPORTS_W_MAP MXU_EXPORTS
#define MXU_IN_OUT
#define MXU_OUT
#define MXU_PROP
#define MXU_PROP_RW
#define MXU_WRAP
#define MXU_WRAP_AS(synonym)
#define MXU_WRAP_DEFAULT(value)

/* MxuArr* is used to pass arbitrary
 * array-like data structures
 * into functions where the particular
 * array type is recognized at runtime:
 */
typedef void MxuArr;

typedef union Mxu32suf
{
    int i;
    unsigned u;
    float f;
}
Mxu32suf;

typedef union Mxu64suf
{
    int64 i;
    uint64 u;
    double f;
}
Mxu64suf;

typedef int MXUStatus;
/****************************************************************************************\
*                             Common macros and inline functions                         *
\****************************************************************************************/
#define MXU_PI   3.1415926535897932384626433832795
#define MXU_LOG2 0.69314718055994530941723212145818

#define MXU_SWAP(a,b,t) ((t) = (a), (a) = (b), (b) = (t))

#ifndef MIN
#  define MIN(a,b)  ((a) > (b) ? (b) : (a))
#endif

#ifndef MAX
#  define MAX(a,b)  ((a) < (b) ? (b) : (a))
#endif

/* min & max without jumps */
#define  MXU_IMIN(a, b)  ((a) ^ (((a)^(b)) & (((a) < (b)) - 1)))

#define  MXU_IMAX(a, b)  ((a) ^ (((a)^(b)) & (((a) > (b)) - 1)))

/* absolute value without jumps */
#ifndef __cplusplus
#  define  MXU_IABS(a)     (((a) ^ ((a) < 0 ? -1 : 0)) - ((a) < 0 ? -1 : 0))
#else
#  define  MXU_IABS(a)     abs(a)
#endif
#define  MXU_CMP(a,b)    (((a) > (b)) - ((a) < (b)))
#define  MXU_SIGN(a)     MXU_CMP((a),0)

MXU_INLINE  int  mxuRound( double value )
{
    double intpart, fractpart;
    fractpart = modf(value, &intpart);
    if ((fabs(fractpart) != 0.5) || ((((int)intpart) % 2) != 0))
        return (int)(value + (value >= 0 ? 0.5 : -0.5));
    else
        return (int)intpart;
}

MXU_INLINE  int  mxuFloor( double value )
{
    int i = mxuRound(value);
    float diff = (float)(value - i);
    return i - (diff < 0);
}


MXU_INLINE  int  mxuCeil( double value )
{
    int i = mxuRound(value);
    float diff = (float)(i - value);
    return i + (diff < 0);
}

#define mxuInvSqrt(value) ((float)(1./sqrt(value)))
#define mxuSqrt(value)  ((float)sqrt(value))

MXU_INLINE int mxuIsNaN( double value )
{
    Mxu64suf ieee754;
    ieee754.f = value;
    return ((unsigned)(ieee754.u >> 32) & 0x7fffffff) +
           ((unsigned)ieee754.u != 0) > 0x7ff00000;
}


MXU_INLINE int mxuIsInf( double value )
{
    Mxu64suf ieee754;
    ieee754.f = value;
    return ((unsigned)(ieee754.u >> 32) & 0x7fffffff) == 0x7ff00000 &&
           (unsigned)ieee754.u == 0;
}

/*************** Random number generation *******************/

typedef uint64 MxuRNG;

#define MXU_RNG_COEFF 4164903690U

MXU_INLINE MxuRNG mxuRNG( int64 seed MXU_DEFAULT(-1))
{
    MxuRNG rng = seed ? (uint64)seed : (uint64)(int64)-1;
    return rng;
}

/* Return random 32-bit unsigned integer: */
MXU_INLINE unsigned mxuRandInt( MxuRNG* rng )
{
    uint64 temp = *rng;
    temp = (uint64)(unsigned)temp*MXU_RNG_COEFF + (temp >> 32);
    *rng = temp;
    return (unsigned)temp;
}

/* Returns random floating-point number between 0 and 1: */
MXU_INLINE double mxuRandReal( MxuRNG* rng )
{
    return mxuRandInt(rng)*2.3283064365386962890625e-10 /* 2^-32 */;
}

/****************************************************************************************\
*                                  Image type (IftImage)                                 *
\****************************************************************************************/

/*
 * The following definitions (until #endif)
 * is an extract from IFT headers.
 * Copyright (c) 1995 Intel Corporation.
 */
#define IFT_DEPTH_SIGN 0x80000000

#define IFT_DEPTH_1U     1
#define IFT_DEPTH_8U     8
#define IFT_DEPTH_16U   16
#define IFT_DEPTH_32F   32

#define IFT_DEPTH_8S  (IFT_DEPTH_SIGN| 8)
#define IFT_DEPTH_16S (IFT_DEPTH_SIGN|16)
#define IFT_DEPTH_32S (IFT_DEPTH_SIGN|32)

#define IFT_DATA_ORDER_PIXEL  0
#define IFT_DATA_ORDER_PLANE  1

#define IFT_ORIGIN_TL 0
#define IFT_ORIGIN_BL 1

#define IFT_ALIGN_4BYTES   4
#define IFT_ALIGN_8BYTES   8
#define IFT_ALIGN_16BYTES 16
#define IFT_ALIGN_32BYTES 32

#define IFT_ALIGN_DWORD   IFT_ALIGN_4BYTES
#define IFT_ALIGN_QWORD   IFT_ALIGN_8BYTES

#define IFT_BORDER_CONSTANT   0
#define IFT_BORDER_REPLICATE  1
#define IFT_BORDER_REFLECT    2
#define IFT_BORDER_WRAP       3

typedef struct _IftImage
{
    int  nSize;             /* sizeof(IftImage) */
    int  ID;                /* version (=0)*/
    int  nChannels;         /* Most of OpenMXU functions support 1,2,3 or 4 channels */
    //int  alphaChannel;      /* Ignored by OpenMXU */
    int  depth;             /* Pixel depth in bits: IFT_DEPTH_8U, IFT_DEPTH_8S, IFT_DEPTH_16S,
                               IFT_DEPTH_32S, IFT_DEPTH_32F and IFT_DEPTH_64F are supported.  */
    //char colorModel[4];     /* Ignored by OpenMXU */
    char channelSeq[4];     /* ditto */
    int  dataOrder;         /* 0 - interleaved color channels, 1 - separate color channels.
                               mxuCreateImage can only create interleaved images */
    int  origin;            /* 0 - top-left origin,
                               1 - bottom-left origin (Windows bitmaps style).  */
    //int  align;             /* Alignment of image rows (4 or 8).
                               //OpenMXU ignores it and uses widthStep instead.    */
    int  width;             /* Image width in pixels.                           */
    int  height;            /* Image height in pixels.                          */
    struct _IftROI *roi;    /* Image ROI. If NULL, the whole image is selected. */
    struct _IftImage *maskROI;      /* Must be NULL. */
    void  *imageId;                 /* "           " */
    struct _IftTileInfo *tileInfo;  /* "           " */
    int  imageSize;         /* Image data size in bytes
                               (==image->height*image->widthStep
                               in case of interleaved data)*/
    char *imageData;        /* Pointer to aligned image data.         */
    int  widthStep;         /* Size of aligned image row in bytes.    */
    //int  BorderMode[4];     /* Ignored by OpenMXU.                     */
    int  BorderConst[4];    /* Ditto.                                 */
    char *imageDataOrigin;  /* Pointer to very origin of image data
                               (not necessarily aligned) -
                               needed for correct deallocation */
}
IftImage;

typedef struct _IftTileInfo IftTileInfo;

typedef struct _IftROI
{
    int  coi; /* 0 - no COI (all channels are selected), 1 - 0th channel is selected ...*/
    int  xOffset;
    int  yOffset;
    int  width;
    int  height;
}
IftROI;

typedef struct _IftConvKernel
{
    int  nCols;
    int  nRows;
    int  anchorX;
    int  anchorY;
    int *values;
    int  nShiftR;
}
IftConvKernel;

typedef struct _IftConvKernelFP
{
    int  nCols;
    int  nRows;
    int  anchorX;
    int  anchorY;
    float *values;
}
IftConvKernelFP;

#define IFT_IMAGE_HEADER 1
#define IFT_IMAGE_DATA   2
#define IFT_IMAGE_ROI    4

/* extra border mode */
#define IFT_BORDER_REFLECT_101    4
#define IFT_BORDER_TRANSPARENT    5

#define IFT_IMAGE_MAGIC_VAL  ((int)sizeof(IftImage))
#define MXU_TYPE_NAME_IMAGE "jzmxu-image"

#define MXU_IS_IMAGE_HDR(img) \
    ((img) != NULL && ((const IftImage*)(img))->nSize == sizeof(IftImage))

#define MXU_IS_IMAGE(img) \
    (MXU_IS_IMAGE_HDR(img) && ((IftImage*)img)->imageData != NULL)

/* for storing double-precision
   floating point data in IftImage's */
#define IFT_DEPTH_64F  64

/* get reference to pixel at (col,row),
   for multi-channel images (col) should be multiplied by number of channels */
#define MXU_IMAGE_ELEM( image, elemtype, row, col )       \
    (((elemtype*)((image)->imageData + (image)->widthStep*(row)))[(col)])

/****************************************************************************************\
*                                  Matrix type (MxuMat)                                   *
\****************************************************************************************/
#define MXU_CN_MAX     512
#define MXU_CN_SHIFT   3
#define MXU_DEPTH_MAX  (1 << MXU_CN_SHIFT)

#define MXU_8U   0
#define MXU_8S   1
#define MXU_16U  2
#define MXU_16S  3
#define MXU_32S  4
#define MXU_32F  5
#define MXU_64F  6
#define MXU_USRTYPE1 7

#define MXU_MAT_DEPTH_MASK       (MXU_DEPTH_MAX - 1)
#define MXU_MAT_DEPTH(flags)     ((flags) & MXU_MAT_DEPTH_MASK)

#define MXU_MAKETYPE(depth,cn) (MXU_MAT_DEPTH(depth) + (((cn)-1) << MXU_CN_SHIFT))
#define MXU_MAKE_TYPE MXU_MAKETYPE

#define MXU_8UC1 MXU_MAKETYPE(MXU_8U,1)
#define MXU_8UC2 MXU_MAKETYPE(MXU_8U,2)
#define MXU_8UC3 MXU_MAKETYPE(MXU_8U,3)
#define MXU_8UC4 MXU_MAKETYPE(MXU_8U,4)
#define MXU_8UC(n) MXU_MAKETYPE(MXU_8U,(n))

#define MXU_8SC1 MXU_MAKETYPE(MXU_8S,1)
#define MXU_8SC2 MXU_MAKETYPE(MXU_8S,2)
#define MXU_8SC3 MXU_MAKETYPE(MXU_8S,3)
#define MXU_8SC4 MXU_MAKETYPE(MXU_8S,4)
#define MXU_8SC(n) MXU_MAKETYPE(MXU_8S,(n))

#define MXU_16UC1 MXU_MAKETYPE(MXU_16U,1)
#define MXU_16UC2 MXU_MAKETYPE(MXU_16U,2)
#define MXU_16UC3 MXU_MAKETYPE(MXU_16U,3)
#define MXU_16UC4 MXU_MAKETYPE(MXU_16U,4)
#define MXU_16UC(n) MXU_MAKETYPE(MXU_16U,(n))

#define MXU_16SC1 MXU_MAKETYPE(MXU_16S,1)
#define MXU_16SC2 MXU_MAKETYPE(MXU_16S,2)
#define MXU_16SC3 MXU_MAKETYPE(MXU_16S,3)
#define MXU_16SC4 MXU_MAKETYPE(MXU_16S,4)
#define MXU_16SC(n) MXU_MAKETYPE(MXU_16S,(n))

#define MXU_32SC1 MXU_MAKETYPE(MXU_32S,1)
#define MXU_32SC2 MXU_MAKETYPE(MXU_32S,2)
#define MXU_32SC3 MXU_MAKETYPE(MXU_32S,3)
#define MXU_32SC4 MXU_MAKETYPE(MXU_32S,4)
#define MXU_32SC(n) MXU_MAKETYPE(MXU_32S,(n))

#define MXU_32FC1 MXU_MAKETYPE(MXU_32F,1)
#define MXU_32FC2 MXU_MAKETYPE(MXU_32F,2)
#define MXU_32FC3 MXU_MAKETYPE(MXU_32F,3)
#define MXU_32FC4 MXU_MAKETYPE(MXU_32F,4)
#define MXU_32FC(n) MXU_MAKETYPE(MXU_32F,(n))

#define MXU_64FC1 MXU_MAKETYPE(MXU_64F,1)
#define MXU_64FC2 MXU_MAKETYPE(MXU_64F,2)
#define MXU_64FC3 MXU_MAKETYPE(MXU_64F,3)
#define MXU_64FC4 MXU_MAKETYPE(MXU_64F,4)
#define MXU_64FC(n) MXU_MAKETYPE(MXU_64F,(n))

#define MXU_AUTO_STEP  0x7fffffff
#define MXU_WHOLE_ARR  mxuSlice( 0, 0x3fffffff )

#define MXU_MAT_CN_MASK          ((MXU_CN_MAX - 1) << MXU_CN_SHIFT)
#define MXU_MAT_CN(flags)        ((((flags) & MXU_MAT_CN_MASK) >> MXU_CN_SHIFT) + 1)
#define MXU_MAT_TYPE_MASK        (MXU_DEPTH_MAX*MXU_CN_MAX - 1)
#define MXU_MAT_TYPE(flags)      ((flags) & MXU_MAT_TYPE_MASK)
#define MXU_MAT_CONT_FLAG_SHIFT  14
#define MXU_MAT_CONT_FLAG        (1 << MXU_MAT_CONT_FLAG_SHIFT)
#define MXU_IS_MAT_CONT(flags)   ((flags) & MXU_MAT_CONT_FLAG)
#define MXU_IS_CONT_MAT          MXU_IS_MAT_CONT
#define MXU_SUBMAT_FLAG_SHIFT    15
#define MXU_SUBMAT_FLAG          (1 << MXU_SUBMAT_FLAG_SHIFT)
#define MXU_IS_SUBMAT(flags)     ((flags) & MXU_MAT_SUBMAT_FLAG)

#define MXU_MAGIC_MASK       0xFFFF0000
#define MXU_MAT_MAGIC_VAL    0x42420000
#define MXU_TYPE_NAME_MAT    "jzmxu-matrix"

typedef struct MxuMat
{
    int type;
    int step;

    /* for internal use only */
    int* refcount;
    int hdr_refcount;

    union
    {
        uchar* ptr;
        short* s;
        int* i;
        float* fl;
        double* db;
    } data;

#ifdef __cplusplus
    union
    {
        int rows;
        int height;
    };

    union
    {
        int cols;
        int width;
    };
#else
    int rows;
    int cols;
#endif

}
MxuMat;

#define MXU_IS_MAT_HDR(mat) \
    ((mat) != NULL && \
    (((const MxuMat*)(mat))->type & MXU_MAGIC_MASK) == MXU_MAT_MAGIC_VAL && \
    ((const MxuMat*)(mat))->cols > 0 && ((const MxuMat*)(mat))->rows > 0)

#define MXU_IS_MAT_HDR_Z(mat) \
    ((mat) != NULL && \
    (((const MxuMat*)(mat))->type & MXU_MAGIC_MASK) == MXU_MAT_MAGIC_VAL && \
    ((const MxuMat*)(mat))->cols >= 0 && ((const MxuMat*)(mat))->rows >= 0)

#define MXU_IS_MAT(mat) \
    (MXU_IS_MAT_HDR(mat) && ((const MxuMat*)(mat))->data.ptr != NULL)

#define MXU_IS_MASK_ARR(mat) \
    (((mat)->type & (MXU_MAT_TYPE_MASK & ~MXU_8SC1)) == 0)

#define MXU_ARE_TYPES_EQ(mat1, mat2) \
    ((((mat1)->type ^ (mat2)->type) & MXU_MAT_TYPE_MASK) == 0)

#define MXU_ARE_CNS_EQ(mat1, mat2) \
    ((((mat1)->type ^ (mat2)->type) & MXU_MAT_CN_MASK) == 0)

#define MXU_ARE_DEPTHS_EQ(mat1, mat2) \
    ((((mat1)->type ^ (mat2)->type) & MXU_MAT_DEPTH_MASK) == 0)

#define MXU_ARE_SIZES_EQ(mat1, mat2) \
    ((mat1)->rows == (mat2)->rows && (mat1)->cols == (mat2)->cols)

#define MXU_IS_MAT_CONST(mat)  \
    (((mat)->rows|(mat)->cols) == 1)

/* Size of each channel item,
   0x124489 = 1000 0100 0100 0010 0010 0001 0001 ~ array of sizeof(arr_type_elem) */
#define MXU_ELEM_SIZE1(type) \
    ((((sizeof(size_t)<<28)|0x8442211) >> MXU_MAT_DEPTH(type)*4) & 15)

/* 0x3a50 = 11 10 10 01 01 00 00 ~ array of log2(sizeof(arr_type_elem)) */
#define MXU_ELEM_SIZE(type) \
    (MXU_MAT_CN(type) << ((((sizeof(size_t)/4+1)*16384|0x3a50) >> MXU_MAT_DEPTH(type)*2) & 3))

#define IFT2MXU_DEPTH(depth) \
    ((((MXU_8U)+(MXU_16U<<4)+(MXU_32F<<8)+(MXU_64F<<16)+(MXU_8S<<20)+ \
    (MXU_16S<<24)+(MXU_32S<<28)) >> ((((depth) & 0xF0) >> 2) + \
    (((depth) & IFT_DEPTH_SIGN) ? 20 : 0))) & 15)

/* Inline constructor. No data is allocated internally!!!
 * (Use together with mxuCreateData, or use mxuCreateMat instead to
 * get a matrix with allocated data):
 */
MXU_INLINE MxuMat mxuMat( int rows, int cols, int type, void* data MXU_DEFAULT(NULL))
{
    MxuMat m;

    assert( (unsigned)MXU_MAT_DEPTH(type) <= MXU_64F );
    type = MXU_MAT_TYPE(type);
    m.type = MXU_MAT_MAGIC_VAL | MXU_MAT_CONT_FLAG | type;
    m.cols = cols;
    m.rows = rows;
    m.step = m.cols*MXU_ELEM_SIZE(type);
    m.data.ptr = (uchar*)data;
    m.refcount = NULL;
    m.hdr_refcount = 0;

    return m;
}


#define MXU_MAT_ELEM_PTR_FAST( mat, row, col, pix_size )  \
    (assert( (unsigned)(row) < (unsigned)(mat).rows &&   \
             (unsigned)(col) < (unsigned)(mat).cols ),   \
     (mat).data.ptr + (size_t)(mat).step*(row) + (pix_size)*(col))

#define MXU_MAT_ELEM_PTR( mat, row, col )                 \
    MXU_MAT_ELEM_PTR_FAST( mat, row, col, MXU_ELEM_SIZE((mat).type) )

#define MXU_MAT_ELEM( mat, elemtype, row, col )           \
    (*(elemtype*)MXU_MAT_ELEM_PTR_FAST( mat, row, col, sizeof(elemtype)))


MXU_INLINE  double  mxumGet( const MxuMat* mat, int row, int col )
{
    int type;

    type = MXU_MAT_TYPE(mat->type);
    assert( (unsigned)row < (unsigned)mat->rows &&
            (unsigned)col < (unsigned)mat->cols );

    if( type == MXU_32FC1 )
        return ((float*)(void*)(mat->data.ptr + (size_t)mat->step*row))[col];
    else
    {
        assert( type == MXU_64FC1 );
        return ((double*)(void*)(mat->data.ptr + (size_t)mat->step*row))[col];
    }
}


MXU_INLINE  void  mxumSet( MxuMat* mat, int row, int col, double value )
{
    int type;
    type = MXU_MAT_TYPE(mat->type);
    assert( (unsigned)row < (unsigned)mat->rows &&
            (unsigned)col < (unsigned)mat->cols );

    if( type == MXU_32FC1 )
        ((float*)(void*)(mat->data.ptr + (size_t)mat->step*row))[col] = (float)value;
    else
    {
        assert( type == MXU_64FC1 );
        ((double*)(void*)(mat->data.ptr + (size_t)mat->step*row))[col] = value;
    }
}


MXU_INLINE int mxuIftDepth( int type )
{
    int depth = MXU_MAT_DEPTH(type);
    return MXU_ELEM_SIZE1(depth)*8 | (depth == MXU_8S || depth == MXU_16S ||
           depth == MXU_32S ? IFT_DEPTH_SIGN : 0);
}

/****************************************************************************************\
*                       Multi-dimensional dense array (MxuMatND)                          *
\****************************************************************************************/

#define MXU_MATND_MAGIC_VAL    0x42430000
#define MXU_TYPE_NAME_MATND    "jzmxu-nd-matrix"

#define MXU_MAX_DIM            32
#define MXU_MAX_DIM_HEAP       1024

typedef struct MxuMatND
{
    int type;
    int dims;

    int* refcount;
    int hdr_refcount;

    union
    {
        uchar* ptr;
        float* fl;
        double* db;
        int* i;
        short* s;
    } data;

    struct
    {
        int size;
        int step;
    }
    dim[MXU_MAX_DIM];
}
MxuMatND;

#define MXU_IS_MATND_HDR(mat) \
    ((mat) != NULL && (((const MxuMatND*)(mat))->type & MXU_MAGIC_MASK) == MXU_MATND_MAGIC_VAL)

#define MXU_IS_MATND(mat) \
    (MXU_IS_MATND_HDR(mat) && ((const MxuMatND*)(mat))->data.ptr != NULL)


/****************************************************************************************\
*                      Multi-dimensional sparse array (MxuSparseMat)                      *
\****************************************************************************************/

#define MXU_SPARSE_MAT_MAGIC_VAL    0x42440000
#define MXU_TYPE_NAME_SPARSE_MAT    "jzmxu-sparse-matrix"

struct MxuSet;

typedef struct MxuSparseMat
{
    int type;
    int dims;
    int* refcount;
    int hdr_refcount;

    struct MxuSet* heap;
    void** hashtable;
    int hashsize;
    int valoffset;
    int idxoffset;
    int size[MXU_MAX_DIM];
}
MxuSparseMat;

#define MXU_IS_SPARSE_MAT_HDR(mat) \
    ((mat) != NULL && \
    (((const MxuSparseMat*)(mat))->type & MXU_MAGIC_MASK) == MXU_SPARSE_MAT_MAGIC_VAL)

#define MXU_IS_SPARSE_MAT(mat) \
    MXU_IS_SPARSE_MAT_HDR(mat)

/**************** iteration through a sparse array *****************/

typedef struct MxuSparseNode
{
    unsigned hashval;
    struct MxuSparseNode* next;
}
MxuSparseNode;

typedef struct MxuSparseMatIterator
{
    MxuSparseMat* mat;
    MxuSparseNode* node;
    int curidx;
}
MxuSparseMatIterator;

#define MXU_NODE_VAL(mat,node)   ((void*)((uchar*)(node) + (mat)->valoffset))
#define MXU_NODE_IDX(mat,node)   ((int*)((uchar*)(node) + (mat)->idxoffset))

/****************************************************************************************\
*                                         Histogram                                      *
\****************************************************************************************/

typedef int MxuHistType;

#define MXU_HIST_MAGIC_VAL     0x42450000
#define MXU_HIST_UNIFORM_FLAG  (1 << 10)

/* indicates whether bin ranges are set already or not */
#define MXU_HIST_RANGES_FLAG   (1 << 11)

#define MXU_HIST_ARRAY         0
#define MXU_HIST_SPARSE        1
#define MXU_HIST_TREE          MXU_HIST_SPARSE

/* should be used as a parameter only,
   it turns to MXU_HIST_UNIFORM_FLAG of hist->type */
#define MXU_HIST_UNIFORM       1

typedef struct MxuHistogram
{
    int     type;
    MxuArr*  bins;
    float   thresh[MXU_MAX_DIM][2];  /* For uniform histograms.                      */
    float** thresh2;                /* For non-uniform histograms.                  */
    MxuMatND mat;                    /* Embedded matrix header for array histograms. */
}
MxuHistogram;

#define MXU_IS_HIST( hist ) \
    ((hist) != NULL  && \
     (((MxuHistogram*)(hist))->type & MXU_MAGIC_MASK) == MXU_HIST_MAGIC_VAL && \
     (hist)->bins != NULL)

#define MXU_IS_UNIFORM_HIST( hist ) \
    (((hist)->type & MXU_HIST_UNIFORM_FLAG) != 0)

#define MXU_IS_SPARSE_HIST( hist ) \
    MXU_IS_SPARSE_MAT((hist)->bins)

#define MXU_HIST_HAS_RANGES( hist ) \
    (((hist)->type & MXU_HIST_RANGES_FLAG) != 0)

/****************************************************************************************\
*                      Other supplementary data type definitions                         *
\****************************************************************************************/

/*************************************** MxuRect *****************************************/

typedef struct MxuRect
{
    int x;
    int y;
    int width;
    int height;
}
MxuRect;

MXU_INLINE  MxuRect  mxuRect( int x, int y, int width, int height )
{
    MxuRect r;

    r.x = x;
    r.y = y;
    r.width = width;
    r.height = height;

    return r;
}


MXU_INLINE  IftROI  mxuRectToROI( MxuRect rect, int coi )
{
    IftROI roi;
    roi.xOffset = rect.x;
    roi.yOffset = rect.y;
    roi.width = rect.width;
    roi.height = rect.height;
    roi.coi = coi;

    return roi;
}


MXU_INLINE  MxuRect  mxuROIToRect( IftROI roi )
{
    return mxuRect( roi.xOffset, roi.yOffset, roi.width, roi.height );
}

/*********************************** MxuTermCriteria *************************************/

#define MXU_TERMCRIT_ITER    1
#define MXU_TERMCRIT_NUMBER  MXU_TERMCRIT_ITER
#define MXU_TERMCRIT_EPS     2

typedef struct MxuTermCriteria
{
    int    type;  /* may be combination of
                     MXU_TERMCRIT_ITER
                     MXU_TERMCRIT_EPS */
    int    max_iter;
    double epsilon;
}
MxuTermCriteria;

MXU_INLINE  MxuTermCriteria  mxuTermCriteria( int type, int max_iter, double epsilon )
{
    MxuTermCriteria t;

    t.type = type;
    t.max_iter = max_iter;
    t.epsilon = (float)epsilon;

    return t;
}
/******************************* MxuPoint and variants ***********************************/

typedef struct MxuPoint
{
    int x;
    int y;
}
MxuPoint;


MXU_INLINE  MxuPoint  mxuPoint( int x, int y )
{
    MxuPoint p;

    p.x = x;
    p.y = y;

    return p;
}


typedef struct MxuPoint2D32f
{
    float x;
    float y;
}
MxuPoint2D32f;


MXU_INLINE  MxuPoint2D32f  mxuPoint2D32f( double x, double y )
{
    MxuPoint2D32f p;

    p.x = (float)x;
    p.y = (float)y;

    return p;
}


MXU_INLINE  MxuPoint2D32f  mxuPointTo32f( MxuPoint point )
{
    return mxuPoint2D32f( (float)point.x, (float)point.y );
}


MXU_INLINE  MxuPoint  mxuPointFrom32f( MxuPoint2D32f point )
{
    MxuPoint ipt;
    ipt.x = mxuRound(point.x);
    ipt.y = mxuRound(point.y);

    return ipt;
}


typedef struct MxuPoint3D32f
{
    float x;
    float y;
    float z;
}
MxuPoint3D32f;


MXU_INLINE  MxuPoint3D32f  mxuPoint3D32f( double x, double y, double z )
{
    MxuPoint3D32f p;

    p.x = (float)x;
    p.y = (float)y;
    p.z = (float)z;

    return p;
}


typedef struct MxuPoint2D64f
{
    double x;
    double y;
}
MxuPoint2D64f;


MXU_INLINE  MxuPoint2D64f  mxuPoint2D64f( double x, double y )
{
    MxuPoint2D64f p;

    p.x = x;
    p.y = y;

    return p;
}


typedef struct MxuPoint3D64f
{
    double x;
    double y;
    double z;
}
MxuPoint3D64f;


MXU_INLINE  MxuPoint3D64f  mxuPoint3D64f( double x, double y, double z )
{
    MxuPoint3D64f p;

    p.x = x;
    p.y = y;
    p.z = z;

    return p;
}

/******************************** MxuSize's & MxuBox **************************************/

typedef struct MxuSize
{
    int width;
    int height;
}
MxuSize;

MXU_INLINE  MxuSize  mxuSize( int width, int height )
{
    MxuSize s;

    s.width = width;
    s.height = height;

    return s;
}

typedef struct MxuSize2D32f
{
    float width;
    float height;
}
MxuSize2D32f;


MXU_INLINE  MxuSize2D32f  mxuSize2D32f( double width, double height )
{
    MxuSize2D32f s;

    s.width = (float)width;
    s.height = (float)height;

    return s;
}

typedef struct MxuBox2D
{
    MxuPoint2D32f center;  /* Center of the box.                          */
    MxuSize2D32f  size;    /* Box width and length.                       */
    float angle;          /* Angle between the horizontal axis           */
                          /* and the first side (i.e. length) in degrees */
}
MxuBox2D;


/* Line iterator state: */
typedef struct MxuLineIterator
{
    /* Pointer to the current point: */
    uchar* ptr;

    /* Bresenham algorithm state: */
    int  err;
    int  plus_delta;
    int  minus_delta;
    int  plus_step;
    int  minus_step;
}
MxuLineIterator;



/************************************* MxuSlice ******************************************/

typedef struct MxuSlice
{
    int  start_index, end_index;
}
MxuSlice;

MXU_INLINE  MxuSlice  mxuSlice( int start, int end )
{
    MxuSlice slice;
    slice.start_index = start;
    slice.end_index = end;

    return slice;
}

#define MXU_WHOLE_SEQ_END_INDEX 0x3fffffff
#define MXU_WHOLE_SEQ  mxuSlice(0, MXU_WHOLE_SEQ_END_INDEX)


/************************************* MxuScalar *****************************************/

typedef struct MxuScalar
{
    double val[4];
}
MxuScalar;

MXU_INLINE  MxuScalar  mxuScalar( double val0, double val1 MXU_DEFAULT(0),
                               double val2 MXU_DEFAULT(0), double val3 MXU_DEFAULT(0))
{
    MxuScalar scalar;
    scalar.val[0] = val0; scalar.val[1] = val1;
    scalar.val[2] = val2; scalar.val[3] = val3;
    return scalar;
}


MXU_INLINE  MxuScalar  mxuRealScalar( double val0 )
{
    MxuScalar scalar;
    scalar.val[0] = val0;
    scalar.val[1] = scalar.val[2] = scalar.val[3] = 0;
    return scalar;
}

MXU_INLINE  MxuScalar  mxuScalarAll( double val0123 )
{
    MxuScalar scalar;
    scalar.val[0] = val0123;
    scalar.val[1] = val0123;
    scalar.val[2] = val0123;
    scalar.val[3] = val0123;
    return scalar;
}



/****************************************************************************************\
*                                   Dynamic Data structures                              *
\****************************************************************************************/
//add by cfwang
#define MXU_CMP_EQ   0
#define MXU_CMP_GT   1
#define MXU_CMP_GE   2
#define MXU_CMP_LT   3
#define MXU_CMP_LE   4
#define MXU_CMP_NE   5



/******************************** Memory storage ****************************************/

typedef struct MxuMemBlock
{
    struct MxuMemBlock*  prev;
    struct MxuMemBlock*  next;
}
MxuMemBlock;

#define MXU_STORAGE_MAGIC_VAL    0x42890000

typedef struct MxuMemStorage
{
    int signature;
    MxuMemBlock* bottom;           /* First allocated block.                   */
    MxuMemBlock* top;              /* Current memory block - top of the stack. */
    struct  MxuMemStorage* parent; /* We get new blocks from parent as needed. */
    int block_size;               /* Block size.                              */
    int free_space;               /* Remaining free space in current block.   */
}
MxuMemStorage;

#define MXU_IS_STORAGE(storage)  \
    ((storage) != NULL &&       \
    (((MxuMemStorage*)(storage))->signature & MXU_MAGIC_MASK) == MXU_STORAGE_MAGIC_VAL)


typedef struct MxuMemStoragePos
{
    MxuMemBlock* top;
    int free_space;
}
MxuMemStoragePos;

/* Creates new memory storage.
 *    block_size == 0 means that default,
 *       somewhat optimal size, is used (currently, it is 64K) */
MXUAPI(MxuMemStorage*)  mxuCreateMemStorage( int block_size MXU_DEFAULT(0));


/*********************************** Sequence *******************************************/

typedef struct MxuSeqBlock
{
    struct MxuSeqBlock*  prev; /* Previous sequence block.                   */
    struct MxuSeqBlock*  next; /* Next sequence block.                       */
  int    start_index;         /* Index of the first element in the block +  */
                              /* sequence->first->start_index.              */
    int    count;             /* Number of elements in the block.           */
    schar* data;              /* Pointer to the first element of the block. */
}
MxuSeqBlock;


#define MXU_TREE_NODE_FIELDS(node_type)                               \
    int       flags;             /* Miscellaneous flags.     */      \
    int       header_size;       /* Size of sequence header. */      \
    struct    node_type* h_prev; /* Previous sequence.       */      \
    struct    node_type* h_next; /* Next sequence.           */      \
    struct    node_type* v_prev; /* 2nd previous sequence.   */      \
    struct    node_type* v_next  /* 2nd next sequence.       */

/*
   Read/Write sequence.
   Elements can be dynamically inserted to or deleted from the sequence.
*/
#define MXU_SEQUENCE_FIELDS()                                              \
    MXU_TREE_NODE_FIELDS(MxuSeq);                                           \
    int       total;          /* Total number of elements.            */  \
    int       elem_size;      /* Size of sequence element in bytes.   */  \
    schar*    block_max;      /* Maximal bound of the last block.     */  \
    schar*    ptr;            /* Current write pointer.               */  \
    int       delta_elems;    /* Grow seq this many at a time.        */  \
    MxuMemStorage* storage;    /* Where the seq is stored.             */  \
    MxuSeqBlock* free_blocks;  /* Free blocks list.                    */  \
    MxuSeqBlock* first;        /* Pointer to the first sequence block. */

typedef struct MxuSeq
{
    MXU_SEQUENCE_FIELDS()
}
MxuSeq;


#define MXU_SEQ_READER_FIELDS()                                      \
    int          header_size;                                       \
    MxuSeq*       seq;        /* sequence, beign read */             \
    MxuSeqBlock*  block;      /* current block */                    \
    schar*       ptr;        /* pointer to element be read next */  \
    schar*       block_min;  /* pointer to the beginning of block */\
    schar*       block_max;  /* pointer to the end of block */      \
    int          delta_index;/* = seq->first->start_index   */      \
    schar*       prev_elem;  /* pointer to previous element */


typedef struct MxuSeqReader
{
    MXU_SEQ_READER_FIELDS()
}
MxuSeqReader;



#define MXU_SEQ_MAGIC_VAL             0x42990000

#define MXU_IS_SEQ(seq) \
    ((seq) != NULL && (((MxuSeq*)(seq))->flags & MXU_MAGIC_MASK) == MXU_SEQ_MAGIC_VAL)


#define MXU_SEQ_ELTYPE_PTR            MXU_USRTYPE1
#define MXU_SEQ_ELTYPE_PPOINT         MXU_SEQ_ELTYPE_PTR  /* &(x,y) */
#define MXU_SEQ_ELTYPE_BITS           12
#define MXU_SEQ_ELTYPE_MASK           ((1 << MXU_SEQ_ELTYPE_BITS) - 1)
#define MXU_SEQ_ELTYPE( seq )   ((seq)->flags & MXU_SEQ_ELTYPE_MASK)
/* type checking macros */
#define MXU_IS_SEQ_POINT_SET( seq ) \
    ((MXU_SEQ_ELTYPE(seq) == MXU_32SC2 || MXU_SEQ_ELTYPE(seq) == MXU_32FC2))

/* types of sequences */
#define MXU_SEQ_KIND_GENERIC     (0 << MXU_SEQ_ELTYPE_BITS)
#define MXU_SEQ_KIND_CURVE       (1 << MXU_SEQ_ELTYPE_BITS)
#define MXU_SEQ_KIND_BIN_TREE    (2 << MXU_SEQ_ELTYPE_BITS)


#define MXU_SEQ_KIND_BITS        2
#define MXU_SEQ_KIND_MASK        (((1 << MXU_SEQ_KIND_BITS) - 1)<<MXU_SEQ_ELTYPE_BITS)
#define MXU_SEQ_KIND( seq )     ((seq)->flags & MXU_SEQ_KIND_MASK )

/* Move reader position forward: */
#define MXU_NEXT_SEQ_ELEM( elem_size, reader )                 \
{                                                             \
    if( ((reader).ptr += (elem_size)) >= (reader).block_max ) \
    {                                                         \
        mxuChangeSeqBlock( &(reader), 1 );                     \
    }                                                         \
}


/* Read element and move read position forward: */
#define MXU_READ_SEQ_ELEM( elem, reader )                       \
{                                                              \
    assert( (reader).seq->elem_size == sizeof(elem));          \
    memcpy( &(elem), (reader).ptr, sizeof((elem)));            \
    MXU_NEXT_SEQ_ELEM( sizeof(elem), reader )                   \
}



enum {
 MXU_StsNullPtr=                -27, /* null pointer */
 MXU_StsUnsupportedFormat=      -210, /* the data format/type is not supported by the function*/
 MXU_StsOutOfRange=             -211, /* some of parameters are out of range */
};

/*********************************** Chain/Countour *************************************/

typedef struct MxuChain
{
    MXU_SEQUENCE_FIELDS()
    MxuPoint  origin;
}
MxuChain;

#define MXU_CONTOUR_FIELDS()  \
    MXU_SEQUENCE_FIELDS()     \
    MxuRect rect;             \
    int color;               \
    int reserved[3];

typedef struct MxuContour
{
    MXU_CONTOUR_FIELDS()
}
MxuContour;

typedef MxuContour MxuPoint2DSeq;




/****************************************************************************************/
/*                            Sequence writer & reader                                  */
/****************************************************************************************/

#define MXU_SEQ_WRITER_FIELDS()                                     \
    int          header_size;                                      \
    MxuSeq*       seq;        /* the sequence written */            \
    MxuSeqBlock*  block;      /* current block */                   \
    schar*       ptr;        /* pointer to free space */           \
    schar*       block_min;  /* pointer to the beginning of block*/\
    schar*       block_max;  /* pointer to the end of block */

typedef struct MxuSeqWriter
{
    MXU_SEQ_WRITER_FIELDS()
}
MxuSeqWriter;








/****************************************************************************************\
*                                    Sequence types                                      *
\****************************************************************************************/


#define MXU_SEQ_ELTYPE_INDEX          MXU_32SC1  /* #(x,y) */


#define MXU_SEQ_KIND_BITS        2
#define MXU_SEQ_ELTYPE_GENERIC        0
#define MXU_SEQ_ELTYPE_BITS           12
#define MXU_SEQ_FLAG_SHIFT       (MXU_SEQ_KIND_BITS + MXU_SEQ_ELTYPE_BITS)

/* flags for curves */
#define MXU_SEQ_FLAG_CLOSED     (1 << MXU_SEQ_FLAG_SHIFT)
#define MXU_SEQ_FLAG_SIMPLE     (0 << MXU_SEQ_FLAG_SHIFT)
#define MXU_SEQ_FLAG_CONVEX     (0 << MXU_SEQ_FLAG_SHIFT)
#define MXU_SEQ_FLAG_HOLE       (2 << MXU_SEQ_FLAG_SHIFT)


#define MXU_IS_SEQ_CURVE( seq )      (MXU_SEQ_KIND(seq) == MXU_SEQ_KIND_CURVE)
#define MXU_IS_SEQ_CLOSED( seq )     (((seq)->flags & MXU_SEQ_FLAG_CLOSED) != 0)
#define MXU_IS_SEQ_CONVEX( seq )     0
#define MXU_IS_SEQ_HOLE( seq )       (((seq)->flags & MXU_SEQ_FLAG_HOLE) != 0)
#define MXU_IS_SEQ_SIMPLE( seq )     1


#define MXU_IS_SEQ_POLYLINE( seq )   \
    (MXU_SEQ_KIND(seq) == MXU_SEQ_KIND_CURVE && MXU_IS_SEQ_POINT_SET(seq))

#define MXU_IS_SEQ_CHAIN( seq )   \
    (MXU_SEQ_KIND(seq) == MXU_SEQ_KIND_CURVE && (seq)->elem_size == 1)

/****************************************************************************************/
/*                                Operations on sequences                               */
/****************************************************************************************/

#define MXU_WRITE_SEQ_ELEM( elem, writer )             \
{                                                     \
    assert( (writer).seq->elem_size == sizeof(elem)); \
    if( (writer).ptr >= (writer).block_max )          \
    {                                                 \
        mxuCreateSeqBlock( &writer);                   \
    }                                                 \
    assert( (writer).ptr <= (writer).block_max - sizeof(elem));\
    memcpy((writer).ptr, &(elem), sizeof(elem));      \
    (writer).ptr += sizeof(elem);                     \
}


#endif /*__JZ_MXU_CORE_TYPE_C_H__*/

/* End of file. */
