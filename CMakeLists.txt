cmake_minimum_required(VERSION 3.14)

include(cmake/prelude.cmake)

project(
    toyws
    VERSION 0.1.0
    DESCRIPTION "A toy webserver written in std C++."
    HOMEPAGE_URL "https://github.com/andreasruden/toyws"
    LANGUAGES CXX
)

include(cmake/project-is-top-level.cmake)
include(cmake/variables.cmake)

# ---- Declare library ----

add_library(
    toyws_toyws
    source/http_io.cpp
    source/client_pool.cpp
    source/test_client.cpp
    source/toyws.cpp
)
add_library(toyws::toyws ALIAS toyws_toyws)

include(GenerateExportHeader)
generate_export_header(
    toyws_toyws
    BASE_NAME toyws
    EXPORT_FILE_NAME export/toyws/toyws_export.hpp
    CUSTOM_CONTENT_FROM_VARIABLE pragma_suppress_c4251
)

if(NOT BUILD_SHARED_LIBS)
  target_compile_definitions(toyws_toyws PUBLIC TOYWS_STATIC_DEFINE)
endif()

set_target_properties(
    toyws_toyws PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN YES
    VERSION "${PROJECT_VERSION}"
    SOVERSION "${PROJECT_VERSION_MAJOR}"
    EXPORT_NAME toyws
    OUTPUT_NAME toyws
)

target_include_directories(
    toyws_toyws ${warning_guard}
    PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
)

target_include_directories(
    toyws_toyws ${warning_guard}
    PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/source>"
)

target_include_directories(
    toyws_toyws SYSTEM
    PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/export>"
)

target_compile_features(toyws_toyws PUBLIC cxx_std_20)

find_package(fmt REQUIRED)
find_package(liburing REQUIRED)
target_link_libraries(toyws_toyws PRIVATE fmt::fmt)
target_link_libraries(toyws_toyws PUBLIC liburing::liburing)

# ---- Examples ----
# TODO: Only include if examples are "requested"
add_subdirectory(example/ssr)

# ---- Install rules ----

if(NOT CMAKE_SKIP_INSTALL_RULES)
  include(cmake/install-rules.cmake)
endif()

# ---- Developer mode ----

if(NOT toyws_DEVELOPER_MODE)
  return()
elseif(NOT PROJECT_IS_TOP_LEVEL)
  message(
      AUTHOR_WARNING
      "Developer mode is intended for developers of toyws"
  )
endif()

include(cmake/dev-mode.cmake)
