#pragma once
#ifndef TREEBLOCK
#define TREEBLOCK

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <utility>
#include <stdint.h>
#include <time.h>
#include <signal.h>
#include "bitmap.h"
#include <sys/stat.h>

using namespace std;

// Maximum number of bits for node configuration
typedef uint64_t node_type; 
// Maximum number of nodes/preorder numbers
typedef uint64_t preorder_type;
typedef uint16_t level_type;
typedef uint64_t symbol_type;
typedef uint8_t dimension_type;

// Depth of the trie (leaves are pointers to treeBlocks)
const level_type trie_depth = 3;
// Maximum number of nodes in a treeblock
const preorder_type max_tree_nodes = 256;
const preorder_type null_node = -1;
// MAX_DPETH specifies the range of each dimension (0 to 2^MAX_DEPTH -1)
const level_type max_depth = 10;

#endif

// Struct for each point that we want to insert
class leafConfig
{
public:
    uint16_t *coordinates = NULL;
    leafConfig(dimension_type d) {
        coordinates = (uint16_t *)calloc(d, sizeof(uint16_t));
    }
};

// nodeInfo and subtree info are used to obtain subtree size when splitting the treeBlock
class nodeInfo
{
public:
    preorder_type preorder = 0;
    preorder_type nChildren = 0;
    nodeInfo(){};
    nodeInfo(preorder_type _preorder, preorder_type _nChildren)
    {
        preorder = _preorder;
        nChildren = _nChildren;
    };
};

class subtreeInfo
{
public:
    preorder_type preorder = 0;
    preorder_type subtreeSize = 0;
    subtreeInfo(){};
    subtreeInfo(preorder_type _preorder, preorder_type _subtreeSize)
    {
        preorder = _preorder;
        subtreeSize = _subtreeSize;
    };
};

class trieNode
{
public:
    trieNode **children = NULL;
    trieNode(symbol_type nBranches) {
        children = (trieNode **)malloc(nBranches * sizeof(trieNode *));
    }
    void *block;
};

class treeBlock
{
public:
    int dimensions;
    symbol_type nBranches;    
    treeBlock(int d) {
        dimensions = d;
        nBranches = pow(2, d);
    }    

    level_type rootDepth;
    preorder_type nNodes;
    preorder_type maxNodes;
    bitmap::Bitmap *dfuds;
    void *frontiers;
    preorder_type nFrontiers;

    void insert(node_type, leafConfig *, level_type, level_type, preorder_type);
    node_type child(treeBlock *&, node_type &, symbol_type, level_type &, preorder_type &);
    node_type skipChildrenSubtree(node_type &, symbol_type, level_type, preorder_type &);
    struct treeBlock *getPointer(preorder_type);
    preorder_type getPreOrder(preorder_type);
    void setPreOrder(preorder_type, preorder_type);
    void setPointer(preorder_type, treeBlock *);
    node_type selectSubtree(preorder_type &, preorder_type &);
};

class frontierNode
{
public:
    preorder_type preOrder;
    treeBlock *pointer;
};

class mdTrie
{
public:
    int dimensions;
    symbol_type nBranches;
    trieNode *root = NULL;
    mdTrie(int d) {
        dimensions = d;
        nBranches = pow(2, d);
    }    
    bool check(leafConfig *, level_type);
    void insertar(treeBlock *, leafConfig *, level_type, level_type);
    void insertTrie(leafConfig *, level_type);
    treeBlock *walkTrie(trieNode *, leafConfig *, level_type &);
    bool walkTree(treeBlock *, leafConfig *, level_type, level_type);
    trieNode *createNewTrieNode();
};

// Todo: better arrangement of these functions
// See how createNewTrieNode is in a different place than createNewTreeBlock
treeBlock *createNewTreeBlock(level_type, preorder_type, preorder_type, int);
symbol_type leafToSymbol(leafConfig *, level_type, int);
preorder_type getChildSkipT(bitmap::Bitmap *, node_type, symbol_type, symbol_type);
preorder_type getNChildrenT(bitmap::Bitmap *, node_type, symbol_type);
void copyNodeCod(bitmap::Bitmap *, bitmap::Bitmap *, node_type, node_type, symbol_type);
