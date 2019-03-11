cmake_minimum_required (VERSION 3.10)
#### NeMo2 Main Project ######
# =====
# NeMo Configuration and defs
# =====

set (PROJECT_NAME "NeMo2")
set (PROJECT_DESCRIPTION "A Parallel discrete event simulation based model of neuromorphic hardware.")
set (PROJECT_AUTHOR "Mark Plagge")
set(CMAKE_CXX_STANDARD 14)

project (${PROJECT_NAME} VERSION 0.1.0 LANGUAGES CXX C)

# ===========================
# CMake Modules and required packages config
# ===========================
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
message(${CMAKE_MODULE_PATH})
include(CTest)


# ==========================
# External Projects and Libraries
# ==========================


## Set up submodules & Dependencies
option(USE_EXTERNAL_C++_JSON "Use external JSON C++ libraries" OFF)

set (AUTO_INIT 0 CACHE BOOL "Automatic Git submodule init? Will break if no external
	network connection is available")
set (AUTO_MPI 1 CACHE BOOL "Automatically configure MPI libs/headers?
	ALternatively, you need to set the C compiler to the location of
	mpicc, and the C++ compiler to mpicxx. ")
#if(AUTO_MPI)
#    find_package(MPI QUIET)
#    add_library(mpi-l INTERFACE IMPORTED)
#    set_property(TARGET mpi-l PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${MPI_C_INCLUDE_PATH})
#    add_library(mpi SHARED IMPORTED)
#    set_property(TARGET mpi PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${MPI_C_INCLUDE_PATH})
#    set_property(TARGET mpi PROPERTY IMPORTED_LOCATION ${MPI_C_LIBRARIES})
#    message("MPI Include Path: ${MPI_C_INCLUDE_PATH}")
#    message("MPI INCLUDE CXX PATH: ${MPI_CXX_INCLUDE_PATH}")
#    include_directories(${MPI_INCLUDE_PATH})
#
#    include_directories(SYSTEM ${MPI_INCLUDE_PATH})
#    link_libraries(mpi)
#    #target_link_libraries(ROSS MPI_C)
#    #Modern MPI
#    #target_link_libraries(main PRIVATE MPI_C)
#
#endif(AUTO_MPI)
find_package(Git QUIET)
if(AUTO_INIT AND GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
# Update submodules as needed
    option(GIT_SUBMODULE "Check submodules during build" OFF)

    if(GIT_SUBMODULE)
        message(STATUS "Submodule update")
        execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
                        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                        RESULT_VARIABLE GIT_SUBMOD_RESULT)
        if(NOT GIT_SUBMOD_RESULT EQUAL "0")
            message(FATAL_ERROR "git submodule update --init failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
        endif()
    endif()
else(message(WARNING "CMAKE was unable to find GIT - please run git submodule up
date --init --recursive in root NeMo folder."))
endif()

### ROSS SUBMODULE ###

if(NOT EXISTS "${PROJECT_SOURCE_DIR}/external/ROSS/CMakeLists.txt")
    message(FATAL_ERROR "The submodules were not downloaded! GIT_SUBMODULE was
    turned off or failed. Please update submodules and try again. You may also download ROSS directly into /external/ROSS,
    or ln -s ~PATH_TO_ROSS~ ~NEMO_ROOT_FOLDER/external/ROSS")
endif()

### GoogleTest SUBMODULE ###
if(BUILD_TESTING)
    add_subdirectory(test)
endif(BUILD_TESTING)



### CMAKE SUBDIRS & SUB PROJECTS
## External Libs
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/external)
## ROSS Libs
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/external/ROSS)
set_target_properties(ROSS PROPERTIES ROSS_timing OFF)

set(ROSS_timing OFF CACHE BOOL "Disable ROSS Timing" FORCE6)
set(EXTERNAL_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/external)





# ==========================
# IMPORTANT GLOBAL BUILD FLAGS 
# ========================

# ==========================
# NeMo Project
# ==========================
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src)

# ==========================
# Doxygen Config
# ==========================
# @TODO: Put this in ./doc/CMakeLists.txt
get_property(doxygen_src_files GLOBAL PROPERTY DOC_SRCS)
string(REPLACE ";" " " doxygen_src_files ${doxygen_src_files})
message(${doxygen_src_files})
# first we can indicate the documentation build as an option and set it to ON by default
option(BUILD_DOC "Build documentation" ON)
if(BUILD_DOC)

    set(USE_MATHJAX "YES" CACHE  STRING "Use MathJax in documentation")
    set(MATHJAX_FORMAT "NativeMML" CACHE STRING "MathJax format")
        set(SORT_GROUP_NAMES "YES")
    set(CLANG_ASSISTED_PARSNG 1)
    # check if Doxygen is installed
    find_package(Doxygen)
    if (DOXYGEN_FOUND)
        # set input and output files
        set(DOXYGEN_IN ${CMAKE_SOURCE_DIR}/docs/Doxyfile.in)
        set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/docs/Doxyfile)

        # request to configure the file
        configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
        message("Doxygen build started")

        # note the option ALL which allows to build the docs together with the application
        add_custom_target( doc_doxygen ALL
                COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
                WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                COMMENT "Generating documentation with Doxygen"
                VERBATIM )
    else (DOXYGEN_FOUND)
        message("Doxygen need to be installed to generate the doxygen documentation")
    endif (DOXYGEN_FOUND)

endif()
