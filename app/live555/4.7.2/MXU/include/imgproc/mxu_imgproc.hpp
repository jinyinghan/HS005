
#ifndef __JZ_MXU_IMGPROC_HPP__
#define __JZ_MXU_IMGPROC_HPP__

#include "./../core/mxu_core.hpp"
#include "mxu_types_c.h"

#ifdef __cplusplus

#define openBlur 1
/*! \namespace mxu
 Namespace where all the C++ JzMxu functionality resides
 */
namespace mxu
{

static inline Point normalizeAnchor( Point anchor, Size ksize )
{
    if( anchor.x == -1 )
        anchor.x = ksize.width/2;
    if( anchor.y == -1 )
        anchor.y = ksize.height/2;
    assert( anchor.inside(Rect(0, 0, ksize.width, ksize.height)) );
    return anchor;
}

//! various border interpolation methods
enum { BORDER_REPLICATE=IFT_BORDER_REPLICATE, BORDER_CONSTANT=IFT_BORDER_CONSTANT,
       BORDER_REFLECT=IFT_BORDER_REFLECT, BORDER_WRAP=IFT_BORDER_WRAP,
       BORDER_REFLECT_101=IFT_BORDER_REFLECT_101, BORDER_REFLECT101=BORDER_REFLECT_101,
       BORDER_TRANSPARENT=IFT_BORDER_TRANSPARENT,
       BORDER_DEFAULT=BORDER_REFLECT_101, BORDER_ISOLATED=16 };

//! 1D interpolation function: returns coordinate of the "donor" pixel for the specified location p.
MXU_EXPORTS_W int borderInterpolate( int p, int len, int borderType );

/*!
 The Base Class for 1D or Row-wise Filters
*/
class MXU_EXPORTS BaseRowFilter
{
public:
    //! the default constructor
    BaseRowFilter();
    //! the destructor
    virtual ~BaseRowFilter();
    //! the filtering operator. Must be overridden in the derived classes. The horizontal border interpolation is done outside of the class.
    virtual void operator()(const uchar* src, uchar* dst,
                            int width, int cn) = 0;
    int ksize, anchor;
};


/*!
 The Base Class for Column-wise Filters
 */
class MXU_EXPORTS BaseColumnFilter
{
public:
    //! the default constructor
    BaseColumnFilter();
    //! the destructor
    virtual ~BaseColumnFilter();
    //! the filtering operator. Must be overridden in the derived classes. The vertical border interpolation is done outside of the class.
    virtual void operator()(const uchar** src, uchar* dst, int dststep,
                            int dstcount, int width) = 0;
    //! resets the internal buffers, if any
    virtual void reset();
    int ksize, anchor;
};

/*!
 The Base Class for Non-Separable 2D Filters.
*/
class MXU_EXPORTS BaseFilter
{
public:
    //! the default constructor
    BaseFilter();
    //! the destructor
    virtual ~BaseFilter();
    //! the filtering operator. The horizontal and the vertical border interpolation is done outside of the class.
    virtual void operator()(const uchar** src, uchar* dst, int dststep,
                            int dstcount, int width, int cn) = 0;
    //! resets the internal buffers, if any
    virtual void reset();
    Size ksize;
    Point anchor;
};

/*!
 The Main Class for Image Filtering.
*/
class MXU_EXPORTS FilterEngine
{
public:
    //! the default constructor
    FilterEngine();
    //! the full constructor. Either _filter2D or both _rowFilter and _columnFilter must be non-empty.
    FilterEngine(const Ptr<BaseFilter>& _filter2D,
                 const Ptr<BaseRowFilter>& _rowFilter,
                 const Ptr<BaseColumnFilter>& _columnFilter,
                 int srcType, int dstType, int bufType,
                 int _rowBorderType=BORDER_REPLICATE,
                 int _columnBorderType=-1,
                 const Scalar& _borderValue=Scalar());
    //! the destructor
    virtual ~FilterEngine();
    //! reinitializes the engine. The previously assigned filters are released.
    void init(const Ptr<BaseFilter>& _filter2D,
              const Ptr<BaseRowFilter>& _rowFilter,
              const Ptr<BaseColumnFilter>& _columnFilter,
              int srcType, int dstType, int bufType,
              int _rowBorderType=BORDER_REPLICATE, int _columnBorderType=-1,
              const Scalar& _borderValue=Scalar());
    //! starts filtering of the specified ROI of an image of size wholeSize.
    virtual int start(Size wholeSize, Rect roi, int maxBufRows=-1);
    //! starts filtering of the specified ROI of the specified image.
    virtual int start(const Mat& src, const Rect& srcRoi=Rect(0,0,-1,-1),
                      bool isolated=false, int maxBufRows=-1);
    //! processes the next srcCount rows of the image.
    virtual int proceed(const uchar* src, int srcStep, int srcCount,
                        uchar* dst, int dstStep);
    //! applies filter to the specified ROI of the image. if srcRoi=(0,0,-1,-1), the whole image is filtered.
    virtual void apply( const Mat& src, Mat& dst,
                        const Rect& srcRoi=Rect(0,0,-1,-1),
                        Point dstOfs=Point(0,0),
                        bool isolated=false);
    //! returns true if the filter is separable
    bool isSeparable() const { return (const BaseFilter*)filter2D == 0; }
    //! returns the number
    int remainingInputRows() const;
    int remainingOutputRows() const;

    int srcType, dstType, bufType;
    Size ksize;
    Point anchor;
    int maxWidth;
    Size wholeSize;
    Rect roi;
    int dx1, dx2;
    int rowBorderType, columnBorderType;
    vector<int> borderTab;
    int borderElemSize;
    vector<uchar> ringBuf;
    vector<uchar> srcRow;
    vector<uchar> constBorderValue;
    vector<uchar> constBorderRow;
    int bufStep, startY, startY0, endY, rowCount, dstY;
    vector<uchar*> rows;

    Ptr<BaseFilter> filter2D;
    Ptr<BaseRowFilter> rowFilter;
    Ptr<BaseColumnFilter> columnFilter;
};

//! type of the kernel
enum { KERNEL_GENERAL=0, KERNEL_SYMMETRICAL=1, KERNEL_ASYMMETRICAL=2,
       KERNEL_SMOOTH=4, KERNEL_INTEGER=8 };

//! returns type (one of KERNEL_*) of 1D or 2D kernel specified by its coefficients.
MXU_EXPORTS int getKernelType(InputArray kernel, Point anchor);

//! returns the primitive row filter with the specified kernel
MXU_EXPORTS Ptr<BaseRowFilter> getLinearRowFilter(int srcType, int bufType,
                                            InputArray kernel, int anchor,
                                            int symmetryType);

//! returns the primitive column filter with the specified kernel
MXU_EXPORTS Ptr<BaseColumnFilter> getLinearColumnFilter(int bufType, int dstType,
                                            InputArray kernel, int anchor,
                                            int symmetryType, double delta=0,
                                            int bits=0);

//! returns 2D filter with the specified kernel
MXU_EXPORTS Ptr<BaseFilter> getLinearFilter(int srcType, int dstType,
                                           InputArray kernel,
                                           Point anchor=Point(-1,-1),
                                           double delta=0, int bits=0);

//! returns the separable linear filter engine
MXU_EXPORTS Ptr<FilterEngine> createSeparableLinearFilter(int srcType, int dstType,
                          InputArray rowKernel, InputArray columnKernel,
                          Point anchor=Point(-1,-1), double delta=0,
                          int rowBorderType=BORDER_DEFAULT,
                          int columnBorderType=-1,
                          const Scalar& borderValue=Scalar());

//! returns the non-separable linear filter engine
MXU_EXPORTS Ptr<FilterEngine> createLinearFilter(int srcType, int dstType,
                 InputArray kernel, Point _anchor=Point(-1,-1),
                 double delta=0, int rowBorderType=BORDER_DEFAULT,
                 int columnBorderType=-1, const Scalar& borderValue=Scalar());

#if openBlur
//! returns horizontal 1D box filter
MXU_EXPORTS Ptr<BaseRowFilter> getRowSumFilter(int srcType, int sumType,
                                              int ksize, int anchor=-1);
//! returns vertical 1D box filter
MXU_EXPORTS Ptr<BaseColumnFilter> getColumnSumFilter( int sumType, int dstType,
                                                     int ksize, int anchor=-1,
                                                     double scale=1);
//! returns box filter engine
MXU_EXPORTS Ptr<FilterEngine> createBoxFilter( int srcType, int dstType, Size ksize,
                                              Point anchor=Point(-1,-1),
                                              bool normalize=true,
                                              int borderType=BORDER_DEFAULT);
#endif


//! type of morphological operation
enum { MORPH_ERODE=MXU_MOP_ERODE, MORPH_DILATE=MXU_MOP_DILATE,
       MORPH_OPEN=MXU_MOP_OPEN, MORPH_CLOSE=MXU_MOP_CLOSE,
       MORPH_GRADIENT=MXU_MOP_GRADIENT, MORPH_TOPHAT=MXU_MOP_TOPHAT,
       MORPH_BLACKHAT=MXU_MOP_BLACKHAT };

//! returns horizontal 1D morphological filter
MXU_EXPORTS Ptr<BaseRowFilter> getMorphologyRowFilter(int op, int type, int ksize, int anchor=-1);
//! returns vertical 1D morphological filter
MXU_EXPORTS Ptr<BaseColumnFilter> getMorphologyColumnFilter(int op, int type, int ksize, int anchor=-1);
//! returns 2D morphological filter
MXU_EXPORTS Ptr<BaseFilter> getMorphologyFilter(int op, int type, InputArray kernel,
                                               Point anchor=Point(-1,-1));

//! returns "magic" border value for erosion and dilation. It is automatically transformed to Scalar::all(-DBL_MAX) for dilation.
static inline Scalar morphologyDefaultBorderValue() { return Scalar::all(DBL_MAX); }

//! returns morphological filter engine. Only MORPH_ERODE and MORPH_DILATE are supported.
MXU_EXPORTS Ptr<FilterEngine> createMorphologyFilter(int op, int type, InputArray kernel,
                    Point anchor=Point(-1,-1), int rowBorderType=BORDER_CONSTANT,
                    int columnBorderType=-1,
                    const Scalar& borderValue=morphologyDefaultBorderValue());

//! shape of the structuring element
enum { MORPH_RECT=0, MORPH_CROSS=1, MORPH_ELLIPSE=2 };
//! returns structuring element of the specified shape and size
MXU_EXPORTS_W Mat getStructuringElement(int shape, Size ksize, Point anchor=Point(-1,-1));

template<> MXU_EXPORTS void Ptr<IftConvKernel>::delete_obj();

//! copies 2D array to a larger destination array with extrapolation of the outer part of src using the specified border mode
MXU_EXPORTS_W void copyMakeBorder( InputArray src, OutputArray dst,
                                int top, int bottom, int left, int right,
                                int borderType, const Scalar& value=Scalar() );

  //#if openBlur
//! smooths the image using the box filter. Each pixel is processed in O(1) time
MXU_EXPORTS_W void boxFilter( InputArray src, OutputArray dst, int ddepth,
                             Size ksize, Point anchor=Point(-1,-1),
                             bool normalize=true,
                             int borderType=BORDER_DEFAULT );
//! a synonym for normalized box filter
//! and blur 3*3 src = mxu_8uc1 dst = mxu_16sc1
MXU_EXPORTS_W void blur( InputArray src, OutputArray dst,
                        Size ksize, Point anchor=Point(-1,-1),
                        int borderType=BORDER_DEFAULT );
  //#else
//!blur 3*3 src = mxu_8uc1 dst = mxu_16sc1 and blur of C
MXU_EXPORTS_W void blur( InputArray src, OutputArray dst );
  //#endif

//! applies non-separable 2D linear filter to the image
MXU_EXPORTS_W void filter2D( InputArray src, OutputArray dst, int ddepth,
                            InputArray kernel, Point anchor=Point(-1,-1),
                            double delta=0, int borderType=BORDER_DEFAULT );

//! applies separable 2D linear filter to the image
MXU_EXPORTS_W void sepFilter2D( InputArray src, OutputArray dst, int ddepth,
                               InputArray kernelX, InputArray kernelY,
                               Point anchor=Point(-1,-1),
                               double delta=0, int borderType=BORDER_DEFAULT );

//! applies generalized Sobel operator to the image
MXU_EXPORTS_W void Sobel( InputArray src, OutputArray dst, int ddepth,
                         int dx, int dy, int ksize=3,
                         double scale=1, double delta=0,
                         int borderType=BORDER_DEFAULT );

//! applies the vertical or horizontal Scharr operator to the image
MXU_EXPORTS_W void Scharr( InputArray src, OutputArray dst, int ddepth,
                          int dx, int dy, double scale=1, double delta=0,
                          int borderType=BORDER_DEFAULT );

//! applies Laplacian operator to the image
MXU_EXPORTS_W void Laplacian( InputArray src, OutputArray dst, int ddepth,
                             int ksize=1, double scale=1, double delta=0,
                             int borderType=BORDER_DEFAULT );
//sobel 3*3 src = mxu_8uc1 dst = mxu_16sc1
MXU_EXPORTS_W void HighSobel(const InputArray src,OutputArray dst,int xorder,int yorder,int aperture_size);






//! erodes the image (applies the local minimum operator)
MXU_EXPORTS_W void erode( InputArray src, OutputArray dst, InputArray kernel,
                         Point anchor=Point(-1,-1), int iterations=1,
                         int borderType=BORDER_CONSTANT,
                         const Scalar& borderValue=morphologyDefaultBorderValue() );

/** @brief Calculates the minimal eigenvalue of gradient matrices for corner detection.

The function is similar to cornerEigenValsAndVecs but it calculates and stores only the minimal
eigenvalue of the covariance matrix of derivatives, that is, \f$\min(\lambda_1, \lambda_2)\f$ in terms
of the formulae in the cornerEigenValsAndVecs description.

@param src Input single-channel 8-bit or floating-point image.
@param dst Image to store the minimal eigenvalues. It has the type MXU_32FC1 and the same size as
src .
@param blockSize Neighborhood size (see the details on cornerEigenValsAndVecs ).
@param ksize Aperture parameter for the Sobel operator.
@param borderType Pixel extrapolation method. See mxu::BorderTypes.
 */
MXU_EXPORTS_W void cornerMinEigenVal( InputArray src, OutputArray dst,
                                     int blockSize, int ksize = 3,
                                     int borderType = BORDER_DEFAULT );

//! dilates the image (applies the local maximum operator)
MXU_EXPORTS_W void dilate( InputArray src, OutputArray dst, InputArray kernel,
                          Point anchor=Point(-1,-1), int iterations=1,
                          int borderType=BORDER_CONSTANT,
                          const Scalar& borderValue=morphologyDefaultBorderValue() );

//! applies an advanced morphological operation to the image
MXU_EXPORTS_W void morphologyEx( InputArray src, OutputArray dst,
                                int op, InputArray kernel,
                                Point anchor=Point(-1,-1), int iterations=1,
                                int borderType=BORDER_CONSTANT,
                                const Scalar& borderValue=morphologyDefaultBorderValue() );

//! erodes the image (applies the local minimum operator)
MXU_EXPORTS_W void higherode( InputArray src, OutputArray dst, InputArray kernel,
                         Point anchor=Point(-1,-1), int iterations=1,
                         int borderType=BORDER_CONSTANT,
                         const Scalar& borderValue=morphologyDefaultBorderValue() );

//! adjusts the corner locations with sub-pixel accuracy to maximize the certain cornerness criteria
MXU_EXPORTS_W void cornerSubPix( InputArray image, InputOutputArray corners,
                                Size winSize, Size zeroZone,
                                TermCriteria criteria );

//! dilates the image (applies the local maximum operator)
MXU_EXPORTS_W void highdilate( InputArray src, OutputArray dst, InputArray kernel,
                          Point anchor=Point(-1,-1), int iterations=1,
                          int borderType=BORDER_CONSTANT,
                          const Scalar& borderValue=morphologyDefaultBorderValue() );

//! finds the strong enough corners where the cornerMinEigenVal() or cornerHarris() report the local maxima duank
MXU_EXPORTS_W void goodFeaturesToTrack( InputArray image, OutputArray corners,
                                     int maxCorners, double qualityLevel, double minDistance,
                                     InputArray mask=noArray(), int blockSize=3,
                                     bool useHarrisDetector=false, double k=0.04 );

//! applies an advanced morphological operation to the image
MXU_EXPORTS_W void highmorphologyEx( InputArray src, OutputArray dst,
                                int op, InputArray kernel,
                                Point anchor=Point(-1,-1), int iterations=1,
                                int borderType=BORDER_CONSTANT,
                                const Scalar& borderValue=morphologyDefaultBorderValue() );








//! type of the threshold operation
enum { THRESH_BINARY=MXU_THRESH_BINARY, THRESH_BINARY_INV=MXU_THRESH_BINARY_INV,
       THRESH_TRUNC=MXU_THRESH_TRUNC, THRESH_TOZERO=MXU_THRESH_TOZERO,
       THRESH_TOZERO_INV=MXU_THRESH_TOZERO_INV, THRESH_MASK=MXU_THRESH_MASK,
       THRESH_OTSU=MXU_THRESH_OTSU };

enum { ADAPTIVE_THRESH_MEAN_C=0, ADAPTIVE_THRESH_GAUSSIAN_C=1};

//! applies fixed threshold to the image
MXU_EXPORTS_W double threshold( InputArray src, OutputArray dst,double thresh, double maxval, int type );

MXU_EXPORTS_W void adaptiveThreshold( InputArray _src, OutputArray _dst, double maxValue,
       int method, int type, int blockSize, double delta);


/** @brief Blurs an image and downsamples it.

By default, size of the output image is computed as `Size((src.cols+1)/2, (src.rows+1)/2)`, but in
any case, the following conditions should be satisfied:

\f[\begin{array}{l} | \texttt{dstsize.width} *2-src.cols| \leq 2 \\ | \texttt{dstsize.height} *2-src.rows| \leq 2 \end{array}\f]

The function performs the downsampling step of the Gaussian pyramid construction. First, it
convolves the source image with the kernel:

\f[\frac{1}{256} \begin{bmatrix} 1 & 4 & 6 & 4 & 1  \\ 4 & 16 & 24 & 16 & 4  \\ 6 & 24 & 36 & 24 & 6  \\ 4 & 16 & 24 & 16 & 4  \\ 1 & 4 & 6 & 4 & 1 \end{bmatrix}\f]

Then, it downsamples the image by rejecting even rows and columns.

@param src input image.
@param dst output image; it has the specified size and the same type as src.
@param dstsize size of the output image.
@param borderType Pixel extrapolation method, see mxu::BorderTypes (BORDER_CONSTANT isn't supported)
 */
MXU_EXPORTS_W void pyrDown( InputArray src, OutputArray dst,
                           const Size& dstsize = Size(), int borderType = BORDER_DEFAULT );

MXU_EXPORTS_W double mergethreshold( InputArray src, OutputArray dst,double thresh, double maxval, int type );

MXU_EXPORTS_W int contours(InputOutputArray _src,vector< vector<MxuPoint> >& contours,int  mode,int  method, Point offset);

MXU_EXPORTS_W Rect boundRect( vector<MxuPoint> contour);

//! computes the integral image
MXU_EXPORTS_W void integral( InputArray src, OutputArray sum, int sdepth=-1 );

//! computes the integral image and integral for the squared image
MXU_EXPORTS_AS(integral2) void integral( InputArray src, OutputArray sum,
                                        OutputArray sqsum, int sdepth=-1 );
//! computes the integral image, integral for the squared image and the tilted integral image
MXU_EXPORTS_AS(integral3) void integral( InputArray src, OutputArray sum,
                                        OutputArray sqsum, OutputArray tilted,
                                        int sdepth=-1 );

//! computes PSNR image/video quality metric
MXU_EXPORTS_W double PSNR(InputArray src1, InputArray src2);

//! computes the minimal rotated rectangle for a set of points
MXU_EXPORTS_W RotatedRect minAreaRect( InputArray points );


//! type of the template matching operation
enum { TM_SQDIFF=0, TM_SQDIFF_NORMED=1, TM_CCORR=2, TM_CCORR_NORMED=3, TM_CCOEFF=4, TM_CCOEFF_NORMED=5 };

//! computes the proximity map for the raster template and the image where the template is searched for
MXU_EXPORTS_W void matchTemplate( InputArray image, InputArray templ,
                                 OutputArray result, int method );

//add by cfwang
enum
{
	INTER_BITS=5, INTER_BITS2=INTER_BITS*2,
	INTER_TAB_SIZE=(1<<INTER_BITS),
	INTER_TAB_SIZE2=INTER_TAB_SIZE*INTER_TAB_SIZE
};

enum
{
	INTER_NEAREST=MXU_INTER_NN, //!< nearest neighbor interpolation
	INTER_LINEAR=MXU_INTER_LINEAR, //!< bilinear interpolation
	INTER_CUBIC=MXU_INTER_CUBIC, //!< bicubic interpolation
	INTER_AREA=MXU_INTER_AREA, //!< area-based (or super) interpolation
	INTER_LANCZOS4=MXU_INTER_LANCZOS4, //!< Lanczos interpolation over 8x8 neighborhood
	INTER_MAX=7,
	WARP_INVERSE_MAP=MXU_WARP_INVERSE_MAP
};

MXU_EXPORTS_W void resize(InputArray _src,OutputArray _dst,Size dsize,double inv_scale_x=0,double inv_scale_y=0, int interpolation=INTER_LINEAR);
MXU_EXPORTS_W void fast_resize(InputArray _src,OutputArray _dst,float inv_scale_x,float inv_scale_y);
MXU_EXPORTS_W void fast_resize_new(InputArray _src,OutputArray _dst,uint16_t* resize_buf);

MXU_EXPORTS_W void equalize_Hist(InputArray _src);
MXU_EXPORTS_W Mat mxugetPerspectiveTransform(const Point2f src[], const Point2f dst[]);
MXU_EXPORTS_W void mxuwarpPerspective(InputArray _src, OutputArray _dst, InputArray _M0,
        Size dsize, int flags = 1, int borderType = 0, const Scalar& borderValue = Scalar(0));

//! approximates contour or a curve using Douglas-Peucker algorithm
MXU_EXPORTS_W void approxPolyDP( InputArray curve,
    OutputArray approxCurve,
    double epsilon, bool closed );




//findcontours
enum ContourApproximationModes {
    /** stores absolutely all the contour points. That is, any 2 subsequent points (x1,y1) and
    (x2,y2) of the contour will be either horizontal, vertical or diagonal neighbors, that is,
    max(abs(x1-x2),abs(y2-y1))==1. */
    CHAIN_APPROX_NONE      = 1,
    /** compresses horizontal, vertical, and diagonal segments and leaves only their end points.
    For example, an up-right rectangular contour is encoded with 4 points. */
    CHAIN_APPROX_SIMPLE    = 2,
    /** applies one of the flavors of the Teh-Chin chain approximation algorithm @cite TehChin89 */
    CHAIN_APPROX_TC89_L1   = 3,
    /** applies one of the flavors of the Teh-Chin chain approximation algorithm @cite TehChin89 */
    CHAIN_APPROX_TC89_KCOS = 4
};

enum RetrievalModes {
    /** retrieves only the extreme outer contours. It sets `hierarchy[i][2]=hierarchy[i][3]=-1` for
    all the contours. */
    RETR_EXTERNAL  = 0,
    /** retrieves all of the contours without establishing any hierarchical relationships. */
    RETR_LIST      = 1,
    /** retrieves all of the contours and organizes them into a two-level hierarchy. At the top
    level, there are external boundaries of the components. At the second level, there are
    boundaries of the holes. If there is another contour inside a hole of a connected component, it
    is still put at the top level. */
    RETR_CCOMP     = 2,
    /** retrieves all of the contours and reconstructs a full hierarchy of nested contours.*/
    RETR_TREE      = 3,
    RETR_FLOODFILL = 4 //!<
};




class  MXU_EXPORTS Moments
{
public:
    Moments();
    Moments(double m00, double m10, double m01, double m20, double m11,
            double m02, double m30, double m21, double m12, double m03 );
 //   Moments( const MxuMoments& moments );
   // operator MxuMoments() const;
    // spatial moments（空间矩）
    double  m00, m10, m01, m20, m11, m02, m30, m21, m12, m03;
    // central moments（中心矩）
    double  mu20, mu11, mu02, mu30, mu21, mu12, mu03;
    // central normalized moments（归一化中心矩）
    double  nu20, nu11, nu02, nu30, nu21, nu12, nu03;
};


//Moments
MXU_EXPORTS  void HuMoments( const Moments& moments, double hu[7] );
MXU_EXPORTS_W  void HuMoments( const Moments& m, OutputArray hu );
//MXU_EXPORTS_W Moments moments( InputArray array, bool binaryImage = false );
MXU_EXPORTS_W Moments moments( vector<MxuPoint> contour, bool binaryImage = false );


//matchShapes
//MXU_EXPORTS_W double matchShapes( InputArray contour1, InputArray contour2,int method, double parameter );
MXU_EXPORTS_W double matchShapes(  vector<MxuPoint> contour1,  vector<MxuPoint> contour2,int method, double parameter );





}


#endif  /* __cplusplus */

#endif
/* End of file. */
