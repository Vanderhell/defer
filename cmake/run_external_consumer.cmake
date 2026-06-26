if (NOT DEFINED DEFER_SOURCE_DIR OR NOT DEFINED DEFER_BUILD_DIR OR NOT DEFINED DEFER_INSTALL_PREFIX)
    message(FATAL_ERROR "missing required variables")
endif()

execute_process(
    COMMAND "${CMAKE_COMMAND}" --install "${DEFER_BUILD_DIR}" --prefix "${DEFER_INSTALL_PREFIX}"
    RESULT_VARIABLE install_result
)
if (NOT install_result EQUAL 0)
    message(FATAL_ERROR "install step failed")
endif()

set(consumer_build_dir "${DEFER_BUILD_DIR}/external-consumer")

execute_process(
    COMMAND "${CMAKE_COMMAND}"
        -S "${DEFER_SOURCE_DIR}/tests/consumer"
        -B "${consumer_build_dir}"
        -DCMAKE_PREFIX_PATH=${DEFER_INSTALL_PREFIX}
    RESULT_VARIABLE configure_result
)
if (NOT configure_result EQUAL 0)
    message(FATAL_ERROR "consumer configure failed")
endif()

execute_process(
    COMMAND "${CMAKE_COMMAND}" --build "${consumer_build_dir}"
    RESULT_VARIABLE build_result
)
if (NOT build_result EQUAL 0)
    message(FATAL_ERROR "consumer build failed")
endif()

if (MSVC)
    set(consumer_exe "${consumer_build_dir}/Debug/defer_consumer.exe")
    if (NOT EXISTS "${consumer_exe}")
        set(consumer_exe "${consumer_build_dir}/Release/defer_consumer.exe")
    endif()
else()
    set(consumer_exe "${consumer_build_dir}/defer_consumer")
endif()

execute_process(
    COMMAND "${consumer_exe}"
    RESULT_VARIABLE run_result
)
if (NOT run_result EQUAL 0)
    message(FATAL_ERROR "consumer run failed")
endif()
