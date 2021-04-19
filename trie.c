#include "trie.h"
#include "bitmap.h"

uint8_t getNodeCod(bitmap::Bitmap *dfuds, NODE_TYPE node)
{
    return dfuds->GetValPos(node * nBranches, nBranches);
}

void setNodeCod(bitmap::Bitmap *dfuds, NODE_TYPE node, uint8_t nodeCod)
{
    dfuds->SetValPos(node * nBranches, nodeCod, nBranches);
}

treeBlock *treeBlock::getPointer(uint16_t curFrontier)
{
    return ((frontierPtr *)ptr)[curFrontier].pointer;
}

uint16_t treeBlock::getPreOrder(uint16_t curFrontier)
{
    return ((frontierPtr *)ptr)[curFrontier].preOrder;
}

void treeBlock::setPreOrder(uint16_t curFrontier, uint16_t preOrder)
{
    ((frontierPtr *)ptr)[curFrontier].preOrder = preOrder;
}

void printString(uint8_t *str, uint64_t length)
{
    printf("string: ");
    for (int i = 0; i < length; i++)
    {
        printf("%d ", str[i]);
    }
    printf("\n");
}

void printTable(int8_t T[nBranches * nBranches][nBranches])
{
    for (int row = 0; row < nBranches * nBranches; row++)
    {
        for (int col = 0; col < nBranches; col++)
        {
            printf("%d ", T[row][col]);
        }
        printf("\n");
    }
}

void printDFUDS(bitmap::Bitmap *dfuds, uint16_t nNodes)
{
    printf("dfuds: ");
    for (int i = 0; i < nNodes; i++)
    {
        printf("%d, ", getNodeCod(dfuds, i));
    }
    printf("\n");
}

uint8_t symbol2NodeT(uint8_t symbol)
{
    return (uint8_t)1 << (nBranches - symbol - 1);
}

void createNChildrenT()
{
    for (int row = 0; row < nBranches * nBranches; row++)
    {
        int tmp = row;
        int count = 0;
        while (tmp != 0)
        {
            int last_digit = tmp & 0x1;
            if (last_digit == 1)
                count++;
            tmp = tmp >> 1;
        }
        nChildrenT[row] = count;
    }
}

void createChildSkipT()
{
    for (int row = 0; row < nBranches * nBranches; row++)
    {
        int tmp = row;
        int count = 1;
        while (tmp != 0)
        {
            int last_digit = tmp & 0x1;
            childSkipT[row][nBranches - count] = last_digit;
            count++;
            tmp = tmp >> 1;
        }
        int cur_sum = 0;
        for (int col = 0; col < nBranches; col++)
        {
            if (childSkipT[row][col] == 1)
            {
                childSkipT[row][col] = cur_sum;
                cur_sum++;
            }
            else
                childSkipT[row][col] = cur_sum;
        }
    }
}

void createChildT()
{
    for (int row = 0; row < nBranches * nBranches; row++)
    {
        int tmp = row;
        int count = 1;

        while (tmp != 0)
        {
            int last_digit = tmp & 0x1;
            childT[row][nBranches - count] = last_digit;
            count++;
            tmp = tmp >> 1;
        }
        int cur_sum = 0;
        for (int col = 0; col < nBranches; col++)
        {
            if (childT[row][col] == 1)
            {
                cur_sum++;
                childT[row][col] = cur_sum;
            }
            else
            {
                childT[row][col] = -1;
            }
        }
    }
}

void createInsertT()
{
    for (int row = 0; row < nBranches * nBranches; row++)
    {
        for (int col = 0; col < nBranches; col++)
        {
            int col_digit = row >> (nBranches - col - 1) & 1;

            if (col_digit == 1)
                insertT[row][col] = row;
            else
                insertT[row][col] = row + (1 << (nBranches - col - 1));
        }
    }
}

treeBlock *createNewTreeBlock()
{
    treeBlock *tBlock = (treeBlock *)malloc(sizeof(treeBlock));

    bitmap::Bitmap dfuds(2 * sizeof(uint16_t));
    dfuds.SetValPos(0, 0, 2 * sizeof(uint16_t));
    tBlock->dfuds = dfuds;
    tBlock->rootDepth = L1;
    tBlock->nNodes = 1;
    tBlock->ptr = NULL;
    tBlock->nPtrs = 0;
    tBlock->maxNodes = nBranches * nBranches;
    return tBlock;
}

trieNode *createNewTrieNode()
{
    trieNode *tNode = (trieNode *)malloc(sizeof(trieNode));
    for (int i = 0; i < nBranches; i++)
        tNode->children[i] = NULL;

    tNode->block = NULL;
    return tNode;
}

void treeBlock::insert(NODE_TYPE node, uint8_t str[], uint64_t length, uint16_t level, uint64_t maxDepth, uint16_t curFrontier)
{
    NODE_TYPE nodeOriginal = node;
    if (length == 1)
    {
        uint8_t nodeCod = getNodeCod(&dfuds, node);
        setNodeCod(&dfuds, node, nodeCod);
        return;
    }
    else if (nNodes + length - 1 <= maxNodes)
    {
        node = skipChildrenSubtree(node, str[0], level, maxDepth, curFrontier);

        length--;

        NODE_TYPE destNode = nNodes + length - 1;
        NODE_TYPE origNode = nNodes - 1;

        while (origNode >= node)
        {
            uint8_t origNodeCod = getNodeCod(&dfuds, origNode);
            setNodeCod(&dfuds, destNode, origNodeCod);
            destNode--;
            origNode--;
        }

        uint8_t nodeCod = getNodeCod(&dfuds, nodeOriginal);

        setNodeCod(&dfuds, nodeOriginal, insertT[nodeCod][str[0]]);
        origNode++;

        for (uint16_t i = 1; i <= length; i++)
        {
            setNodeCod(&dfuds, origNode, symbol2NodeT(str[i]));
            nNodes++;
            origNode++;
        }

        if (ptr)
            for (uint16_t i = curFrontier; i < nPtrs; ++i)
                setPreOrder(i, getPreOrder(i) + length);
    }
}

NODE_TYPE treeBlock::skipChildrenSubtree(NODE_TYPE &node, uint8_t symbol, uint16_t &curLevel, uint16_t maxLevel, uint16_t &curFrontier)
{
    if (curLevel == maxLevel)
        return node;
    int16_t sTop = -1;

    uint8_t cNodeCod = getNodeCod(&dfuds, node);

    uint8_t skipChild = (uint8_t)childSkipT[cNodeCod][symbol];

    uint8_t nChildren = nChildrenT[cNodeCod];

    uint8_t diff = nChildren - skipChild;

    int8_t stack[100];
    stack[++sTop] = nChildren;

    NODE_TYPE curNode = node + 1;

    if (ptr != NULL && curFrontier < nPtrs && curNode > getPreOrder(curFrontier))
        ++curFrontier;

    uint16_t nextFrontierPreOrder;

    if (nPtrs == 0 || curFrontier >= nPtrs)
        nextFrontierPreOrder = -1;
    else
        nextFrontierPreOrder = getPreOrder(curFrontier);

    ++curLevel;
    while (curNode < nNodes && sTop >= 0 && diff < stack[0])
    {
        if (curNode == nextFrontierPreOrder)
        {
            ++curFrontier;
            if (nPtrs == 0 || curFrontier >= nPtrs)
                nextFrontierPreOrder = -1;
            else
                nextFrontierPreOrder = getPreOrder(curFrontier);
            --stack[sTop];
        }
        else if (curLevel < maxLevel)
        {
            stack[++sTop] = nChildrenT[curNode];
            ++curLevel;
        }
        else
            --stack[sTop];

        ++curNode;
        while (sTop >= 0 && stack[sTop] == 0)
        {
            --sTop;
            --curLevel;
            if (sTop >= 0)
                --stack[sTop];
        }
    }
    return curNode;
}

NODE_TYPE treeBlock::child(treeBlock *&p, NODE_TYPE &node, uint8_t symbol, uint16_t &curLevel, uint16_t maxLevel, uint16_t &curFrontier)
{
    uint8_t cNodeCod = getNodeCod(&dfuds, node);

    uint8_t soughtChild = (uint8_t)childT[cNodeCod][symbol];
    if (soughtChild == (uint8_t)-1)
        return NULL_NODE;

    if (curLevel == maxLevel && soughtChild != (uint8_t)-1)
        return node;

    NODE_TYPE curNode;

    if (ptr != NULL && curFrontier < nPtrs && node == getPreOrder(curFrontier))
    {
        p = getPointer(curFrontier);
        curFrontier = 0;
        NODE_TYPE auxNode = 0;
        curNode = p->skipChildrenSubtree(auxNode, symbol, curLevel, maxLevel, curFrontier);
    }
    else
        curNode = skipChildrenSubtree(node, symbol, curLevel, maxLevel, curFrontier);

    return curNode;
}

void insertar(treeBlock *root, uint8_t *str, uint64_t length, uint16_t level, uint16_t maxDepth)
{
    treeBlock *curBlock = root;
    printDFUDS(&curBlock->dfuds, curBlock->nNodes);
    uint64_t i;

    NODE_TYPE curNodeAux = 0;
    NODE_TYPE curNode = 0;
    uint16_t curFrontier = 0;

    for (i = 0; i < length; i++)
    {
        curNodeAux = curBlock->child(curBlock, curNode, str[i], level, maxDepth, curFrontier);
        if (curNodeAux == (NODE_TYPE)-1)
            break;

        curNode = curNodeAux;

        if (curBlock->nPtrs > 0 && curNode == curFrontier)
        {
            curBlock = curBlock->getPointer(curFrontier);
            curNode = (NODE_TYPE)-1;
        }
    }
    if (length == i)
        return;
    curBlock->insert(curNode, &str[i], length - i, level, maxDepth, curFrontier);
    printDFUDS(&curBlock->dfuds, curBlock->nNodes);
}

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

    printString(str, length);
    printString(&str[i], length - i);
    insertar(tBlock, &str[i], length - i, i, maxDepth);
}

int main()
{
    treeBlock B;
    trieNode *tNode = createNewTrieNode();
    int nStrings = 5;
    const int stringLength = 6;
    uint8_t str[stringLength];
    int maxDepth = 22;

    createChildT();
    createChildSkipT();
    createNChildrenT();
    createInsertT();

    for (int i = 0; i < nStrings; i++)
    {
        scanf("%s", str);
        for (int j = 0; j < stringLength; j++)
        {
            str[j] = ((int)str[j]) % nBranches;
        }
        insertTrie(tNode, str, stringLength, maxDepth);
    }
}