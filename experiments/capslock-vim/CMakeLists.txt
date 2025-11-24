cmake_minimum_required(VERSION 3.16)
project(capslock_vim LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(capslock_vim
    main.cpp
)

target_link_libraries(capslock_vim
    "-framework ApplicationServices"
    "-framework Carbon"
    "-framework CoreFoundation"
    "-framework IOKit"
)
