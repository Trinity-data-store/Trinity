include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

add_library(mdtrie
        src/bitmap.h
        src/tqdm.h
        src/trie.h
        src/data_point.h
        src/defs.h
        src/point_array.h
        src/tree_block.h
        src/trie_node.h
        src/rand_utils.h)

set_target_properties(mdtrie PROPERTIES LINKER_LANGUAGE CXX)

if (BUILD_TESTS)
  include_directories(${CATCH_INCLUDE_DIR})
  add_executable(mdtrie_tests
          test/test_main.cpp
          test/mdtrie_test.cpp)

  add_executable(range_search_tests
          test/test_main.cpp
          )

  target_link_libraries(mdtrie_tests mdtrie)
  target_link_libraries(range_search_tests mdtrie)

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

target_link_libraries(mdtrie_bench mdtrie)
target_link_libraries(range_search_bench mdtrie)
target_link_libraries(concurrency_bench mdtrie)