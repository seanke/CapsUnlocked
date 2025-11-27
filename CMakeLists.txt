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
    target_link_libraries(caps_platform PUBLIC
        caps_core
        "-framework ApplicationServices"
        "-framework CoreFoundation"
        "-framework IOKit"
    )

    add_executable(CapsUnlocked src/macos_main.cpp)
    target_link_libraries(CapsUnlocked PRIVATE caps_core caps_platform)
else()
    # On unsupported platforms (like Linux), we can still build and test the core library.
    message(STATUS "Unsupported platform for CapsUnlocked executable; building core library and tests only.")
endif()

if(MSVC)
    target_compile_options(caps_core PRIVATE /W4 /permissive-)
    if(TARGET caps_platform)
        target_compile_options(caps_platform PRIVATE /W4 /permissive-)
    endif()
else()
    target_compile_options(caps_core PRIVATE -Wall -Wextra -Wpedantic)
    if(TARGET caps_platform)
        target_compile_options(caps_platform PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endif()

if (BUILD_TESTING)
    find_package(GTest CONFIG QUIET)
    if (NOT GTest_FOUND)
        find_package(GTest QUIET)
    endif()
    if (NOT GTest_FOUND)
        include(FetchContent)
        FetchContent_Declare(
            googletest
            URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
            DOWNLOAD_EXTRACT_TIMESTAMP TRUE
        )
        set(gtest_force_shared_crt ON CACHE BOOL "Force shared CRT for MSVC compatibility" FORCE)
        FetchContent_MakeAvailable(googletest)
    endif()

    add_executable(caps_core_tests
        tests/core/config_loader_test.cpp
        tests/core/mapping_engine_test.cpp
        tests/core/layer_controller_test.cpp
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
