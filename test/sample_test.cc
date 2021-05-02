#include <gtest/gtest.h>
#include "../trie.h"
// #include "../trie.cpp"

// Demonstrate some basic assertions.
TEST(SampleTest, BasicAssertions) {
  // Expect two strings not to be equal.
  EXPECT_STRNE("hello", "world");
  // Expect equality.
  EXPECT_EQ(7 * 6, 42);
}

TEST(SampleTest, ImportTrie) {

  treeBlock B;
  // depthVector[0] = 1;
  createChildT();

}