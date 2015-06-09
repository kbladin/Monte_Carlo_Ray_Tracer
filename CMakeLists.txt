cmake_minimum_required(VERSION 2.8)

# Create project
SET(PROJECT_NAME global_illumination)
project(${PROJECT_NAME})

#SET(CMAKE_C_COMPILER gcc)
#SET(CMAKE_CXX_COMPILER g++)

# Add the external module path
# set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake/Modules)

#find_package(OPENMP REQUIRED)

#find_package(OpenMP)
#if (OPENMP_FOUND)
#    set (CMAKE_C_FLAGS ${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS})
#    set (CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS})
#endif()

# Add external libraries
# GLM (only include, no source needed)
set(GLM_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/external_libraries/)
include_directories(${GLM_INCLUDE_DIRS})
include_directories(${OPENMP_INCLUDE_DIR})

# Our own include directory
set(COMMON_INCLUDES ${PROJECT_SOURCE_DIR}/include)
include_directories(${COMMON_INCLUDES})

# Source files
file(GLOB INTERNAL_SOURCE ${PROJECT_SOURCE_DIR}/src/*.cpp)
file(GLOB INTERNAL_HEADERS ${PROJECT_SOURCE_DIR}/include/*.h)

# Create the executable from our sources
add_executable(${PROJECT_NAME} ${INTERNAL_SOURCE} ${INTERNAL_HEADERS})

#set(COMPILE_FLAGS CMAKE_CXX_FLAGS CMAKE_C_FLAGS)

# C++11 compatability
set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "-std=c++11")

# Later link other libraries here
# ----- #