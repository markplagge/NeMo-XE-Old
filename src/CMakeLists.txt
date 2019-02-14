# Target: NeMo
# Type: Application
#############################
# Main CMake Project for NeMo2 #
#############################
cmake_minimum_required(VERSION 3.12.0 FATAL_ERROR)
include_directories(${ROSS_SOURCE_DIR} ${ROSS_BINARY_DIR})
include_directories(${EXTERNAL_INCLUDE})
message(" --- ROSS AT ${ROSS_SOURCE_DIR}")
## NEMO COMPILE-TIME CONFIG SETTINGS ##
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

## Configure options for global compile
find_package(OpenMP)

# Special Config options for OMP offload tests


## Include the TN Model JSON Parsing Library

## Set up the main project
set(nemo_lib_srcs globals.h globals.cpp neuro/core.cpp neuro/core.h neuro/neuron_generic.cpp neuro/neuron_generic.h)
set(nemo_app_srcs main.cpp globals.h globals.cpp)

## Add required libraries

add_library(nemo2_lib STATIC ${nemo_lib_srcs})
add_executable(nemo2 ${nemo_app_srcs})
target_link_libraries(nemo2_lib ROSS OpenMP::OpenMP_CXX)

target_link_libraries(nemo2 ROSS nemo2_lib)
#target_link_libraries(nemo2 PRIVATE OpenMP::OpenMP_CXX)

target_include_directories(nemo2 PUBLIC ${EXTERNAL_INCLUDE} ${EXTERNAL_INCLUDE}/ROSS})

## DOXYGEN
list(APPEND full_src ${nemo_lib_srcs} ${nemo_app_srcs})
message("${nemo_app_srcs}")
string(REPLACE ";" " ${CMAKE_SOURCE_DIR}/src/"  doxygen_src_files ";${nemo_app_srcs}")
set_property(GLOBAL APPEND PROPERTY DOC_SRCS "${doxygen_src_files}")
set(nemo_doc_srcs ${doxygen_src_files} PARENT_SCOPE)
############### EXTERNAL LIBRARIES



