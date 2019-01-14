cmake_minimum_required(VERSION 3.5)
project(samples)

set(CMAKE_CXX_STANDARD 11)

find_package(OpenCV REQUIRED)

set(CeleX ../Samples)
set(CeleX_LIB CeleX)

include_directories(${CeleX}/include)
link_directories(${CeleX}/lib/Linux/x64)

if(CMAKE_VERSION VERSION_LESS "2.8.11")
    # Add OpenCV headers location to your include paths
    include_directories(${OpenCV_INCLUDE_DIRS})
endif()


add_executable(samples ./getFrameBuffer.cpp)

#specify the executable directory 
set_target_properties(samples PROPERTIES OUTPUT_NAME ../build/getFrameBuffer)

# Link your application with OpenCV libraries
target_link_libraries(samples ${OpenCV_LIBS})
target_link_libraries(samples ${CeleX_LIB})
