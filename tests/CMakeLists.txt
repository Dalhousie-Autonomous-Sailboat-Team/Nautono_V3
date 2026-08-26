set(USER_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../User/Inc)
set(USER_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../User/Src)
set(TEST_FAKE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/fakes)

function(add_windbound_test target)
    add_executable(${target} ${ARGN})
    target_include_directories(${target} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${USER_INCLUDE_DIR}
    )

    if(MSVC)
        target_compile_definitions(${target} PRIVATE _CRT_SECURE_NO_WARNINGS)
        target_compile_options(${target} PRIVATE /W4)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
    endif()

    add_test(NAME ${target} COMMAND ${target})
endfunction()

add_windbound_test(control_logic_tests
    test_control_logic.c
    ${USER_SOURCE_DIR}/control_logic.c
)

add_windbound_test(pid_tests
    test_pid.c
    ${USER_SOURCE_DIR}/pid.c
)

add_windbound_test(app_protocol_tests
    test_app_protocol.c
    ${USER_SOURCE_DIR}/app_protocol.c
)

add_windbound_test(app_state_tests
    test_app_state.c
    ${USER_SOURCE_DIR}/app_state.c
)
target_include_directories(app_state_tests BEFORE PRIVATE ${TEST_FAKE_DIR})

add_windbound_test(encoder_tests
    test_encoder.c
    ${USER_SOURCE_DIR}/encoder.c
)
target_include_directories(encoder_tests BEFORE PRIVATE
    ${TEST_FAKE_DIR}/encoder
    ${TEST_FAKE_DIR}
)

add_windbound_test(fw_assert_tests
    test_fw_assert.c
    ${USER_SOURCE_DIR}/fw_assert.c
)
target_include_directories(fw_assert_tests BEFORE PRIVATE ${TEST_FAKE_DIR})

add_windbound_test(board_io_tests
    test_board_io.c
    ${USER_SOURCE_DIR}/board_io.c
)
target_include_directories(board_io_tests BEFORE PRIVATE ${TEST_FAKE_DIR})

add_windbound_test(system_tasks_tests
    test_system_tasks.c
    ${USER_SOURCE_DIR}/system_tasks.c
)
target_include_directories(system_tasks_tests BEFORE PRIVATE ${TEST_FAKE_DIR})

add_windbound_test(command_pipeline_tests
    test_command_pipeline.c
    ${USER_SOURCE_DIR}/app_protocol.c
    ${USER_SOURCE_DIR}/app_state.c
    ${USER_SOURCE_DIR}/control_logic.c
)
target_include_directories(command_pipeline_tests BEFORE PRIVATE ${TEST_FAKE_DIR})
