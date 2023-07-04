include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

add_library(mdtrie INTERFACE)
target_include_directories(mdtrie INTERFACE src/)
set(MDTRIE_LIBS "${HEAP_MANAGER_LIBRARY}" "${CMAKE_DL_LIBS}" "mdtrie")

if (BUILD_TESTS)
  include_directories(${CATCH_INCLUDE_DIR})
  add_executable(mdtrie_tests
          test/test_main.cpp
          test/mdtrie_test.cpp)

  add_executable(mdtrie_lookup_tests
          test/test_main.cpp
          test/mdtrie_lookup_test.cpp)

  add_executable(range_search_test
          test/test_main.cpp
          test/range_search_test.cpp)

  target_link_libraries(mdtrie_tests ${MDTRIE_LIBS})
  target_link_libraries(mdtrie_lookup_tests ${MDTRIE_LIBS})
  target_link_libraries(range_search_test ${MDTRIE_LIBS})

  add_dependencies(mdtrie_tests catch_ep)
  add_dependencies(mdtrie_lookup_tests catch_ep)
  add_dependencies(range_search_test catch_ep)

  add_test(MdTrieTest mdtrie_tests)
  add_test(MdTrieLookupTest mdtrie_lookup_tests)
  add_test(MdTrieRangeSearchTest range_search_test)

endif ()

add_executable(microbench
        bench/microbench.cpp)

target_link_libraries(microbench ${MDTRIE_LIBS})
