cmake_minimum_required(VERSION 2.8)

# Set gcc / g++ compiler to be able to use openMP 
SET(CMAKE_C_COMPILER "/usr/local/bin/gcc-5")
SET(CMAKE_CXX_COMPILER "/usr/local/bin/g++-5")

# Create project
SET(PROJECT_NAME global_illumination)
project(${PROJECT_NAME})

# Add the external module path
# set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake/Modules)

FIND_PACKAGE( OpenMP )
if(OPENMP_FOUND)
	message("OPENMP FOUND")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()

# Add external libraries
# GLM (only include, no source needed)
set(GLM_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/external_libraries/)
include_directories(${GLM_INCLUDE_DIRS})
include_directories(${OPENMP_INCLUDE_DIR})

# Our own include directory
set(COMMON_INCLUDES ${PROJECT_SOURCE_DIR}/include)
include_directories(${COMMON_INCLUDES})

# Source files
file(GLOB INTERNAL_SOURCE ${PROJECT_SOURCE_DIR}/src/*.cpp ${PROJECT_SOURCE_DIR}/external_libraries/common_src/*.cpp)
file(GLOB INTERNAL_HEADERS ${PROJECT_SOURCE_DIR}/include/*.h ${PROJECT_SOURCE_DIR}/external_libraries/common_include/*.h)

# Create the executable from our sources
add_executable(${PROJECT_NAME} ${INTERNAL_SOURCE} ${INTERNAL_HEADERS})

#set(COMPILE_FLAGS CMAKE_CXX_FLAGS CMAKE_C_FLAGS)

# C++11 compatability
set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "-std=c++11")

# Later link other libraries here
# ----- #