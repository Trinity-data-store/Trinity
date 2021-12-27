include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../libmdtrie/src)

add_executable(MDTrieShardTest
        src/MDTrieShardTest.cpp
        src/MDTrieShardClient.h
        src/MDTrieShard.h
        src/TrinityBenchShared.h
        src/MDTrieShard.cpp)

add_executable(TrinityFS
        src/TrinityFS.cpp
        src/TrinityBenchShared.h
        src/MDTrieShardClient.h
        src/MDTrieShard.h
        src/MDTrieShard.cpp)

add_executable(TrinityOSM
        src/TrinityOSM.cpp
        src/MDTrieShardClient.h
        src/MDTrieShard.h
        src/TrinityBenchShared.h
        src/MDTrieShard.cpp)

add_executable(TrinityTPCH
        src/TrinityTPCH.cpp
        src/MDTrieShardClient.h
        src/MDTrieShard.h
        src/TrinityBenchShared.h
        src/MDTrieShard.cpp)

add_executable(MDTrieShardServer
        src/MDTrieShardServer.cpp
        src/MDTrieShard.h
        src/MDTrieShard.cpp)

set(RPC_LIBS ${THRIFT_LIBRARY}
            ${THRIFTNB_LIBRARY}
            ${LIBEVENT_LIBRARY})

target_link_libraries(MDTrieShardTest ${MDTRIE_LIBS} ${RPC_LIBS} event thrift)
add_dependencies(MDTrieShardTest thrift_ep)

target_link_libraries(TrinityFS ${MDTRIE_LIBS} ${RPC_LIBS} event thrift)
add_dependencies(TrinityFS thrift_ep)

target_link_libraries(TrinityOSM ${MDTRIE_LIBS} ${RPC_LIBS} event thrift)
add_dependencies(TrinityOSM thrift_ep)

target_link_libraries(TrinityTPCH ${MDTRIE_LIBS} ${RPC_LIBS} event thrift)
add_dependencies(TrinityTPCH thrift_ep)

target_link_libraries(MDTrieShardServer ${MDTRIE_LIBS} ${RPC_LIBS} event thrift)
add_dependencies(MDTrieShardServer thrift_ep)