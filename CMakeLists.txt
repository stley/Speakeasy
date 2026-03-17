cmake_minimum_required(VERSION 3.20)

project(RakChat VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- RakNet Sources ---
file(GLOB RAKNET_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/RakNet/Source/*.cpp
)

file(GLOB MICROUI_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/microui/src/*.c
)

add_library(microui STATIC ${MICROUI_SOURCES})

add_library(raknet STATIC ${RAKNET_SOURCES})

set(PA_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(OPUS_BUILD_TESTING OFF CACHE BOOL "" FORCE)

add_subdirectory(opus)
add_subdirectory(portaudio)

target_include_directories(raknet PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/RakNet/Source
)
target_include_directories(microui PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/microui/src    
)


# Some RakNet builds require this on Windows
if (WIN32)
    target_compile_definitions(raknet PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

if (WIN32)
    target_link_libraries(raknet PRIVATE ws2_32)
endif()

# --- Server ---
add_executable(SpeakeasyServer
    server/server.cpp
    server/RakChatChannel.cpp
    server/RakChatUser.cpp

)
target_link_libraries(SpeakeasyServer PRIVATE
    raknet
    portaudio
    opus)

# --- Client ---
add_executable(SpeakeasyClient
    client/client.cpp
    client/device.cpp
    client/encode.cpp
    client/voice.cpp
)

add_executable(SpeakeasyUI
    client-ui/client.cpp
    client-ui/device.cpp
    client-ui/encode.cpp
    client-ui/voice.cpp
)

target_link_libraries(SpeakeasyUI PRIVATE
    raknet
    portaudio
    opus
    microui
)

target_link_libraries(SpeakeasyClient PRIVATE
    raknet
    portaudio
    opus
)

target_include_directories(SpeakeasyUI PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/common)

target_include_directories(SpeakeasyClient PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/common)

target_include_directories(SpeakeasyServer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/common)

