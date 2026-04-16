#pragma once

#include "marcatili/physics/single_guide.hpp"

namespace marcatili {

SingleGuideResult SolveMetalGuideClosedForm(const SingleGuideConfig& config);
SingleGuideResult SolveMetalGuideExact(const SingleGuideConfig& config);
SingleGuideResult SolveMetalGuide(const SingleGuideConfig& config);

}  // namespace marcatili
