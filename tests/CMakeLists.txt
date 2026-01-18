add_executable(chess_tests
    num_positions.cpp
)

target_link_libraries(chess_tests PRIVATE chess_lib)

add_test(NAME chess_tests COMMAND chess_tests)
