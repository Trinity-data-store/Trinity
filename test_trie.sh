g++ -g trie.cpp -c -mcmodel=medium
g++ -g test-main.o trie.o tests.cpp -o tests -mcmodel=medium
./tests