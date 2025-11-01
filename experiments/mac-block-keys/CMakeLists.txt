cmake_minimum_required(VERSION 3.16)
project(mac_block_keys LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(mac_block_keys
    main.cpp
)

if(APPLE)
    target_compile_definitions(mac_block_keys PRIVATE MAC_OS_X_VERSION_MIN_REQUIRED=101300)
    find_library(COCOA_LIBRARY Cocoa)
    find_library(IOKIT_LIBRARY IOKit)
    find_library(CARBON_LIBRARY Carbon)
    find_library(APP_SERVICES ApplicationServices)
    target_link_libraries(mac_block_keys
        PRIVATE
            ${COCOA_LIBRARY}
            ${IOKIT_LIBRARY}
            ${CARBON_LIBRARY}
            ${APP_SERVICES}
    )
endif()
