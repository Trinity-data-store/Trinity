#ifndef TREEBLOCK
#define TREEBLOCK

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <utility>
#include <stdint.h>
#include <time.h>

using namespace std;

const int dimensions = 2;
const int nBranches = (int)pow(2, dimensions);

#define NODE_TYPE uint16_t
#define OFFSET_TYPE uint8_t

#define L1 8

struct trieNode
{
    void *block;
    trieNode *children[nBranches];
};

typedef pair<NODE_TYPE, OFFSET_TYPE> treeNode;
const treeNode NULL_NODE = treeNode((NODE_TYPE)-1, 0);

struct treeBlock
{
    uint8_t rootDepth;
    uint16_t nNodes;
    uint16_t maxNodes;
    uint16_t *dfuds;
    void *ptr;
    uint16_t nPtrs;

    void insert(treeNode, uint8_t[], uint64_t, uint16_t, uint64_t, uint16_t);
    treeNode child(treeBlock *&, treeNode &, uint8_t, uint16_t &, uint16_t, uint16_t &);
    struct treeBlock *getPointer(uint16_t);
};

typedef struct
{
    uint16_t flag;
    treeBlock *P;
} blockPtr;

#endif
