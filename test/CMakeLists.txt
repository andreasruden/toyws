cmake_minimum_required(VERSION 3.14)

project(toywsTests LANGUAGES CXX)

include(../cmake/project-is-top-level.cmake)
include(../cmake/folders.cmake)

# ---- Dependencies ----

if(PROJECT_IS_TOP_LEVEL)
  find_package(toyws REQUIRED)
  enable_testing()
endif()

find_package(Catch2 REQUIRED)
include(Catch)

# ---- Tests ----

add_executable(toyws_test source/toyws_test.cpp)
target_link_libraries(
    toyws_test PRIVATE
    toyws::toyws
    Catch2::Catch2WithMain
)
target_compile_features(toyws_test PRIVATE cxx_std_20)

catch_discover_tests(toyws_test)

# ---- End-of-file commands ----

add_folders(Test)
