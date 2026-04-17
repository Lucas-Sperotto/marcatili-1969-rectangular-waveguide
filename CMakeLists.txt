cmake_minimum_required(VERSION 3.16)

project(
    marcatili_1969_rectangular_waveguide
    VERSION 0.1.0
    LANGUAGES CXX
)

# ----------------------------------------------------------------------
# Configuração global do projeto
# ----------------------------------------------------------------------

include(CTest)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Todos os executáveis gerados irão para build/bin
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# Diretório central para saídas dos testes
set(MARCATILI_TEST_OUTPUT_DIR "${CMAKE_BINARY_DIR}/test_outputs")

# ----------------------------------------------------------------------
# Núcleo compartilhado da biblioteca
# ----------------------------------------------------------------------

set(MARCATILI_CORE_SOURCES
    # Math
    src/math/root_finding.cpp
    src/math/waveguide_math.cpp

    # IO
    src/io/coupler_io.cpp
    src/io/fig11_io.cpp
    src/io/fig10_io.cpp
    src/io/fig8_io.cpp
    src/io/table1_io.cpp
    src/io/fig7_io.cpp
    src/io/fig6_io.cpp
    src/io/schema_json.cpp
    src/io/single_guide_io.cpp
    src/io/text_io.cpp

    # Physics
    src/physics/coupler.cpp
    src/physics/metal_guide.cpp
    src/physics/fig11.cpp
    src/physics/fig10.cpp
    src/physics/fig8.cpp
    src/physics/table1.cpp
    src/physics/fig7.cpp
    src/physics/fig6.cpp
    src/physics/slab_guide.cpp
    src/physics/single_guide.cpp

    # Placeholder / utilitário comum
    src/placeholder_app.cpp
)

add_library(marcatili_core STATIC ${MARCATILI_CORE_SOURCES})

target_include_directories(marcatili_core
    PUBLIC
        "${PROJECT_SOURCE_DIR}/include"
)

target_compile_features(marcatili_core
    PUBLIC
        cxx_std_17
)

# ----------------------------------------------------------------------
# Função auxiliar para registrar executáveis
# ----------------------------------------------------------------------

function(marcatili_add_app app_name)
    add_executable("${app_name}" "src/apps/${app_name}.cpp")
    target_link_libraries("${app_name}" PRIVATE marcatili_core)
endfunction()

set(MARCATILI_APPS
    solve_single_guide
    solve_coupler
    reproduce_fig6
    reproduce_fig7
    reproduce_fig8
    reproduce_fig10
    reproduce_fig11
    reproduce_table1
)

foreach(app_name IN LISTS MARCATILI_APPS)
    marcatili_add_app("${app_name}")
endforeach()

# ----------------------------------------------------------------------
# Função auxiliar para smoke tests
# ----------------------------------------------------------------------

function(marcatili_add_smoke_test test_name target_name input_file output_file)
    add_test(
        NAME "${test_name}"
        COMMAND "${target_name}"
                "${input_file}"
                "${output_file}"
    )
endfunction()

# ----------------------------------------------------------------------
# Testes
# ----------------------------------------------------------------------

if(BUILD_TESTING)
    # Garante que o diretório de saída dos testes exista
    file(MAKE_DIRECTORY "${MARCATILI_TEST_OUTPUT_DIR}")

    marcatili_add_smoke_test(
        solve_single_guide_smoke
        solve_single_guide
        "${PROJECT_SOURCE_DIR}/data/input/solve_single_guide.json"
        "${MARCATILI_TEST_OUTPUT_DIR}/solve_single_guide.json"
    )

    marcatili_add_smoke_test(
        solve_single_guide_legacy_flat_smoke
        solve_single_guide
        "${PROJECT_SOURCE_DIR}/data/input/solve_single_guide_legacy_flat.json"
        "${MARCATILI_TEST_OUTPUT_DIR}/solve_single_guide_legacy_flat.json"
    )

    marcatili_add_smoke_test(
        solve_coupler_smoke
        solve_coupler
        "${PROJECT_SOURCE_DIR}/data/input/solve_coupler.json"
        "${MARCATILI_TEST_OUTPUT_DIR}/solve_coupler.json"
    )

    marcatili_add_smoke_test(
        reproduce_fig6_smoke
        reproduce_fig6
        "${PROJECT_SOURCE_DIR}/data/input/reproduce_fig6.json"
        "${MARCATILI_TEST_OUTPUT_DIR}/reproduce_fig6.json"
    )

    marcatili_add_smoke_test(
        reproduce_fig6_slab_smoke
        reproduce_fig6
        "${PROJECT_SOURCE_DIR}/data/input/fig6/SG-006d.json"
        "${MARCATILI_TEST_OUTPUT_DIR}/reproduce_fig6_slab.json"
    )

    marcatili_add_smoke_test(
        reproduce_fig7_smoke
        reproduce_fig7
        "${PROJECT_SOURCE_DIR}/data/input/reproduce_fig7.json"
        "${MARCATILI_TEST_OUTPUT_DIR}/reproduce_fig7.json"
    )

    marcatili_add_smoke_test(
        reproduce_fig8_smoke
        reproduce_fig8
        "${PROJECT_SOURCE_DIR}/data/input/reproduce_fig8.json"
        "${MARCATILI_TEST_OUTPUT_DIR}/reproduce_fig8.json"
    )

    marcatili_add_smoke_test(
        reproduce_fig10_smoke
        reproduce_fig10
        "${PROJECT_SOURCE_DIR}/data/input/reproduce_fig10.json"
        "${MARCATILI_TEST_OUTPUT_DIR}/reproduce_fig10.json"
    )

    marcatili_add_smoke_test(
        reproduce_fig11_smoke
        reproduce_fig11
        "${PROJECT_SOURCE_DIR}/data/input/reproduce_fig11.json"
        "${MARCATILI_TEST_OUTPUT_DIR}/reproduce_fig11.json"
    )

    marcatili_add_smoke_test(
        reproduce_table1_smoke
        reproduce_table1
        "${PROJECT_SOURCE_DIR}/data/input/reproduce_table1.json"
        "${MARCATILI_TEST_OUTPUT_DIR}/reproduce_table1.json"
    )
endif()
