#pragma once

#include <string>

namespace marcatili {

/**
 * @brief Famílias modais do modelo reduzido de Marcatili.
 *
 * @details
 * - `kEy`: família predominantemente polarizada em y;
 * - `kEx`: família predominantemente polarizada em x.
 */
enum class SingleGuideFamily {
    kEy,
    kEx
};

/**
 * @brief Modelo numérico usado para resolver o guia único.
 *
 * @details
 * - `kClosedForm`: usa as aproximações algébricas em forma fechada;
 * - `kExact`: resolve numericamente as equações transcendentais do modelo reduzido.
 */
enum class SingleGuideSolverModel {
    kClosedForm,
    kExact
};

/**
 * @brief Configuração de entrada para um solve pontual do guia único.
 */
struct SingleGuideConfig {
    std::string case_id;
    std::string article_target;
    std::string csv_output_path;

    SingleGuideSolverModel solver_model = SingleGuideSolverModel::kClosedForm;
    SingleGuideFamily family = SingleGuideFamily::kEy;

    int p = 1;
    int q = 1;

    double wavelength = 0.0;
    double a = 0.0;
    double b = 0.0;

    double n1 = 0.0;
    double n2 = 0.0;
    double n3 = 0.0;
    double n4 = 0.0;
    double n5 = 0.0;
};

/**
 * @brief Diagnósticos auxiliares da validade assintótica do modelo.
 */
struct SingleGuideApproximationChecks {
    double kx_a3_over_pi_squared = 0.0;
    double kx_a5_over_pi_squared = 0.0;
    double ky_a2_over_pi_squared = 0.0;
    double ky_a4_over_pi_squared = 0.0;
};

/**
 * @brief Resultado completo de um solve pontual do guia único.
 */
struct SingleGuideResult {
    SingleGuideConfig config;

    bool domain_valid = false;
    bool guided = false;

    std::string status;
    std::string status_class;
    std::string equations_used;

    double k0 = 0.0;
    double k1 = 0.0;
    double k2 = 0.0;
    double k3 = 0.0;
    double k4 = 0.0;
    double k5 = 0.0;

    double A2 = 0.0;
    double A3 = 0.0;
    double A4 = 0.0;
    double A5 = 0.0;

    double kx = 0.0;
    double ky = 0.0;
    double kz = 0.0;

    double xi3 = 0.0;
    double xi5 = 0.0;
    double eta2 = 0.0;
    double eta4 = 0.0;

    double b_over_A4 = 0.0;
    double kz_normalized_against_n4 = 0.0;
    double critical_external_index = 0.0;
    double critical_external_wave_number = 0.0;

    SingleGuideApproximationChecks approximation_checks;
};

std::string ToString(SingleGuideFamily family);
SingleGuideFamily ParseSingleGuideFamily(const std::string& family_text);

std::string ToString(SingleGuideSolverModel solver_model);
SingleGuideSolverModel ParseSingleGuideSolverModel(const std::string& solver_model_text);

/**
 * @brief Resolve o guia único com a aproximação em forma fechada.
 *
 * @details
 * - Para a família `E_y`, usa as aproximações associadas às Eqs. (12) e (13);
 * - Para a família `E_x`, usa as aproximações associadas às Eqs. (22) e (23).
 */
SingleGuideResult SolveSingleGuideClosedForm(const SingleGuideConfig& config);

/**
 * @brief Resolve o guia único numericamente no modelo transcendental reduzido.
 *
 * @details
 * Aqui, `exact` significa resolver numericamente:
 * - Eq. (6) e Eq. (7), para a família `E_y`;
 * - Eq. (20) e Eq. (21), para a família `E_x`.
 *
 * Isso não corresponde à solução vetorial completa do problema 2D sem aproximações.
 */
SingleGuideResult SolveSingleGuideExact(const SingleGuideConfig& config);

/**
 * @brief Dispatcher principal do solver do guia único.
 */
SingleGuideResult SolveSingleGuide(const SingleGuideConfig& config);

}  // namespace marcatili
