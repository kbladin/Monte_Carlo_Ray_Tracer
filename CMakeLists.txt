cmake_minimum_required(VERSION 2.8)

# Create project
SET(PROJECT_NAME global_illumination)
project(${PROJECT_NAME})

# Add the external module path
# set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake/Modules)

# Add external libraries
# GLM (only include, no source needed)
set(GLM_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/external_libraries/glm)
include_directories(${GLM_INCLUDE_DIRS})

# Our own include directory
set(COMMON_INCLUDES ${PROJECT_SOURCE_DIR}/include)
include_directories(${COMMON_INCLUDES})

# Source files
file(GLOB INTERNAL_SOURCE ${PROJECT_SOURCE_DIR}/src/*.cpp)
file(GLOB INTERNAL_HEADERS ${PROJECT_SOURCE_DIR}/include/*.h)

# Create the executable from our sources
add_executable(${PROJECT_NAME} ${INTERNAL_SOURCE} ${INTERNAL_HEADERS})

# C++11 compatability
set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "-std=c++11")

# Later link other libraries here
# ----- #