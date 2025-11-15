cmake_minimum_required(VERSION 3.20)
project(RandKey VERSION 2.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(RANDKEY_BUILD_TESTS "Build RandKey test suite" ON)

add_library(randkey_core
    src/random_engine.cpp
    src/charset_registry.cpp
    src/generator.cpp
    src/options.cpp
    src/encoding.cpp
    src/i18n/catalog.cpp
    src/i18n/messages.cpp
    src/platform/encoding.cpp
    src/platform/language.cpp
)

target_include_directories(randkey_core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_compile_definitions(randkey_core PRIVATE RANDKEY_VERSION="${PROJECT_VERSION}")

if (WIN32)
    target_sources(randkey_core PRIVATE src/platform/windows/random_device.cpp)
    target_compile_definitions(randkey_core PRIVATE RANDKEY_PLATFORM_WINDOWS)
else()
    target_sources(randkey_core PRIVATE src/platform/posix/random_device.cpp)
    target_compile_definitions(randkey_core PRIVATE RANDKEY_PLATFORM_POSIX)
endif()

add_executable(randkey src/main.cpp)
target_link_libraries(randkey PRIVATE randkey_core)
target_compile_definitions(randkey PRIVATE RANDKEY_VERSION="${PROJECT_VERSION}")

if (WIN32)
    target_link_libraries(randkey_core PRIVATE bcrypt)
endif()

if (RANDKEY_BUILD_TESTS)
    enable_testing()
    add_executable(randkey_tests
        tests/test_main.cpp
        tests/test_charset.cpp
        tests/test_generator.cpp
        tests/test_cli.cpp
    )
    target_link_libraries(randkey_tests PRIVATE randkey_core)
    target_compile_features(randkey_tests PRIVATE cxx_std_20)
    add_test(NAME randkey_tests COMMAND randkey_tests)
endif()
