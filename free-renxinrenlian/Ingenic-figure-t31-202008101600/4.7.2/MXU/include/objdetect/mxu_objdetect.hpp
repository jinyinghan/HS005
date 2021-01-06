/*
 * IMP FrameSource header file.
 *
 * Copyright (C) 2015 Ingenic Semiconductor Co.,Ltd
 * Author: Codi <dong.li@ingenic.com>
 */

//-------------------------------------所有类的定义都在头文件中，方法都在.cpp文件中给予
//-------------------------------------（除了inline函数以及一些函数过于简单的方法会在头文件中给出）
#ifndef _MXU_OBJDETECT_HPP
#define _MXU_OBJDETECT_HPP

#include "./../core/mxu_core.hpp"
#define MXU_HAAR_DO_CANNY_PRUNING    1
#define MXU_HAAR_SCALE_IMAGE         2
#define MXU_HAAR_FIND_BIGGEST_OBJECT 4
#define MXU_HAAR_DO_ROUGH_SEARCH     8

#define CC_CASCADE_PARAMS "cascadeParams"
#define CC_STAGE_TYPE     "stageType"
#define CC_FEATURE_TYPE   "featureType"
#define CC_HEIGHT         "height"
#define CC_WIDTH          "width"

#define CC_STAGE_NUM    "stageNum"
#define CC_STAGES       "stages"
#define CC_STAGE_PARAMS "stageParams"

#define CC_BOOST            "BOOST"
#define CC_MAX_DEPTH        "maxDepth"
#define CC_WEAK_COUNT       "maxWeakCount"
#define CC_STAGE_THRESHOLD  "stageThreshold"
#define CC_WEAK_CLASSIFIERS "weakClassifiers"
#define CC_INTERNAL_NODES   "internalNodes"
#define CC_LEAF_VALUES      "leafValues"

#define CC_FEATURES       "features"
#define CC_FEATURE_PARAMS "featureParams"
#define CC_MAX_CAT_COUNT  "maxCatCount"

#define CC_HAAR   "HAAR"
#define CC_RECTS  "rects"
#define CC_TILTED "tilted"

#define CC_LBP  "LBP"
#define CC_RECT "rect"

#define CC_HOG  "HOG"

#define USE_treeThreshold

#define MXU_SUM_PTRS( p0, p1, p2, p3, sum, rect, step )                    \
    /* (x, y) */                                                          \
    (p0) = sum + (rect).x + (step) * (rect).y,                            \
    /* (x + w, y) */                                                      \
    (p1) = sum + (rect).x + (rect).width + (step) * (rect).y,             \
    /* (x + w, y) */                                                      \
    (p2) = sum + (rect).x + (step) * ((rect).y + (rect).height),          \
    /* (x + w, y + h) */                                                  \
    (p3) = sum + (rect).x + (rect).width + (step) * ((rect).y + (rect).height)

#define MXU_SUM_PTRS0( p0, p1, p2, p3, sum, rect, step )                    \
    /* (x, y) */                                                          \
    (p0) = sum + (rect).x + (step) * (rect).y,                            \
    /* (x + w, y) */							\
    (p1) = (p0) + (rect).width,			        		\
    /* (x + w, y) */							\
    (p2) = (p0)+ (step) * (rect).height ,				\
    /* (x + w, y + h) */						\
    (p3) = (p2) + (rect).width

#define MXU_TILTED_PTRS( p0, p1, p2, p3, tilted, rect, step )                        \
    /* (x, y) */                                                                    \
    (p0) = tilted + (rect).x + (step) * (rect).y,                                   \
    /* (x - h, y + h) */                                                            \
    (p1) = tilted + (rect).x - (rect).height + (step) * ((rect).y + (rect).height), \
    /* (x + w, y + w) */                                                            \
    (p2) = tilted + (rect).x + (rect).width + (step) * ((rect).y + (rect).width),   \
    /* (x + w - h, y + w + h) */                                                    \
    (p3) = tilted + (rect).x + (rect).width - (rect).height                         \
           + (step) * ((rect).y + (rect).width + (rect).height)

#define CALC_SUM_(p0, p1, p2, p3, offset) \
    ((p0)[offset] - (p1)[offset] - (p2)[offset] + (p3)[offset])

#define CALC_SUM(rect,offset) CALC_SUM_((rect)[0], (rect)[1], (rect)[2], (rect)[3], offset)

//-------------------------------处理获得的返回值为1的RECT区域，确定哪些是人脸区域，根据minNeighbors(facedetect.cpp)-------------------------//
using namespace mxu;
using namespace std;

#ifndef _ATOFANDAOTI
#define _ATOFANDAOTI
bool judge_digital(char a);
int Multiatoi(int d[], const string s);
int Multiatof(float d[], const string s);
int Multiatoiandatof(int d[], float fd[], const string s, int k);
#endif /* _ATOFANDAOTI */


void integralHistogram( const Mat& srcImage, vector<Mat> &histogram);
void IVS_FUNC_HistogramGA( const Mat& srcImage, vector<Mat> &histogram);
void IVS_FUNC_HistogramMAA(vector<Mat> &histogram, Mat &grad,Mat &qangle);
void IVS_FUNC_MagAndAng(const Mat& img, Mat& grad, Mat& qangle);
void IVS_FUNC_MagAndAng_9(const Mat& img, Mat& grad, Mat& qangle);


class SimilarRects
{
public:
    SimilarRects(double _eps) : eps(_eps) {}
    inline bool operator()(const Rect& r1, const Rect& r2) const
    {
        double delta = eps*(std::min(r1.width, r2.width) + std::min(r1.height, r2.height))*0.5;
        return std::abs(r1.x - r2.x) <= delta &&
            std::abs(r1.y - r2.y) <= delta &&
            std::abs(r1.x + r1.width - r2.x - r2.width) <= delta &&
            std::abs(r1.y + r1.height - r2.y - r2.height) <= delta;
    }
    double eps;
};

void groupRectangles(vector<Rect>& rectList, int groupThreshold, double eps=0.2);
void groupRectangles(vector<Rect>& rectList, vector<int>& weights, int groupThreshold, double eps=0.2);
void groupRectangles(vector<Rect>& rectList, int groupThreshold, double eps, vector<int>* weights, vector<double>* levelWeights );
void groupRectangles(vector<Rect>& rectList, vector<int>& rejectLevels,
                                vector<double>& levelWeights, int groupThreshold, double eps=0.2);

class   MXU_FeatureEvaluator
{
public:
    enum { HAAR = 0, LBP = 1, HOG = 2 };
    virtual ~MXU_FeatureEvaluator();

    virtual bool read(int ki);
    virtual bool read();
    virtual bool read_head();
    virtual int getFeatureType() const;
    virtual Ptr<MXU_FeatureEvaluator> clone() const;
    virtual bool setImage(const Mat& img, Size origWinSize);
    virtual bool setImage1(const Mat& img,Size origWinSize,bool judgehist,vector<Mat> &imghist_svm);//fbtian add
    virtual bool setWindow(Point p);

    virtual double calcOrd(int featureIdx) const;
    virtual int calcOrd0(int featureIdx) const;  //++++
    virtual int calcCat(int featureIdx) const;

    static Ptr<MXU_FeatureEvaluator> create(int type);
};


class MXU_HaarEvaluator : public MXU_FeatureEvaluator
{
public:
    struct Feature
    {
        Feature();

        float calc( int offset ) const;
        void updatePtrs( const Mat& sum );

//-----------------------wrtcv----------------------------//

        bool tilted;

        enum { RECT_NUM = 3 };

        struct
        {
            Rect r;
            float weight;
        } rect[RECT_NUM];

        const int* p[RECT_NUM][4];
    };

    MXU_HaarEvaluator();
    virtual ~MXU_HaarEvaluator();

//------------------------wrtcv------------------------//
    virtual Ptr<MXU_FeatureEvaluator> clone() const;
    virtual int getFeatureType() const { return MXU_FeatureEvaluator::HAAR; }

    virtual bool setImage(const Mat&, Size origWinSize);
    virtual bool setWindow(Point pt);

    double operator()(int featureIdx) const
    { return featuresPtr[featureIdx].calc(offset) * varianceNormFactor; }
    virtual double calcOrd(int featureIdx) const
    { return (*this)(featureIdx); }

protected:
    Size origWinSize;
    Ptr<vector<Feature> > features;
    Feature* featuresPtr; // optimization
    bool hasTiltedFeatures;

    Mat sum0, sqsum0, tilted0;
    Mat sum, sqsum, tilted;

    Rect normrect;
    const int *p[4];
    const double *pq[4];

    int offset;
    double varianceNormFactor;
};


class MXU_LBPEvaluator : public MXU_FeatureEvaluator
{
public:
    struct Feature
    {
        Feature();
        Feature( int x, int y, int _block_w, int _block_h  ) :
        rect(x, y, _block_w, _block_h) {}

        bool read(int n);
        int calc( int offset ) const;
        void updatePtrs( const Mat& sum );

//---------------------------wrtcv------------------------//
        Rect rect; // weight and height for block
        const int* p[16]; // fast
    };

    MXU_LBPEvaluator();
    virtual ~MXU_LBPEvaluator();

//---------------------------wrtcv-------------------------//
    virtual Ptr<MXU_FeatureEvaluator> clone() const;
    virtual int getFeatureType() const { return MXU_FeatureEvaluator::LBP; }
    virtual bool read();
    virtual bool setImage(const Mat& image, Size _origWinSize);
    virtual bool setWindow(Point pt);


//---------------------很重要的过程，很耗时-------------------//
    int operator()(int featureIdx) const
    { return featuresPtr[featureIdx].calc(offset); }


    virtual int calcCat(int featureIdx) const
    { return (*this)(featureIdx); }
protected:
    Size origWinSize;
    Ptr<vector<Feature> > features;
    Feature* featuresPtr; // optimization
    Mat sum0, sum;
    Rect normrect;

    int offset;
};

//---------------------------------------------- HOGEvaluator -------------------------------------------

class MXU_HOGEvaluator : public MXU_FeatureEvaluator
{
public:
  struct Feature
  {
    Feature();
    int calc( unsigned short offset ) const;

    void updatePtrs( const vector<Mat>& _hist);
    bool read(int n, int ki);
    bool read(int n);
    enum { CELL_NUM = 4, BIN_NUM = 9 };

    Rect rect[CELL_NUM];
    Rect res;
    uchar featComponent0; //component index from 0 to 35
   // uchar featComponent1; 
//   const int* pF[4]; //for feature calculation
    int * pF[4]; 
    ///const int* pN[4]; //for normalization calculation
  };
  MXU_HOGEvaluator();
  virtual ~MXU_HOGEvaluator();
  bool read(int ki);
  bool read_head();
  virtual Ptr<MXU_FeatureEvaluator> clone() const;
  virtual int getFeatureType() const { return MXU_FeatureEvaluator::HOG; }
  virtual bool setImage(const Mat& image, Size winSize );
  virtual bool setImage1(const Mat& image, Size winSize, bool judgehist, vector<Mat> &imghist_svm);
  virtual bool setWindow( Point pt );
  int operator()(unsigned short featureIdx) const    ///yes!
  {
    return featuresPtr[featureIdx].calc(offset);
  }
  virtual int calcOrd0( unsigned short featureIdx ) const
  {
    return (*this)(featureIdx);
  }
 unsigned short offset;
 Feature* featuresPtr;

private:
  Size origWinSize;
  Ptr<vector<Feature> > features;
  vector<Mat> hist;
  Mat normSum;
};


inline MXU_HOGEvaluator::Feature :: Feature()
{
    rect[0] = rect[1] = rect[2] = rect[3] = Rect();
    res = Rect();
    pF[0] = pF[1] = pF[2] = pF[3] = 0;
    featComponent0 = 0;
    //featComponent1 = 0;
}

inline int MXU_HOGEvaluator::Feature :: calc( unsigned short _offset ) const
{
    return CALC_SUM(pF, _offset);;
}

inline void MXU_HOGEvaluator::Feature :: updatePtrs( const vector<Mat> &_hist/*, const Mat &_normSum*/ )
//updatePtrs函数是要根据梯度直方图和归一图来更新每个Feature中保存的四个指针
{
    uchar binIdx = featComponent0;//featComponent % 9;   //计算要更新的角度
    //uchar cellIdx = featComponent1;//featComponent / 9;
    int* featBuf = (int*)_hist[binIdx].data;
    size_t featStep = _hist[0].step >> 2;
  
    //MXU_SUM_PTRS0( pF[0], pF[1], pF[2], pF[3], featBuf, rect[cellIdx], featStep );  //更新四个直方积分图中的指针
    MXU_SUM_PTRS0( pF[0], pF[1], pF[2], pF[3], featBuf, res, featStep ); 
}

//----------------------------------------------



//------------------为了避免同namespace cv下的命名冲突，在类名中添加MXU_---------------------------//
class MXU_CascadeClassifier
{
public:
  MXU_CascadeClassifier(int detectionType, int cols=0, int rows=0);
    MXU_CascadeClassifier();
    virtual ~MXU_CascadeClassifier();

    virtual bool empty() const;
    unsigned short* resize_buf;

/**
 * xml文件读取
 *
 * @fn  bool xmldata2vec();
 *
 * @retval true  成功
 * @retval false 失败
 *
 * @attention 行人检测专用的xml数据，检测前必须提前调用.
 */
   bool xmldata2vec();  //fbtian add
   bool headxmldata2vec();

/**
 * 行人检测入口
 *
 * @fn void detectMultiScale( const Mat& image, \n
 *                            vector<Rect>& objects, \n
 *                            double scaleFactor=1.1, \n
 *                            int minNeighbors=3, int flags=0, \n
 *                            Size minSize=Size(), \n
 *                            Size maxSize=Size());
 *
 * @param[in] image 		输入图像,注意是灰度图像,Mat型数据
 * @param[in] objects 		空的容器，用于存储检测的结果矩阵
 * @param[in] scaleFactor 	图像缩放比
 * @param[in] flags 		特征使用标记，预留(暂时无意义)
 * @param[in] minNeighbors 	对检测结果聚类的参数
 * @param[in] minSize 		最小检测窗口
 * @param[in] maxSize 		最大检测窗口
 * 示例：
 * @code
 * Mat frame;
 * string cascadeName = "haarcascade_frontalface_alt.xml";
 * MXU_CascadeClassifier cascade;
 * vector<Rect> faces;
 *
 * cascade.detectMultiScale( frame, faces, 1.1, 2, 0, Size(30, 30) ); //在调用函数前，frame已经获得了灰度图像
 * @endcode
 *
 * @attention 输入的image参数一定要是灰度图像,在调用函数前，必须保证调用过函数bool xmldata2vec(void).
 */
  virtual void detectMultiScale_3xml( const Mat& image,
                                   vector<Rect>& objects,
                                   double scaleFactor=1.32,
                                   Size minSize=Size(),
                                   Size maxSize=Size());

  virtual void detectMultiScale_1xml( const Mat& image,
				     vector<Rect>& objects,
				     double scaleFactor=1.2,
				     Size minSize=Size(),
				     Size maxSize=Size());
    bool isOldFormatCascade() const;
    virtual Size getOriginalWindowSize() const;
    int getFeatureType() const;
    bool setImage( const Mat& );

protected:
    enum { BOOST = 0 };
    enum { DO_CANNY_PRUNING = 1, SCALE_IMAGE = 2,
           FIND_BIGGEST_OBJECT = 4, DO_ROUGH_SEARCH = 8 };

    friend class MXU_CascadeClassifierInvoker;

    bool setImage( Ptr<MXU_FeatureEvaluator>& feval, const Mat& image);

    class Data
    {
    public:
        struct DTreeNode
        {
	  //int featureIdx;
	  unsigned short featureIdx;
	  // float threshold; // for ordered features only
	  //int threshold0;
	  unsigned short threshold0;
        };

        struct DTree
        {
            int nodeCount;
        };

        struct Stage
        {
	  unsigned short first;
	  unsigned short ntrees;
	  // float threshold;
	  int threshold0; //++++++
        };

        bool read_hog_data(int ki);
        bool read_head_data();

        uchar featureType;
        Size origWinSize;

        vector<Stage> stages;
        vector<DTree> classifiers;
        vector<DTreeNode> nodes;
      //vector<float> leaves;
        vector<int> leaves0;     ///++++++
        vector<int> subsets;
#ifdef USE_treeThreshold
        vector<int> treeThresholds;  ///if USE_treeThreshold
#endif
    };

    //Data data;
    Ptr<MXU_FeatureEvaluator> MXU_featureEvaluator;
    vector< Data >data_vec;
    vector< Ptr<MXU_FeatureEvaluator> >featureEvaluator_vec;
    vector< Mat > imghist_svm;

public:
    Data data;
};

/////////////////////////////////source mxu_HOG///////////////////////////////////
struct  mxuHOGDescriptor
{
public:
    enum { L2Hys=0 };
    enum { DEFAULT_NLEVELS=64 };

    mxuHOGDescriptor() : winSize(64,128), blockSize(16,16), blockStride(8,8),
        cellSize(8,8), nbins(9), derivAperture(1), winSigma(-1),
        histogramNormType(mxuHOGDescriptor::L2Hys), L2HysThreshold(0.2), gammaCorrection(true),
        nlevels(mxuHOGDescriptor::DEFAULT_NLEVELS)
    {}
/*
    mxuHOGDescriptor(Size _winSize, Size _blockSize, Size _blockStride,
                  Size _cellSize, int _nbins, int _derivAperture=1, double _winSigma=-1,
                  int _histogramNormType=HOGDescriptor::L2Hys,
                  double _L2HysThreshold=0.2, bool _gammaCorrection=false,
                  int _nlevels=HOGDescriptor::DEFAULT_NLEVELS)
    : winSize(_winSize), blockSize(_blockSize), blockStride(_blockStride), cellSize(_cellSize),
    nbins(_nbins), derivAperture(_derivAperture), winSigma(_winSigma),
    histogramNormType(_histogramNormType), L2HysThreshold(_L2HysThreshold),
    gammaCorrection(_gammaCorrection), nlevels(_nlevels)
    {}
*/
     mxuHOGDescriptor(const String& filename)
    {
     //   load(filename);
    }


    mxuHOGDescriptor(const mxuHOGDescriptor& d)
    {
        d.copyTo(*this);
    }


    virtual ~mxuHOGDescriptor() {}


     size_t getDescriptorSize() const;
/*
     bool checkDetectorSize() const;
*/
     double getWinSigma() const;
     virtual void setSVMDetector(vector<float> _svmdetector);

/*
    virtual bool read(FileNode& fn);
    virtual void write(FileStorage& fs, const String& objname) const;

     virtual bool load(const String& filename, const String& objname=String());
     virtual void save(const String& filename, const String& objname=String()) const;
*/

    virtual void copyTo(mxuHOGDescriptor& c) const;

/*
     virtual void compute(const Mat& img,
                         Mat vector<float>& descriptors,
                         Size winStride=Size(), Size padding=Size(),
                         const vector<Point>& locations=vector<Point>()) const;
*/

    //with found weights output
     virtual void detect(const Mat& img, vector<Point>& foundLocations,
                        vector<double>& weights,
                        double hitThreshold=0, Size winStride=Size(),
                        Size padding=Size(),
                        const vector<Point>& searchLocations=vector<Point>()) const;


    //with result weights output
     virtual void detectMultiScale(const Mat& img, vector<Rect>& foundLocations,
                                  vector<double>& foundWeights, double hitThreshold=0,
                                  Size winStride=Size(), Size padding=Size(), double scale=1.05,
                                  double finalThreshold=2.0,bool useMeanshiftGrouping = false) const;
    //without found weights output
    virtual void detectMultiScale(const Mat& img, vector<Rect>& foundLocations,
                                  double hitThreshold=0, Size winStride=Size(),
                                  Size padding=Size(), double scale=1.05,
                                  double finalThreshold=2.0, bool useMeanshiftGrouping = false) const;

     virtual void computeGradient(const Mat& img, Mat& grad, Mat& angleOfs,
                                 Size paddingTL=Size(), Size paddingBR=Size()) const;

     static vector<float> getDefaultPeopleDetector();
     static vector<float> getDaimlerPeopleDetector();

     Size winSize;
     Size blockSize;
     Size blockStride;
     Size cellSize;
     int nbins;
     int derivAperture;
     double winSigma;
     int histogramNormType;
     double L2HysThreshold;
     bool gammaCorrection;
     vector<float> svmDetector;
     int nlevels;

/*
   // evaluate specified ROI and return confidence value for each location
   void detectROI(const cv::Mat& img, const vector<cv::Point> &locations,
                                   Mat std::vector<cv::Point>& foundLocations, Mat std::vector<double>& confidences,
                                   double hitThreshold = 0, cv::Size winStride = Size(),
                                   cv::Size padding = Size()) const;

   // evaluate specified ROI and return confidence value for each location in multiple scales
   void detectMultiScaleROI(const cv::Mat& img,
                                                       Mat std::vector<cv::Rect>& foundLocations,
                                                       std::vector<DetectionROI>& locations,
                                                       double hitThreshold = 0,
                                                       int groupThreshold = 0) const;

   // read/parse Dalal's alt model file
   void readALTModel(std::string modelfile);
*/
   void groupRectangles(vector<Rect>& rectList, vector<double>& weights, int groupThreshold, double eps) const;
};

/*
//////////////// HOG (Histogram-of-Oriented-Gradients) Descriptor and Object Detector //////////////

// struct for detection region of interest (ROI)
struct DetectionROI
{
   // scale(size) of the bounding box
   double scale;
   // set of requrested locations to be evaluated
   vector<mxu::Point> locations;
   // vector that will contain confidence values for each location
   vector<double> confidences;
};
*/


///////////////////////////////////chaged mxu_HOG////////////////////////////////////////////////////////
struct  mxuCHOGDescriptor
{
public:
    enum { L2Hys=0 };
    enum { DEFAULT_NLEVELS=64 };

    mxuCHOGDescriptor() : winSize(64,128), blockSize(16,16), blockStride(8,8),
        cellSize(8,8), nbins(9),
        histogramNormType(mxuCHOGDescriptor::L2Hys), L2HysThreshold(0.2),
        nlevels(mxuCHOGDescriptor::DEFAULT_NLEVELS)
    {}

    mxuCHOGDescriptor(Size _winSize, Size _blockSize, Size _blockStride,
                  Size _cellSize, int _nbins,
                  int _histogramNormType=mxuCHOGDescriptor::L2Hys,
                  double _L2HysThreshold=0.2,
                  int _nlevels=mxuCHOGDescriptor::DEFAULT_NLEVELS)
    : winSize(_winSize), blockSize(_blockSize), blockStride(_blockStride), cellSize(_cellSize),
    nbins(_nbins), histogramNormType(_histogramNormType), L2HysThreshold(_L2HysThreshold),
     nlevels(_nlevels)
    {}

     mxuCHOGDescriptor(const String& filename)
    {
      //load(filename);
    }


    mxuCHOGDescriptor(const mxuCHOGDescriptor& d)
    {
        d.copyTo(*this);
    }


    virtual ~mxuCHOGDescriptor() {}


     size_t getDescriptorSize() const;

     bool checkDetectorSize() const;
     Size getOriginalWindowSize() const;
     virtual void setSVMDetector(vector<float> _svmdetector);


    virtual void copyTo(mxuCHOGDescriptor& c) const;

#ifdef unNorm
     virtual void compute(const Mat& img,
                         vector<int>& descriptors,
                         Size winStride=Size(),
                         const vector<Point>& locations=vector<Point>()) const;
#else
    virtual void compute(const Mat& img,
			 vector<float>& descriptors,
                         Size winStride=Size(),
                         const vector<Point>& locations=vector<Point>()) const;
#endif
    //with found weights output
     virtual void detect(const Mat& img, vector<Point>& foundLocations,
                        vector<double>& weights,
                        double hitThreshold=0, Size winStride=Size(),
                        const vector<Point>& searchLocations=vector<Point>()) const;
  //without found weights output
    // virtual void detect(const Mat& img, vector<Point>& foundLocations,
    //                     double hitThreshold=0, Size winStride=Size(),
    //                     const vector<Point>& searchLocations=vector<Point>()) const;

    //with result weights output
     virtual void detectMultiScale(const Mat& img, vector<Rect>& foundLocations,
                                  vector<double>& foundWeights, double hitThreshold=0,
                                  Size winStride=Size(), double scale=1.05,
				   double finalThreshold=2.0,bool useMeanshiftGrouping = false) const;
    //without found weights output
    virtual void detectMultiScale(const Mat& img, vector<Rect>& foundLocations,
                                  double hitThreshold=0, Size winStride=Size(),
                                   double scale=1.05,
                                  double finalThreshold=2.0, bool useMeanshiftGrouping = false) const;

  virtual void computeGradient(const Mat& img, Mat& grad, Mat& angleOf) const;

     static vector<float> getDefaultPeopleDetector();
     static vector<float> getDaimlerPeopleDetector();

     Size winSize;
     Size blockSize;
     Size blockStride;
     Size cellSize;
     int nbins;
     int histogramNormType;
     double L2HysThreshold;
     vector<float> svmDetector;
     int nlevels;


   // evaluate specified ROI and return confidence value for each location
  /*
   void detectROI(const mxu::Mat& img, const vector<mxu::Point> &locations,
                  std::vector<mxu::Point>& foundLocations, std::vector<double>& confidences,
                  double hitThreshold = 0, mxu::Size winStride = Size(),mxu::Size padding = Size()) const;
  */
   // evaluate specified ROI and return confidence value for each location in multiple scales
  /*
   void detectMultiScaleROI(const mxu::Mat& img,
                            std::vector<mxu::Rect>& foundLocations,
                            std::vector<DetectionROI>& locations,
                            double hitThreshold = 0,
                            int groupThreshold = 0) const;
  */
   // read/parse Dalal's alt model file
   //void readALTModel(std::string modelfile);

   void groupRectangles(vector<Rect>& rectList, vector<double>& weights, int groupThreshold, double eps) const;
};

inline MXU_HaarEvaluator::Feature :: Feature()
{
    tilted = false;
    rect[0].r = rect[1].r = rect[2].r = Rect();
    rect[0].weight = rect[1].weight = rect[2].weight = 0;
    p[0][0] = p[0][1] = p[0][2] = p[0][3] =
        p[1][0] = p[1][1] = p[1][2] = p[1][3] =
        p[2][0] = p[2][1] = p[2][2] = p[2][3] = 0;
}

inline float MXU_HaarEvaluator::Feature :: calc( int _offset ) const
{
    float ret = rect[0].weight * CALC_SUM(p[0], _offset) + rect[1].weight * CALC_SUM(p[1], _offset);

    if( rect[2].weight != 0.0f )
        ret += rect[2].weight * CALC_SUM(p[2], _offset);

    return ret;
}

inline void MXU_HaarEvaluator::Feature :: updatePtrs( const Mat& _sum )
{
    const int* ptr = (const int*)_sum.data;
    size_t step = _sum.step/sizeof(ptr[0]);

    if (tilted)
    {
        MXU_TILTED_PTRS( p[0][0], p[0][1], p[0][2], p[0][3], ptr, rect[0].r, step );
        MXU_TILTED_PTRS( p[1][0], p[1][1], p[1][2], p[1][3], ptr, rect[1].r, step );
        if (rect[2].weight)
            MXU_TILTED_PTRS( p[2][0], p[2][1], p[2][2], p[2][3], ptr, rect[2].r, step );
    }
    else
    {
        MXU_SUM_PTRS( p[0][0], p[0][1], p[0][2], p[0][3], ptr, rect[0].r, step );
        MXU_SUM_PTRS( p[1][0], p[1][1], p[1][2], p[1][3], ptr, rect[1].r, step );
        if (rect[2].weight)
            MXU_SUM_PTRS( p[2][0], p[2][1], p[2][2], p[2][3], ptr, rect[2].r, step );
    }
}

inline MXU_LBPEvaluator::Feature :: Feature()
{
    rect = Rect();
    for( int i = 0; i < 16; i++ )
        p[i] = 0;
}

inline int MXU_LBPEvaluator::Feature :: calc( int _offset ) const
{
    int cval = CALC_SUM_( p[5], p[6], p[9], p[10], _offset );
    return (CALC_SUM_( p[0], p[1], p[4], p[5], _offset ) >= cval ? 128 : 0) |   // 0
           (CALC_SUM_( p[1], p[2], p[5], p[6], _offset ) >= cval ? 64 : 0) |    // 1
           (CALC_SUM_( p[2], p[3], p[6], p[7], _offset ) >= cval ? 32 : 0) |    // 2
           (CALC_SUM_( p[6], p[7], p[10], p[11], _offset ) >= cval ? 16 : 0) |  // 5
           (CALC_SUM_( p[10], p[11], p[14], p[15], _offset ) >= cval ? 8 : 0)|  // 8
           (CALC_SUM_( p[9], p[10], p[13], p[14], _offset ) >= cval ? 4 : 0)|   // 7
           (CALC_SUM_( p[8], p[9], p[12], p[13], _offset ) >= cval ? 2 : 0)|    // 6
           (CALC_SUM_( p[4], p[5], p[8], p[9], _offset ) >= cval ? 1 : 0);
}

inline void MXU_LBPEvaluator::Feature :: updatePtrs( const Mat& _sum )
{
    const int* ptr = (const int*)_sum.data;
    size_t step = _sum.step/sizeof(ptr[0]);
    Rect tr = rect;
    MXU_SUM_PTRS( p[0], p[1], p[4], p[5], ptr, tr, step );
    tr.x += 2*rect.width;
    MXU_SUM_PTRS( p[2], p[3], p[6], p[7], ptr, tr, step );
    tr.y += 2*rect.height;
    MXU_SUM_PTRS( p[10], p[11], p[14], p[15], ptr, tr, step );
    tr.x -= 2*rect.width;
    MXU_SUM_PTRS( p[8], p[9], p[12], p[13], ptr, tr, step );
}





#endif /* _MXU_OBJDETECT_HPP */


