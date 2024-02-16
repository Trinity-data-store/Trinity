include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

add_library(mdtrie INTERFACE)
target_include_directories(mdtrie INTERFACE src/)
set(MDTRIE_LIBS "${HEAP_MANAGER_LIBRARY}" "${CMAKE_DL_LIBS}" "mdtrie")

add_executable(microbench
        bench/microbench.cpp)

add_executable(dimension
        bench/dimension.cpp)

add_executable(example
        bench/example.cpp)

target_link_libraries(microbench ${MDTRIE_LIBS})
target_link_libraries(dimension ${MDTRIE_LIBS})
target_link_libraries(example ${MDTRIE_LIBS})
