// C++ implementation of search and insert
// operations on Trie
#include <bits/stdc++.h>
using namespace std;
#include <tqdm.h>

const int ALPHABET_SIZE = 10;
 
// trie node
struct TrieNode
{
    struct TrieNode *children[ALPHABET_SIZE];
    // isEndOfWord is true if the node represents
    // end of a word
    bool isEndOfWord;
};
 
// Returns new trie node (initialized to NULLs)
struct TrieNode *getNode(void)
{
    struct TrieNode *pNode = new TrieNode;
 
    pNode->isEndOfWord = false;
 
    for (int i = 0; i < ALPHABET_SIZE; i++)
        pNode->children[i] = NULL;
 
    return pNode;
}
 
// If not present, inserts key into trie
// If the key is prefix of trie node, just marks leaf node
void insert(struct TrieNode *root, int key)
{
    struct TrieNode *pCrawl = root;
    if (key == 0){
        // mark last node as leaf
        pCrawl->isEndOfWord = true;        
    } 
    else {
        int index = key % 10;
        if (!pCrawl->children[index])
            pCrawl->children[index] = getNode();
        insert(pCrawl->children[index], key / 10);        
    }
}
 
// Returns true if key presents in trie, else
// false
bool search(struct TrieNode *root, int key)
{
    struct TrieNode *pCrawl = root;
    if (key == 0){
        return (pCrawl->isEndOfWord);
    }
    else {
        int index = key % 10;
        if (!pCrawl->children[index])
            return false;
        search(pCrawl->children[index], key / 10);
    }
    return true;
}


void bench()
{
    struct TrieNode *root = getNode();
    int n_points = 0;
    int n_lines = 14583357;

    // char *line = nullptr;
    // size_t len = 0;
    // ssize_t read;
    // FILE *fp = fopen("../libmdtrie/bench/data/sample_shuf.txt", "r");

    // tqdm bar;
    // const int DIMENSION = 3;
    // while ((read = getline(&line, &len, fp)) != -1)
    // {
    //     bar.progress(n_points, n_lines);
    //     char *token = strtok(line, " ");
    //     char *ptr;
    //     uint64_t value = 0;
    //     for (int i = 0; i < 2; i ++){
    //         token = strtok(nullptr, " ");
    //     }
    //     for (int i = 0; i < DIMENSION; i++)
    //     {
    //         token = strtok(nullptr, " ");
    //         value << 32;
    //         leaf_point->set_coordinate(i, strtoul(token, &ptr, 10));

    //     }
    //     mdtrie->insert_trie(leaf_point, max_depth);
    //     n_points ++;
    // }
    // bar.finish();  


    tqdm bar;
    for (n_points = 0; n_points < n_lines; n_points++)
    {
        bar.progress(n_points, n_lines);
        insert(root, n_points);
    }  

    bar.finish();      
    for (n_points = 0; n_points < n_lines; n_points++)
    {
        bar.progress(n_points, n_lines);
        if (!search(root, n_points)){
            fprintf(stderr, "not found!\n");
            raise(SIGINT);
        }
    }  
    bar.finish();  
}

// Driver
int main()
{
    bench();
    return 0;
}