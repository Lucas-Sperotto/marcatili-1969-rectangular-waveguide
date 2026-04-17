#pragma once

#include <limits>
#include <string>
#include <vector>

#include "marcatili/physics/single_guide.hpp"

namespace marcatili {

/**
 * @brief Especificação de uma reta modal do nomograma da Fig. 7.
 *
 * @details
 * Cada reta modal representa a condição
 *   p X + q Y = 1
 * para uma dada família modal e para índices modais fixos.
 */
struct Figure7ModeSpec {
    SingleGuideFamily family = SingleGuideFamily::kEy;
    int p = 1;
    int q = 1;
    std::string line_id;
};

/**
 * @brief Especificação de uma reta de C no nomograma da Fig. 7.
 *
 * @details
 * Cada reta de C representa uma condição geométrica/material específica
 * no plano (X, Y).
 */
struct Figure7CLineSpec {
    double c_value = 0.0;
    std::string line_id;
};

/**
 * @brief Configuração de entrada para a reprodução da Fig. 7.
 *
 * @details
 * Esta configuração reúne:
 * - os parâmetros geométricos e materiais do guia;
 * - o conjunto de retas modais a serem desenhadas;
 * - o conjunto de retas de C;
 * - os dados auxiliares para conferência contra a leitura do artigo.
 */
struct Figure7Config {
    std::string case_id;
    std::string article_target;

    std::string lines_csv_output_path;
    std::string intersections_csv_output_path;

    std::string article_reference_mode_line_id;
    std::string article_reference_note;

    double wavelength = 0.0;
    double a = 0.0;
    double b = 0.0;

    double n1 = 0.0;
    double n2 = 0.0;
    double n3 = 0.0;
    double n4 = 0.0;
    double n5 = 0.0;

    int line_point_count = 0;

    double reference_c_value = 0.0;
    double article_reference_y_readoff = std::numeric_limits<double>::quiet_NaN();

    std::vector<Figure7ModeSpec> modes;
    std::vector<Figure7CLineSpec> c_lines;
};

/**
 * @brief Amostra individual de uma reta do nomograma.
 *
 * @details
 * Pode representar tanto:
 * - uma reta modal; quanto
 * - uma reta de C.
 */
struct Figure7LineSample {
    std::string line_kind;
    std::string line_id;

    SingleGuideFamily family = SingleGuideFamily::kEy;
    int p = 0;
    int q = 0;

    double c_value = 0.0;
    int sample_index = 0;

    double x = 0.0;
    double y = 0.0;
};

/**
 * @brief Interseção entre uma reta modal e uma reta de C.
 *
 * @details
 * Além das coordenadas geométricas da interseção no plano (X, Y),
 * a estrutura também guarda a interpretação modal obtida a partir
 * do solver do guia único.
 */
struct Figure7Intersection {
    std::string mode_line_id;
    std::string c_line_id;

    SingleGuideFamily family = SingleGuideFamily::kEy;
    int p = 1;
    int q = 1;

    double c_value = 0.0;
    double x = 0.0;
    double y = 0.0;

    bool is_reference_c = false;
    bool domain_valid = false;
    bool guided = false;

    double kz = 0.0;
    double kz_normalized_against_n4 = 0.0;
};

/**
 * @brief Resumo do exemplo de projeto discutido em torno da Fig. 7.
 *
 * @details
 * Esta estrutura é usada para registrar grandezas derivadas do exemplo
 * analítico apresentado na documentação associada ao nomograma.
 */
struct Figure7DesignExampleSummary {
    bool symmetric_material_pairs = false;
    double a_over_b = 0.0;

    double delta_from_n35 = std::numeric_limits<double>::quiet_NaN();
    double delta_prime_from_n24 = std::numeric_limits<double>::quiet_NaN();
    double sqrt_delta_prime_over_delta = std::numeric_limits<double>::quiet_NaN();
};

/**
 * @brief Verificação opcional contra uma leitura manual do artigo.
 *
 * @details
 * Permite comparar uma interseção calculada com um valor aproximado
 * lido diretamente da figura do artigo.
 */
struct Figure7ArticleReferenceCheck {
    bool available = false;

    std::string mode_line_id;
    std::string note;

    double c_value = std::numeric_limits<double>::quiet_NaN();

    double exact_x = std::numeric_limits<double>::quiet_NaN();
    double exact_y = std::numeric_limits<double>::quiet_NaN();

    double article_y_readoff = std::numeric_limits<double>::quiet_NaN();
    double article_y_absolute_error = std::numeric_limits<double>::quiet_NaN();
    double article_y_relative_error = std::numeric_limits<double>::quiet_NaN();
};

/**
 * @brief Resultado completo da reprodução da Fig. 7.
 *
 * @details
 * Além das linhas e interseções, esta estrutura também guarda:
 * - constantes ópticas derivadas;
 * - parâmetros A2, A3, A4 e A5;
 * - numeradores usados na construção de X e Y;
 * - valor derivado de C;
 * - resumos auxiliares para validação e documentação.
 */
struct Figure7Result {
    Figure7Config config;
    std::string status;

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

    double x_numerator = 0.0;
    double y_numerator = 0.0;

    double derived_c = 0.0;
    double reference_c_absolute_error = std::numeric_limits<double>::quiet_NaN();
    double reference_c_relative_error = std::numeric_limits<double>::quiet_NaN();

    Figure7DesignExampleSummary design_example;
    Figure7ArticleReferenceCheck article_reference_check;

    std::vector<Figure7LineSample> line_samples;
    std::vector<Figure7Intersection> intersections;
};

/**
 * @brief Faz o parsing de uma especificação modal para a Fig. 7.
 *
 * @throws std::invalid_argument Se o texto estiver em formato inválido.
 */
Figure7ModeSpec ParseFigure7ModeSpec(const std::string& mode_text);

/**
 * @brief Faz o parsing de uma especificação de reta de C para a Fig. 7.
 *
 * @throws std::invalid_argument Se o texto estiver em formato inválido.
 */
Figure7CLineSpec ParseFigure7CLineSpec(const std::string& c_text);

/**
 * @brief Resolve a reprodução do nomograma da Fig. 7.
 *
 * @throws std::invalid_argument Se a configuração for inválida.
 * @throws std::runtime_error Se houver falha numérica durante o processo.
 */
Figure7Result SolveFigure7(const Figure7Config& config);

}  // namespace marcatili
