cmake_minimum_required(VERSION 3.14)
project(my_project)

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)

# GoogleTest requires at least C++11
set(CMAKE_CXX_STANDARD 11)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()
find_package(Threads REQUIRED)

file(GLOB test_sources test/sample_test.cc trie.c trie.h bitmap.h)

add_executable(
  trie_test
  ${test_sources}
)
target_link_libraries(
  trie_test
  gtest_main
)

include(GoogleTest)
gtest_discover_tests(trie_test)