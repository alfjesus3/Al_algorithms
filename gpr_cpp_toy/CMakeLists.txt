cmake_minimum_required(VERSION 3.2)
project(GprCppToy)

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost 1.45.0 REQUIRED COMPONENTS system filesystem thread regex unit_test_framework)
if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
endif()

find_package(Eigen3 REQUIRED)
include_directories(${EIGEN3_INCLUDE_DIR})

find_package(matplotlib_cpp REQUIRED)


add_executable(my_limbo_test main.cpp)

target_include_directories(my_limbo_test
  PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
)

target_link_libraries(my_limbo_test
  PUBLIC
    ${TBB_LIBRARIES}
    ${Boost_LIBRARIES}
    Eigen3::Eigen
    matplotlib_cpp::matplotlib_cpp
)
