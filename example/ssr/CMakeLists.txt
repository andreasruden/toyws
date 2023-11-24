cmake_minimum_required(VERSION 3.14)

project(toywsSsrExample LANGUAGES CXX)

include(../../cmake/project-is-top-level.cmake)
include(../../cmake/folders.cmake)

# ---- Dependencies ----

if(PROJECT_IS_TOP_LEVEL)
  find_package(toyws REQUIRED)
endif()

find_package(fmt REQUIRED)

# ---- Build Instructions ----

add_executable(toyws_ssr main.cpp)
target_link_libraries(
    toyws_ssr PRIVATE
    toyws::toyws
    fmt::fmt
)
target_compile_features(toyws_ssr PRIVATE cxx_std_20)
