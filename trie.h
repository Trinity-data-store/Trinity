#ifndef TREEBLOCK
#define TREEBLOCK

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <utility>
#include <stdint.h>
#include <time.h>
#include "bitmap.h"
#include <sys/stat.h>

using namespace std;

const int dimensions = 3;
const uint64_t nBranches = (int)pow(2, dimensions);
const uint64_t nNodeConf = (int)pow(2, nBranches);
uint64_t childT[nNodeConf][nBranches];
uint64_t childSkipT[nNodeConf][nBranches];
uint64_t nChildrenT[nNodeConf];
uint64_t insertT[nNodeConf][nBranches];

int8_t stack[100];

struct nodeInfo
{
    uint64_t preorder;
    uint64_t nChildren;
    nodeInfo(){};
    nodeInfo(uint64_t _preorder, uint64_t _nChildren)
    {
        preorder = _preorder;
        nChildren = _nChildren;
    };
};

struct subtreeInfo
{
    uint64_t preorder;
    uint64_t subtreeSize;
    subtreeInfo(){};
    subtreeInfo(uint64_t _preorder, uint64_t _subtreeSize)
    {
        preorder = _preorder;
        subtreeSize = _subtreeSize;
    };
};

nodeInfo stackSS[4096];
subtreeInfo subtrees[4096];
uint64_t depthVector[4096];

#define NODE_TYPE uint64_t
#define L1 0

struct trieNode
{
    void *block;
    trieNode *children[nBranches];
};

NODE_TYPE NULL_NODE = -1;

struct treeBlock
{
    uint16_t rootDepth;
    uint64_t nNodes;
    uint64_t maxNodes;
    bitmap::Bitmap dfuds;
    void *frontiers;
    uint64_t nFrontiers;

    void insert(NODE_TYPE, uint64_t[], uint64_t, uint64_t, uint64_t, uint64_t);
    NODE_TYPE child(treeBlock *&, NODE_TYPE &, uint64_t, uint64_t &, uint64_t, uint64_t &);
    NODE_TYPE skipChildrenSubtree(NODE_TYPE &, uint64_t, uint64_t &, uint64_t, uint64_t &);
    struct treeBlock *getPointer(uint64_t);
    uint64_t getPreOrder(uint64_t);
    void setPreOrder(uint64_t, uint64_t);
    void setPointer(uint64_t, treeBlock *);
    void grow(uint64_t);
    void shrink(uint64_t);
    NODE_TYPE selectSubtree(uint64_t, uint64_t &, uint64_t &);
};

typedef struct
{
    uint64_t preOrder;
    treeBlock *pointer;
} frontierNode;

#endif
