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

using namespace std;

const int dimensions = 2;
const int nBranches = (int)pow(2, dimensions);
int8_t childT[nBranches * nBranches][nBranches];
int8_t childSkipT[nBranches * nBranches][nBranches];
uint8_t nChildrenT[nBranches * nBranches];
int8_t insertT[nBranches * nBranches][nBranches];

int8_t stack[100];

struct nodeInfo
{
    uint16_t preorder;
    uint16_t nChildren;
    nodeInfo(){};
    nodeInfo(uint16_t _preorder, uint16_t _nChildren)
    {
        preorder = _preorder;
        nChildren = _nChildren;
    };
};

struct subtreeInfo
{
    uint16_t preorder;
    uint16_t subtreeSize;
    subtreeInfo(){};
    subtreeInfo(uint16_t _preorder, uint16_t _subtreeSize)
    {
        preorder = _preorder;
        subtreeSize = _subtreeSize;
    };
};

nodeInfo stackSS[4096];
subtreeInfo subtrees[4096];
uint16_t depthVector[4096];

#define NODE_TYPE uint16_t
#define L1 2

struct trieNode
{
    void *block;
    trieNode *children[nBranches];
};

NODE_TYPE NULL_NODE = -1;

struct treeBlock
{
    uint8_t rootDepth;
    uint16_t nNodes;
    uint16_t maxNodes;
    bitmap::Bitmap dfuds;
    void *frontiers;
    uint16_t nFrontiers;

    void insert(NODE_TYPE, uint8_t[], uint64_t, uint16_t, uint64_t, uint16_t);
    NODE_TYPE child(treeBlock *&, NODE_TYPE &, uint8_t, uint16_t &, uint16_t, uint16_t &);
    NODE_TYPE skipChildrenSubtree(NODE_TYPE &, uint8_t, uint16_t &, uint16_t, uint16_t &);
    struct treeBlock *getPointer(uint16_t);
    uint16_t getPreOrder(uint16_t);
    void setPreOrder(uint16_t, uint16_t);
    void setPointer(uint16_t, treeBlock *);
    void grow(uint16_t);
    void shrink(uint16_t);
    NODE_TYPE selectSubtree(uint16_t, uint16_t &, uint16_t &);
};

typedef struct
{
    uint16_t preOrder;
    treeBlock *pointer;
} frontierPtr;

#endif
