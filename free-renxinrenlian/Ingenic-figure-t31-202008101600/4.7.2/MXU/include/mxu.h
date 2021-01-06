
#ifndef __JZ_MXU_MXU_H__
#define __JZ_MXU_MXU_H__

#include "./core/mxu_core_c.h"
#include "./core/mxu_core.hpp"
#include "./core/vector.h"
#include "./imgproc/mxu_imgproc_c.h"
#include "./imgproc/mxu_imgproc.hpp"
#include "./video/mxu_tracking.hpp"
#include "./merge/mxu_merge_c.h"
#include "./merge/mxu_merge.hpp"
#include "./objdetect/mxu_objdetect.hpp"
#include "./contrib/mxu_contrib.hpp"
#include "./contrib/face_detect.hpp"
#include "./contrib/tracker.hpp"
#include "./contrib/figure_detection_tracker.hpp"
#include "./contrib/face_detection_tracker.hpp"
#include "./objdetect/mxu_microCascade.hpp"

#if !defined(MXU_IMPL)
#define MXU_IMPL extern "C"
#endif //MXU_IMPL

#if defined(__cplusplus)
#include "./core/mxu_internal.hpp"
#endif //__cplusplus

#endif
