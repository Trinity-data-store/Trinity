include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../libmdtrie/src)

add_executable(MDTrieShardClient
        src/MDTrieShardClient.cpp
        src/MDTrieShard.h
        src/MDTrieShard.cpp)

add_executable(MDTrieShardServer
        src/MDTrieShardServer.cpp
        src/MDTrieShard.h
        src/MDTrieShard.cpp)

target_link_libraries(MDTrieShardClient ${MDTRIE_LIBS} ${RPC_LIBS})
add_dependencies(MDTrieShardClient thrift_ep)

target_link_libraries(MDTrieShardServer ${MDTRIE_LIBS} ${RPC_LIBS})
add_dependencies(MDTrieShardServer thrift_ep)