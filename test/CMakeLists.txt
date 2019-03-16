cmake_minimum_required(VERSION 3.2)
project(nemo_tests)

add_subdirectory(./googletest)

include_directories(${NEMO_HEADERS_DIR})
include_directories(./googletest/googletest/include)
include_directories(../external/ROSS)
include_directories(${ROSS_SOURCE_DIR} ${ROSS_BINARY_DIR})
include_directories(${CMAKE_BINARY_DIR}/external/ROSS/core)
include_directories(../external/ROSS/core)

set(SOURCE_FILES core_test.cpp TrueNorthCoreLogger.h TrueNorthCoreLogger.cpp writer_test.cpp
        LIFCore_test.cpp INeuroCoreBase_test.cpp ross_testing_wrapper.cpp ross_testing_wrapper.h globals_test.cpp )

add_executable(nemo_tests ${SOURCE_FILES})
target_include_directories(nemo_tests PUBLIC "./googletest/googletest/include" PUBLIC "./googletest/googlemock/include" ${ROSS_SOURCE_DIR} ${ROSS_BINARY_DIR})
target_link_libraries(nemo_tests gtest gtest_main nemo2_lib)
install(TARGETS nemo_tests   DESTINATION bin)


## documentation srcs
string(REPLACE ";" " ${CMAKE_SOURCE_DIR}/test/" doxygen_src_files ";${SOURCE_FILES}" )
set_property(GLOBAL APPEND PROPERTY DOC_SRCS ${doxygen_src_files})