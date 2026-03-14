cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
project(audio_stream LANGUAGES C CXX VERSION 1.0.0)


set(CMAKE_CXX_STANDARD ON)
set(CMAKE_CXX_STANDARD_REQUIRED 17)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

if (NOT CMAKE_CXX_FLAGS_DEBUG)
    set(CMAKE_CXX_FLAGS_DEBUG "-g3 -O0")
endif()

if (NOT CMAKE_CXX_FLAGS_RELEASE)
    set(CMAKE_CXX_FLAGS_RELEASE "-g0 -O3")
endif()


add_subdirectory(deps)
add_subdirectory(src)
add_subdirectory(test)
