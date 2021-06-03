#include "catch.hpp"
#include "trie.h"
#include "tqdm.h"

bool testRandomData(int nPoints)
{
    treeBlock B;
    trieNode *tNode = createNewTrieNode();

    int itr = 0;
    leafConfig *leafPoint = new leafConfig(dimensions);

    tqdm bar;
    for (int itr = 1; itr <= nPoints; itr ++){

        bar.progress(itr, nPoints);

        for (int i = 0; i < dimensions; i++){
            leafPoint->coordinates[i] = rand() % nBranches;
        }
        insertTrie(tNode, leafPoint, max_depth);
        if (!check(tNode, leafPoint, max_depth)){
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
    leafConfig *leafPoint = new leafConfig(dimensions);

    tqdm bar;
    for (int itr = 1; itr <= nPoints; itr ++){

        bar.progress(itr, nPoints);
        
        symbol_type first_half_value = rand() % nBranches;
        for (int i = 0; i < dimensions / 2; i++){
            leafPoint->coordinates[i] = first_half_value;
        }
        symbol_type second_half_value = rand() % nBranches;
        for (int i = dimensions / 2; i < dimensions; i++){
            leafPoint->coordinates[i] = second_half_value;
        }
        insertTrie(tNode, leafPoint, max_depth);
        if (!check(tNode, leafPoint, max_depth)){
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
    leafConfig *leafPoint = new leafConfig(dimensions);

    tqdm bar;
    for (int itr = 1; itr <= nPoints; itr ++){

        bar.progress(itr, nPoints);

        for (int i = 0; i < dimensions; i++){
            leafPoint->coordinates[i] = rand() % (nBranches / 2);
        }
        insertTrie(tNode, leafPoint, max_depth);
    }
    for (int itr = 1; itr <= nPoints; itr ++){

        bar.progress(itr, nPoints);
        for (int i = 0; i < dimensions; i++){
            leafPoint->coordinates[i] = rand() % (nBranches / 2) + nBranches / 2;
        }
        if (check(tNode, leafPoint, max_depth)){
            return false;
        }
    }
    return true;
}

TEST_CASE( "Check Random Data Insertion", "[trie]" ) {
    printf("Checking random points: \n");
    REQUIRE(testRandomData(10000));
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

