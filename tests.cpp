#include "catch.hpp"
#include "trie.hpp"

void test()
{
    treeBlock B;
    trieNode *tNode = createNewTrieNode();
    int maxDepth = 0;

    createTables();

    char *line = NULL;
    size_t len = 0, read;

    FILE *fp = fopen("test_txt/paper_2d_test.txt", "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);

    int i = 0;
    while ((read = getline(&line, &len, fp)) != -1)
    {
        i++;
        char *pos;
        int extra_char = 1;
        if ((pos = strchr(line, '\n')) != NULL)
            *pos = '\0';
        else
            extra_char = 0;
        int strLen = (int)strlen(line) - extra_char;

        if (strLen > maxDepth){
            maxDepth = strLen;
        }
        uint64_t str[strLen];
        for (int j = 0; j < strLen; j++)
        {
            str[j] = ((int)line[j]) % nBranches;
        }
        insertTrie(tNode, str, strLen, maxDepth);
    }
    fclose(fp);
    free(line);
}

TEST_CASE( "Import trie", "[single-file]" ) {
    test();

}
