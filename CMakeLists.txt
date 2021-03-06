#
# Copyright (c) 2015-2019 CNRS INRIA
# Copyright (c) 2015 Wandercraft, 86 rue de Paris 91400 Orsay, France.
#

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

# Check if the submodule cmake have been initialized
IF(NOT EXISTS "${PROJECT_SOURCE_DIR}/cmake/base.cmake")
  MESSAGE(FATAL_ERROR "\nPlease run the following command first:\ngit submodule update --init\n")
ENDIF()

INCLUDE(cmake/base.cmake)
INCLUDE(cmake/boost.cmake)
INCLUDE(cmake/python.cmake)
INCLUDE(cmake/ide.cmake)
INCLUDE(cmake/test.cmake)

SET(PROJECT_NAME pinocchio)
SET(PROJECT_DESCRIPTION "Rigid multi body dynamics algorithms")
SET(PROJECT_URL "http://github.com/stack-of-tasks/pinocchio")
SET(PROJECT_CUSTOM_HEADER_EXTENSION "hpp")
OPTION(INSTALL_DOCUMENTATION "Generate and install the documentation" ON)

SET(DOXYGEN_USE_MATHJAX YES)

IF(APPLE)
  SET(CMAKE_MACOSX_RPATH TRUE)
  SET(CMAKE_SKIP_BUILD_RPATH  FALSE)
  SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
  SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

  set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
  list(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/lib" isSystemDir)
  if("${isSystemDir}" STREQUAL "-1")
     set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
  endif("${isSystemDir}" STREQUAL "-1")
ENDIF(APPLE)

# Disable -Werror on Unix for now.
SET(CXX_DISABLE_WERROR True)
SET(CMAKE_VERBOSE_MAKEFILE True)

SETUP_PROJECT()

IF(WIN32)
  SET(LINK copy_if_different)
ELSE(WIN32)
  SET(LINK create_symlink)
ENDIF(WIN32)

# --- OPTIONS ----------------------------------------
OPTION (BUILD_BENCHMARK "Build the benchmarks" OFF)
OPTION (BUILD_UNIT_TESTS "Build the unitary tests" ON)
OPTION (BUILD_UTILS "Build the utils" OFF)
OPTION (INITIALIZE_WITH_NAN "Initialize Eigen entries with NaN" OFF)
OPTION (BUILD_PYTHON_INTERFACE "Build the python binding" ON)
OPTION (BUILD_WITH_LUA_SUPPORT "Build the lua parser" OFF)
OPTION (BUILD_WITH_COMMIT_VERSION "Build libraries by setting specific commit version" OFF)

IF (INITIALIZE_WITH_NAN)
  MESSAGE (STATUS "Initialize with NaN all the Eigen entries.")
  ADD_DEFINITIONS(-DEIGEN_INITIALIZE_MATRICES_BY_NAN)
ENDIF (INITIALIZE_WITH_NAN)

IF(BUILD_UNIT_TESTS)
  SET(DISABLE_TESTS OFF)
ELSE(BUILD_UNIT_TESTS)
  SET(DISABLE_TESTS ON)
ENDIF(BUILD_UNIT_TESTS)

MACRO(TAG_LIBRARY_VERSION target)
  SET_TARGET_PROPERTIES(${target} PROPERTIES SOVERSION ${PROJECT_VERSION})
ENDMACRO(TAG_LIBRARY_VERSION)

# ----------------------------------------------------
# --- DEPENDENCIES -----------------------------------
# ----------------------------------------------------
ADD_REQUIRED_DEPENDENCY("eigen3 >= 3.0.5")
ADD_OPTIONAL_DEPENDENCY("urdfdom >= 0.2.0")

find_package(catkin QUIET COMPONENTS hpp-fcl)  # compatability with catkin-packaged hpp-fcl
ADD_OPTIONAL_DEPENDENCY("hpp-fcl >= 1.0.0")

ADD_OPTIONAL_DEPENDENCY("cppad >= 20180000.0")

IF(CPPAD_FOUND)
  ADD_DEFINITIONS(-DPINOCCHIO_WITH_CPPAD_SUPPORT)
  PKG_CONFIG_APPEND_CFLAGS("-DPINOCCHIO_WITH_CPPAD_SUPPORT")
  IF(NOT ${CPPAD_VERSION} VERSION_GREATER "3.3.0")
    ADD_DEFINITIONS(-DPINOCCHIO_CPPAD_REQUIRES_MATRIX_BASE_PLUGIN)
    PKG_CONFIG_APPEND_CFLAGS("-DPINOCCHIO_CPPAD_REQUIRES_MATRIX_BASE_PLUGIN")
  ENDIF(NOT ${CPPAD_VERSION} VERSION_GREATER "3.3.0")

  ADD_OPTIONAL_DEPENDENCY("cppadcg")
  IF(CPPADCG_FOUND)
    ADD_DEFINITIONS(-DPINOCCHIO_WITH_CPPADCG_SUPPORT)
    PKG_CONFIG_APPEND_CFLAGS("-DPINOCCHIO_WITH_CPPADCG_SUPPORT")
  ENDIF(CPPADCG_FOUND)
ENDIF(CPPAD_FOUND)

# Special care of urdfdom version
IF(URDFDOM_FOUND)
  IF(${URDFDOM_VERSION} VERSION_LESS "0.3.0")
    ADD_DEFINITIONS(-DPINOCCHIO_URDFDOM_COLLISION_WITH_GROUP_NAME)
    PKG_CONFIG_APPEND_CFLAGS("-DPINOCCHIO_URDFDOM_COLLISION_WITH_GROUP_NAME")
  ENDIF(${URDFDOM_VERSION} VERSION_LESS "0.3.0")

  # defines types from version 0.4.0
  IF(NOT ${URDFDOM_VERSION} VERSION_LESS "0.4.0")
    ADD_DEFINITIONS(-DPINOCCHIO_URDFDOM_TYPEDEF_SHARED_PTR)
    PKG_CONFIG_APPEND_CFLAGS("-DPINOCCHIO_URDFDOM_TYPEDEF_SHARED_PTR")
  ENDIF(NOT ${URDFDOM_VERSION} VERSION_LESS "0.4.0")
  
  # std::shared_ptr appears from version 1.0.0
  IF(${URDFDOM_VERSION} VERSION_GREATER "0.4.2")
    ADD_DEFINITIONS(-DPINOCCHIO_URDFDOM_USE_STD_SHARED_PTR)
    PKG_CONFIG_APPEND_CFLAGS("-DPINOCCHIO_URDFDOM_USE_STD_SHARED_PTR")
  ENDIF(${URDFDOM_VERSION} VERSION_GREATER "0.4.2")
ENDIF(URDFDOM_FOUND)

# Special care of lua which can be of versions 5.1 or 5.2
# Priority is set to lua5.2
IF(BUILD_WITH_LUA_SUPPORT)
  ADD_OPTIONAL_DEPENDENCY("lua5.2")
  IF(LUA5_2_FOUND)
    SET(LUA5_FOUND TRUE)
    SET(LUA5_PACKAGE "lua5.2")
    ADD_DEFINITIONS(-DPINOCCHIO_LUA_VERSION_GREATER_5_2)
    PKG_CONFIG_APPEND_CFLAGS("-DPINOCCHIO_LUA_VERSION_GREATER_5_2")
  ELSE(LUA5_2_FOUND)
    ADD_OPTIONAL_DEPENDENCY("lua5.1")
    IF(LUA5_1_FOUND)
      SET(LUA5_FOUND TRUE)
      SET(LUA5_PACKAGE "lua5.1")
    ELSE(LUA5_1_FOUND)
      MESSAGE(FATAL_ERROR "Cannot find a suitable version of Lua 5.1 or 5.2")
    ENDIF(LUA5_1_FOUND)
  ENDIF(LUA5_2_FOUND)
ENDIF(BUILD_WITH_LUA_SUPPORT)

SET(BOOST_REQUIERED_COMPONENTS filesystem system)
SET(BOOST_BUILD_COMPONENTS unit_test_framework)
SET(BOOST_OPTIONAL_COMPONENTS "")

IF(BUILD_PYTHON_INTERFACE)
  SET(BOOST_OPTIONAL_COMPONENTS ${BOOST_OPTIONAL_COMPONENTS} python)
  FINDPYTHON()
  INCLUDE_DIRECTORIES(SYSTEM ${PYTHON_INCLUDE_DIRS}) 
ENDIF(BUILD_PYTHON_INTERFACE)

SET(BOOST_COMPONENTS ${BOOST_REQUIERED_COMPONENTS} ${BOOST_OPTIONAL_COMPONENTS} ${BOOST_BUILD_COMPONENTS})
SEARCH_FOR_BOOST()

# Variable containing all the cflags definition relative to optional dependencies
SET(CFLAGS_DEPENDENCIES)

# Path to boost headers
INCLUDE_DIRECTORIES(SYSTEM ${Boost_INCLUDE_DIRS})

# ----------------------------------------------------
# --- INCLUDE ----------------------------------------
# ----------------------------------------------------
FILE(GLOB_RECURSE HEADERS 
     ${PROJECT_SOURCE_DIR}/src/*.hpp 
     ${PROJECT_SOURCE_DIR}/src/*.hxx 
     ) 

IF(URDFDOM_FOUND)
  ADD_DEFINITIONS(-DPINOCCHIO_WITH_URDFDOM)
  LIST(APPEND CFLAGS_DEPENDENCIES "-DPINOCCHIO_WITH_URDFDOM")
ELSE(URDFDOM_FOUND)
  LIST(REMOVE_ITEM HEADERS
    ${PROJECT_SOURCE_DIR}/src/parsers/urdf.hpp
    ${PROJECT_SOURCE_DIR}/src/parsers/urdf/model.hxx
    ${PROJECT_SOURCE_DIR}/src/parsers/urdf/geometry.hxx
    ${PROJECT_SOURCE_DIR}/src/parsers/urdf/utils.hpp
    ${PROJECT_SOURCE_DIR}/src/parsers/urdf/types.hpp
    )
ENDIF(URDFDOM_FOUND)


IF(HPP_FCL_FOUND)
  ADD_DEFINITIONS(-DPINOCCHIO_WITH_HPP_FCL)
  LIST(APPEND CFLAGS_DEPENDENCIES "-DPINOCCHIO_WITH_HPP_FCL")
ELSE(HPP_FCL_FOUND)
  LIST(REMOVE_ITEM HEADERS 
    ${PROJECT_SOURCE_DIR}/src/spatial/fcl-pinocchio-conversions.hpp
    )
ENDIF(HPP_FCL_FOUND)

IF(LUA5_FOUND)
  ADD_DEFINITIONS(-DPINOCCHIO_WITH_LUA5)
  LIST(APPEND CFLAGS_DEPENDENCIES "-DPINOCCHIO_WITH_LUA5")
ELSE(LUA5_FOUND)
  LIST(REMOVE_ITEM HEADERS
    ${PROJECT_SOURCE_DIR}/src/parsers/lua.hpp
    ${PROJECT_SOURCE_DIR}/src/parsers/lua/lua_tables.hpp
    )
ENDIF(LUA5_FOUND)


LIST(APPEND HEADERS
     exception.hpp
     macros.hpp
     )

MAKE_DIRECTORY("${${PROJECT_NAME}_BINARY_DIR}/include/pinocchio")
MAKE_DIRECTORY("${${PROJECT_NAME}_BINARY_DIR}/include/pinocchio/math")
MAKE_DIRECTORY("${${PROJECT_NAME}_BINARY_DIR}/include/pinocchio/spatial")
MAKE_DIRECTORY("${${PROJECT_NAME}_BINARY_DIR}/include/pinocchio/multibody")
MAKE_DIRECTORY("${${PROJECT_NAME}_BINARY_DIR}/include/pinocchio/multibody/joint")
MAKE_DIRECTORY("${${PROJECT_NAME}_BINARY_DIR}/include/pinocchio/multibody/liegroup")
MAKE_DIRECTORY("${${PROJECT_NAME}_BINARY_DIR}/include/pinocchio/parsers/lua")
MAKE_DIRECTORY("${${PROJECT_NAME}_BINARY_DIR}/include/pinocchio/parsers")
MAKE_DIRECTORY("${${PROJECT_NAME}_BINARY_DIR}/include/pinocchio/parsers/urdf")
MAKE_DIRECTORY("${${PROJECT_NAME}_BINARY_DIR}/include/pinocchio/utils")
MAKE_DIRECTORY("${${PROJECT_NAME}_BINARY_DIR}/include/pinocchio/algorithm")
MAKE_DIRECTORY("${${PROJECT_NAME}_BINARY_DIR}/include/pinocchio/container")
MAKE_DIRECTORY("${${PROJECT_NAME}_BINARY_DIR}/include/pinocchio/codegen")
SET(HEADERS_)
FOREACH(header ${HEADERS})
  STRING(REGEX REPLACE "${PROJECT_SOURCE_DIR}/src/" "" header ${header})
  LIST(APPEND HEADERS_ ${header})
  GET_FILENAME_COMPONENT(headerName ${header} NAME)
  GET_FILENAME_COMPONENT(headerPath ${header} PATH)
  EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E ${LINK}
    ${${PROJECT_NAME}_SOURCE_DIR}/src/${header}
    ${${PROJECT_NAME}_BINARY_DIR}/include/${PROJECT_NAME}/${header})
  INSTALL(FILES ${${PROJECT_NAME}_SOURCE_DIR}/src/${header}
    DESTINATION ${CMAKE_INSTALL_PREFIX}/include/${PROJECT_NAME}/${headerPath}
          PERMISSIONS OWNER_READ GROUP_READ WORLD_READ OWNER_WRITE)
ENDFOREACH(header)
SET(HEADERS ${HEADERS_})

# --- MAIN LIBRARY -------------------------------------------------------------
ADD_SUBDIRECTORY(src)

# --- BINDINGS -----------------------------------------------------------------
ADD_SUBDIRECTORY(bindings)

# --- EXECUTABLES --------------------------------------------------------------
ADD_SUBDIRECTORY(utils)

# --- UNIT TESTS ---------------------------------------------------------------
ADD_SUBDIRECTORY(unittest)

# --- CHECK EXAMPLES -----------------------------------------------------------
ADD_SUBDIRECTORY(examples)

# --- BENCHMARKS ---------------------------------------------------------------
ADD_SUBDIRECTORY(benchmark)

# --- PACKAGING ----------------------------------------------------------------
PKG_CONFIG_APPEND_LIBS (${PROJECT_NAME})
PKG_CONFIG_APPEND_BOOST_LIBS(${BOOST_REQUIERED_COMPONENTS})

FOREACH(cflags ${CFLAGS_DEPENDENCIES})
  PKG_CONFIG_APPEND_CFLAGS(${cflags})
ENDFOREACH(cflags ${CFLAGS_DEPENDENCIES})

SETUP_PROJECT_FINALIZE()
