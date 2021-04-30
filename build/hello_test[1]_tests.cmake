add_test( HelloTest.BasicAssertions [==[/mnt/e/Work/Spring 2021/Research Anurag/md-trie/build/hello_test]==] [==[--gtest_filter=HelloTest.BasicAssertions]==] --gtest_also_run_disabled_tests)
set_tests_properties( HelloTest.BasicAssertions PROPERTIES WORKING_DIRECTORY [==[/mnt/e/Work/Spring 2021/Research Anurag/md-trie/build]==])
set( hello_test_TESTS HelloTest.BasicAssertions)
