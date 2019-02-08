# Target: NeMo
# Type: Application
#############################
#Main CMake Project for NeMo2
#############################
cmake_minimum_required(VERSION 3.12.0 FATAL_ERROR)
include_directories(${ROSS_SOURCE_DIR} ${ROSS_BINARY_DIR})
include_directories(${EXTERNAL_INCLUDE})
message(" --- ROSS AT ${ROSS_SOURCE_DIR}")
        ## NEMO COMPILE-TIME CONFIG SETTINGS ##
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

## Configure options for global compile

## Include the TN Model JSON Parsing Library
## Include the NeMo library
## Set the main project

############### EXTERNAL LIBRARIES



