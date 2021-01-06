#ifndef __WRTCV_OBJDETECT_DBT_HPP__
#define __WRTCV_OBJDETECT_DBT_HPP__

#include <vector>
#include "./../objdetect/mxu_objdetect.hpp"

namespace mxu
{
enum { FULLBODY = 0, FACE = 1, HEAD = 2 };

class /*MXU_EXPORTS*/ FigureDetection
{
    public:
        struct Parameters
        {
            int maxTrackLifetime;
            int minDetectionPeriod;
            Parameters();
        };

        class IDetector
        {
            public:
                IDetector():
                    minObjSize(40, 40),
                    maxObjSize(INT_MAX, INT_MAX),
                    minNeighbours(2),
                    scaleFactor(1.1f)
                {}

	  virtual void detect(const Mat& image, std::vector<Rect>& objects, int dType) = 0;

                void setMinObjectSize(const Size& min)
                {
                    minObjSize = min;
                }
                void setMaxObjectSize(const Size& max)
                {
                    maxObjSize = max;
                }
                Size getMinObjectSize() const
                {
                    return minObjSize;
                }
                Size getMaxObjectSize() const
                {
                    return maxObjSize;
                }
                float getScaleFactor()
                {
                    return scaleFactor;
                }
                void setScaleFactor(float value)
                {
                    scaleFactor = value;
                }
                int getMinNeighbours()
                {
                    return minNeighbours;
                }
                void setMinNeighbours(int value)
                {
                    minNeighbours = value;
                }
                virtual ~IDetector() {}

            protected:
                Size minObjSize;
                Size maxObjSize;
                int minNeighbours;
                float scaleFactor;
        };

        FigureDetection(Ptr<IDetector> mainDetector, Ptr<IDetector> trackingDetector, const Parameters& params);
        virtual ~FigureDetection();

        virtual bool run();
        virtual void stop();
        virtual void resetTracking();

  virtual void process(const Mat& imageGray, int ddType, bool flag_detect);

        bool setParameters(const Parameters& params);
        const Parameters& getParameters() const;


        typedef std::pair<Rect, int> Object;
        virtual void getObjects(std::vector<Rect>& result, std::vector<int>& result_i, std::vector<int>& scores);
        //int in, out;   ///head used

        enum ObjectStatus
        {
            DETECTED_NOT_SHOWN_YET,
            DETECTED,
            DETECTED_TEMPORARY_LOST,
            WRONG_OBJECT
        };
        struct ExtObject
        {
            int id;
            Rect location;
            ObjectStatus status;
            ExtObject(int _id, Rect _location, ObjectStatus _status)
                :id(_id), location(_location), status(_status)
            {
            }
        };

        virtual int addObject(const Rect& location); //returns id of the new object

    protected:
        class SeparateDetectionWork;
        Ptr<SeparateDetectionWork> separateDetectionWork;
        friend void* workcycleObjectDetectorFunction(void* p);

        struct InnerParameters
        {
            int numLastPositionsToTrack;
            int numStepsToWaitBeforeFirstShow;
            int numStepsToTrackWithoutDetectingIfObjectHasNotBeenShown;
            int numStepsToShowWithoutDetecting;

            float coeffTrackingWindowSize;
            float coeffObjectSizeToTrack;
            float coeffObjectSpeedUsingInPrediction;

	    int detType;

            InnerParameters();
        };
        Parameters parameters;
        InnerParameters innerParameters;

        struct TrackedObject
        {
            typedef std::vector<Rect> PositionsVector;

            PositionsVector lastPositions;
	    bool incorrespondence;
	    int Id;
	    float score;

            int numDetectedFrames;
            int numFramesNotDetected;
            int id;
	    Rect firstPosition;

            TrackedObject(const Rect& rect):numDetectedFrames(1), numFramesNotDetected(0)
            {
                lastPositions.push_back(rect);
                id=getNextId();
            };

            static int getNextId()
            {
                static int _id=0;
                return _id++;
            }
        };

        int numTrackedSteps;
        //std::vector<TrackedObject> trackedObjects;

        std::vector<float> weightsPositionsSmoothing;
        std::vector<float> weightsSizesSmoothing;

        Ptr<IDetector> cascadeForTracking;

        void updateTrackedObjects(const std::vector<Rect>& detectedObjects);
        Rect calcTrackedObjectPositionToShow(int i) const;
        Rect calcTrackedObjectPositionToShow(int i, ObjectStatus& status) const;
        void detectInRegion(const Mat& img, const Rect& r, std::vector<Rect>& detectedObjectsInRegions);
public:
        std::vector<TrackedObject> trackedObjects;
};

//! @} objdetect

class CascadeDetectorAdapter: public FigureDetection::IDetector
{
    public:
  CascadeDetectorAdapter(Ptr<MXU_CascadeClassifier> detector):
            IDetector(),
            Detector(detector)
        {
            assert(detector);
        }

    void detect(const Mat &Image, std::vector<Rect> &objects, int dType)
    {
      if(dType == FULLBODY)
        Detector->detectMultiScale_3xml( Image, objects, 1.3, Size(32, 32), Size(235, 235) );
      else if(dType == HEAD)
	Detector->detectMultiScale_1xml( Image, objects, 1.2, Size(32, 32), Size(200, 200) ); //upper
    }

    virtual ~CascadeDetectorAdapter()
    {}

    private:
        CascadeDetectorAdapter();
        Ptr<MXU_CascadeClassifier> Detector;
 };

} //end of mxu namespace
//#endif

#endif
