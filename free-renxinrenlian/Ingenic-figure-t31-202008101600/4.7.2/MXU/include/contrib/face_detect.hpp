#ifndef __FACE_DETECT_HPP__
#define __FACE_DETECT_HPP__

#include "../mxu.h"
#include <cctype>
#include <iostream>
#include <iterator>
#include <stdio.h>

#include <sys/time.h>

using namespace std;
using namespace mxu;

class FaceDetect
{

public:
  Mat LBPMap;

protected:

  class FaceData
  { 
  public:
    uchar OW;
    uchar OH;
    uchar cs;
    unsigned short wt;
    unsigned short ssS;
    vector<unsigned short> snt;
    vector<unsigned short> sft;
    vector<int> treeThresholds;
    vector<int> subsets_array;
    vector<int> leaves_array;  
    vector<uchar> rx;
    vector<uchar> ry;

  };

  vector< FaceData > XMLData;
  FaceData eyeData;
 
  virtual void getData(FaceData& data, int n);
  virtual void getEyeData();

public:
  FaceDetect(int cols, int rows);
  FaceDetect(bool eye);
  virtual ~FaceDetect();
  unsigned short* resize_buf;

  virtual void detect(const Mat& img,  double factor, vector<Rect>& face, Size minObjectSize, Size maxObjectSize);
  virtual void detect_1xml(const Mat& img,  double factor, vector<Rect>& face, Size minObjectSize, Size maxObjectSize);
};

#endif
