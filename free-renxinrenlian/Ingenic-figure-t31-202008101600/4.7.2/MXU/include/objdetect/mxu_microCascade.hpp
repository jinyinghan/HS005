/*M///////////////////////////////////////////////////////////////////////////////////////
//<!-- MiniCro Cascade for pedestrian detection or other detection based on machine learning. -->
//<!-- From : combination of Gentle Adboost and Soft Cascade -->
//<!-- Date : Saturday, 1 August 2015-->
//<!-- Autor: hyu-->
//M*/

//#ifndef __MXU_MICROCASCADE_HPP__
//#define __MXU_MICROCASCADE_HPP__

#include <iostream>

#include "./../core/mxu_core.hpp"
#include "./../core/mxu_core_c.h"
#include "./../objdetect/Markup.h"
#include "./../objdetect/mxu_objdetect.hpp"

#include "./../imgproc/mxu_imgproc.hpp"
#include "./../imgproc/mxu_imgproc_c.h"
#include "./../core/mxu_internal.hpp"

#define USE_treeThreshold

using namespace mxu;
using namespace std;

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

#define MXU_SUM_PTRS( p0, p1, p2, p3, sum, rect, step )                    \
    /* (x, y) */                                                          \
    (p0) = sum + (rect).x + (step) * (rect).y,                            \
    /* (x + w, y) */                                                      \
    (p1) = sum + (rect).x + (rect).width + (step) * (rect).y,             \
    /* (x + w, y) */                                                      \
    (p2) = sum + (rect).x + (step) * ((rect).y + (rect).height),          \
    /* (x + w, y + h) */                                                  \
    (p3) = sum + (rect).x + (rect).width + (step) * ((rect).y + (rect).height)

#define CALC_SUM_(p0, p1, p2, p3, offset) \
  ((p0)[offset] - (p1)[offset] - (p2)[offset] + (p3)[offset])

#define CALC_SUM(rect,offset) CALC_SUM_((rect)[0], (rect)[1], (rect)[2], (rect)[3], offset)

struct MXU_Octave
{
   MXU_Octave() {};
   MXU_Octave(const int i, const Size& origObjSize)
  {
    index = i;
    scale = origObjSize.width/40.f;
    //scale = (index == 2.) ? 1 : (float)(origObjSize.width)/48.f;
    size = Size(origObjSize.width, origObjSize.height);
    //if(index == 0)
    // tempSize = size;
  }

  int index;
  //Size tempSize;

  float scale;
  Size size;
};

struct MXU_Levels
{
  const MXU_Octave* octave;

  float origScale;
  float relScale;
  int scaleshift;

  Size workRect;
  Size objSize;

  float scaling; // 0-th for channels <= 6, 1-st otherwise

  MXU_Levels(const MXU_Octave& oct, const float scale, const int w, const int h)
  :  octave(&oct), origScale(scale), relScale(scale / oct.scale),
       workRect(Size(w, h)),
       objSize(Size(mxuRound(oct.size.width * relScale), mxuRound(oct.size.height * relScale)))
  {
    scaling = ((relScale >= 1.f)? 1.f : (0.89f * std::pow(relScale, 1.099f / std::log(2.f)))) / (relScale * relScale);
    //scaling[1] = 1.f;
    scaleshift = static_cast<int>(relScale * (1 << 16));
  }

  virtual void addDetection(const int x, const int y, vector<Rect>& detections) const;
  virtual float rescale(Rect& scaledRect, const float threshold) const;
};

class MXU_FeatureComputer
{
public:
    enum { HOG = 2, HOG_YUV = 1};
    virtual ~MXU_FeatureComputer();

    virtual bool read(int ki);
    virtual Ptr<MXU_FeatureComputer> clone() const;
    virtual int getFeatureType() const;
    virtual void getRect(const vector<Mat>& _hist) ;
    virtual void getRects(const vector<Mat>& _hist) ;
    virtual void setImage(const Mat& img);
    virtual void setWindow(Point pt, const vector<Mat>& _hist);

    static Ptr<MXU_FeatureComputer> create(int type);
};


class MXU_featureStorage : public MXU_FeatureComputer
{
public:
    struct Feature
    {
      Feature();

      int calc( int offset ) const;
      void getRect(const vector<Mat>& _hist);
      void updatePtr( const Rect& _rect);

      bool read(int n, int ki);

      enum { CELL_NUM = 4, BIN_NUM = 9 };

      Rect rect[CELL_NUM];
      int featComponent; //component index from 0 to 35
      const int* pF[4]; //for feature calculation
      float rarea;
      Rect featureRect;
      const int* featBuf;
      size_t featStep;
    };

  MXU_featureStorage();
  virtual ~MXU_featureStorage();

  virtual bool read(int ki);
  virtual Ptr<MXU_FeatureComputer> clone() const;
  virtual int getFeatureType() const { return MXU_FeatureComputer::HOG; }
  virtual void setImage( const Mat& image);
  virtual void setWindow( Point pt, const vector<Mat>& _hist);
  virtual void getRects(const vector<Mat>& _hist);

  int operator()(int featureIdx) const
  {
    return featuresPtr[featureIdx].calc(offset);
  }
  Ptr< vector<Feature> > features;
  Feature* featuresPtr;
  vector<Mat> hist;
private:
  virtual void integralHistogram( const Mat& srcImage, vector<Mat> &histogram, int nbins ) const;

  int origObjWidth;
  int origObjHeight;

  int offset;
};

//------------------为了避免同namespace cv下的命名冲突，在类名中添加MXU_---------------------------//
class MXU_microCascade
{
 public:

  enum { NO_REJECT = 1, DOLLAR = 2, /*PASCAL = 4,*/ DEFAULT = NO_REJECT};

  MXU_microCascade();
  MXU_microCascade( const string& filename );
  virtual ~MXU_microCascade();

  // virtual bool load(const string& filename);
  virtual bool xmldata2vec();

  virtual void detect(const Mat& image, vector<Rect>& objects, double scaleFactor, int minNeighbors,
		      Size minObjectSize, Size maxObjectSize) ;
  virtual int detectAt(Ptr<MXU_FeatureComputer>& evaluator, Point pt, const MXU_Levels& level, const MXU_featureStorage& storage, int& index);

  void groupRectangles(vector<Rect>& rectList, int groupThreshold, double eps) const;
  void groupRectangles0( vector<Rect>& rectList, int groupThreshold, double eps, vector<int>* weights, vector<double>* levelWeights ) const;

private:

  float factor;

protected:

  enum { BOOST = 0 };
 template<class FEval>
   friend int predictStump(MXU_microCascade& cascade, Ptr<MXU_FeatureComputer> &featurecomputer, const MXU_Levels& level, double& weight,  int& index);

 class Data
 {
 public:
   struct DTreeNode
   {
     int featureIdx;
     float threshold; // for ordered features only
     int threshold0;
     int left;
     int right;
   };

   struct DTree
   {
     int nodeCount;
   };

   struct Stage
   {
     int first;
     int ntrees;
     float threshold;
     int threshold0;
   };

   bool read_data(int ki);

   bool isStumpBased;

   int stageType;
   int featureType;
   int ncategories;

   vector<Stage> stages;
   vector<DTree> classifiers;
   vector<DTreeNode> nodes;
   vector<float> leaves;
   vector<int> leaves0;
   vector<int> subsets;
#ifdef USE_treeThreshold
   vector<float> treeThresholds;  ///add by hyu
#endif

   int origObjWidth;
   int origObjHeight;

   Size minObjectSize;
   Size maxObjectSize;
   double scaleFactor;

   vector<MXU_Levels> levels;
   //vector<Octave> octaves;
   MXU_Octave octave;

   Size frameSize;

   typedef vector<MXU_Octave>::iterator octIt_t;
   octIt_t fitOctave(const float& factor, vector<MXU_Octave>& _octaves)
   { //find the most close to the  model.
     float minAbsLog = FLT_MAX;
     octIt_t res = _octaves.begin();
     for (octIt_t oct = _octaves.begin(); oct < _octaves.end(); ++oct)
       {
	 const MXU_Octave& octave_s =*oct;
	 float octaveScale = octave_s.scale;
	 float absScale = fabs(factor - octaveScale);

	 if(absScale < minAbsLog)
	   {
	     res = oct;
	     minAbsLog = absScale;
	   }
       }
     return res;
   }

   // compute levels of full pyramid
   void calcLevels(const Size& curr, double factor, Size mins, Size maxs);
 };

 Data data;
 // vector<Octave> octaves;
 Ptr<MXU_FeatureComputer> featurecomputer;

 vector< Data > data_vec;
 vector< Ptr<MXU_FeatureComputer> > featurecomputer_vec;

};


inline MXU_featureStorage::Feature :: Feature()
{
    rect[0] = rect[1] = rect[2] = rect[3] = Rect();
    pF[0] = pF[1] = pF[2] = pF[3] = 0;
    featComponent = 0;
}

inline void MXU_featureStorage::Feature::getRect(const vector<Mat>& hist)
{
  int cellIdx = featComponent / BIN_NUM;
  int binIdx = featComponent % BIN_NUM;
  featBuf = (const int*)hist[binIdx].data;
  featStep = hist[0].step / sizeof(featBuf[0]);

  featureRect = rect[cellIdx];
  rarea = 1.f / (rect[cellIdx].width * rect[cellIdx].height);
}

inline void MXU_featureStorage::Feature::updatePtr(const Rect& _rect)
{
  MXU_SUM_PTRS( pF[0], pF[1], pF[2], pF[3], featBuf, _rect, featStep );
}

inline int MXU_featureStorage::Feature::calc( int _offset ) const
{
  int res = CALC_SUM(pF, _offset);
  return res;
}

template<class FEval>
inline int predictStump(MXU_microCascade& cascade, Ptr<MXU_FeatureComputer> &_featurecomputer, const MXU_Levels& level, double& sum, int& index)
{
    int nodeOfs = 0, leafOfs = 0;
    FEval& featurecomputer = (FEval&)*_featurecomputer;
    ///float* cascadeLeaves = &cascade.data_vec[i].leaves[0];
    int* cascadeLeaves = &cascade.data_vec[index].leaves0[0];    ///*
    MXU_microCascade::Data::DTreeNode* cascadeNodes = &cascade.data_vec[index].nodes[0];
    MXU_microCascade::Data::Stage* cascadeStages = &cascade.data_vec[index].stages[0];
#ifdef USE_treeThreshold
    float* treeThresholds = &cascade.data_vec[index].treeThresholds[0];
#endif
    int nstages = (int)cascade.data_vec[index].stages.size();

    for( int stageIdx = 0; stageIdx < nstages; stageIdx++ )
    {
        MXU_microCascade::Data::Stage& stage = cascadeStages[stageIdx];
        sum = 0.0;

        int ntrees = stage.ntrees;
        for( int i = 0; i < ntrees; i++, nodeOfs++, leafOfs+= 2 )
        {
#ifdef USE_treeThreshold
	    float treeThreshold = treeThresholds[nodeOfs];
#endif

	  ///std::cout<<"treeThreshold = "<<treeThreshold<<std::endl;
            MXU_microCascade::Data::DTreeNode& node = cascadeNodes[nodeOfs];

	    Rect scaledRect = featurecomputer.featuresPtr[node.featureIdx].featureRect;
            float threshold = level.rescale(scaledRect, node.threshold0)  * (featurecomputer.featuresPtr[node.featureIdx].rarea);
	    // cout<<"node.threshold0 = "<<node.threshold0<<",rescale.threshold = "<<level.rescale(scaledRect, node.threshold0) <<"ff = "<<(featurecomputer.featuresPtr[node.featureIdx].rarea)<<",threshold = "<<threshold<<endl;
  	    featurecomputer.featuresPtr[node.featureIdx].updatePtr(scaledRect);

	    //int value = featurecomputer(node.featureIdx);
	    double value = (double)(featurecomputer(node.featureIdx)<<1);
	    ///cout<<"value = "<<value<<endl;
            sum += cascadeLeaves[ value < threshold ? leafOfs : leafOfs + 1 ];

#ifdef USE_treeThreshold
	    if(sum < treeThreshold)
	    return -stageIdx;
#endif
        }
#if 1
	if( sum < stage.threshold0 ) //sum < stage.threshold0
	    return -stageIdx;
#endif
    }

    return 1;
}

