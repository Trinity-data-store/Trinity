# CMake generated Testfile for 
# Source directory: E:/Work/Spring 2021/Research Anurag/md-trie/test
# Build directory: E:/Work/Spring 2021/Research Anurag/md-trie/build/test
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
if("${CTEST_CONFIGURATION_TYPE}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
  add_test(trieTests "E:/Work/Spring 2021/Research Anurag/md-trie/build/test/Debug/trieTests.exe")
  set_tests_properties(trieTests PROPERTIES  _BACKTRACE_TRIPLES "E:/Work/Spring 2021/Research Anurag/md-trie/test/CMakeLists.txt;16;add_test;E:/Work/Spring 2021/Research Anurag/md-trie/test/CMakeLists.txt;0;")
elseif("${CTEST_CONFIGURATION_TYPE}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
  add_test(trieTests "E:/Work/Spring 2021/Research Anurag/md-trie/build/test/Release/trieTests.exe")
  set_tests_properties(trieTests PROPERTIES  _BACKTRACE_TRIPLES "E:/Work/Spring 2021/Research Anurag/md-trie/test/CMakeLists.txt;16;add_test;E:/Work/Spring 2021/Research Anurag/md-trie/test/CMakeLists.txt;0;")
elseif("${CTEST_CONFIGURATION_TYPE}" MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
  add_test(trieTests "E:/Work/Spring 2021/Research Anurag/md-trie/build/test/MinSizeRel/trieTests.exe")
  set_tests_properties(trieTests PROPERTIES  _BACKTRACE_TRIPLES "E:/Work/Spring 2021/Research Anurag/md-trie/test/CMakeLists.txt;16;add_test;E:/Work/Spring 2021/Research Anurag/md-trie/test/CMakeLists.txt;0;")
elseif("${CTEST_CONFIGURATION_TYPE}" MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
  add_test(trieTests "E:/Work/Spring 2021/Research Anurag/md-trie/build/test/RelWithDebInfo/trieTests.exe")
  set_tests_properties(trieTests PROPERTIES  _BACKTRACE_TRIPLES "E:/Work/Spring 2021/Research Anurag/md-trie/test/CMakeLists.txt;16;add_test;E:/Work/Spring 2021/Research Anurag/md-trie/test/CMakeLists.txt;0;")
else()
  add_test(trieTests NOT_AVAILABLE)
endif()
