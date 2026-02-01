add_executable(perftTests
    Perft.cpp
    perftTestSuite.cpp
)

target_link_libraries(perftTests PRIVATE chess_lib)

add_test(NAME perftTests COMMAND perftTests)


add_executable(engineSpeedTest
    engineSpeedTest.cpp
)

target_link_libraries(engineSpeedTest PRIVATE chess_lib)

add_test(NAME engineSpeedTest COMMAND engineSpeedTest)