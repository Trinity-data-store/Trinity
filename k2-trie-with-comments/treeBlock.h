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


// A node within a block represented as a pair <index of the short int that contains the node, offset of the node within the short int>. The offset is 0, 1, 2, or 3

#define NODE_TYPE uint16_t
#define OFFSET_TYPE uint8_t
#define MAX_UINT_16 65535

  // Global table that stores the sizes of the blocks, used for growing them
  uint16_t *sizeArray; // array of sizes

  // Minimum and maximum block sizes
  uint16_t N1;
  uint16_t Nt;

#define S1 64
#define S2 128
#define S3 1024
#define L1 8
#define L2 16


struct trieNode
 {
    void *block;
    trieNode *children[4];
 };


typedef std::pair <NODE_TYPE,OFFSET_TYPE> treeNode;

uint16_t absolutePosition(treeNode &node);

struct treeBlock
 {
    uint8_t rootDepth;  // depth of the root of the block    
    
    uint16_t nNodes;  // number of nodes in the tree block
    
    uint16_t maxNodes; // maximum number of nodes in the block.
                       // Also, it is the size of the next array.
    
    uint16_t *dfuds;  // DFUDS of the tree block
    
    void /*blockPTR*/ *ptr;    // Pointers to child blocks 
    
    uint16_t nPtrs;   // number of pointers to child blocks
 
 
    void insert(treeNode, uint8_t[], uint64_t, uint16_t, uint64_t, uint16_t);
    
    treeNode skipChildrenSubtree(treeNode &, uint8_t, uint16_t &, uint16_t, uint16_t &);
    
    treeNode child(treeBlock *&, treeNode &, uint8_t, uint16_t &, uint16_t, uint16_t &);
    
    void grow(uint16_t extraNodes);
    
    void shrink(uint16_t deletedNodes);
    
    treeNode selectSubtree(uint16_t maxDepth, uint16_t & subTreeSize, uint16_t & depthSelectN);

    treeNode selectSubtree2(uint16_t maxDepth, uint16_t & subTreeSize, uint16_t & depthSelectN);
        
    struct treeBlock *getPointer(uint16_t);
    
    uint64_t size();

    void freeTreeBlock();   
    
    ~treeBlock(){;};
 };
 
 
typedef struct
 {
    uint16_t flag;   // node that owns the pointer to child block
    treeBlock *P;
 } blockPtr;



/* Auxiliary tables */

/* Table used for insertion. Represents the DFUDS patterns using nibbles
within a byte*/

const uint8_t nibblePattern[2][4] = {{0x80,0x40,0x20,0x10},
                               {0x08,0x04,0x02,0x01}};

/*Given a node and a symbol, gives the rank of the symbol within the children of the node.
  If the children does not exist, returns -1 */

const int8_t childT[16][4] = {
/*0000*/ {-1, -1, -1, -1}, 
/*0001*/ {-1, -1, -1,  1},
/*0010*/ {-1, -1,  1, -1},
/*0011*/ {-1, -1,  1,  2},
/*0100*/ {-1,  1, -1, -1},
/*0101*/ {-1,  1, -1,  2},
/*0110*/ {-1,  1,  2, -1},
/*0111*/ {-1,  1,  2,  3},
/*1000*/ { 1, -1, -1, -1}, 
/*1001*/ { 1, -1, -1,  2}, 
/*1010*/ { 1, -1,  2, -1},
/*1011*/ { 1, -1,  2,  3},
/*1100*/ { 1,  2, -1, -1},
/*1101*/ { 1,  2, -1,  3},
/*1110*/ { 1,  2,  3, -1},
/*1111*/ { 1,  2,  3,  4}
};


/*initial mask to obtain the binary code of a node according to its offset within a short int*/
const uint16_t maskInitT[4] = {0xf000,0x0f00,0x00f0,0x000f};

const uint8_t firstChildT[16] = {(uint8_t)-1,1,2,1,3,1,2,1,4,1,2,1,3,1,2,1};

const uint8_t nChildrenT[16] = {0,1,1,2,1,2,2,3,1,2,2,3,2,3,3,4};

const uint16_t shiftT[4] = {12,8,4,0};

// Given the code of a node and a symbol to insert, gives the insertion rank of the symbol

const int8_t childRankT[16][4] = {
/*0000*/ { 1,  1,  1,  1}, 
/*0001*/ { 1,  1,  1, -1},
/*0010*/ { 1,  2, -1,  3},
/*0011*/ { 1,  2, -1, -1},
/*0100*/ { 1, -1,  2,  3},
/*0101*/ { 1, -1,  2, -1},
/*0110*/ { 1, -1, -1,  2},
/*0111*/ { 1, -1, -1, -1},
/*1000*/ {-1,  1,  2,  3}, 
/*1001*/ {-1,  1,  2, -1}, 
/*1010*/ {-1,  1, -1,  2},
/*1011*/ {-1,  1, -1, -1},
/*1100*/ {-1, -1,  1,  2},
/*1101*/ {-1, -1,  1, -1},
/*1110*/ {-1, -1, -1,  1},
/*1111*/ {-1, -1, -1, -1}
};


// given a node and a symbol, this table says how many subtrees of the given node must be skipped to obtain the corresponding child subtree

const int8_t childSkipT[16][4] = {
/*0000*/ { 0,  0,  0,  0}, 
/*0001*/ { 0,  0,  0,  0},
/*0010*/ { 0,  0,  0,  1},
/*0011*/ { 0,  0,  0,  1},
/*0100*/ { 0,  0,  1,  1},
/*0101*/ { 0,  0,  1,  1},
/*0110*/ { 0,  0,  1,  2},
/*0111*/ { 0,  0,  1,  2},
/*1000*/ { 0,  1,  1,  1}, 
/*1001*/ { 0,  1,  1,  1}, 
/*1010*/ { 0,  1,  1,  2},
/*1011*/ { 0,  1,  1,  2},
/*1100*/ { 0,  1,  2,  2},
/*1101*/ { 0,  1,  2,  2},
/*1110*/ { 0,  1,  2,  3},
/*1111*/ { 0,  1,  2,  3}
};


// given a symbol in {0, 1, 2, 3}, yields a unary node representing that symbol
const int8_t symbol2NodeT[4] = {0x8,0x4,0x2,0x1};

const int8_t insertT[16][4] = {
/*0000*/ {0x8, 0x4, 0x2, 0x1}, 
/*0001*/ {0x9, 0x5, 0x3, 0x1},
/*0010*/ {0xa, 0x6, 0x2, 0x3},
/*0011*/ {0xb, 0x7, 0x3, 0x3},
/*0100*/ {0xc, 0x4, 0x6, 0x5},
/*0101*/ {0xd, 0x5, 0x7, 0x5},
/*0110*/ {0xe, 0x6, 0x6, 0x7},
/*0111*/ {0xf, 0x7, 0x7, 0x7},
/*1000*/ {0x8, 0xc, 0xa, 0x9}, 
/*1001*/ {0x9, 0xd, 0xb, 0x9}, 
/*1010*/ {0xa, 0xe, 0xa, 0xb},
/*1011*/ {0xb, 0xf, 0xb, 0xb},
/*1100*/ {0xc, 0xc, 0xe, 0xd},
/*1101*/ {0xd, 0xd, 0xf, 0xd},
/*1110*/ {0xe, 0xe, 0xe, 0xf},
/*1111*/ {0xf, 0xf, 0xf, 0xf}
};

#endif
