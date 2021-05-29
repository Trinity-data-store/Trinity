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
#define NODE_TYPE uint64_t
// Maximum number of nodes/preorder numbers
#define PREORDER_TYPE uint64_t
#define LEVEL_TYPE uint16_t
#define SYMBOL_TYPE uint64_t
#define DIMENSION_TYPE uint8_t

const DIMENSION_TYPE dimensions = 12;
const SYMBOL_TYPE nBranches = pow(2, dimensions);
#endif

// Depth of the trie (leaves are pointers to treeBlocks)
#define TRIE_DEPTH 3
// Maximum number of nodes in a treeblock
#define MAX_TREE_NODES 256
#define NULL_NODE -1
// MAX_DPETH specifies the range of each dimension (0 to 2^MAX_DEPTH -1)
#define MAX_DEPTH 10

// Struct for each point that we want to insert
typedef struct
{
    uint16_t coordinates[dimensions];
} leafConfig;

// nodeInfo and subtree info are used to obtain subtree size when splitting the treeBlock
struct nodeInfo
{
    PREORDER_TYPE preorder;
    PREORDER_TYPE nChildren;
    nodeInfo(){};
    nodeInfo(PREORDER_TYPE _preorder, PREORDER_TYPE _nChildren)
    {
        preorder = _preorder;
        nChildren = _nChildren;
    };
};

struct subtreeInfo
{
    PREORDER_TYPE preorder;
    PREORDER_TYPE subtreeSize;
    subtreeInfo(){};
    subtreeInfo(PREORDER_TYPE _preorder, PREORDER_TYPE _subtreeSize)
    {
        preorder = _preorder;
        subtreeSize = _subtreeSize;
    };
};

struct trieNode
{
    void *block;
    trieNode *children[nBranches];
};

struct treeBlock
{
    LEVEL_TYPE rootDepth;
    PREORDER_TYPE nNodes;
    PREORDER_TYPE maxNodes;
    bitmap::Bitmap *dfuds;
    void *frontiers;
    PREORDER_TYPE nFrontiers;

    void insert(NODE_TYPE, leafConfig *, LEVEL_TYPE, LEVEL_TYPE, PREORDER_TYPE);
    NODE_TYPE child(treeBlock *&, NODE_TYPE &, SYMBOL_TYPE, LEVEL_TYPE &, PREORDER_TYPE &);
    NODE_TYPE skipChildrenSubtree(NODE_TYPE &, SYMBOL_TYPE, LEVEL_TYPE, PREORDER_TYPE &);
    struct treeBlock *getPointer(PREORDER_TYPE);
    PREORDER_TYPE getPreOrder(PREORDER_TYPE);
    void setPreOrder(PREORDER_TYPE, PREORDER_TYPE);
    void setPointer(PREORDER_TYPE, treeBlock *);
    NODE_TYPE selectSubtree(PREORDER_TYPE &, PREORDER_TYPE &);
};

typedef struct
{
    PREORDER_TYPE preOrder;
    treeBlock *pointer;
} frontierNode;

bool check(trieNode *, leafConfig *, LEVEL_TYPE);
PREORDER_TYPE getChildSkipT(bitmap::Bitmap *, NODE_TYPE, SYMBOL_TYPE);
PREORDER_TYPE getNChildrenT(bitmap::Bitmap *, NODE_TYPE);
void copyNodeCod(bitmap::Bitmap *, bitmap::Bitmap *, NODE_TYPE, NODE_TYPE);
treeBlock *createNewTreeBlock(LEVEL_TYPE, PREORDER_TYPE, PREORDER_TYPE);
trieNode *createNewTrieNode();
void insertar(treeBlock *, leafConfig *, LEVEL_TYPE, LEVEL_TYPE);
void insertTrie(trieNode *, leafConfig *, LEVEL_TYPE);
treeBlock *walkTrie(trieNode *, leafConfig *, LEVEL_TYPE &);
bool walkTree(treeBlock *, leafConfig *, LEVEL_TYPE, LEVEL_TYPE);
SYMBOL_TYPE leafToSymbol(leafConfig *, LEVEL_TYPE);
