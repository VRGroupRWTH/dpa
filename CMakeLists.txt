##################################################    Project     ##################################################
cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
project               (dpa VERSION 1.0 LANGUAGES C CXX)
list                  (APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
set_property          (GLOBAL PROPERTY USE_FOLDERS ON)
set                   (CMAKE_CXX_STANDARD 17)
set                   (CMAKE_BUILD_TYPE Release)

include               (set_max_warning_level)
set_max_warning_level ()

##################################################    Options     ##################################################
option(BUILD_TESTS "Build tests." OFF)
option(DPA_FTLE_SUPPORT "Build with FTLE support (i.e. particle gathering and grid remapping)." ON)
option(DPA_SCOREP_SUPPORT "Build with SCORE-P support (i.e. performance benchmarking)." ON)

if   (DPA_FTLE_SUPPORT)
list (APPEND PROJECT_COMPILE_DEFINITIONS -DDPA_FTLE_SUPPORT)
endif()

if   (DPA_SCOREP_SUPPORT)
list (APPEND PROJECT_COMPILE_DEFINITIONS -DDPA_SCOREP_SUPPORT -DSCOREP_USER_ENABLE)
endif()

##################################################    Sources     ##################################################
file(GLOB_RECURSE PROJECT_HEADERS include/*.h include/*.hpp)
file(GLOB_RECURSE PROJECT_SOURCES source/*.c source/*.cpp)
file(GLOB_RECURSE PROJECT_CMAKE_UTILS cmake/*.cmake)
file(GLOB_RECURSE PROJECT_MISC *.md *.txt)
set (PROJECT_FILES 
  ${PROJECT_HEADERS} 
  ${PROJECT_SOURCES} 
  ${PROJECT_CMAKE_UTILS} 
  ${PROJECT_MISC})

include            (assign_source_group)
assign_source_group(${PROJECT_FILES})

##################################################  Dependencies  ##################################################
include(import_library)

find_package  (Boost REQUIRED date_time mpi regex)
import_library(Boost_INCLUDE_DIRS Boost_DATE_TIME_LIBRARY_DEBUG Boost_DATE_TIME_LIBRARY_RELEASE)
import_library(Boost_INCLUDE_DIRS Boost_MPI_LIBRARY_DEBUG Boost_MPI_LIBRARY_RELEASE)
import_library(Boost_INCLUDE_DIRS Boost_REGEX_LIBRARY_DEBUG Boost_REGEX_LIBRARY_RELEASE)
import_library(Boost_INCLUDE_DIRS Boost_SERIALIZATION_LIBRARY_DEBUG Boost_SERIALIZATION_LIBRARY_RELEASE)

find_package  (Catch2 REQUIRED)
list          (APPEND PROJECT_LIBRARIES Catch2::Catch2)

find_package  (Eigen3 3.3 REQUIRED NO_MODULE)
list          (APPEND PROJECT_LIBRARIES Eigen3::Eigen)

set           (HDF5_PREFER_PARALLEL TRUE)
find_package  (HDF5 REQUIRED)
import_library(HDF5_INCLUDE_DIRS HDF5_C_LIBRARIES)
list          (APPEND PROJECT_COMPILE_DEFINITIONS -DH5_USE_BOOST -DH5_BUILT_AS_DYNAMIC_LIB)

find_package  (MKL REQUIRED)
list          (APPEND PROJECT_LIBRARIES MKL::MKL)

find_package  (MPI REQUIRED)
import_library(MPI_C_INCLUDE_DIRS MPI_C_LIBRARIES)

find_package  (nlohmann_json 3.7.0 REQUIRED)
list          (APPEND PROJECT_LIBRARIES nlohmann_json::nlohmann_json)

find_package  (TBB REQUIRED)
list          (APPEND PROJECT_LIBRARIES TBB::tbb TBB::tbbmalloc)
  
if(UNIX)
  find_package(Threads REQUIRED)
  list        (APPEND PROJECT_LIBRARIES Threads::Threads)
endif()

if(DPA_SCOREP_SUPPORT)
  find_package  (ScoreP REQUIRED)
  import_library(SCOREP_INCLUDE_DIR)
endif()

# ADD LIBRARIES HERE. Vcpkg toolchain file will automatically locate them.
# Examples:
# - Header Only:
#   find_package  (GLM REQUIRED)
#   import_library(GLM_INCLUDE_DIRS)
# - Identical Debug and Release:
#   find_package  (OpenGL REQUIRED)
#   import_library(OPENGL_INCLUDE_DIR OPENGL_LIBRARIES)
# - Separate Debug and Release:
#   find_package  (Boost REQUIRED iostreams)
#   import_library(Boost_INCLUDE_DIRS Boost_IOSTREAMS_LIBRARY_DEBUG Boost_IOSTREAMS_LIBRARY_RELEASE)
# - You may also set the PROJECT_INCLUDE_DIRS and PROJECT_LIBRARIES instead of using import_library.

##################################################    Targets     ##################################################
add_executable(${PROJECT_NAME} ${PROJECT_FILES})
target_include_directories(${PROJECT_NAME} PUBLIC 
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
  $<INSTALL_INTERFACE:include> PRIVATE source)
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_INCLUDE_DIRS})
target_link_libraries     (${PROJECT_NAME} PUBLIC ${PROJECT_LIBRARIES})
target_compile_definitions(${PROJECT_NAME} PUBLIC ${PROJECT_COMPILE_DEFINITIONS})
set_target_properties     (${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)

if(NOT BUILD_SHARED_LIBS)
  string               (TOUPPER ${PROJECT_NAME} PROJECT_NAME_UPPER)
  set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS -D${PROJECT_NAME_UPPER}_STATIC)
endif()

##################################################    Testing     ##################################################
if(BUILD_TESTS)
  enable_testing     ()
  set                (TEST_MAIN_NAME catch_main)
  set                (TEST_MAIN_SOURCES tests/catch/main.cpp)
  add_library        (${TEST_MAIN_NAME} OBJECT ${TEST_MAIN_SOURCES})
  set_property       (TARGET ${TEST_MAIN_NAME} PROPERTY FOLDER tests/catch)
  assign_source_group(${TEST_MAIN_SOURCES})

  file(GLOB PROJECT_TEST_CPPS tests/*.cpp)
  foreach(_SOURCE ${PROJECT_TEST_CPPS})
    get_filename_component    (_NAME ${_SOURCE} NAME_WE)
    add_executable            (${_NAME} ${_SOURCE} $<TARGET_OBJECTS:${TEST_MAIN_NAME}>)
    target_include_directories(${_NAME} PUBLIC 
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
      $<INSTALL_INTERFACE:include> PRIVATE source)
    target_include_directories(${_NAME} PUBLIC ${PROJECT_INCLUDE_DIRS})
    target_link_libraries     (${_NAME} PUBLIC ${PROJECT_LIBRARIES})
    target_compile_definitions(${_NAME} PUBLIC ${PROJECT_COMPILE_DEFINITIONS})
    add_test                  (${_NAME} ${_NAME})
    set_property              (TARGET ${_NAME} PROPERTY FOLDER tests)
    assign_source_group       (${_SOURCE})
  endforeach()
endif()

##################################################  Installation  ##################################################
install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}-config
  RUNTIME DESTINATION bin)
install(DIRECTORY include/ DESTINATION include)
install(EXPORT  ${PROJECT_NAME}-config DESTINATION cmake)
export (TARGETS ${PROJECT_NAME}        FILE        ${PROJECT_NAME}-config.cmake)
