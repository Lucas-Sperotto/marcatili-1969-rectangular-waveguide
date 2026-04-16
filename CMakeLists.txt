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

add_library(marcatili_placeholder STATIC
    src/placeholder_app.cpp
)

target_include_directories(marcatili_placeholder
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

target_compile_features(marcatili_placeholder PUBLIC cxx_std_17)

function(marcatili_add_app app_name)
    add_executable(${app_name} apps/${app_name}.cpp)
    target_link_libraries(${app_name} PRIVATE marcatili_placeholder)
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
    )
endif()

