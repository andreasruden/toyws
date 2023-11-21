if(PROJECT_IS_TOP_LEVEL)
  set(
      CMAKE_INSTALL_INCLUDEDIR "include/toyws-${PROJECT_VERSION}"
      CACHE STRING ""
  )
  set_property(CACHE CMAKE_INSTALL_INCLUDEDIR PROPERTY TYPE PATH)
endif()

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

# find_package(<package>) call for consumers to find this project
set(package toyws)

install(
    DIRECTORY
    include/
    "${PROJECT_BINARY_DIR}/export/"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
    COMPONENT toyws_Development
)

install(
    TARGETS toyws_toyws
    EXPORT toywsTargets
    RUNTIME #
    COMPONENT toyws_Runtime
    LIBRARY #
    COMPONENT toyws_Runtime
    NAMELINK_COMPONENT toyws_Development
    ARCHIVE #
    COMPONENT toyws_Development
    INCLUDES #
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

write_basic_package_version_file(
    "${package}ConfigVersion.cmake"
    COMPATIBILITY SameMajorVersion
)

# Allow package maintainers to freely override the path for the configs
set(
    toyws_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/${package}"
    CACHE STRING "CMake package config location relative to the install prefix"
)
set_property(CACHE toyws_INSTALL_CMAKEDIR PROPERTY TYPE PATH)
mark_as_advanced(toyws_INSTALL_CMAKEDIR)

install(
    FILES cmake/install-config.cmake
    DESTINATION "${toyws_INSTALL_CMAKEDIR}"
    RENAME "${package}Config.cmake"
    COMPONENT toyws_Development
)

install(
    FILES "${PROJECT_BINARY_DIR}/${package}ConfigVersion.cmake"
    DESTINATION "${toyws_INSTALL_CMAKEDIR}"
    COMPONENT toyws_Development
)

install(
    EXPORT toywsTargets
    NAMESPACE toyws::
    DESTINATION "${toyws_INSTALL_CMAKEDIR}"
    COMPONENT toyws_Development
)

if(PROJECT_IS_TOP_LEVEL)
  include(CPack)
endif()
