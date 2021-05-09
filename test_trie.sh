g++ trie.cpp -c -mcmodel=medium
g++ test-main.o trie.o tests.cpp -o tests -mcmodel=medium
./tests