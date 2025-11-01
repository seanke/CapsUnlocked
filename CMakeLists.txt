cmake_minimum_required(VERSION 3.16)

project(CapsUnlocked LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB_RECURSE CAPS_CORE_SOURCES
    CONFIGURE_DEPENDS
    src/core/*.cpp
)

add_library(caps_core STATIC ${CAPS_CORE_SOURCES})
target_include_directories(caps_core PUBLIC src)

include(CTest)

if (WIN32)
    file(GLOB_RECURSE CAPS_PLATFORM_SOURCES
        CONFIGURE_DEPENDS
        src/platform/windows/*.cpp
    )
    add_library(caps_platform STATIC ${CAPS_PLATFORM_SOURCES})
    target_include_directories(caps_platform PUBLIC src)
    target_link_libraries(caps_platform PUBLIC caps_core)

    add_executable(CapsUnlocked src/windows_main.cpp)
    target_link_libraries(CapsUnlocked PRIVATE caps_core caps_platform)
elseif(APPLE)
    file(GLOB_RECURSE CAPS_PLATFORM_SOURCES
        CONFIGURE_DEPENDS
        src/platform/macos/*.cpp
    )
    add_library(caps_platform STATIC ${CAPS_PLATFORM_SOURCES})
    target_include_directories(caps_platform PUBLIC src)
    target_link_libraries(caps_platform PUBLIC caps_core)

    add_executable(CapsUnlocked src/macos_main.cpp)
    target_link_libraries(CapsUnlocked PRIVATE caps_core caps_platform)
else()
    message(FATAL_ERROR "Unsupported platform for CapsUnlocked skeleton")
endif()

if(MSVC)
    target_compile_options(caps_core PRIVATE /W4 /permissive-)
    target_compile_options(caps_platform PRIVATE /W4 /permissive-)
else()
    target_compile_options(caps_core PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(caps_platform PRIVATE -Wall -Wextra -Wpedantic)
endif()

if (BUILD_TESTING)
    find_package(GTest CONFIG QUIET)
    if (NOT GTest_FOUND)
        find_package(GTest REQUIRED)
    endif()

    add_executable(caps_core_tests
        tests/core/hello_test.cpp
    )
    target_link_libraries(caps_core_tests PRIVATE caps_core GTest::gtest_main)

    if (MSVC)
        target_compile_options(caps_core_tests PRIVATE /W4 /permissive-)
    else()
        target_compile_options(caps_core_tests PRIVATE -Wall -Wextra -Wpedantic)
    endif()

    include(GoogleTest)
    gtest_discover_tests(caps_core_tests)
endif()
