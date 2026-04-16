#pragma once

#include "marcatili/physics/single_guide.hpp"

namespace marcatili {

SingleGuideResult SolveSlabGuideClosedForm(const SingleGuideConfig& config);
SingleGuideResult SolveSlabGuideExact(const SingleGuideConfig& config);
SingleGuideResult SolveSlabGuide(const SingleGuideConfig& config);

}  // namespace marcatili
