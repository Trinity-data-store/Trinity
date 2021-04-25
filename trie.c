#include "trie.h"
#include "bitmap.h"

void treeBlock::grow(uint16_t extraNodes)
{
}

void treeBlock::shrink(uint16_t deletedNodes)
{
}

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
    return ((frontierPtr *)frontiers)[curFrontier].pointer;
}

uint16_t treeBlock::getPreOrder(uint16_t curFrontier)
{
    return ((frontierPtr *)frontiers)[curFrontier].preOrder;
}

void treeBlock::setPreOrder(uint16_t curFrontier, uint16_t preOrder)
{
    ((frontierPtr *)frontiers)[curFrontier].preOrder = preOrder;
}

void treeBlock::setPointer(uint16_t curFrontier, treeBlock *pointer)
{
    ((frontierPtr *)frontiers)[curFrontier].pointer = pointer;
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

treeBlock *createNewTreeBlock(uint8_t rootDepth = L1, uint16_t nNodes = 1, uint16_t maxNodes = nBranches * nBranches)
{
    treeBlock *tBlock = (treeBlock *)malloc(sizeof(treeBlock));
    bitmap::Bitmap dfuds(2 * sizeof(uint16_t));
    dfuds.SetValPos(0, 0, 2 * sizeof(uint16_t));
    tBlock->dfuds = dfuds;
    tBlock->rootDepth = rootDepth;
    tBlock->nNodes = nNodes;
    tBlock->frontiers = NULL;
    tBlock->nFrontiers = 0;
    tBlock->maxNodes = maxNodes;
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

NODE_TYPE treeBlock::selectSubtree(uint16_t maxDepth, uint16_t &subTreeSize, uint16_t &depthSelectN)
{
    NODE_TYPE node = 0;

    uint16_t depth;

    uint16_t curFrontier = 0;

    //  Corresponds to stackSS, subtrees, depthVector
    uint16_t ssTop = 0, subtreeTop = 0, depthTop = 0;

    uint8_t cNodeCod = getNodeCod(&dfuds, 0);

    stackSS[ssTop].preorder = 0;
    stackSS[ssTop++].nChildren = nChildrenT[cNodeCod];

    node++;

    depth = rootDepth + 1;

    int32_t nextFrontier;

    if (nFrontiers == 0 || curFrontier >= nFrontiers)
        nextFrontier = -1;
    else
        nextFrontier = getPreOrder(curFrontier);

    for (uint16_t i = 1; i < nNodes; ++i)
    {
        if (i == nextFrontier)
        {
            ++curFrontier;
            if (nFrontiers == 0 || curFrontier >= nFrontiers)
                nextFrontier = -1;
            else
                nextFrontier = getPreOrder(curFrontier);
            --stackSS[ssTop - 1].nChildren;
        }

        else if (depth < maxDepth)
        {
            stackSS[ssTop].preorder = i;
            cNodeCod = getNodeCod(&dfuds, i);
            stackSS[ssTop++].nChildren = nChildrenT[cNodeCod];
            depth++;
        }
        else
            --stackSS[ssTop - 1].nChildren;
        while (ssTop > 0 && stackSS[ssTop - 1].nChildren == 0)
        {
            subtrees[subtreeTop].preorder = stackSS[ssTop - 1].preorder;
            subtrees[subtreeTop++].subtreeSize = i - stackSS[ssTop - 1].preorder + 1;
            --ssTop;
            depthVector[depthTop++] = --depth;
            if (ssTop == 0)
                break;
            else
                stackSS[ssTop - 1].nChildren--;
        }
    }
    int16_t nodemin, min, posmin;

    uint16_t lowerB = 0.25 * nNodes, upperB = 0.75 * nNodes;
    int16_t diff;

    nodemin = subtrees[0].preorder,
    min = nNodes - 2 * subtrees[0].subtreeSize;
    posmin = 0;

    for (uint16_t i = 1; i < subtreeTop; ++i)
    {
        diff = nNodes - 2 * subtrees[i].subtreeSize;
        if (diff < 0)
            diff = -diff;
        if (diff < min)
        {
            min = diff;
            nodemin = subtrees[i].preorder;
            posmin = i;
        }
    }
    subTreeSize = subtrees[posmin].subtreeSize;
    depthSelectN = depthVector[posmin];
    return nodemin;
}

void treeBlock::insert(NODE_TYPE node, uint8_t str[], uint64_t length, uint16_t level, uint64_t maxDepth, uint16_t curFrontier)
{
    NODE_TYPE nodeOriginal = node;
    uint16_t maxBlockSize = 50;
    if (frontiers != NULL && curFrontier < nFrontiers && node == getPreOrder(curFrontier))
    {
        uint8_t nodeCod = getNodeCod(&dfuds, node);
        setNodeCod(&dfuds, node, insertT[nodeCod][str[0]]);
        getPointer(curFrontier)->insert(0, str, length, level, maxDepth, 0);

        return;
    }
    else if (length == 1)
    {
        uint8_t nodeCod = getNodeCod(&dfuds, node);
        setNodeCod(&dfuds, node, insertT[nodeCod][str[0]]);
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

        if (frontiers)
            for (uint16_t i = curFrontier; i < nFrontiers; ++i)
                setPreOrder(i, getPreOrder(i) + length);
    }
    else
    {
        if (nNodes + length - 1 <= maxBlockSize)
        {
            grow(length - 1);
            insert(node, str, length, level, maxDepth, curFrontier);
        }
        else
        {
            uint16_t subTreeSize, depthSelectedNode;
            NODE_TYPE selectedNode = selectSubtree(maxDepth, subTreeSize, depthSelectedNode);
            NODE_TYPE origSelectedNode = selectedNode;

            uint16_t frontier;
            for (frontier = 0; frontier < nFrontiers; frontier++)
                if (getPreOrder(frontier) > selectedNode)
                    break;

            uint16_t frontierSelectedNode = frontier;
            NODE_TYPE insertionNode;
            NODE_TYPE destNode = 0;

            uint16_t copiedNodes = 0;
            bool insertionInNewBlock = false;
            bool isInRoot = false;

            uint16_t newPointerIndex = 0;
            uint16_t copiedFrontier = 0;

            frontierPtr *newPointerArray;
            if (nFrontiers > 0)
                newPointerArray = (frontierPtr *)malloc(sizeof(frontierPtr) * nFrontiers);
            else
                newPointerArray = NULL;

            uint16_t curFrontierNewBlock = 0;
            while (copiedNodes < subTreeSize)
            {
                if (selectedNode == node)
                {
                    insertionInNewBlock = true;
                    if (destNode != 0)
                        insertionNode = destNode;
                    else
                    {
                        insertionNode = node;
                        isInRoot = true;
                    }
                    curFrontierNewBlock = copiedFrontier;
                }

                if (newPointerArray != NULL && frontier < nFrontiers && selectedNode == getPreOrder(frontier))
                {
                    newPointerArray[newPointerIndex].preOrder = destNode;
                    newPointerArray[newPointerIndex].pointer = getPointer(frontier);
                    frontier++;
                    newPointerIndex++;
                    copiedFrontier++;
                }

                setNodeCod(&dfuds, destNode, getNodeCod(&dfuds, selectedNode));
                if (selectedNode != origSelectedNode)
                {
                    setNodeCod(&dfuds, selectedNode, 0);
                }
                selectedNode += 1;
                destNode += 1;
                copiedNodes += 1;
            }

            bool insertionBeforeSelectedTree = true;
            if (!insertionInNewBlock && frontier <= curFrontier)
                insertionBeforeSelectedTree = false;

            treeBlock *new_block = createNewTreeBlock(rootDepth, subTreeSize);

            if (newPointerIndex == 0)
            {
                if (newPointerArray != NULL)
                    free(newPointerArray);

                frontiers = realloc(frontiers, sizeof(frontierPtr) * (nFrontiers + 1));
                for (uint16_t i = nFrontiers; i > frontierSelectedNode; --i)
                {
                    setPointer(i, getPointer(i - 1));
                    setPreOrder(i, getPreOrder(i - 1) - subTreeSize + 1);
                }
            }
            else
            {
                newPointerArray = (frontierPtr *)realloc(newPointerArray, sizeof(frontierPtr) * newPointerIndex);
                new_block->frontiers = newPointerArray;
                new_block->nFrontiers = newPointerIndex;
                for (uint16_t i = frontierSelectedNode + 1; frontier < nFrontiers; ++i, ++frontier)
                {
                    setPointer(i, getPointer(frontier));
                    setPreOrder(i, getPreOrder(frontier) - subTreeSize + 1);
                }
                nFrontiers = nFrontiers - copiedFrontier + 1;
                frontiers = realloc(frontiers, sizeof(frontierPtr) * nFrontiers);
            }
            setPreOrder(frontierSelectedNode, origSelectedNode);
            setPointer(frontierSelectedNode, new_block);

            frontier = frontierSelectedNode + 1;
            origSelectedNode++;

            while (selectedNode < nNodes)
            {
                setNodeCod(&dfuds, origSelectedNode, getNodeCod(&dfuds, selectedNode));
                if (selectedNode == node)
                    insertionNode = origSelectedNode;
                selectedNode++;
                origSelectedNode++;
            }

            if (subTreeSize > length)
                shrink(subTreeSize - 1 - length + 1);
            else
                shrink(subTreeSize - 1);

            nNodes -= (subTreeSize - 1);

            if (!insertionBeforeSelectedTree)
                curFrontier -= copiedFrontier;

            if (insertionInNewBlock && !isInRoot)
            {
                new_block->insert(insertionNode, str, length, level, maxDepth, curFrontierNewBlock);
            }
            else
                insert(insertionNode, str, length, level, maxDepth, curFrontier);
        }
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

    if (frontiers != NULL && curFrontier < nFrontiers && curNode > getPreOrder(curFrontier))
        ++curFrontier;

    uint16_t nextFrontierPreOrder;

    if (nFrontiers == 0 || curFrontier >= nFrontiers)
        nextFrontierPreOrder = -1;
    else
        nextFrontierPreOrder = getPreOrder(curFrontier);

    ++curLevel;
    while (curNode < nNodes && sTop >= 0 && diff < stack[0])
    {
        // printf("inSkipChildren - curNode: %d, stack[0]: %d, stack_top: %d, curLevel: %d\n", curNode, stack[0], stack[sTop], curLevel);
        if (curNode == nextFrontierPreOrder)
        {
            ++curFrontier;
            if (nFrontiers == 0 || curFrontier >= nFrontiers)
                nextFrontierPreOrder = -1;
            else
                nextFrontierPreOrder = getPreOrder(curFrontier);
            --stack[sTop];
        }
        // We don't count the leaf level
        else if (curLevel + 1 < maxLevel)
        {
            cNodeCod = getNodeCod(&dfuds, curNode);
            stack[++sTop] = nChildrenT[cNodeCod];
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
    // printf("skipChildren: curNode[%d], node[%d], symbol: [%d]\n", curNode, node, symbol);
    return curNode;
}

NODE_TYPE treeBlock::child(treeBlock *&p, NODE_TYPE &node, uint8_t symbol, uint16_t &curLevel, uint16_t maxLevel, uint16_t &curFrontier)
{
    // printf("child: node: [%d], symbol: [%d]\n", node, symbol);
    uint8_t cNodeCod = getNodeCod(&dfuds, node);
    uint8_t soughtChild = (uint8_t)childT[cNodeCod][symbol];
    if (soughtChild == (uint8_t)-1)
    {
        return NULL_NODE;
    }
    if (curLevel == maxLevel && soughtChild != (uint8_t)-1)
        return node;

    NODE_TYPE curNode;

    if (frontiers != NULL && curFrontier < nFrontiers && node == getPreOrder(curFrontier))
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

        if (curBlock->nFrontiers > 0 && curNode == curFrontier)
        {
            curBlock = curBlock->getPointer(curFrontier);
            curNode = (NODE_TYPE)-1;
        }
    }
    if (length == i)
        return;
    // printString(str + i, length - i);
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

    insertar(tBlock, &str[i], length - i, i, maxDepth);
}

int main()
{
    treeBlock B;
    trieNode *tNode = createNewTrieNode();
    int maxDepth = 0;

    createChildT();
    createChildSkipT();
    createNChildrenT();
    createInsertT();

    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    FILE *fp = fopen("input.txt", "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);

    while ((read = getline(&line, &len, fp)) != -1)
    {
        char *pos;
        int extra_char = 1;
        if ((pos = strchr(line, '\n')) != NULL)
            *pos = '\0';
        else
            extra_char = 0;
        int strLen = (int)strlen(line) - extra_char;
        printf("Retrieved line of length %d, string: %s\n", strLen, line);

        if (strLen > maxDepth){
            maxDepth = strLen;
        }
        uint8_t str[strLen];
        for (int j = 0; j < strLen; j++)
        {
            str[j] = ((int)line[j]) % nBranches;
        }
        insertTrie(tNode, str, strLen, maxDepth);
        printf("\n");
    }
}