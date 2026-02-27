cmake_minimum_required(VERSION 3.20)

project(RakChat VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- RakNet Sources ---
file(GLOB RAKNET_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/RakNet/Source/*.cpp
)

add_library(raknet STATIC ${RAKNET_SOURCES})

set(PA_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(OPUS_BUILD_TESTING OFF CACHE BOOL "" FORCE)

add_subdirectory(opus)
add_subdirectory(portaudio)

target_include_directories(raknet PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/RakNet/Source
    ${CMAKE_CURRENT_SOURCE_DIR}/common
)

# Some RakNet builds require this on Windows
if (WIN32)
    target_compile_definitions(raknet PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

if (WIN32)
    target_link_libraries(raknet PRIVATE ws2_32)
endif()

# --- Server ---
add_executable(chat_server 
    server/server.cpp
    server/RakChatUser.cpp
)
target_link_libraries(chat_server PRIVATE
    raknet
    portaudio
    opus)

# --- Client ---
add_executable(chat_client 
    client/client.cpp
    client/device.cpp
    client/encode.cpp
    client/voice.cpp
)
target_link_libraries(chat_client PRIVATE
    raknet
    portaudio
    opus
)

