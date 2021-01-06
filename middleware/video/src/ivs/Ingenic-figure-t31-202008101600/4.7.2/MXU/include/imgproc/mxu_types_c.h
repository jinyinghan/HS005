
#ifndef __JZ_MXU_IMGPROC_TYPES_C_H__
#define __JZ_MXU_IMGPROC_TYPES_C_H__

#include "./../core/mxu_core_c.h"

#ifdef __cplusplus
extern "C" {
#endif


/* Contour approximation algorithms */
enum
{
    MXU_POLY_APPROX_DP = 0 
};



/* Image smooth methods */
enum
{
    MXU_BLUR_NO_SCALE =0,
    MXU_BLUR  =1,
    MXU_GAUSSIAN  =2,
    MXU_MEDIAN =3,
    MXU_BILATERAL =4
};

/* Filters used in pyramid decomposition */
enum
{
    MXU_GAUSSIAN_5x5 = 7
};

/* Special filters */
enum
{
    MXU_SCHARR =-1,
    MXU_MAX_SOBEL_KSIZE =7
};



/* Shapes of a structuring element for morphological operations */
enum
{
    MXU_SHAPE_RECT      =0,
    MXU_SHAPE_CROSS     =1,
    MXU_SHAPE_ELLIPSE   =2,
    MXU_SHAPE_CUSTOM    =100
};

/* Morphological operations */
enum
{
    MXU_MOP_ERODE        =0,
    MXU_MOP_DILATE       =1,
    MXU_MOP_OPEN         =2,
    MXU_MOP_CLOSE        =3,
    MXU_MOP_GRADIENT     =4,
    MXU_MOP_TOPHAT       =5,
    MXU_MOP_BLACKHAT     =6
};


/* Threshold types */
enum
{
    MXU_THRESH_BINARY      =0,  /* value = value > threshold ? max_value : 0       */
    MXU_THRESH_BINARY_INV  =1,  /* value = value > threshold ? 0 : max_value       */
    MXU_THRESH_TRUNC       =2,  /* value = value > threshold ? threshold : value   */
    MXU_THRESH_TOZERO      =3,  /* value = value > threshold ? value : 0           */
    MXU_THRESH_TOZERO_INV  =4,  /* value = value > threshold ? 0 : value           */
    MXU_THRESH_MASK        =7,
    MXU_THRESH_OTSU        =8  /* use Otsu algorithm to choose the optimal threshold value;
                                 combine the flag with one of the above MXU_THRESH_* values */
};

/* Adaptive threshold methods */
enum
{
    MXU_ADAPTIVE_THRESH_MEAN_C  =0,
    MXU_ADAPTIVE_THRESH_GAUSSIAN_C  =1
};

/* Template matching methods */
enum
{
    MXU_TM_SQDIFF        =0,
    MXU_TM_SQDIFF_NORMED =1,
    MXU_TM_CCORR         =2,
    MXU_TM_CCORR_NORMED  =3,
    MXU_TM_CCOEFF        =4,
    MXU_TM_CCOEFF_NORMED =5
};

/* Contour retrieval modes */
enum
{
    MXU_RETR_EXTERNAL=0,
    MXU_RETR_LIST=1,
    MXU_RETR_CCOMP=2,
    MXU_RETR_TREE=3,
    MXU_RETR_FLOODFILL=4
};

/* Contour approximation methods */
enum
{
    MXU_CHAIN_CODE=0,
    MXU_CHAIN_APPROX_NONE=1,
    MXU_CHAIN_APPROX_SIMPLE=2,
    MXU_CHAIN_APPROX_TC89_L1=3,
    MXU_CHAIN_APPROX_TC89_KCOS=4,
    MXU_LINK_RUNS=5
};

/* Shape orientation */
enum
{
  MXU_CLOCKWISE         =1,
  MXU_COUNTER_CLOCKWISE =2
};


//add by cfwang
enum
{
  MXU_WARP_FILL_OUTLIERS =8,
  MXU_WARP_INVERSE_MAP  =16
};
enum
{
  MXU_INTER_NN        =0,
  MXU_INTER_LINEAR    =1,
  MXU_INTER_CUBIC     =2,
  MXU_INTER_AREA      =3,
  MXU_INTER_LANCZOS4  =4
};


enum ShapeMatchModes
{
    MXU_CONTOURS_MATCH_I1  =1, //!< \f[I_1(A,B) =  \sum _{i=1...7}  \left |  \frac{1}{m^A_i} -  \frac{1}{m^B_i} \right |\f]
    MXU_CONTOURS_MATCH_I2  =2, //!< \f[I_2(A,B) =  \sum _{i=1...7}  \left | m^A_i - m^B_i  \right |\f]
    MXU_CONTOURS_MATCH_I3  =3  //!< \f[I_3(A,B) =  \max _{i=1...7}  \frac{ \left| m^A_i - m^B_i \right| }{ \left| m^A_i \right| }\f]
};



/** Spatial and central moments */

typedef struct MxuMoments
{
    double  m00, m10, m01, m20, m11, m02, m30, m21, m12, m03; /**< spatial moments */
    double  mu20, mu11, mu02, mu30, mu21, mu12, mu03; /**< central moments */
    double  inv_sqrt_m00; /**< m00 != 0 ? 1/sqrt(m00) : 0 */
}MxuMoments;
/** Hu invariants */
typedef struct MxuHuMoments
{
   double hu1, hu2, hu3, hu4, hu5, hu6, hu7; /**< Hu invariants */
}MxuHuMoments;
/** Contour retrieval modes */

#ifdef __cplusplus
}
#endif

#endif
