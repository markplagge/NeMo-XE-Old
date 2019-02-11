cmake_minimum_required(VERSION 3.2)
project(nemo_tests)

add_subdirectory(./googletest)

include_directories(${NEMO_HEADERS_DIR})
include_directories(./googletest/googletest/include)

set(SOURCE_FILES main.cpp )

add_executable(nemo_tests ${SOURCE_FILES})
target_link_libraries(nemo_tests gtest)
install(TARGETS nemo_tests   DESTINATION bin)
