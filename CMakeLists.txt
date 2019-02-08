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
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake/Modules")
message(${CMAKE_MODULE_PATH})

# ==========================
# External Projects and Libraries
# ==========================

# ==========================
# IMPORTANT GLOBAL BUILD FLAGS 
# ========================

# ==========================
# NeMo Project
# ==========================

