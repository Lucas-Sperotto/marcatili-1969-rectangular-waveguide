#pragma once

#include "marcatili/physics/single_guide.hpp"

namespace marcatili {

/**
 * @brief Resolve o limite de lâmina usando a aproximação em forma fechada.
 *
 * @details
 * No modelo de lâmina, existe apenas uma direção transversal confinada.
 * O resultado é reportado no mesmo contêiner usado pelo guia retangular,
 * com `kx = 0` por construção.
 */
SingleGuideResult SolveSlabGuideClosedForm(const SingleGuideConfig& config);

/**
 * @brief Resolve o limite de lâmina numericamente no modelo transcendental reduzido.
 *
 * @details
 * Aqui, `exact` significa resolver numericamente a equação transcendental 1D
 * herdada do modelo reduzido de Marcatili, e não um problema vetorial completo.
 */
SingleGuideResult SolveSlabGuideExact(const SingleGuideConfig& config);

/**
 * @brief Dispatcher principal do solver de lâmina.
 */
SingleGuideResult SolveSlabGuide(const SingleGuideConfig& config);

}  // namespace marcatili
