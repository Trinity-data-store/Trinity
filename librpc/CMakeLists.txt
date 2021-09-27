include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../libmdtrie/src)

add_executable(MDTrieShardTest
        src/MDTrieShardTest.cpp
        src/MDTrieShardClient.h
        src/MDTrieShard.h
        src/MDTrieShard.cpp)

add_executable(MDTrieShardServer
        src/MDTrieShardServer.cpp
        src/MDTrieShard.h
        src/MDTrieShard.cpp)

set(RPC_LIBS ${THRIFT_LIBRARY}
            ${THRIFTNB_LIBRARY}
            ${LIBEVENT_LIBRARY})

target_link_libraries(MDTrieShardTest ${MDTRIE_LIBS} ${RPC_LIBS})
add_dependencies(MDTrieShardTest thrift_ep)

target_link_libraries(MDTrieShardServer ${MDTRIE_LIBS} ${RPC_LIBS})
add_dependencies(MDTrieShardServer thrift_ep)