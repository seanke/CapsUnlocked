cmake_minimum_required(VERSION 3.16)

project(CapsUnlocked LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CAPS_UNLOCKED_SOURCES
    src/app.cpp
    src/hook.cpp
    src/config.cpp
    src/input.cpp
    src/util.cpp
)

if (WIN32)
    add_executable(CapsUnlocked WIN32 ${CAPS_UNLOCKED_SOURCES})
else()
    add_executable(CapsUnlocked ${CAPS_UNLOCKED_SOURCES})
endif()

if(MSVC)
    target_compile_options(CapsUnlocked PRIVATE /W4 /permissive-)
else()
    target_compile_options(CapsUnlocked PRIVATE -Wall -Wextra -Wpedantic)
endif()

if (WIN32)
    target_link_libraries(CapsUnlocked PRIVATE user32 shell32)
elseif(APPLE)
    target_link_libraries(CapsUnlocked PRIVATE "-framework ApplicationServices" "-framework CoreFoundation")
endif()
