cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
project(cpp-backend VERSION 0.1.0 LANGUAGES C CXX)

find_package(PCL 1.3 REQUIRED)

include_directories(${PCL_INCLUDE_DIRS})
link_directories(${PCL_LIBRARY_DIRS})
add_definitions(${PCL_DEFINITIONS})

include(CTest)
enable_testing()

add_executable(cpp-backend main.cpp)

include_directories(${CMAKE_SOURCE_DIR}/libs)

target_link_libraries(cpp-backend pthread ${PCL_LIBRARIES})

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)