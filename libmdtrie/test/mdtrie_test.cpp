#include "catch.hpp"
#include "trie.h"

bool testRandomData(int nPoints, int dimensions)
{
    symbol_type nBranches = pow(2, dimensions);
    mdTrie *mdtrie = new mdTrie(dimensions);

    int itr = 0;
    leafConfig *leafPoint = new leafConfig(dimensions);

    for (int itr = 1; itr <= nPoints; itr ++){

        for (int i = 0; i < dimensions; i++){
            leafPoint->coordinates[i] = rand() % nBranches;
        }
        mdtrie->insertTrie(leafPoint, max_depth);
        if (!mdtrie->check(leafPoint, max_depth)){
            return false;
        }
    }
    return true;
}

bool testContiguousData(int nPoints, int dimensions)
{
    symbol_type nBranches = pow(2, dimensions);
    mdTrie *mdtrie = new mdTrie(dimensions);

    int itr = 0;
    leafConfig *leafPoint = new leafConfig(dimensions);

    for (int itr = 1; itr <= nPoints; itr ++){
        
        symbol_type first_half_value = rand() % nBranches;
        for (int i = 0; i < dimensions / 2; i++){
            leafPoint->coordinates[i] = first_half_value;
        }
        symbol_type second_half_value = rand() % nBranches;
        for (int i = dimensions / 2; i < dimensions; i++){
            leafPoint->coordinates[i] = second_half_value;
        }
        mdtrie->insertTrie(leafPoint, max_depth);
        if (!mdtrie->check(leafPoint, max_depth)){
            return false;
        }
    }
    return true;
}

bool testNonexistentData(int nPoints, int dimensions)
{
    symbol_type nBranches = pow(2, dimensions);
    mdTrie *mdtrie = new mdTrie(dimensions);

    int itr = 0;
    leafConfig *leafPoint = new leafConfig(dimensions);

    for (int itr = 1; itr <= nPoints; itr ++){

        for (int i = 0; i < dimensions; i++){
            leafPoint->coordinates[i] = rand() % (nBranches / 2);
        }
        mdtrie->insertTrie(leafPoint, max_depth);
    }
    for (int itr = 1; itr <= nPoints; itr ++){

        for (int i = 0; i < dimensions; i++){
            leafPoint->coordinates[i] = rand() % (nBranches / 2) + nBranches / 2;
        }
        if (mdtrie->check(leafPoint, max_depth)){
            return false;
        }
    }
    return true;
}

TEST_CASE( "Check Random Data Insertion", "[trie]" ) {
    REQUIRE(testRandomData(50000, 10));
}

TEST_CASE( "Check Nonexistent Data", "[trie]" ) {
    REQUIRE(testNonexistentData(10000, 10));
}

TEST_CASE( "Check Contiguous Data", "[trie]" ) {
    REQUIRE(testContiguousData(10000, 10));
}

