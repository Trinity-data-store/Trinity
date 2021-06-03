#include "trie.h"

void copyNodeCod(bitmap::Bitmap *from_dfuds, bitmap::Bitmap *to_dfuds, node_type from, node_type to)
{
    symbol_type visited = 0;
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

treeBlock *treeBlock::getPointer(preorder_type curFrontier)
{
    return ((frontierNode *)frontiers)[curFrontier].pointer;
}

preorder_type treeBlock::getPreOrder(preorder_type curFrontier)
{
    return ((frontierNode *)frontiers)[curFrontier].preOrder;
}

void treeBlock::setPreOrder(preorder_type curFrontier, preorder_type preOrder)
{
    ((frontierNode *)frontiers)[curFrontier].preOrder = preOrder;
}

void treeBlock::setPointer(preorder_type curFrontier, treeBlock *pointer)
{
    ((frontierNode *)frontiers)[curFrontier].pointer = pointer;
}

preorder_type getNChildrenT(bitmap::Bitmap *dfuds, node_type node){

    return dfuds->popcount(node * nBranches, nBranches);
}

preorder_type getChildSkipT(bitmap::Bitmap *dfuds, node_type node, symbol_type col)
{
    return dfuds->popcount(node * nBranches, col);
}

treeBlock *createNewTreeBlock(level_type rootDepth, preorder_type nNodes, preorder_type maxNodes)
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
    trieNode *tNode = new trieNode(nBranches);
    for (symbol_type i = 0; i < nBranches; i++)
        tNode->children[i] = NULL;

    tNode->block = NULL;
    return tNode;
}

// This function selectes the subTree starting from node 0
// The selected subtree has the maximum subtree size
node_type treeBlock::selectSubtree(preorder_type &subTreeSize, preorder_type &depthSelectN)
{
    // index -> Number of children & preorder
    nodeInfo stackSS[4096];
    // index -> size of subtree & preorder
    subtreeInfo subtrees[4096];
    // Index -> depth of the node
    preorder_type depthVector[4096];

    //  Corresponds to stackSS, subtrees, depthVector
    preorder_type ssTop = 0, subtreeTop = 0, depthTop = 0;
    node_type node = 0;
    preorder_type depth;
    preorder_type curFrontier = 0;

    stackSS[ssTop].preorder = 0;
    stackSS[ssTop++].nChildren = getNChildrenT(dfuds, 0);
    node++;
    depth = rootDepth + 1;
    preorder_type nextFrontierPreOrder;

    if (nFrontiers == 0 || curFrontier >= nFrontiers)
        nextFrontierPreOrder = -1;
    else
        nextFrontierPreOrder = getPreOrder(curFrontier);

    for (preorder_type i = 1; i < nNodes; ++i)
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
        else if (depth < max_depth - 1)
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
    preorder_type nodemin, min, posmin;
    int diff;

    nodemin = subtrees[0].preorder,
    min = nNodes - 2 * subtrees[0].subtreeSize;
    posmin = 0;

    for (preorder_type i = 1; i < subtreeTop; ++i)
    {
        diff = nNodes - 2 * subtrees[i].subtreeSize;
        if (diff < 0)
            diff = -diff;
        if (diff < (int) min)
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
void treeBlock::insert(node_type node, leafConfig *leafPoint, level_type level, level_type length, preorder_type curFrontier)
{
    if (level == length){
        return;
    }
    node_type nodeOriginal = node;

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
        
        node_type destNode = nNodes + (length - level) - 2;
        node_type origNode = nNodes - 1;

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
        for (level_type i = level; i < length; i++)
        {
            dfuds->ClearWidth(origNode * nBranches, nBranches);
            dfuds->SetBit(origNode * nBranches + leafToSymbol(leafPoint, i));
            nNodes++;
            origNode++;
        }
        // shift the flags by length since all nodes have been shifted by that amount
        if (frontiers != NULL)
            for (preorder_type j = curFrontier; j < nFrontiers; ++j)
                setPreOrder(j, getPreOrder(j) + length - level);
    }
    else
    {
        preorder_type subTreeSize, depthSelectedNode;
        node_type selectedNode = selectSubtree(subTreeSize, depthSelectedNode);
        node_type origSelectedNode = selectedNode;
        bitmap::Bitmap *new_dfuds = new bitmap::Bitmap((maxNodes + 1) * nBranches);

        preorder_type frontier;
        //  Find the first frontier node > selectedNode
        for (frontier = 0; frontier < nFrontiers; frontier++)
            if (getPreOrder(frontier) > selectedNode)
                break;

        preorder_type frontierSelectedNode = frontier;
        node_type insertionNode = node;

        node_type destNode = 0;
        preorder_type copiedNodes = 0, copiedFrontier = 0;

        bool insertionInNewBlock = false;
        bool isInRoot = false;

        preorder_type newPointerIndex = 0;

        frontierNode *newPointerArray = NULL;
        if (nFrontiers > 0)
        {
            newPointerArray = (frontierNode *)malloc(sizeof(frontierNode) * (nFrontiers + 5));
        }
        preorder_type curFrontierNewBlock = 0;

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

        treeBlock *new_block = createNewTreeBlock(depthSelectedNode, subTreeSize, max_tree_nodes);
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
            for (preorder_type j = nFrontiers; j > frontierSelectedNode; --j)
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

            for (preorder_type j = frontierSelectedNode + 1; frontier < nFrontiers; j++, frontier++)
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
node_type treeBlock::skipChildrenSubtree(node_type &node, symbol_type symbol, level_type curLevel, preorder_type &curFrontier)
{
    if (curLevel == max_depth)
        return node;
    int sTop = -1;
    preorder_type skipChild = getChildSkipT(dfuds, node, symbol);
    preorder_type nChildren = getNChildrenT(dfuds, node);
    preorder_type diff = nChildren - skipChild;
    preorder_type stack[100];
    stack[++sTop] = nChildren;

    node_type curNode = node + 1;

    if (frontiers != NULL && curFrontier < nFrontiers && curNode > getPreOrder(curFrontier))
        ++curFrontier;
    preorder_type nextFrontierPreOrder;

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
        else if (curLevel < max_depth - 1)
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
node_type treeBlock::child(treeBlock *&p, node_type &node, symbol_type symbol, level_type &curLevel, preorder_type &curFrontier)
{
    bool hasChild = (bool) dfuds->GetBit(node * nBranches + symbol);
    if (!hasChild)
        return null_node;
    if (curLevel == max_depth && hasChild)
        return node;

    node_type curNode;

    if (frontiers != NULL && curFrontier < nFrontiers && node == getPreOrder(curFrontier))
    {
        p = getPointer(curFrontier);
        curFrontier = 0;
        node_type auxNode = 0;
        curNode = p->skipChildrenSubtree(auxNode, symbol, curLevel, curFrontier);
    }
    else
        curNode = skipChildrenSubtree(node, symbol, curLevel, curFrontier);

    return curNode;
}

// Traverse the current TreeBlock, going into frontier nodes as needed
// Until it cannot traverse further and calls insertion
void insertar(treeBlock *root, leafConfig *leafPoint, level_type length, level_type level)
{
    treeBlock *curBlock = root;
    node_type curNode = 0;
    preorder_type curFrontier = 0;

    node_type curNodeAux = 0;
    while (level < length)
    {
        curNodeAux = curBlock->child(curBlock, curNode, leafToSymbol(leafPoint, level), level, curFrontier);
        if (curNodeAux == (node_type)-1)
            break;
        curNode = curNodeAux;
        if (curBlock->nFrontiers > 0 && curFrontier < curBlock->nFrontiers && curNode == curBlock->getPreOrder(curFrontier))
        {
            curBlock = curBlock->getPointer(curFrontier);
            curNode = (node_type)0;
            curFrontier = 0;
        }
        level++;
    }
    curBlock->insert(curNode, leafPoint, level, length, curFrontier);
}

// This function is used for testing.
// It differs from above as it only returns True or False.
bool walkTree(treeBlock *curBlock, leafConfig *leafPoint, level_type length, level_type level){

    preorder_type curFrontier = 0;
    node_type curNode = 0;
    node_type curNodeAux = 0;
    while (level < length)
    {
        curNodeAux = curBlock->child(curBlock, curNode, leafToSymbol(leafPoint, level), level, curFrontier);
        if (curNodeAux == (node_type)-1)
            return false;
        curNode = curNodeAux;

        if (curBlock->nFrontiers > 0 && curFrontier < curBlock->nFrontiers && curNode == curBlock->getPreOrder(curFrontier))
        {
            curBlock = curBlock->getPointer(curFrontier);
            curNode = (node_type)0;
            curFrontier = 0;
        }
        level++;
    }
    return true;        
}

// This function goes down the trie
// Return the treeBlock at the leaf of the trie
treeBlock *walkTrie(trieNode *tNode, leafConfig *leafPoint, level_type &level){
    
    while (tNode->children[leafToSymbol(leafPoint, level)])
        tNode = tNode->children[leafToSymbol(leafPoint, level++)];
    while (level < trie_depth)
    {
        tNode->children[leafToSymbol(leafPoint, level)] = createNewTrieNode();
        tNode = tNode->children[leafToSymbol(leafPoint, level)];
        level++;
    }
    treeBlock *tBlock = NULL;
    if (tNode->block == NULL)
    {
        tBlock = createNewTreeBlock(trie_depth, 1, max_tree_nodes);
        tNode->block = tBlock;
    }
    else
        tBlock = (treeBlock *)tNode->block;
    return tBlock;    
}

// This function inserts a string into a trieNode.
// The first part it traverses is the trie, followed by traversing the treeblock
void insertTrie(trieNode *tNode, leafConfig *leafPoint, level_type length)
{
    level_type level = 0;
    treeBlock *tBlock = walkTrie(tNode, leafPoint, level);
    insertar(tBlock, leafPoint, length, level);
}

// Given the leafPoint and the level we are at, return the Morton code corresponding to that level
symbol_type leafToSymbol(leafConfig *leafPoint, level_type level){
    symbol_type result = 0;
    for (int j = 0; j < dimensions; j++){
        int coordinate = leafPoint->coordinates[j];
        int bit = (coordinate >> (max_depth - level - 1)) & 1;
        result *= 2;
        result += bit;
    }
    return result;
}

// Used for Test script to check whether a leafPoint is present
bool check(trieNode *tNode, leafConfig *leafPoint, level_type strlen){
    level_type level = 0;
    treeBlock *tBlock = walkTrie(tNode, leafPoint, level);
    return walkTree(tBlock, leafPoint, strlen, level);        
}