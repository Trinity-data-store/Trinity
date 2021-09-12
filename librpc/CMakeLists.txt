include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

add_executable(MDTrieShardClient
        src/MDTrieShardClient.cpp
        src/MDTrieShard.h)

add_executable(MDTrieShardServer
        src/MDTrieShardServer.cpp
        src/MDTrieShard.h)
