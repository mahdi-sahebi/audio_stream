# Options
option(BUILD_TESTS "To build the tests" ON)


# Variables
set(TEST test_${PROJECT_NAME})
set(SOURCES test.cpp)
set(HEADERS "")


# Find Packages
find_package(GTest REQUIRED)


# Sub Directories


# Executables
add_executable(${TEST} ${SOURCES} ${HEADERS})


# Include Directories


# Libraries
target_link_libraries(${TEST}
    PRIVATE GTest::gtest stdc++fs audio_stream
)


# Dependencies
file(COPY 
    ${CMAKE_CURRENT_SOURCE_DIR}/deps
    DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
