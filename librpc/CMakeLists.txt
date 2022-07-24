include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../libmdtrie/src)

if(FALSE)
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

endif()

add_executable(TrinityTPCHQuery
        src/TrinityTPCHQuery.cpp
        src/MDTrieShardClient.h
        src/MDTrieShard.h
        src/TrinityBenchShared.h
        src/MDTrieShard.cpp)

add_executable(TrinityGithubQuery
        src/TrinityGithubQuery.cpp
        src/MDTrieShardClient.h
        src/MDTrieShard.h
        src/TrinityBenchShared.h
        src/MDTrieShard.cpp)

add_executable(TrinityTPCHLookup
        src/TrinityTPCHLookup.cpp
        src/MDTrieShardClient.h
        src/MDTrieShard.h
        src/TrinityBenchShared.h
        src/MDTrieShard.cpp)

add_executable(TrinityTPCHInsert
        src/TrinityTPCHInsert.cpp
        src/MDTrieShardClient.h
        src/MDTrieShard.h
        src/TrinityBenchShared.h
        src/MDTrieShard.cpp)

add_executable(TrinityGithubInsert
        src/TrinityGithubInsert.cpp
        src/MDTrieShardClient.h
        src/MDTrieShard.h
        src/TrinityBenchShared.h
        src/MDTrieShard.cpp)

add_executable(TrinityNYCInsert
        src/TrinityNYCInsert.cpp
        src/MDTrieShardClient.h
        src/MDTrieShard.h
        src/TrinityBenchShared.h
        src/MDTrieShard.cpp)

add_executable(TrinityStorage
        src/TrinityStorage.cpp
        src/MDTrieShardClient.h
        src/MDTrieShard.h
        src/TrinityBenchShared.h
        src/MDTrieShard.cpp)

add_executable(TrinityTPCHInsertQuery
        src/TrinityTPCHInsertQuery.cpp
        src/MDTrieShardClient.h
        src/MDTrieShard.h
        src/TrinityBenchShared.h
        src/MDTrieShard.cpp)

add_executable(TrinityTPCHInsertLookup
        src/TrinityTPCHInsertLookup.cpp
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

if(FALSE)
target_link_libraries(TrinityFS ${MDTRIE_LIBS} ${RPC_LIBS} event thrift)
add_dependencies(TrinityFS thrift_ep)

target_link_libraries(TrinityOSM ${MDTRIE_LIBS} ${RPC_LIBS} event thrift)
add_dependencies(TrinityOSM thrift_ep)

target_link_libraries(TrinityTPCH ${MDTRIE_LIBS} ${RPC_LIBS} event thrift)
add_dependencies(TrinityTPCH thrift_ep)

endif()

target_link_libraries(TrinityTPCHQuery ${MDTRIE_LIBS} ${RPC_LIBS} event thrift)
add_dependencies(TrinityTPCHQuery thrift_ep)

target_link_libraries(TrinityGithubQuery ${MDTRIE_LIBS} ${RPC_LIBS} event thrift)
add_dependencies(TrinityGithubQuery thrift_ep)

target_link_libraries(TrinityStorage ${MDTRIE_LIBS} ${RPC_LIBS} event thrift)
add_dependencies(TrinityStorage thrift_ep)

target_link_libraries(TrinityTPCHLookup ${MDTRIE_LIBS} ${RPC_LIBS} event thrift)
add_dependencies(TrinityTPCHLookup thrift_ep)

target_link_libraries(TrinityTPCHInsert ${MDTRIE_LIBS} ${RPC_LIBS} event thrift)
add_dependencies(TrinityTPCHInsert thrift_ep)

target_link_libraries(TrinityNYCInsert ${MDTRIE_LIBS} ${RPC_LIBS} event thrift)
add_dependencies(TrinityNYCInsert thrift_ep)

target_link_libraries(TrinityTPCHInsertQuery ${MDTRIE_LIBS} ${RPC_LIBS} event thrift)
add_dependencies(TrinityTPCHInsertQuery thrift_ep)

target_link_libraries(TrinityTPCHInsertLookup ${MDTRIE_LIBS} ${RPC_LIBS} event thrift)
add_dependencies(TrinityTPCHInsertLookup thrift_ep)

target_link_libraries(TrinityGithubInsert ${MDTRIE_LIBS} ${RPC_LIBS} event thrift)
add_dependencies(TrinityGithubInsert thrift_ep)

target_link_libraries(MDTrieShardServer ${MDTRIE_LIBS} ${RPC_LIBS} event thrift)
add_dependencies(MDTrieShardServer thrift_ep)