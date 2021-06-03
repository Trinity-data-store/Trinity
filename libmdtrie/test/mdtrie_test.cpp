#include "catch.hpp"
#include "trie.hpp"
#include "tqdm.h"

bool testRandomData(int nPoints)
{
    treeBlock B;
    trieNode *tNode = createNewTrieNode();

    int itr = 0;
    leafConfig *leafPoint = (leafConfig *)malloc(sizeof(leafConfig));

    tqdm bar;
    for (int itr = 1; itr <= nPoints; itr ++){

        bar.progress(itr, nPoints);

        for (int i = 0; i < dimensions; i++){
            leafPoint->coordinates[i] = rand() % nBranches;
        }
        insertTrie(tNode, leafPoint, MAX_DEPTH);
        if (!check(tNode, leafPoint, MAX_DEPTH)){
            raise(SIGINT);
            return false;
        }
    }
    return true;
}

bool testContiguousData(int nPoints)
{
    treeBlock B;
    trieNode *tNode = createNewTrieNode();

    int itr = 0;
    leafConfig *leafPoint = (leafConfig *)malloc(sizeof(leafConfig));

    tqdm bar;
    for (int itr = 1; itr <= nPoints; itr ++){

        bar.progress(itr, nPoints);
        
        SYMBOL_TYPE first_half_value = rand() % nBranches;
        for (int i = 0; i < dimensions / 2; i++){
            leafPoint->coordinates[i] = first_half_value;
        }
        SYMBOL_TYPE second_half_value = rand() % nBranches;
        for (int i = dimensions / 2; i < dimensions; i++){
            leafPoint->coordinates[i] = second_half_value;
        }
        insertTrie(tNode, leafPoint, MAX_DEPTH);
        if (!check(tNode, leafPoint, MAX_DEPTH)){
            raise(SIGINT);
            return false;
        }
    }
    return true;
}

bool testNonexistentData(int nPoints)
{
    treeBlock B;
    trieNode *tNode = createNewTrieNode();

    int itr = 0;
    leafConfig *leafPoint = (leafConfig *)malloc(sizeof(leafConfig));

    tqdm bar;
    for (int itr = 1; itr <= nPoints; itr ++){

        bar.progress(itr, nPoints);

        for (int i = 0; i < dimensions; i++){
            leafPoint->coordinates[i] = rand() % (nBranches / 2);
        }
        insertTrie(tNode, leafPoint, MAX_DEPTH);
    }
    for (int itr = 1; itr <= nPoints; itr ++){

        bar.progress(itr, nPoints);
        for (int i = 0; i < dimensions; i++){
            leafPoint->coordinates[i] = rand() % (nBranches / 2) + nBranches / 2;
        }
        if (check(tNode, leafPoint, MAX_DEPTH)){
            return false;
        }
    }
    return true;
}

TEST_CASE( "Check Random Data Insertion", "[trie]" ) {
    printf("Checking random points: \n");
    REQUIRE(testRandomData(50000));
    printf("done!\n");
}

TEST_CASE( "Check Nonexistent Data", "[trie]" ) {
    printf("Checking nonexistent points: \n");
    REQUIRE(testNonexistentData(10000));
    printf("done!\n");
}

TEST_CASE( "Check Contiguous Data", "[trie]" ) {
    printf("Checking contiguous points: \n");
    REQUIRE(testContiguousData(10000));
    printf("done!\n");
}

