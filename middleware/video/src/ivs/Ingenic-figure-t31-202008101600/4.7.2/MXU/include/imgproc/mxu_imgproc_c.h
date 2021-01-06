#ifndef __JZ_MXU_IMGPROC_IMGPROC_C_H__
#define __JZ_MXU_IMGPROC_IMGPROC_C_H__

//#include "/home/lzwang/function/common/include/core/mxu_core_c.h"
//#include "/home/lzwang/function/common/include/imgproc/mxu_types_c.h"
#include "./../core/mxu_core_c.h"
#include "mxu_types_c.h"
#include <stdlib.h>

using namespace std;

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************************************\
*                                    Image Processing                                    *
\****************************************************************************************/

/* Convolves the image with the kernel */
MXUAPI(void) mxuFilter2D( const MxuArr* src, MxuArr* dst, const MxuMat* kernel,
                        MxuPoint anchor MXU_DEFAULT(mxuPoint(-1,-1)));

///////////////////////////////deriv///////////////////////////////////
MXUAPI(void) mxuHighSobel(const MxuArr* src,MxuArr* dst,int xorder,
                           int yorder,int aperture_size);
/* Calculates an image derivative using generalized Sobel
   (aperture_size = 1,3,5,7) or Scharr (aperture_size = -1) operator.
   Scharr can be used only for the first dx or dy derivative */
MXUAPI(void) mxuSobel( const MxuArr* src, MxuArr* dst,
                    int xorder, int yorder,
                    int aperture_size MXU_DEFAULT(3));

/* Calculates the image Laplacian: (d2/dx + d2/dy)I */
MXUAPI(void) mxuLaplace( const MxuArr* src, MxuArr* dst,
                      int aperture_size MXU_DEFAULT(3) );



/* creates structuring element used for morphological operations */
MXUAPI(IftConvKernel*)  mxuCreateStructuringElementEx(
            int cols, int  rows, int  anchor_x, int  anchor_y,
            int shape, int* values MXU_DEFAULT(NULL) );

/* releases structuring element */
MXUAPI(void)  mxuReleaseStructuringElement( IftConvKernel** element );

/* erodes input image (applies minimum filter) one or more times.
   If element pointer is NULL, 3x3 rectangular element is used */
MXUAPI(void)  mxuErode( const MxuArr* src, MxuArr* dst,
                      IftConvKernel* element MXU_DEFAULT(NULL),
                      int iterations MXU_DEFAULT(1) );

/* dilates input image (applies maximum filter) one or more times.
   If element pointer is NULL, 3x3 rectangular element is used */
MXUAPI(void)  mxuDilate( const MxuArr* src, MxuArr* dst,
                       IftConvKernel* element MXU_DEFAULT(NULL),
                       int iterations MXU_DEFAULT(1) );

/* Performs complex morphological transformation */
MXUAPI(void)  mxuMorphologyEx( const MxuArr* src, MxuArr* dst,
                             MxuArr* temp, IftConvKernel* element,
                             int operation, int iterations MXU_DEFAULT(1) );

MXUAPI(void)  convertConvKernel( const IftConvKernel* src, mxu::Mat& dst, mxu::Point& anchor);

MXUAPI(void)  mxuHighErode( const MxuArr* src, MxuArr* dst,
                      IftConvKernel* element MXU_DEFAULT(NULL),
                      int iterations MXU_DEFAULT(1) );

MXUAPI(void)  mxuHighDilate( const MxuArr* src, MxuArr* dst,
                       IftConvKernel* element MXU_DEFAULT(NULL),
                       int iterations MXU_DEFAULT(1) );

MXUAPI(void)  mxuHighMorphologyEx( const MxuArr* src, MxuArr* dst,
                             MxuArr* temp, IftConvKernel* element,
                             int operation, int iterations MXU_DEFAULT(1) );



MXUAPI(void) mxuThreshold(const MxuArr* src, MxuArr* dst,
			  double  threshold, double  max_value,
                            int threshold_type );

MXUAPI(void) mxuAdaptiveThreshold( const MxuArr* srcIm, MxuArr* dstIm, double maxValue,
         int method, int type, int blockSize, double delta);

//////////////////////////////findcontours/////////////////////////////
MXUAPI(void) mxuMergeThreshold(const MxuArr* src, MxuArr* dst,
			  double  threshold, double  max_value,
                            int threshold_type );

MXUAPI(int) mxuContours(void* img,vector< vector<MxuPoint> >& contours,
			int  mode,int  method, MxuPoint offset);

MXUAPI(MxuRect) mxuBoundRect( vector<MxuPoint> contour);

MXUAPI(void) drawcontours(MxuMat* src,const vector< vector<MxuPoint> >& contours);

MXUAPI(void) drawrects(MxuMat* src,const vector< vector<MxuPoint> >& contours);


///////////////////////////////integral////////////////////////////////
MXUAPI(void) mxuIntegral( const MxuArr* image, MxuArr* sumImage,
                          MxuArr* sumSqImage, MxuArr* tiltedSumImage );


MXUAPI(void) mxuResize(const MxuArr*srcarr,MxuArr*dstarr,int method);

/* Finds minimum area rotated rectangle bounding a set of points */
MXUAPI(MxuBox2D)  mxuMinAreaRect2( const MxuArr* points,
                                    MxuMemStorage* storage MXU_DEFAULT(NULL));

/****************************************************************************************\
 * *                            Contour Processing and Shape Analysis                       *
 * \****************************************************************************************/
/* Approximates a single polygonal curve (contour) or
 *    a tree of polygonal curves (contours) */
MXUAPI(MxuSeq*)  mxuApproxPoly( const void* src_seq,
    int header_size, MxuMemStorage* storage,
    int method, double eps,
    int recursive MXU_DEFAULT(0));



/* Initializes sequence header for a matrix (column or row vector) of points -
 *    a wrapper for mxuMakeSeqHeaderForArray (it does not initialize bounding rectangle!!!) */
MXUAPI(MxuSeq*) mxuPointSeqFromMat( int seq_kind, const MxuArr* mat,
    MxuContour* contour_header,
    MxuSeqBlock* block );

/* Calculates exact convex hull of 2d point set */
MXUAPI(MxuSeq*) mxuConvexHull2( const MxuArr* input,
    void* hull_storage MXU_DEFAULT(NULL),
    int orientation MXU_DEFAULT(MXU_CLOCKWISE),
    int return_points MXU_DEFAULT(0));

/* Calculates contour bounding rectangle (update=1) or
 *    just retrieves pre-calculated rectangle (update=0) */
MXUAPI(MxuRect)  mxuBoundingRect( MxuArr* points, int update MXU_DEFAULT(0) );




#ifdef __cplusplus
}
#endif

#endif
