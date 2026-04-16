cmake_minimum_required(VERSION 3.16)

project(
    marcatili_1969_rectangular_waveguide
    VERSION 0.1.0
    LANGUAGES CXX
)

include(CTest)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_library(marcatili_core STATIC
    src/io/fig8_io.cpp
    src/io/table1_io.cpp
    src/io/fig7_io.cpp
    src/io/fig6_io.cpp
    src/io/schema_json.cpp
    src/io/single_guide_io.cpp
    src/io/text_io.cpp
    src/physics/metal_guide.cpp
    src/physics/fig8.cpp
    src/physics/table1.cpp
    src/physics/fig7.cpp
    src/physics/fig6.cpp
    src/physics/slab_guide.cpp
    src/physics/single_guide.cpp
    src/placeholder_app.cpp
)

target_include_directories(marcatili_core
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

target_compile_features(marcatili_core PUBLIC cxx_std_17)

function(marcatili_add_app app_name)
    add_executable(${app_name} src/apps/${app_name}.cpp)
    target_link_libraries(${app_name} PRIVATE marcatili_core)
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
    marcatili_add_app(${app_name})
endforeach()

if(BUILD_TESTING)
    add_test(
        NAME solve_single_guide_smoke
        COMMAND solve_single_guide
                ${PROJECT_SOURCE_DIR}/data/input/solve_single_guide.json
                ${CMAKE_BINARY_DIR}/test_outputs/solve_single_guide.json
    )

    add_test(
        NAME reproduce_fig6_smoke
        COMMAND reproduce_fig6
                ${PROJECT_SOURCE_DIR}/data/input/reproduce_fig6.json
                ${CMAKE_BINARY_DIR}/test_outputs/reproduce_fig6.json
    )

    add_test(
        NAME reproduce_fig6_slab_smoke
        COMMAND reproduce_fig6
                ${PROJECT_SOURCE_DIR}/data/input/fig6/SG-006d.json
                ${CMAKE_BINARY_DIR}/test_outputs/reproduce_fig6_slab.json
    )

    add_test(
        NAME reproduce_fig7_smoke
        COMMAND reproduce_fig7
                ${PROJECT_SOURCE_DIR}/data/input/reproduce_fig7.json
                ${CMAKE_BINARY_DIR}/test_outputs/reproduce_fig7.json
    )

    add_test(
        NAME reproduce_fig8_smoke
        COMMAND reproduce_fig8
                ${PROJECT_SOURCE_DIR}/data/input/reproduce_fig8.json
                ${CMAKE_BINARY_DIR}/test_outputs/reproduce_fig8.json
    )

    add_test(
        NAME reproduce_table1_smoke
        COMMAND reproduce_table1
                ${PROJECT_SOURCE_DIR}/data/input/reproduce_table1.json
                ${CMAKE_BINARY_DIR}/test_outputs/reproduce_table1.json
    )
endif()
