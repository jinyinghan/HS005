/*M///////////////////////////////////////////////////////////////////////////////////////
 //
 //  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
 //
 //  By downloading, copying, installing or using the software you agree to this license.
 //  If you do not agree to this license, do not download, install,
 //  copy or use the software.
 //
 //
 //                           License Agreement
 //                For Open Source Computer Vision Library
 //
 // Copyright (C) 2013, OpenCV Foundation, all rights reserved.
 // Third party copyrights are property of their respective owners.
 //
 // Redistribution and use in source and binary forms, with or without modification,
 // are permitted provided that the following conditions are met:
 //
 //   * Redistribution's of source code must retain the above copyright notice,
 //     this list of conditions and the following disclaimer.
 //
 //   * Redistribution's in binary form must reproduce the above copyright notice,
 //     this list of conditions and the following disclaimer in the documentation
 //     and/or other materials provided with the distribution.
 //
 //   * The name of the copyright holders may not be used to endorse or promote products
 //     derived from this software without specific prior written permission.
 //
 // This software is provided by the copyright holders and contributors "as is" and
 // any express or implied warranties, including, but not limited to, the implied
 // warranties of merchantability and fitness for a particular purpose are disclaimed.
 // In no event shall the Intel Corporation or contributors be liable for any direct,
 // indirect, incidental, special, exemplary, or consequential damages
 // (including, but not limited to, procurement of substitute goods or services;
 // loss of use, data, or profits; or business interruption) however caused
 // and on any theory of liability, whether in contract, strict liability,
 // or tort (including negligence or otherwise) arising in any way out of
 // the use of this software, even if advised of the possibility of such damage.
 //
 //M*/

#ifndef __OPENCV_TRACKER_HPP__
#define __OPENCV_TRACKER_HPP__

#include "../core/mxu_core.hpp"
#include "../imgproc/mxu_types_c.h"
#include <iostream>


#define BOILERPLATE_CODE(name,classname) \
    static Ptr<classname> createTracker(const classname::Params &parameters=classname::Params());\
    virtual ~classname(){};

/*
 * Partially based on:
 * ====================================================================================================================
 * 	- [AAM] S. Salti, A. Cavallaro, L. Di Stefano, Adaptive Appearance Modeling for Video Tracking: Survey and Evaluation
 *  - [AMVOT] X. Li, W. Hu, C. Shen, Z. Zhang, A. Dick, A. van den Hengel, A Survey of Appearance Models in Visual Object Tracking
 *
 * This Tracking API has been designed with PlantUML. If you modify this API please change UML files under modules/tracking/doc/uml
 *
 */

namespace mxu
{

//! @addtogroup tracking
//! @{

/************************************ Tracker Base Class ************************************/

/** @brief Base abstract class for the long-term tracker:
 */
class MXU_EXPORTS_W Tracker // : public virtual Algorithm
{
 public:

  virtual ~Tracker();

  /** @brief Initialize the tracker with a know bounding box that surrounding the target
    @param image The initial frame
    @param boundingBox The initial boundig box

    @return True if initialization went succesfully, false otherwise
     */
  MXU_WRAP bool init( const Mat& image, const Rect2f& boundingBox );

  /** @brief Update the tracker, find the new most likely bounding box for the target
    @param image The current frame
    @param boundingBox The boundig box that represent the new target location, if true was returned, not
    modified otherwise

    @return True means that target was located and false means that tracker cannot locate target in
    current frame. Note, that latter *does not* imply that tracker has failed, maybe target is indeed
    missing from the frame (say, out of sight)
     */
  MXU_WRAP bool update( const Mat& image, MXU_OUT Rect2f& boundingBox );

  /** @brief Creates a tracker by its name.
    @param trackerType Tracker type

    The following detector types are supported:

    -   "MIL" -- TrackerMIL
    -   "BOOSTING" -- TrackerBoosting
     */
  MXU_WRAP static Ptr<Tracker> create( const String& trackerType );

  // virtual void read( const FileNode& fn )=0;
  // virtual void write( FileStorage& fs ) const=0;

  // Ptr<TrackerModel> getModel()
  // {
  // 	  return model;
  // }


  virtual bool initImpl( const Mat& image, const Rect2f& boundingBox ) = 0;
  virtual bool updateImpl( const Mat& image, vector<Rect>& boundingBoxes, Rect2f& boundingBox, bool& isLearning, float& confidence ) = 0;
 protected:

  bool isInit;

  // Ptr<TrackerFeatureSet> featureSet;
  // Ptr<TrackerSampler> sampler;
  // Ptr<TrackerModel> model;
};

/** @brief KCF is a novel tracking framework that utilizes properties of circulant matrix to enhance the processing speed.
 * This tracking method is an implementation of @cite KCF_ECCV which is extended to KFC with color-names features (@cite KCF_CN).
 * The original paper of KCF is available at <http://home.isr.uc.pt/~henriques/circulant/index.html>
 * as well as the matlab implementation. For more information about KCF with color-names features, please refer to
 * <http://www.cvl.isy.liu.se/research/objrec/visualtracking/colvistrack/index.html>.
 */
class MXU_EXPORTS Tracker2 : public Tracker
{
public:
	/**
	* \brief Feature type to be used in the tracking grayscale, colornames, compressed color-names
	* The modes available now:
	-   "GRAY" -- Use grayscale values as the feature
	-   "CN" -- Color-names feature
	*/
	enum MODE {
		GRAY = (1u << 0),
		CN = (1u << 1),
		CUSTOM = (1u << 2)
	};

	struct MXU_EXPORTS Params
	{
		/**
		* \brief Constructor
		*/
		Params();

		/**
		* \brief Read parameters from file, currently unused
		*/
		// void read(const FileNode& /*fn*/);

		/**
		* \brief Read parameters from file, currently unused
		*/
		// void write(FileStorage& /*fs*/) const;

		double sigma;                 //!<  gaussian kernel bandwidth
		double lambda;                //!<  regularization
		double interp_factor;         //!<  linear interpolation factor for adaptation
		double output_sigma_factor;   //!<  spatial bandwidth (proportional to target)
		double pca_learning_rate;     //!<  compression learning rate
		bool resize;                  //!<  activate the resize feature to improve the processing speed
		bool split_coeff;             //!<  split the training coefficients into two matrices
		bool wrap_kernel;             //!<  wrap around the kernel values
		bool compress_feature;        //!<  activate the pca method to compress the features
		int max_patch_size;           //!<  threshold for the ROI size
		int compressed_size;          //!<  feature size after compression
		unsigned int desc_pca;        //!<  compressed descriptors of Tracker2::MODE
		unsigned int desc_npca;       //!<  non-compressed descriptors of Tracker2::MODE
	};

	virtual void setFeatureExtractor(void(*)(const Mat, const Rect, Mat&), bool pca_func = false);

	/** @brief Constructor
	@param parameters KCF parameters Tracker2::Params
	*/
	BOILERPLATE_CODE("2", Tracker2);
};

//! @}

} /* namespace cv */

#endif
