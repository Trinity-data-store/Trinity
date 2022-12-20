include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

add_library(mdtrie INTERFACE)
target_include_directories(mdtrie INTERFACE src/)
# add_dependencies(mdtrie ${HEAP_MANAGER_EP})
set(MDTRIE_LIBS "${HEAP_MANAGER_LIBRARY}" "${CMAKE_DL_LIBS}" "mdtrie")

if (BUILD_TESTS)
  include_directories(${CATCH_INCLUDE_DIR})
  add_executable(mdtrie_tests
          test/test_main.cpp
          test/mdtrie_test.cpp)

#   add_executable(range_search_tests
#           test/test_main.cpp
#           test/range_search_test.cpp)

  add_executable(mdtrie_lookup_tests
          test/test_main.cpp
          test/mdtrie_lookup_test.cpp)

  target_link_libraries(mdtrie_tests ${MDTRIE_LIBS})
  # target_link_libraries(range_search_tests ${MDTRIE_LIBS})
  target_link_libraries(mdtrie_lookup_tests ${MDTRIE_LIBS})

  add_dependencies(mdtrie_tests catch_ep)
#   add_dependencies(range_search_tests catch_ep)
  add_dependencies(mdtrie_lookup_tests catch_ep)

  # Register test
  add_test(MdTrieTest mdtrie_tests)
  add_test(MdTrieLookupTest mdtrie_lookup_tests)
#   add_test(RangeSearchTest range_search_tests)

endif ()

add_executable(nyc_bench
        bench/nyc_bench.cpp)

add_executable(github_bench
        bench/github_bench.cpp)

add_executable(tpch_bench
        bench/tpch_bench.cpp)

add_executable(sensitivity_query_selectivity
        bench/sensitivity_query_selectivity.cpp)

add_executable(sensitivity_num_dimensions
        bench/sensitivity_num_dimensions.cpp)

add_executable(nyc_optimization
        bench/nyc_optimization.cpp)

target_link_libraries(nyc_bench ${MDTRIE_LIBS})
target_link_libraries(github_bench ${MDTRIE_LIBS})
target_link_libraries(tpch_bench ${MDTRIE_LIBS})
target_link_libraries(sensitivity_query_selectivity ${MDTRIE_LIBS})
target_link_libraries(sensitivity_num_dimensions ${MDTRIE_LIBS})
target_link_libraries(nyc_optimization ${MDTRIE_LIBS})
