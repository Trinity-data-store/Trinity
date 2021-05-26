#include "catch.hpp"
#include "trie.hpp"
 
bool testSameData()
{
    treeBlock B;
    trieNode *tNode = createNewTrieNode();
    int maxDepth = 0;


    char *line = NULL;
    size_t len = 0, read;
    FILE *fp = fopen("test_txt/4d_large_data.txt", "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);

    while ((read = getline(&line, &len, fp)) != -1)
    {
        int strLen;
        uint64_t *str = proc_str(line, strLen, maxDepth);
        insertTrie(tNode, str, strLen, maxDepth);
        free(str);
    }
    line = NULL;
    len = 0;
    while ((read = getline(&line, &len, fp)) != -1)
    {
        int strLen;
        uint64_t *str = proc_str(line, strLen, maxDepth);
        if (!check(tNode, str, strLen, maxDepth)){
            free(str);
            return false;
        }
        free(str);
    }
    return true;    
}

bool testDiffData()
{
    treeBlock B;
    trieNode *tNode = createNewTrieNode();
    int maxDepth = 0;

    char *line = NULL;
    size_t len = 0, read;
    FILE *fp = fopen("test_txt/paper_2d_test.txt", "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);

    while ((read = getline(&line, &len, fp)) != -1)
    {
        int strLen;
        uint64_t *str = proc_str(line, strLen, maxDepth);
        insertTrie(tNode, str, strLen, maxDepth);
        free(str);
    }
    fclose(fp);

    line = NULL;
    len = 0;
    fp = fopen("test_txt/3d_test.txt", "r");
    while ((read = getline(&line, &len, fp)) != -1)
    {
        int strLen;
        uint64_t *str = proc_str(line, strLen, maxDepth);
        if (!check(tNode, str, strLen, maxDepth)){
            free(str);
            return false;
        }
        free(str);
    }
    return true;    
}

void pointToMorton(uint64_t *point, uint64_t *morton){
    for (int i = 0; i < dimensions; i ++){

    }
}

bool testRandomData()
{
    treeBlock B;
    trieNode *tNode = createNewTrieNode();
    // 0 - 1023
    uint64_t maxDepth = 10;
    // uint64_t *current_points = (uint64_t *) malloc(dimensions * sizeof(uint64_t));

    uint64_t *str = (uint64_t *) malloc(maxDepth * sizeof(uint64_t));
    int itr = 0;
    while (itr < 50000){
        itr += 1;
        for (int i = 0; i < maxDepth; i++){
            // current_points[i] = rand() % 1024;
            str[i] = rand() % nBranches;
            printf("%ld, ", str[i]);
        }
        if (str[0] == 1 && str[1] == 3 && str[2] == 29 && str[3] == 31){
            raise(SIGINT);
        }
        insertTrie(tNode, str, maxDepth, maxDepth);

        if (!check(tNode, str, maxDepth, maxDepth)){
            raise(SIGINT);
            check(tNode, str, maxDepth, maxDepth);
            return false;
        }
        
        printf("passed! \n");
    }
    return true;
}

bool check(trieNode *tNode, uint64_t str[], int strlen, int maxDepth){

    uint64_t i = 0;
    treeBlock *tBlock = walkTrie(tNode, str, i);
    uint64_t level = i;
    NODE_TYPE curNode = 0;
    uint64_t curFrontier = 0;
    return walkTree(tBlock, str, strlen, maxDepth, curNode, i, level, curFrontier);        
}

TEST_CASE( "Check Insertion Correctness", "[trie]" ) {
    // REQUIRE(testSameData());
    // REQUIRE_FALSE(testDiffData());
    // for (int i = 1; i <= 10; i ++){
    REQUIRE(testRandomData());
    // }
    
}
