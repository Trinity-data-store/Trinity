#define CATCH_CONFIG_MAIN 
#include "catch.hpp"
#include "trie.h"

void printString(uint64_t *str, uint64_t length)
{
    printf("string: ");
    for (int i = 0; i < length; i++)
    {
        printf("%ld ", str[i]);
    }
    printf("\n");
}

void printTable(uint64_t T[nNodeConf][nBranches])
{
    for (int row = 0; row < nNodeConf; row++)
    {
        for (int col = 0; col < nBranches; col++)
        {
            printf("%ld ", T[row][col]);
        }
        printf("\n");
    }
}

void printDFUDS(bitmap::Bitmap *dfuds, uint64_t nNodes)
{
    printf("dfuds: ");
    for (int i = 0; i < nNodes; i++)
    {
        printf("%ld, ", getNodeCod(dfuds, i));
    }
    printf("\n");
}

void test(char link[])
{
    treeBlock B;
    trieNode *tNode = createNewTrieNode();
    int maxDepth = 0;

    createChildT();
    createChildSkipT();
    createNChildrenT();
    createInsertT();

    char *line = NULL;
    size_t len = 0, read;

    FILE *fp = fopen("test_txt/4d_large_data.txt", "r");

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
    printf("test_complete\n");
}

int Factorial( int number ) {
    return number <= 1 ? 1      : Factorial( number - 1 ) * number;  // pass
}

TEST_CASE( "Factorial of 0 is 1 (fail)", "[single-file]" ) {
    REQUIRE( Factorial(0) == 1 );
}

TEST_CASE( "Factorials of 1 and higher are computed (pass)", "[single-file]" ) {
    REQUIRE( Factorial(1) == 1 );
    REQUIRE( Factorial(2) == 2 );
    REQUIRE( Factorial(3) == 6 );
    REQUIRE( Factorial(10) == 3628800 );
}