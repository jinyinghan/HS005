#ifndef __JZ_MXU_CORE_UTILITY_H__
#define __JZ_MXU_CORE_UTILITY_H__

#ifndef __cplusplus
#  error utility.hpp header must be compiled as C++
#endif

#include "mxu_core.hpp"

namespace mxu
{

/** @brief Base class for parallel data processors
*/
class MXU_EXPORTS ParallelLoopBody
{
public:
    virtual ~ParallelLoopBody();
    virtual void operator() (const Range& range) const = 0;
};


/** @brief Parallel data processor
*/
MXU_EXPORTS void parallel_for_(const Range& range, const ParallelLoopBody& body, double nstripes=-1.);

} //namespace mxu

#endif
