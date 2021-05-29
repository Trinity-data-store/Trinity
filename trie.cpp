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

treeBlock *treeBlock::getPointer(PREORDER_TYPE curFrontier)
{
    return ((frontierNode *)frontiers)[curFrontier].pointer;
}

PREORDER_TYPE treeBlock::getPreOrder(PREORDER_TYPE curFrontier)
{
    return ((frontierNode *)frontiers)[curFrontier].preOrder;
}

void treeBlock::setPreOrder(PREORDER_TYPE curFrontier, PREORDER_TYPE preOrder)
{
    ((frontierNode *)frontiers)[curFrontier].preOrder = preOrder;
}

void treeBlock::setPointer(PREORDER_TYPE curFrontier, treeBlock *pointer)
{
    ((frontierNode *)frontiers)[curFrontier].pointer = pointer;
}

PREORDER_TYPE getNChildrenT(bitmap::Bitmap *dfuds, NODE_TYPE node){

    return dfuds->popcount(node * nBranches, nBranches);
}

PREORDER_TYPE getChildSkipT(bitmap::Bitmap *dfuds, NODE_TYPE node, SYMBOL_TYPE col)
{
    return dfuds->popcount(node * nBranches, col);
}

treeBlock *createNewTreeBlock(LEVEL_TYPE rootDepth, PREORDER_TYPE nNodes, PREORDER_TYPE maxNodes)
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

// This function selectes the subTree starting from node 0
// The selected subtree has the maximum subtree size
NODE_TYPE treeBlock::selectSubtree(PREORDER_TYPE &subTreeSize, PREORDER_TYPE &depthSelectN)
{
    // index -> Number of children & preorder
    nodeInfo stackSS[4096];
    // index -> size of subtree & preorder
    subtreeInfo subtrees[4096];
    // Index -> depth of the node
    PREORDER_TYPE depthVector[4096];

    //  Corresponds to stackSS, subtrees, depthVector
    PREORDER_TYPE ssTop = 0, subtreeTop = 0, depthTop = 0;
    NODE_TYPE node = 0;
    PREORDER_TYPE depth;
    PREORDER_TYPE curFrontier = 0;

    stackSS[ssTop].preorder = 0;
    stackSS[ssTop++].nChildren = getNChildrenT(dfuds, 0);
    node++;
    depth = rootDepth + 1;
    PREORDER_TYPE nextFrontierPreOrder;

    if (nFrontiers == 0 || curFrontier >= nFrontiers)
        nextFrontierPreOrder = -1;
    else
        nextFrontierPreOrder = getPreOrder(curFrontier);

    for (PREORDER_TYPE i = 1; i < nNodes; ++i)
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
        else if (depth < MAX_DEPTH - 1)
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
    PREORDER_TYPE nodemin, min, posmin;
    PREORDER_TYPE diff;

    nodemin = subtrees[0].preorder,
    min = nNodes - 2 * subtrees[0].subtreeSize;
    posmin = 0;

    for (PREORDER_TYPE i = 1; i < subtreeTop; ++i)
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

// This function inserts the string at the node position
void treeBlock::insert(NODE_TYPE node, leafConfig *leafPoint, LEVEL_TYPE level, LEVEL_TYPE length, PREORDER_TYPE curFrontier)
{
    if (level == length){
        return;
    }
    NODE_TYPE nodeOriginal = node;

    //  node is a frontier node
    if (frontiers != NULL && curFrontier < nFrontiers && node == getPreOrder(curFrontier))
    {
        dfuds->SetBit(node * nBranches + leafToSymbol(leafPoint, level));
        getPointer(curFrontier)->insert(0, leafPoint, level, length, 0);

        return;
    }
    //  If there is only one character left
    //  Insert that character into the correct position
    else if (length == 1)
    {
        dfuds->SetBit(node * nBranches + leafToSymbol(leafPoint, level));
        return;
    }
    // there is room in current block for new nodes
    else if (nNodes + (length - level) - 1 <= maxNodes)
    {
        // skipChildrenSubtree returns the position under node where the new str[0] will be inserted
        node = skipChildrenSubtree(node, leafToSymbol(leafPoint, level), level, curFrontier);
        
        NODE_TYPE destNode = nNodes + (length - level) - 2;
        NODE_TYPE origNode = nNodes - 1;

        //  In this while loop, we are making space for str
        //  By shifting nodes to the right of str[i] by len(str) spots
        while (origNode >= node)
        {
            copyNodeCod(dfuds, dfuds, origNode, destNode);
            destNode--;
            origNode--;
        }
        
        dfuds->SetBit(nodeOriginal * nBranches + leafToSymbol(leafPoint, level));
        level++;
        origNode++;
        //  Insert all remaining characters (Remember length -- above)
        for (LEVEL_TYPE i = level; i < length; i++)
        {
            dfuds->ClearWidth(origNode * nBranches, nBranches);
            dfuds->SetBit(origNode * nBranches + leafToSymbol(leafPoint, i));
            nNodes++;
            origNode++;
        }
        // shift the flags by length since all nodes have been shifted by that amount
        if (frontiers != NULL)
            for (PREORDER_TYPE j = curFrontier; j < nFrontiers; ++j)
                setPreOrder(j, getPreOrder(j) + length - level);
    }
    else
    {
        PREORDER_TYPE subTreeSize, depthSelectedNode;
        NODE_TYPE selectedNode = selectSubtree(subTreeSize, depthSelectedNode);
        NODE_TYPE origSelectedNode = selectedNode;
        bitmap::Bitmap *new_dfuds = new bitmap::Bitmap((maxNodes + 1) * nBranches);

        PREORDER_TYPE frontier;
        //  Find the first frontier node > selectedNode
        for (frontier = 0; frontier < nFrontiers; frontier++)
            if (getPreOrder(frontier) > selectedNode)
                break;

        PREORDER_TYPE frontierSelectedNode = frontier;
        NODE_TYPE insertionNode = node;

        NODE_TYPE destNode = 0;
        PREORDER_TYPE copiedNodes = 0, copiedFrontier = 0;

        bool insertionInNewBlock = false;
        bool isInRoot = false;

        PREORDER_TYPE newPointerIndex = 0;

        frontierNode *newPointerArray = NULL;
        if (nFrontiers > 0)
        {
            newPointerArray = (frontierNode *)malloc(sizeof(frontierNode) * (nFrontiers + 5));
        }
        PREORDER_TYPE curFrontierNewBlock = 0;

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

        treeBlock *new_block = createNewTreeBlock(depthSelectedNode, subTreeSize, MAX_TREE_NODES);
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
            for (PREORDER_TYPE j = nFrontiers; j > frontierSelectedNode; --j)
            {
                setPointer(j, getPointer(j - 1));
                setPreOrder(j, getPreOrder(j - 1) - subTreeSize + 1);
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

            for (PREORDER_TYPE j = frontierSelectedNode + 1; frontier < nFrontiers; j++, frontier++)
            {
                setPointer(j, getPointer(frontier));
                setPreOrder(j, getPreOrder(frontier) - subTreeSize + 1);
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
                insert(insertionNode, leafPoint, level, length, curFrontier);
            }
            else {
                new_block->insert(insertionNode, leafPoint, level, length, curFrontierNewBlock);
            }
        }
        // If the insertion is in the old block
        else
        {
            insert(insertionNode, leafPoint, level, length, curFrontier);
        }
    }
}

// This function takes in a node (in preorder) and a symbol (branch index)
// Return the child node (in preorder) designated by that symbol
NODE_TYPE treeBlock::skipChildrenSubtree(NODE_TYPE &node, SYMBOL_TYPE symbol, LEVEL_TYPE curLevel, PREORDER_TYPE &curFrontier)
{
    if (curLevel == MAX_DEPTH)
        return node;
    PREORDER_TYPE sTop = -1;
    PREORDER_TYPE skipChild = getChildSkipT(dfuds, node, symbol);
    PREORDER_TYPE nChildren = getNChildrenT(dfuds, node);
    PREORDER_TYPE diff = nChildren - skipChild;
    PREORDER_TYPE stack[100];
    stack[++sTop] = nChildren;

    NODE_TYPE curNode = node + 1;

    if (frontiers != NULL && curFrontier < nFrontiers && curNode > getPreOrder(curFrontier))
        ++curFrontier;
    PREORDER_TYPE nextFrontierPreOrder;

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
        // It is "-1" because curLevel is 0th indexed.
        else if (curLevel < MAX_DEPTH - 1)
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

// This function takes in a node (in preorder) and a symbol (branch index)
// Return the child node (in preorder) designated by that symbol
// This function differs from skipChildrenSubtree as it checks if that child node is present
NODE_TYPE treeBlock::child(treeBlock *&p, NODE_TYPE &node, SYMBOL_TYPE symbol, LEVEL_TYPE &curLevel, PREORDER_TYPE &curFrontier)
{
    bool hasChild = (bool) dfuds->GetBit(node * nBranches + symbol);
    if (!hasChild)
        return NULL_NODE;
    if (curLevel == MAX_DEPTH && hasChild)
        return node;

    NODE_TYPE curNode;

    if (frontiers != NULL && curFrontier < nFrontiers && node == getPreOrder(curFrontier))
    {
        p = getPointer(curFrontier);
        curFrontier = 0;
        NODE_TYPE auxNode = 0;
        curNode = p->skipChildrenSubtree(auxNode, symbol, curLevel, curFrontier);
    }
    else
        curNode = skipChildrenSubtree(node, symbol, curLevel, curFrontier);

    return curNode;
}

// Traverse the current TreeBlock, going into frontier nodes as needed
// Until it cannot traverse further and calls insertion
void insertar(treeBlock *root, leafConfig *leafPoint, LEVEL_TYPE length, LEVEL_TYPE level)
{
    treeBlock *curBlock = root;
    NODE_TYPE curNode = 0;
    PREORDER_TYPE curFrontier = 0;

    NODE_TYPE curNodeAux = 0;
    while (level < length)
    {
        curNodeAux = curBlock->child(curBlock, curNode, leafToSymbol(leafPoint, level), level, curFrontier);
        if (curNodeAux == (NODE_TYPE)-1)
            break;
        curNode = curNodeAux;
        if (curBlock->nFrontiers > 0 && curFrontier < curBlock->nFrontiers && curNode == curBlock->getPreOrder(curFrontier))
        {
            curBlock = curBlock->getPointer(curFrontier);
            curNode = (NODE_TYPE)0;
            curFrontier = 0;
        }
        level++;
    }
    curBlock->insert(curNode, leafPoint, level, length, curFrontier);
}

// This function is used for testing.
// It differs from above as it only returns True or False.
bool walkTree(treeBlock *curBlock, leafConfig *leafPoint, LEVEL_TYPE length, LEVEL_TYPE level){

    PREORDER_TYPE curFrontier = 0;
    NODE_TYPE curNode = 0;
    NODE_TYPE curNodeAux = 0;
    while (level < length)
    {
        curNodeAux = curBlock->child(curBlock, curNode, leafToSymbol(leafPoint, level), level, curFrontier);
        if (curNodeAux == (NODE_TYPE)-1)
            return false;
        curNode = curNodeAux;

        if (curBlock->nFrontiers > 0 && curFrontier < curBlock->nFrontiers && curNode == curBlock->getPreOrder(curFrontier))
        {
            curBlock = curBlock->getPointer(curFrontier);
            curNode = (NODE_TYPE)0;
            curFrontier = 0;
        }
        level++;
    }
    return true;        
}

// This function goes down the trie
// Return the treeBlock at the leaf of the trie
treeBlock *walkTrie(trieNode *tNode, leafConfig *leafPoint, LEVEL_TYPE &level){
    
    while (tNode->children[leafToSymbol(leafPoint, level)])
        tNode = tNode->children[leafToSymbol(leafPoint, level++)];
    while (level < TRIE_DEPTH)
    {
        tNode->children[leafToSymbol(leafPoint, level)] = createNewTrieNode();
        tNode = tNode->children[leafToSymbol(leafPoint, level)];
        level++;
    }
    treeBlock *tBlock = NULL;
    if (tNode->block == NULL)
    {
        tBlock = createNewTreeBlock(TRIE_DEPTH, 1, MAX_TREE_NODES);
        tNode->block = tBlock;
    }
    else
        tBlock = (treeBlock *)tNode->block;
    return tBlock;    
}

// This function inserts a string into a trieNode.
// The first part it traverses is the trie, followed by traversing the treeblock
void insertTrie(trieNode *tNode, leafConfig *leafPoint, LEVEL_TYPE length)
{
    LEVEL_TYPE level = 0;
    treeBlock *tBlock = walkTrie(tNode, leafPoint, level);
    insertar(tBlock, leafPoint, length, level);
}

// Given the leafPoint and the level we are at, return the Morton code corresponding to that level
SYMBOL_TYPE leafToSymbol(leafConfig *leafPoint, LEVEL_TYPE level){
    SYMBOL_TYPE result = 0;
    for (int j = 0; j < dimensions; j++){
        int coordinate = leafPoint->coordinates[j];
        int bit = (coordinate >> (MAX_DEPTH - level - 1)) & 1;
        result *= 2;
        result += bit;
    }
    return result;
}

// Used for Test script to check whether a leafPoint is present
bool check(trieNode *tNode, leafConfig *leafPoint, LEVEL_TYPE strlen){
    LEVEL_TYPE level = 0;
    treeBlock *tBlock = walkTrie(tNode, leafPoint, level);
    return walkTree(tBlock, leafPoint, strlen, level);        
}