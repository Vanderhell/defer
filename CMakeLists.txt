cmake_minimum_required(VERSION 3.20)

project(defer VERSION 1.0.0 LANGUAGES C)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

find_package(Threads)

option(DEFER_BUILD_TESTS "Build tests" ON)
option(DEFER_BUILD_EXAMPLES "Build examples" ON)
option(DEFER_STRICT_WARNINGS "Enable strict warning flags for tests and examples" ON)

add_library(defer INTERFACE)
add_library(defer::defer ALIAS defer)

target_include_directories(defer INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

target_compile_features(defer INTERFACE c_std_99)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/defer.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(TARGETS defer EXPORT deferTargets)

install(EXPORT deferTargets
    FILE deferTargets.cmake
    NAMESPACE defer::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/defer)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/deferConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/deferConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/defer)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/deferConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/deferConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/deferConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/defer)

function(defer_apply_warnings target)
    if (DEFER_STRICT_WARNINGS)
        if (MSVC)
            target_compile_options(${target} PRIVATE /W4 /WX)
        else()
            target_compile_options(${target} PRIVATE
                -Wall
                -Wextra
                -Wpedantic
                -Werror
                -Wconversion
                -Wshadow
                -Wstrict-prototypes
                -Wmissing-prototypes
                -Wcast-function-type
                -Wstrict-aliasing=2)
        endif()
    endif()
endfunction()

function(defer_add_runtime_executable target)
    add_executable(${target} ${ARGN})
    target_link_libraries(${target} PRIVATE defer)
    set_target_properties(${target} PROPERTIES
        C_STANDARD 99
        C_STANDARD_REQUIRED YES)
    defer_apply_warnings(${target})
endfunction()

function(defer_add_expected_compile_failure name source)
    set(script "${CMAKE_CURRENT_BINARY_DIR}/${name}.cmake")

    if (MSVC)
        set(compile_command
            "\"${CMAKE_C_COMPILER}\""
            "/nologo"
            "/TC"
            "/c"
            "/WX"
            "/W4"
            "/I\"${CMAKE_CURRENT_SOURCE_DIR}\""
            "\"${source}\"")
    else()
        set(compile_command
            "\"${CMAKE_C_COMPILER}\""
            "-std=c11"
            "-Wall"
            "-Wextra"
            "-Wpedantic"
            "-Werror"
            "-fsyntax-only"
            "-I\"${CMAKE_CURRENT_SOURCE_DIR}\""
            "\"${source}\"")
    endif()

    file(WRITE "${script}" "execute_process(COMMAND ${compile_command} RESULT_VARIABLE rc OUTPUT_VARIABLE out ERROR_VARIABLE err)\nif(rc EQUAL 0)\n  message(FATAL_ERROR \"expected compile failure: ${name}\")\nendif()\n")

    add_test(NAME ${name}
        COMMAND ${CMAKE_COMMAND} -P "${script}")
endfunction()

if (DEFER_BUILD_TESTS)
    enable_testing()

    defer_add_runtime_executable(defer_tests
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_defer.c)
    if (TARGET Threads::Threads)
        target_link_libraries(defer_tests PRIVATE Threads::Threads)
    endif()
    add_test(NAME runtime COMMAND defer_tests)

    defer_add_runtime_executable(defer_multi_tu
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/multi_tu_main.c
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/multi_tu_a.c
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/multi_tu_b.c)
    add_test(NAME multi_tu COMMAND defer_multi_tu)

    defer_add_runtime_executable(defer_threaded
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/threaded.c)
    if (TARGET Threads::Threads)
        target_link_libraries(defer_threaded PRIVATE Threads::Threads)
    endif()
    add_test(NAME threaded COMMAND defer_threaded)

    defer_add_expected_compile_failure(incompatible_callback
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/compile_fail/incompatible_callback.c)
    defer_add_expected_compile_failure(jump_into_guarded_scope
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/compile_fail/jump_into_guarded_scope.c)
    defer_add_expected_compile_failure(dismiss_non_guard
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/compile_fail/dismiss_non_guard.c)

    if (MSVC)
        defer_add_expected_compile_failure(msvc_requires_explicit_fallback
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/compile_fail/msvc_requires_explicit_fallback.c)
    endif()

    add_test(NAME external_consumer
        COMMAND ${CMAKE_COMMAND}
            -DDEFER_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}
            -DDEFER_BUILD_DIR=${CMAKE_CURRENT_BINARY_DIR}
            -DDEFER_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR}/_install
            -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/run_external_consumer.cmake)
endif()

if (DEFER_BUILD_EXAMPLES)
    defer_add_runtime_executable(example_memory
        ${CMAKE_CURRENT_SOURCE_DIR}/examples/memory.c)
    defer_add_runtime_executable(example_files
        ${CMAKE_CURRENT_SOURCE_DIR}/examples/files.c)
    defer_add_runtime_executable(example_mutex
        ${CMAKE_CURRENT_SOURCE_DIR}/examples/mutex.c)
    if (TARGET Threads::Threads)
        target_link_libraries(example_mutex PRIVATE Threads::Threads)
    endif()
endif()
