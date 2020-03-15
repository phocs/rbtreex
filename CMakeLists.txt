cmake_minimum_required(VERSION 3.12)

# set the project name and version
project(
    rbtreex
	VERSION 0.0.1
	DESCRIPTION "Subset of the Linux 's red-black tree implementation."
	HOMEPAGE_URL "https://github.com/phocs/rbtreex.git"
	LANGUAGES C
)

# add the main library
add_library(
    ${PROJECT_NAME}
    rbtreex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/rbtreex.h
)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_include_directories(
    ${PROJECT_NAME}
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
)

# install rules
install(TARGETS ${PROJECT_NAME} DESTINATION lib)
install(FILES rbtreex.h DESTINATION include)

# uninstall target
if(NOT TARGET uninstall)
  configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

  add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
endif()
