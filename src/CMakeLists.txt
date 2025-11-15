# Options


# Variables
set(LIB audio_stream)
set(PUBLIC_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
set(HEADERS 
    ${PUBLIC_INCLUDE_DIR}/${PROJECT_NAME}/audio_stream_interface.hpp
    ${PUBLIC_INCLUDE_DIR}/${PROJECT_NAME}/audio_stream.hpp
)
set(SOURCES audio_stream.cpp)


# Find Packages


# Sub Directories


# Executables
add_library(${LIB} STATIC ${SOURCES} ${HEADERS})


# Include Directories
target_include_directories(${LIB}
    PUBLIC ${PUBLIC_INCLUDE_DIR}
)


# Libraries
target_link_libraries(${LIB}
    PRIVATE websockets
)


