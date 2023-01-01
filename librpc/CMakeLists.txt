include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../libmdtrie/src)

add_executable(TrinityTPCH
        src/TrinityTPCH.cpp
        src/MDTrieShardClient.h
        src/MDTrieShard.h
        src/MDTrieShard.cpp)

add_executable(TrinityGithub
        src/TrinityGithub.cpp
        src/MDTrieShardClient.h
        src/MDTrieShard.h
        src/MDTrieShard.cpp)

add_executable(TrinityNYC
        src/TrinityNYC.cpp
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

target_link_libraries(TrinityTPCH ${MDTRIE_LIBS} ${RPC_LIBS} event thrift)
add_dependencies(TrinityTPCH thrift_ep)

target_link_libraries(TrinityGithub ${MDTRIE_LIBS} ${RPC_LIBS} event thrift)
add_dependencies(TrinityGithub thrift_ep)

target_link_libraries(TrinityNYC ${MDTRIE_LIBS} ${RPC_LIBS} event thrift)
add_dependencies(TrinityNYC thrift_ep)

target_link_libraries(MDTrieShardServer ${MDTRIE_LIBS} ${RPC_LIBS} event thrift)
add_dependencies(MDTrieShardServer thrift_ep)