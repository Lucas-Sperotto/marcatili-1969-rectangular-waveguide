#pragma once

#include "marcatili/physics/single_guide.hpp"

namespace marcatili {

SingleGuideResult SolveMetalGuideEyClosedForm(const SingleGuideConfig& config);
SingleGuideResult SolveMetalGuideEyExact(const SingleGuideConfig& config);
SingleGuideResult SolveMetalGuideEy(const SingleGuideConfig& config);

}  // namespace marcatili
