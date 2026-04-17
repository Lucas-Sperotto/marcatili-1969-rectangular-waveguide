#pragma once

#include "marcatili/physics/single_guide.hpp"

namespace marcatili {

/**
 * @brief Resolve o caso do guia metalizado usando a aproximação em forma fechada.
 *
 * @details
 * Esta função aplica a variante aproximada do modelo de Marcatili para o caso
 * com interface metalizada, reutilizando a mesma convenção de entrada e saída
 * do solver de guia único.
 */
SingleGuideResult SolveMetalGuideClosedForm(const SingleGuideConfig& config);

/**
 * @brief Resolve o caso do guia metalizado usando a raiz numérica da equação transversal.
 *
 * @details
 * Aqui, `exact` significa resolver numericamente a versão transcendental do
 * modelo reduzido adotado para o guia metalizado, e não um problema vetorial
 * completo 2D sem aproximações.
 */
SingleGuideResult SolveMetalGuideExact(const SingleGuideConfig& config);

/**
 * @brief Dispatcher principal do solver de guia metalizado.
 *
 * @details
 * Seleciona automaticamente entre `SolveMetalGuideClosedForm` e
 * `SolveMetalGuideExact` com base em `config.solver_model`.
 */
SingleGuideResult SolveMetalGuide(const SingleGuideConfig& config);

}  // namespace marcatili
