cmake_minimum_required(VERSION 3.2)
project(nemo_tests)

add_subdirectory(./googletest)

include_directories(${NEMO_HEADERS_DIR})
include_directories(./googletest/googletest/include)

set(SOURCE_FILES core_test.cpp)

add_executable(nemo_tests ${SOURCE_FILES})
target_include_directories(nemo_tests PUBLIC "./googletest/googletest/include" PUBLIC "./googletest/googlemock/include")
target_link_libraries(nemo_tests gtest gmain)
install(TARGETS nemo_tests   DESTINATION bin)


## documentation srcs
string(REPLACE ";" " ${CMAKE_SOURCE_DIR}/test/" doxygen_src_files ";${SOURCE_FILES}" )
set_property(GLOBAL APPEND PROPERTY DOC_SRCS ${doxygen_src_files})