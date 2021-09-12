include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

add_executable(MDTrieShardClient
        src/MDTrieShardClient.cpp
        src/MDTrieShard.h)

add_executable(MDTrieShardServer
        src/MDTrieShardServer.cpp
        src/MDTrieShard.h)

target_link_libraries(mdtried ${MDTRIE_LIBS} ${RPC_LIBS})
add_dependencies(mdtried thrift)

target_link_libraries(mdtrieclient ${RPC_LIBS})
add_dependencies(mdtrieclient thrift)
