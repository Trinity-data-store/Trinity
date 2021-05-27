#include "trie.hpp"

void copyNodeCod(bitmap::Bitmap *from_dfuds, bitmap::Bitmap *to_dfuds, NODE_TYPE from, NODE_TYPE to)
{
    int visited = 0;
    while (visited < nBranches){
        if (nBranches - visited > 64){
            to_dfuds->SetValPos(to * nBranches + visited, from_dfuds->GetValPos(from * nBranches + visited, 64), 64);
            visited += 64;
        }
        else {
            int left = nBranches - visited;
            to_dfuds->SetValPos(to * nBranches + visited, from_dfuds->GetValPos(from * nBranches + visited, left), left);
            break;
        }
    }
}

treeBlock *treeBlock::getPointer(uint64_t curFrontier)
{
    return ((frontierNode *)frontiers)[curFrontier].pointer;
}

uint64_t treeBlock::getPreOrder(uint64_t curFrontier)
{
    return ((frontierNode *)frontiers)[curFrontier].preOrder;
}

void treeBlock::setPreOrder(uint64_t curFrontier, uint64_t preOrder)
{
    ((frontierNode *)frontiers)[curFrontier].preOrder = preOrder;
}

void treeBlock::setPointer(uint64_t curFrontier, treeBlock *pointer)
{
    ((frontierNode *)frontiers)[curFrontier].pointer = pointer;
}

uint64_t getNChildrenT(bitmap::Bitmap *dfuds, NODE_TYPE node){

    return dfuds->popcount(node * nBranches, nBranches);
}

uint16_t getChildSkipT(bitmap::Bitmap *dfuds, NODE_TYPE node, int col)
{
    return dfuds->popcount(node * nBranches, col);
}

treeBlock *createNewTreeBlock(uint64_t rootDepth, uint64_t nNodes, uint64_t maxNodes)
{
    treeBlock *tBlock = (treeBlock *)malloc(sizeof(treeBlock));
    tBlock->dfuds = new bitmap::Bitmap((maxNodes + 1) * nBranches);
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

NODE_TYPE treeBlock::selectSubtree(uint64_t maxDepth, uint64_t &subTreeSize, uint64_t &depthSelectN)
{
    // index -> Number of children & preorder
    nodeInfo stackSS[4096];
    // index -> size of subtree & preorder
    subtreeInfo subtrees[4096];
    // Index -> depth of the node
    uint64_t depthVector[4096];

    //  Corresponds to stackSS, subtrees, depthVector
    uint64_t ssTop = 0, subtreeTop = 0, depthTop = 0;

    NODE_TYPE node = 0;

    uint64_t depth;

    uint64_t curFrontier = 0;

    stackSS[ssTop].preorder = 0;
    stackSS[ssTop++].nChildren = getNChildrenT(dfuds, 0);

    node++;

    depth = rootDepth + 1;

    uint64_t nextFrontierPreOrder;

    if (nFrontiers == 0 || curFrontier >= nFrontiers)
        nextFrontierPreOrder = -1;
    else
        nextFrontierPreOrder = getPreOrder(curFrontier);

    for (uint64_t i = 1; i < nNodes; ++i)
    {
        // If meet a frontier node
        if (i == nextFrontierPreOrder)
        {
            ++curFrontier;
            if (nFrontiers == 0 || curFrontier >= nFrontiers)
                nextFrontierPreOrder = -1;
            else
                nextFrontierPreOrder = getPreOrder(curFrontier);
            --stackSS[ssTop - 1].nChildren;
        }
        //  Start searching for its children
        else if (depth < maxDepth - 1)
        {
            stackSS[ssTop].preorder = i;
            stackSS[ssTop++].nChildren = getNChildrenT(dfuds, i);
            depth++;
        }
        //  Reached the maxDepth level
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
     // Now I have to go through the subtrees vector to choose the proper subtree
    uint64_t nodemin, min, posmin;

    uint64_t diff;

    nodemin = subtrees[0].preorder,
    min = nNodes - 2 * subtrees[0].subtreeSize;
    posmin = 0;

    for (uint64_t i = 1; i < subtreeTop; ++i)
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

void treeBlock::insert(NODE_TYPE node, uint64_t str[], uint64_t length, uint64_t level, uint64_t maxDepth, uint64_t curFrontier)
{
    if (length == 0){
        return;
    }
    NODE_TYPE nodeOriginal = node;

    //  node is a frontier node
    if (frontiers != NULL && curFrontier < nFrontiers && node == getPreOrder(curFrontier))
    {
        dfuds->SetBit(node * nBranches + str[0]);
        getPointer(curFrontier)->insert(0, str, length, level, maxDepth, 0);

        return;
    }
    //  If there is only one character left
    //  Insert that character into the correct position
    else if (length == 1)
    {
        dfuds->SetBit(node * nBranches + str[0]);
        return;
    }
    // there is room in current block for new nodes
    else if (nNodes + length - 1 <= maxNodes)
    {
        // skipChildrenSubtree returns the position under node where the new str[0] will be inserted
        node = skipChildrenSubtree(node, str[0], level, maxDepth, curFrontier);
        length--;

        NODE_TYPE destNode = nNodes + length - 1;
        NODE_TYPE origNode = nNodes - 1;

        //  In this while loop, we are making space for str
        //  By shifting nodes to the right of str[i] by len(str) spots
        while (origNode >= node)
        {
            copyNodeCod(dfuds, dfuds, origNode, destNode);
            destNode--;
            origNode--;
        }
        
        dfuds->SetBit(nodeOriginal * nBranches + str[0]);
        origNode++;

        //  Insert all remaining characters (Remember length -- above)
        for (uint64_t i = 1; i <= length; i++)
        {
            dfuds->ClearWidth(origNode * nBranches, nBranches);
            dfuds->SetBit(origNode * nBranches + str[i]);
            nNodes++;
            origNode++;
        }
        // shift the flags by length since all nodes have been shifted by that amount
        if (frontiers != NULL)
            for (uint64_t i = curFrontier; i < nFrontiers; ++i)
                setPreOrder(i, getPreOrder(i) + length);
    }
    else
    {
        uint64_t subTreeSize, depthSelectedNode;
        NODE_TYPE selectedNode = selectSubtree(maxDepth, subTreeSize, depthSelectedNode);

        NODE_TYPE origSelectedNode = selectedNode;

        bitmap::Bitmap *new_dfuds = new bitmap::Bitmap((maxNodes + 1) * nBranches);

        uint64_t frontier;
        //  Find the first frontier node > selectedNode
        for (frontier = 0; frontier < nFrontiers; frontier++)
            if (getPreOrder(frontier) > selectedNode)
                break;

        uint64_t frontierSelectedNode = frontier;
        NODE_TYPE insertionNode = node;

        NODE_TYPE destNode = 0;
        uint64_t copiedNodes = 0, copiedFrontier = 0;

        bool insertionInNewBlock = false;
        bool isInRoot = false;

        uint64_t newPointerIndex = 0;

        frontierNode *newPointerArray = NULL;
        if (nFrontiers > 0)
        {
            newPointerArray = (frontierNode *)malloc(sizeof(frontierNode) * (nFrontiers + 5));
        }
        uint64_t curFrontierNewBlock = 0;

        //  Copy all nodes of the subtree to the new block
        while (copiedNodes < subTreeSize)
        {
            //  If we meet the current node (from which we want to do insertion)
            // insertionNode is the new preorder in new block where we want to insert a node
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
            // If we see a frontier node, copy pointer to the new block
            if (newPointerArray != NULL && frontier < nFrontiers && selectedNode == getPreOrder(frontier))
            {
                newPointerArray[newPointerIndex].preOrder = destNode;
                newPointerArray[newPointerIndex].pointer = getPointer(frontier);
                frontier++;
                newPointerIndex++;
                copiedFrontier++;
            }
            copyNodeCod(dfuds, new_dfuds, selectedNode, destNode);

            selectedNode += 1;
            destNode += 1;
            copiedNodes += 1;
        }

        bool insertionBeforeSelectedTree = true;
        if (!insertionInNewBlock && frontier <= curFrontier)
            insertionBeforeSelectedTree = false;

        treeBlock *new_block = createNewTreeBlock(depthSelectedNode, subTreeSize);
        // Memory leak
        new_block->dfuds = new_dfuds;

        //  If no pointer is copied to the new block
        if (newPointerIndex == 0)
        {
            if (newPointerArray != NULL)
                free(newPointerArray);

            // Expand frontiers array to add one more frontier node
            frontiers = realloc(frontiers, sizeof(frontierNode) * (nFrontiers + 1));
            // Shift right one spot to move the pointers from flagSelectedNode + 1 to nPtrs
            for (uint64_t i = nFrontiers; i > frontierSelectedNode; --i)
            {
                setPointer(i, getPointer(i - 1));
                setPreOrder(i, getPreOrder(i - 1) - subTreeSize + 1);
            }
            //  Insert that new frontier node
            setPreOrder(frontierSelectedNode, origSelectedNode);
            setPointer(frontierSelectedNode, new_block);
            nFrontiers ++;
        }
        else
        {
            //  If there are pointers copied to the new block
            newPointerArray = (frontierNode *)realloc(newPointerArray, sizeof(frontierNode) * (newPointerIndex));

            new_block->frontiers = newPointerArray;
            new_block->nFrontiers = newPointerIndex;
            setPreOrder(frontierSelectedNode, origSelectedNode);
            setPointer(frontierSelectedNode, new_block);

            for (uint64_t i = frontierSelectedNode + 1; frontier < nFrontiers; i++, frontier++)
            {
                setPointer(i, getPointer(frontier));
                setPreOrder(i, getPreOrder(frontier) - subTreeSize + 1);
            }
            nFrontiers = nFrontiers - copiedFrontier + 1;
            frontiers = realloc(frontiers, sizeof(frontierNode) * (nFrontiers));

        }

        // Now, delete the subtree copied to the new block
        frontier = frontierSelectedNode + 1;
        origSelectedNode++;

        while (selectedNode < nNodes)
        {
            // selectedNode is the immediate node after the copied block
            // origSelectedNode is the original node where we want to turn into a frontier node
            // node is the node where we want to insert the symbol str[0]
            copyNodeCod(dfuds, dfuds, selectedNode, origSelectedNode);
            if (selectedNode == node)
                insertionNode = origSelectedNode;
            selectedNode++;
            origSelectedNode++;
        }
        nNodes -= (subTreeSize - 1);

        if (!insertionBeforeSelectedTree)
            curFrontier -= copiedFrontier;

        // If the insertion continues in the new block  
        if (insertionInNewBlock)
        {
            if (isInRoot){
                insert(insertionNode, str, length, level, maxDepth, curFrontier);
            }
            else {
                new_block->insert(insertionNode, str, length, level, maxDepth, curFrontierNewBlock);
            }
        }
        // If the insertion is in the old block
        else
        {
            insert(insertionNode, str, length, level, maxDepth, curFrontier);
        }
    }
}

NODE_TYPE treeBlock::skipChildrenSubtree(NODE_TYPE &node, uint64_t symbol, uint64_t &curLevel, uint64_t maxLevel, uint64_t &curFrontier)
{
    if (curLevel == maxLevel)
        return node;
    uint64_t sTop = -1;
    
    uint64_t skipChild = getChildSkipT(dfuds, node, symbol);

    uint64_t nChildren = getNChildrenT(dfuds, node);

    uint64_t diff = nChildren - skipChild;

    uint64_t stack[100];
    stack[++sTop] = nChildren;

    NODE_TYPE curNode = node + 1;

    if (frontiers != NULL && curFrontier < nFrontiers && curNode > getPreOrder(curFrontier))
        ++curFrontier;

    uint64_t nextFrontierPreOrder;

    if (nFrontiers == 0 || curFrontier >= nFrontiers)
        nextFrontierPreOrder = -1;
    else
        nextFrontierPreOrder = getPreOrder(curFrontier);

    ++curLevel;
    while (curNode < nNodes && sTop >= 0 && diff < stack[0])
    {
        if (curNode == nextFrontierPreOrder)
        {
            ++curFrontier;
            if (nFrontiers == 0 || curFrontier >= nFrontiers)
                nextFrontierPreOrder = -1;
            else
                nextFrontierPreOrder = getPreOrder(curFrontier);
            --stack[sTop];
        }
        // curLevel is 0th indexed.
        else if (curLevel < maxLevel - 1)
        {
            stack[++sTop] = getNChildrenT(dfuds, curNode);
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

NODE_TYPE treeBlock::child(treeBlock *&p, NODE_TYPE &node, uint64_t symbol, uint64_t &curLevel, uint64_t maxLevel, uint64_t &curFrontier)
{

    bool hasChild = (bool) dfuds->GetBit(node * nBranches + symbol);

    if (!hasChild)
        return NULL_NODE;

    if (curLevel == maxLevel && hasChild)
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

void insertar(treeBlock *root, uint64_t *str, uint64_t length, uint64_t level, uint64_t maxDepth)
{
    treeBlock *curBlock = root;
    uint64_t i = 0;

    NODE_TYPE curNode = 0;
    uint64_t curFrontier = 0;

//  **************************
    NODE_TYPE curNodeAux = 0;

    while (i < length)
    {
        curNodeAux = curBlock->child(curBlock, curNode, str[i], level, maxDepth, curFrontier);
        if (curNodeAux == (NODE_TYPE)-1)
            break;

        curNode = curNodeAux;

        if (curBlock->nFrontiers > 0 && curFrontier < curBlock->nFrontiers && curNode == curBlock->getPreOrder(curFrontier))
        {
            curBlock = curBlock->getPointer(curFrontier);
            curNode = (NODE_TYPE)0;
            curFrontier = 0;
        }
        i++;
    }
//  *********************
    // walkTree(curBlock, str, length, maxDepth, curNode, i, level, curFrontier); 
    curBlock->insert(curNode, &str[i], length - i, level, maxDepth, curFrontier);
}


bool walkTree(treeBlock *curBlock, uint64_t str[], int strlen, int maxDepth, NODE_TYPE &curNode, uint64_t &i, uint64_t &level, uint64_t &curFrontier){

    NODE_TYPE curNodeAux = 0;

    while (i < strlen)
    {
        curNodeAux = curBlock->child(curBlock, curNode, str[i], level, maxDepth, curFrontier);

        if (curNodeAux == (NODE_TYPE)-1)
            return false;

        curNode = curNodeAux;

        if (curBlock->nFrontiers > 0 && curFrontier < curBlock->nFrontiers && curNode == curBlock->getPreOrder(curFrontier))
        {
            curBlock = curBlock->getPointer(curFrontier);
            curNode = (NODE_TYPE)0;
            curFrontier = 0;
        }
        i++;
    }
    return true;        
}

treeBlock *walkTrie(trieNode *tNode, uint64_t *str, uint64_t &i){

    while (tNode->children[str[i]])
        tNode = tNode->children[str[i++]];
    while (i < TRIE_DEPTH)
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
    return tBlock;    
}

void insertTrie(trieNode *tNode, uint64_t *str, uint64_t length, uint64_t maxDepth)
{
    uint64_t i = 0;
    
    treeBlock *tBlock = walkTrie(tNode, str, i);
    
    insertar(tBlock, &str[i], length - i, i, maxDepth);
}

uint64_t *proc_str(char *line, int &strLen, int &maxDepth){
    char *pos;
    int extra_char = 1;
    if ((pos = strchr(line, '\n')) != NULL)
        *pos = '\0';
    else
        extra_char = 0;
    strLen = (int)strlen(line) - extra_char;

    if (strLen > maxDepth){
        maxDepth = strLen;
    }
    uint64_t *str = (uint64_t *)malloc(sizeof(uint64_t) * strLen);
    
    for (int j = 0; j < strLen; j++)
    {
        str[j] = ((int)line[j]) % nBranches;
        if (str[j] == 64){
            printf("wtffff\n");
        }
    }
    return str;
}