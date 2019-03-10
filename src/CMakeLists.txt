# Target: NeMo
# Type: Application
#############################
# Main CMake Project for NeMo2 #
#############################
cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

include_directories(${ROSS_SOURCE_DIR} ${ROSS_BINARY_DIR})

message(${EXTERNAL_INCLUDE})


include_directories(${EXTERNAL_INCLUDE} ${EXTERNAL_INCLUDE}/include)
message(" --- ROSS AT ${ROSS_SOURCE_DIR}")
## NEMO COMPILE-TIME CONFIG SETTINGS ##
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

## Configure options for global compile / OPEN MP
find_package(OpenMP)
set(NEMO_USE_OPENMP 1 CACHE BOOL "Use OpenMP acceleration for NeMo?")
if(NEMO_USE_OPENMP)
    set(NEMO_USE_GPU 0 CACHE BOOL "Use GPU offloading if available for NeMo?")
endif()

## Configure RapidJSON
set(RAPIDJSON_USE_SSE2 1 CACHE BOOL "Use SSE2 for RapidJSON")
set(RAPIDJSON_USE_SSE4 1 CACHE BOOL "Use SSE4 for RapidJSON")
find_package(RapidJSON)
include_directories(${RAPIDJSON_INCLUDE_DIRS})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${RAPIDJSON_CXX_FLAGS}")
# Special Config options for OMP offload tests


## Include the TN Model JSON Parsing Library

## Set up the main project
set(nemo_lib_srcs include/globals.h globals.cpp neuro/core.cpp neuro/core.h neuro/neuron_generic.cpp
        neuro/neuron_generic.h  mapping.h mapping.cpp io/CoreOutput.cpp include/CoreOutput.h ../external/include/atomicops.h
        ../external/include/readerwriterqueue.h)
set(nemo_app_srcs main.cpp include/globals.h globals.cpp)

## Add required libraries

add_library(nemo2_lib STATIC ${nemo_lib_srcs})
add_executable(nemo2 ${nemo_app_srcs})
##Main NeMo2_lib Required Libraries: ########3
list(APPEND LBS  ROSS)
target_compile_features(nemo2_lib PRIVATE cxx_std_14)
target_compile_features(nemo2 PRIVATE cxx_std_14)

#if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
#    list(APPEND LBS libc++)
#    message(${EXTERNAL_INCLUDE})
#
#    #list(APPEND EXTERNAL_INCLUDE "/Library/Developer/CommandLineTools/usr/include/c++/v1/")
#        include_directories("/Library/Developer/CommandLineTools/usr/include/c++/v1/")
#
#    message(${EXTERNAL_INCLUDE})
#endif()

#Add OpenMP if used ######3
if(OpenMP_CXX_FOUND AND NEMO_USE_OPENMP)
    message("Found and using OpenMP")
    list(APPEND LBS "OpenMP::OpenMP_CXX")
endif()


target_link_libraries(nemo2_lib ${LBS})
target_link_libraries(nemo2 ROSS nemo2_lib)
#target_link_libraries(nemo2 PRIVATE OpenMP::OpenMP_CXX)
target_include_directories(nemo2 PUBLIC ${EXTERNAL_INCLUDE} ${EXTERNAL_INCLUDE}/ROSS})

## configure the GPU Offload options
if(NEMO_USE_GPU AND OpenMP_CXX_FOUND)
    message("using openMP target offload code")
    set(NE_CUDA_ROOT "/opt/cuda" CACHE PATH "Path to CUDA SDK")
    set(NE_OMP_COMPILER "gcc" CACHE STRING "Compiler used for OMP offload")

    set_property(CACHE NE_OMP_COMPILER PROPERTY STRINGS clang XLC gcc)
    if(NE_OMP_COMPILER MATCHES clang)
        set(NE_STDLIB "libc++" CACHE STRING "Which STDLIB to use?")
        set_property(CACHE NE_STDLIB PROPERTY STRINGS libc++ libstdc++)
        message("Clang compiler chosen.")
        target_compile_options(nemo2_lib PUBLIC -fopenmp -fopenmp-targets=nvptx64-nvidia-cuda --cuda-path=${NE_CUDA_ROOT} -stdlib=${NE_STDLIB})
        #target_compile_options(nemo2_lib PUBLIC )
        #target_link_directories(nemo2_lib PUBLIC ${NE_CUDA_ROOT}/lib )
        #target_link_options(nemo2_lib PUBLIC -lm -L/usr/local/lib -L${NE_CUDA_ROOT}/lib)

        #target_link_options(nemo2_lib PUBLIC -L${NE_CUDA_ROOT}/lib )
    elseif(NE_OMP_COMPILER MATCHES gcc)
        message("GCC")

    else()
        message("IBM XL compiler chosen.")
    endif()
else()
    message("Not using OpenMP for build.")
endif()



## DOXYGEN
list(APPEND full_src ${nemo_lib_srcs})
list(APPEND full_src ${nemo_app_srcs})
#message(${nemo_lib_srcs})
#message("${nemo_app_srcs}")
message("dox setup")
string(REPLACE ";" " ${CMAKE_SOURCE_DIR}/src/"  doxygen_src_files ";${full_src}")
set_property(GLOBAL APPEND PROPERTY DOC_SRCS "${doxygen_src_files}")
set(nemo_doc_srcs ${doxygen_src_files} PARENT_SCOPE)
############### EXTERNAL LIBRARIES



