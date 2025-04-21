include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

add_library(mdtrie INTERFACE)
target_include_directories(mdtrie INTERFACE src/)
set(MDTRIE_LIBS "${HEAP_MANAGER_LIBRARY}" "${CMAKE_DL_LIBS}" "mdtrie")

add_executable(microbench
        bench/microbench.cpp)

add_executable(example
        bench/example.cpp)

add_executable(insert_test
        bench/insert_test.cpp)

add_executable(search_test
        bench/search_test.cpp)

target_link_libraries(microbench ${MDTRIE_LIBS})
target_link_libraries(example ${MDTRIE_LIBS})
