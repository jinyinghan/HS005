
#ifndef __JZMXU_TRACKING_HPP__
#define __JZMXU_TRACKING_HPP__

#include "./../core/mxu_core.hpp"
#include "./../imgproc/mxu_imgproc.hpp"

#ifdef __cplusplus
extern "C" {
#endif

/********************************* motion templates *************************************/

/****************************************************************************************\
*        All the motion template functions work only with single channel images.         *
*        Silhouette image must have depth IPL_DEPTH_8U or IPL_DEPTH_8S                   *
*        Motion history image must have depth IPL_DEPTH_32F,                             *
*        Gradient mask - IPL_DEPTH_8U or IPL_DEPTH_8S,                                   *
*        Motion orientation image - IPL_DEPTH_32F                                        *
*        Segmentation mask - IPL_DEPTH_32F                                               *
*        All the angles are in degrees, all the times are in milliseconds                *
\****************************************************************************************/

/* Updates motion history image given motion silhouette */
MXUAPI(void)    mxuUpdateMotionHistory( const MxuArr* silhouette, MxuArr* mhi,
                                      uchar timestamp, int duration);


/* Calculates gradient of the motion history image and fills
   a mask indicating where the gradient is valid */
MXUAPI(void)    mxuCalcMotionGradient( const MxuArr* mhi, MxuArr* mask, MxuArr* orientation,
                                     uchar delta1, uchar delta2,
                                     int aperture_size MXU_DEFAULT(3));

/* Calculates average motion direction within a selected motion region
   (region can be selected by setting ROIs and/or by composing a valid gradient mask
   with the region mask) */
MXUAPI(short)  mxuCalcBaseOrientation( const MxuArr* orientation, const MxuArr* mask);

MXUAPI(short)  mxuCalcGlobalOrientation( const MxuArr* orientation, const MxuArr* mask,
                                        const MxuArr* mhi, uchar timestamp,
                                        uchar duration );


#ifdef __cplusplus
}

enum { OPTFLOW_USE_INITIAL_FLOW = 4,
       OPTFLOW_LK_GET_MIN_EIGENVALS = 8,
       OPTFLOW_FARNEBACK_GAUSSIAN = 256
     };

namespace mxu
{

//! updates motion history image using the current silhouette
MXU_EXPORTS_W void updateMotionHistory( InputArray silhouette, InputOutputArray mhi,
                                       uchar timestamp, int duration);
MXU_EXPORTS_W void updateMotionHistory( MxuMat* silhouette, MxuMat* mhi,
                                      uchar timestamp, int duration);

//! computes the motion gradient orientation image from the motion history image
MXU_EXPORTS_W void calcMotionGradient( InputArray mhi, OutputArray mask,
                                      OutputArray orientation,
                                      uchar delta1, uchar delta2,
                                      int apertureSize=3 );

//! computes the global orientation of the selected motion history image part
MXU_EXPORTS_W short calcBaseOrientation( InputArray orientation, InputArray mask);

MXU_EXPORTS_W short calcGlobalOrientation( InputArray orientation, InputArray mask,
                                           InputArray mhi, uchar timestamp,
                                           uchar duration);

//! constructs a pyramid which can be used as input for calcOpticalFlowPyrLK duank
MXU_EXPORTS_W int buildOpticalFlowPyramid(InputArray img, OutputArrayOfArrays pyramid,
                                         Size winSize, int maxLevel, bool withDerivatives = true,
                                         int pyrBorder = BORDER_REFLECT_101, int derivBorder = BORDER_CONSTANT,
                                         bool tryReuseInputImage = true);

//! computes sparse optical flow using multi-scale Lucas-Kanade algorithm duank
MXU_EXPORTS_W void calcOpticalFlowPyrLK( InputArray prevImg, InputArray nextImg,
                           InputArray prevPts, InputOutputArray nextPts,
                           OutputArray status, OutputArray err,
                           Size winSize=Size(21,21), int maxLevel=3,
                           TermCriteria criteria=TermCriteria(TermCriteria::COUNT+TermCriteria::EPS, 30, 0.01),
                           int flags=0, double minEigThreshold=1e-4);

}
#endif

#endif
