cmake_minimum_required(VERSION 3.8)

set(This trieTests)

set(Sources
    Tests.c
)

add_executable(${This} ${Sources})
target_link_libraries(${This} 
    PUBLIC 
    gtest_main 
    trie
)

add_test(NAME 
    ${This} 
    COMMAND 
    ${This}
)