include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

add_library(mdtrie INTERFACE)
target_include_directories(mdtrie INTERFACE src/)
add_dependencies(mdtrie ${HEAP_MANAGER_EP})
set(MDTRIE_LIBS "${HEAP_MANAGER_LIBRARY}" "${CMAKE_DL_LIBS}" "mdtrie")

if (BUILD_TESTS)
  include_directories(${CATCH_INCLUDE_DIR})
  add_executable(mdtrie_tests
          test/test_main.cpp
          test/mdtrie_test.cpp)

  add_executable(range_search_tests
          test/test_main.cpp)

  target_link_libraries(mdtrie_tests ${MDTRIE_LIBS})
  target_link_libraries(range_search_tests ${MDTRIE_LIBS})

  add_dependencies(mdtrie_tests catch_ep)
  add_dependencies(range_search_tests catch_ep)

  # Register test
  add_test(MdTrieTest mdtrie_tests)
endif ()

add_executable(mdtrie_bench
        bench/mdtrie_bench.cpp)

add_executable(orig_bench
        bench/treeBlock_orig.h
        bench/treeBlock_orig.cpp)

add_executable(orig_bench_no_table
        bench/treeBlock_no_table.cpp)

add_executable(range_search_bench
        bench/range_search_bench.cpp)

add_executable(concurrency_bench
        bench/concurrency_bench.cpp)

add_executable(trie_one_dimension_bench
        bench/trie_one_dimension_bench.cpp)

add_executable(self_index_bench
        bench/self_index_bench.cpp)

target_link_libraries(mdtrie_bench ${MDTRIE_LIBS})
target_link_libraries(range_search_bench ${MDTRIE_LIBS})
target_link_libraries(concurrency_bench ${MDTRIE_LIBS})
target_link_libraries(self_index_bench ${MDTRIE_LIBS})