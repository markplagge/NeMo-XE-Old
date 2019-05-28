# Target: NeMo
# Type: Application
#############################
# Main CMake Project for NeMo2 #
#############################
cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

include_directories(${ROSS_SOURCE_DIR} ${ROSS_BINARY_DIR})

message(STATUS "${EXTERNAL_INCLUDE} -- ${RAPIDJSON_INCLUDE_DIR}")
#get_target_property(ModelReader RAPIDJSON_INCLUDE_DIR RAPIDJSON_INCLUDE_DIR)
include_directories(${EXTERNAL_INCLUDE} ${EXTERNAL_INCLUDE}/include ${RAPIDJSON_INCLUDE_DIR})
## Add script to handle generation of config header for development
# uses python

#add_custom_command(OUTPUT $CMAKE_CURRENT_SOURCE_DIR/test.h
#        COMMAND python3 $CMAKE_CURRENT_SOURCE_DIR/nemo_build_options.h.in
#        COMMENT "Updating in-source build options file")
INCLUDE(FindPython3)

find_package(RapidJSON REQUIRED)

set(CF_ARGS "${CMAKE_CURRENT_SOURCE_DIR}/gen_build_opts.py ${CMAKE_CURRENT_SOURCE_DIR}  test_out.h")


EXECUTE_PROCESS ( COMMAND ${Python3_EXECUTABLE} -c "from distutils.sysconfig import get_python_inc; print(get_python_inc())" OUTPUT_VARIABLE PYTHON_INCLUDE_DIR OUTPUT_STRIP_TRAILING_WHITESPACE RESULT_VARIABLE PY_RES)

MESSAGE(STATUS "Python interpreter: ${Python3_EXECUTABLE}")
MESSAGE(STATUS "Python interpreter verison: ${PYTHON_VERSION}")
MESSAGE(STATUS "Python result ${PY_RES}")


execute_process(
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/gen_build_opts.py ${CMAKE_CURRENT_SOURCE_DIR} nemo_build_options.h
        #WORKING_DIRECTORY {$CMAKE_CURRENT_SOURCE_DIR}
        OUTPUT_VARIABLE CONFIG_OUT
        RESULT_VARIABLE CONFIG_RES
)
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


# Special Config options for OMP offload tests

#primary compile-time options
set(NEMO2_TN_NEURONS_PER_CORE 512 CACHE STRING "Number of TrueNorth neurons per TN core")
set(NEMO2_TN_WEIGHTS_PER_NERUON 4 "Number of TrueNorth weights each neuron can have")
set(NEMO2_TN_MAX_NEURON_OUTPUS 1 "Number of connections a truenorth neuron can make")
option(NEMO2_THREADED_FILE_OUTPUT "Use threading for file IO. Otherwise will use built-in ROSS data handling." ON)
set(NEMO2_LIF_NEURONS_PER_CORE 256 "Number of LIF neurons per LIF core")
set(NEMO2_LIF_NUM_OUTPUTS_PER_NEURON 256 "Number of connections a LIf neuron can make")

configure_file("nemo_build_options_template.in" "nemo_build_options.h")

## Set up the main project
set(nemo_lib_srcs include/globals.h globals.cpp
        neuro/INeuroCoreBase.h neuro/INeuroCoreBase.cpp
        neuro/TrueNorthCore.h neuro/TrueNorthCore.cpp
        include/mapping.h mapping.cpp
        io/CoreOutput.cpp
        include/CoreOutput.h
        ../external/include/atomicops.h
        ../external/include/concurrentqueue.h
        ../external/include/blockingconcurrentqueue.h
        neuro/LIFCore.cpp neuro/LIFCore.h
        neuro/CoreLP.cpp include/CoreLP.h  include/nemo_input.h io/tn_parser_wrapper.cpp
        include/NeMoXe.h info_display.h nemo_build_options.h)
set(nemo_app_srcs main.cpp include/globals.h globals.cpp info_display.cpp)

## Add required libraries

add_library(nemo2_lib STATIC ${nemo_lib_srcs})
include_directories(${CMAKE_SOURCE_DIR}/external/model_reader ${CMAKE_SOURCE_DIR}/external/model_reader/include)

add_executable(nemo2 ${nemo_app_srcs})




##Main NeMo2_lib Required Libraries: ########3
list(APPEND LBS  ROSS ModelReaderLib  )
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
target_include_directories(nemo2 PUBLIC ${CMAKE_SOURCE_DIR}/external/model_reader/include)
#target_link_libraries(nemo2 PRIVATE OpenMP::OpenMP_CXX)
#target_include_directories(nemo2_lib PUBLIC ModelReaderLib)
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



