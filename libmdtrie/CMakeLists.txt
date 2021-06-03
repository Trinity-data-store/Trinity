include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

add_library(mdtrie
        src/bitmap.h
        src/trie.cpp
        src/trie.h)

set_target_properties(mdtrie PROPERTIES LINKER_LANGUAGE CXX)

if (BUILD_TESTS)
  include_directories(${CATCH_INCLUDE_DIR})
  add_executable(mdtrie_tests
          test/test_main.cpp
          test/mdtrie_test.cpp)
  target_link_libraries(mdtrie_tests mdtrie)
  
  add_dependencies(mdtrie_tests catch_ep)
  # Register test
  add_test(MdTrieTest mdtrie_tests)
endif ()

add_executable(mdtrie_bench
        bench/mdtrie_bench.cpp)
target_link_libraries(mdtrie_bench mdtrie)