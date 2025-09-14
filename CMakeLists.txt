cmake_minimum_required(VERSION 3.16)

project(CapsUnlocked LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build a GUI (WIN32) app so it can live in the tray without a console window.
add_executable(CapsUnlocked WIN32
    src/app.cpp
    src/hook.cpp
    src/config.cpp
    src/input.cpp
    src/util.cpp
)

if(MSVC)
    target_compile_options(CapsUnlocked PRIVATE /W4 /permissive-)
else()
    target_compile_options(CapsUnlocked PRIVATE -Wall -Wextra -Wpedantic)
endif()

target_link_libraries(CapsUnlocked PRIVATE user32 shell32 shlwapi)
