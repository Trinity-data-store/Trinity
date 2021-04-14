#include "trie.h"

// Create a new treeBlock
treeBlock *createNewTreeBlock()
{
    treeBlock *tBlock = (treeBlock *)malloc(sizeof(treeBlock));
    tBlock->dfuds = (uint16_t *)calloc(2, sizeof(uint16_t));
    tBlock->rootDepth = L1;
    tBlock->nNodes = 1;
    tBlock->ptr = NULL;
    tBlock->nPtrs = 0;
    tBlock->maxNodes = 4;
    return tBlock;
}

// Create a new trieNode and set its children to NULL
trieNode *createNewTrieNode()
{
    trieNode *tNode = (trieNode *)malloc(sizeof(trieNode));
    for (int i = 0; i < nBranches; i++)
        tNode->children[i] = NULL;

    tNode->block = NULL;
    return tNode;
}

// Return the pointer to a child block given the curFlag
treeBlock *treeBlock::getPointer(uint16_t curFlag)
{
    return ((blockPtr *)ptr)[curFlag].P;
}

// Return the position of the node in dfuds sequence
uint16_t absolutePosition(treeNode &node)
{
    return nBranches * node.first + node.second;
}

void treeBlock::insert(treeNode node, uint8_t str[], uint64_t length, uint16_t level, uint64_t maxDepth, uint16_t curFlag)
{
}

treeNode treeBlock::child(treeBlock *&p, treeNode &node, uint8_t symbol, uint16_t &curLevel, uint16_t maxLevel, uint16_t &curFlag)
{
    return NULL_NODE;
}

// Insert the remainder of the string into the treeBlock
// str: remaining of the string, length: str's length
// Level: levels traversed in the trie (length of string alraedy processed)
void insertar(treeBlock *root, uint8_t *str, uint64_t length, uint16_t level, uint16_t maxDepth)
{
    treeBlock *curBlock = root;
    uint64_t i;

    treeNode curNode(0, 0), curNodeAux;
    uint16_t curFlag = 0;

    for (i = 0; i < length; ++i)
    {
        curNodeAux = curBlock->child(curBlock, curNode, str[i], level, maxDepth, curFlag);

        if (curNodeAux.first == (NODE_TYPE)-1)
            break;
        else
            curNode = curNodeAux;

        if (curBlock->nPtrs > 0 && absolutePosition(curNode) == curFlag)
        {
            curBlock = curBlock->getPointer(curFlag);
            curNode.first = 0;
            curNode.second = 0;
        }
    }

    curBlock->insert(curNode, &str[i], length - i, level, maxDepth, curFlag);
}

// Insert a string into the trie
// length is the string length
// maxDepth is the max depth of the trie
void insertTrie(trieNode *tNode, uint8_t *str, uint64_t length, uint16_t maxDepth)
{
    uint64_t i = 0;

    while (tNode->children[str[i]])
        tNode = tNode->children[str[i++]];

    while (i < L1)
    {
        tNode->children[str[i]] = createNewTrieNode();
        tNode = tNode->children[str[i]];
        i++;
    }

    treeBlock *tBlock = NULL;
    if (tNode->block == NULL)
    {
        tBlock = createNewTreeBlock();
        tNode->block = tBlock;
    }
    else
        tBlock = (treeBlock *)tNode->block;

    insertar(tBlock, &str[i], length - i, i, maxDepth);
}

int main()
{
    treeBlock B;

    trieNode *tNode = createNewTrieNode();

    int nStrings = 5;
    const int stringLength = 23;
    uint8_t str[stringLength];
    int maxDepth = 22;

    for (int i = 0; i < nStrings; i++)
    {
        scanf("%s\n", str);

        for (int j = 0; j < stringLength; j++)
        {
            str[j] = str[j] % nBranches;
        }
        insertTrie(tNode, str, stringLength, maxDepth);
    }
}