cmake_minimum_required(VERSION 3.5)
project (GREMLINS)

#=== FINDING PACKAGES ===#

#--------------------------------
# This is for old cmake versions
set (CMAKE_CXX_STANDARD 11)
#--------------------------------

#=== SETTING VARIABLES ===#
# Compiling flags
set( GCC_COMPILE_FLAGS "-Wall" )
set( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_COMPILE_FLAGS}" )

#Include dir
include_directories( include )

# Define the sources
set(EXECUTABLE_OUTPUT_PATH "../bin")
add_executable(test_data_integrity src/test_data_integrity.cpp )
add_executable(test_list_integrity src/test_list_integrity.cpp )