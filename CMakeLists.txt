cmake_minimum_required (VERSION 3.1)

# =====
# NeMo Configuration and defs
# =====

set (PROJECT_NAME "NeMo2")
set (PROJECT_DESCRIPTION "A Parallel discrete event simulation based model of neuromorphic hardware.")
set (PROJECT_AUTHOR "Mark Plagge")
set(PROJECT_VERSION_MAJOR "0")
set(PROJECT_VERSION_MINOR "0")
set(PROJECT_VERSION_PATCH "1")

project (${PROJECT_NAME} C CXX)


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

set (AUTO_INIT 1 CACHE BOOL "Automatic Git submodule init? Will break if no external
	network connection is available")

find_package(Git QUIET)
if(AUTO_INIT AND GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
# Update submodules as needed
    option(GIT_SUBMODULE "Check submodules during build" ON)
    option(USE_EXTERNAL_C++_JSON "Use external JSON C++ libraries")
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

set (AUTO_MPI 1 CACHE BOOL "Automatically configure MPI libs/headers?
	ALternatively, you need to set the C compiler to the location of
	mpicc, and the C++ compiler to mpicxx. ")
if(AUTO_MPI)
    find_package(MPI)
    add_library(mpi-l INTERFACE IMPORTED)
    set_property(TARGET mpi-l PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${MPI_C_INCLUDE_PATH})
    add_library(mpi SHARED IMPORTED)
    set_property(TARGET mpi PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${MPI_C_INCLUDE_PATH})
    set_property(TARGET mpi PROPERTY IMPORTED_LOCATION ${MPI_C_LIBRARIES})
    message("MPI Include Path: ${MPI_C_INCLUDE_PATH}")
    message("MPI INCLUDE CXX PATH: ${MPI_CXX_INCLUDE_PATH}")
    include_directories(${MPI_INCLUDE_PATH})
endif(AUTO_MPI)

### CMAKE SUBDIRS & SUB PROJECTS
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/external)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/external/ROSS)
set(EXTERNAL_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/external)
### MPI AUTOMATIC MODE
if(AUTO_MPI)
    #target_link_libraries(ROSS MPI_C)

endif()




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

