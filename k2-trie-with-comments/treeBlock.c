
#include "treeBlock.h"

// Transform a node in its pair representation, 
// to its index in the DFUDS sequence
uint16_t absolutePosition(treeNode &node)
 {
   //  Note: for nodes are bundled into one group
   // node.first -> group number
   // node.second -> group offset
    return 4*node.first + node.second;
 }

// Move to next node in preorder
inline void nextNode(treeNode &node)
 {
   //  0x3 is 3;
   // Example:（0，0）-> (0, 1) -> (0, 2) -> (0, 3) -> (1, 0) -> (1, 1)
     node.second = (node.second+1) & 0x3;
     node.first += !node.second;
 }

// Move to previous node in preorder
inline void prevNode(treeNode &node)
 {
     node.first -= !node.second;
     node.second = (node.second-1) & 0x3;
 }

// Free the treeBlock
void treeBlock::freeTreeBlock() 
 {
    free((void *)dfuds);
    for (uint16_t i = 0; i < nPtrs; ++i)
       ((blockPtr *)ptr)[i].P->freeTreeBlock();
    
    free((void*)ptr);
    free(this); 
 }

// Grow the dfuds part of the treeBlock
void treeBlock::grow(uint16_t extraNodes)
 {
       dfuds = (uint16_t *) realloc(dfuds, sizeof(uint16_t)*((sizeArray[nNodes+extraNodes] + 3)/4));
    maxNodes = 4*((sizeArray[nNodes+extraNodes]+3)/4);
 }

// Shrink the dfuds part of the treeBlock
void treeBlock::shrink(uint16_t deletedNodes)
 {
       dfuds = (uint16_t *) realloc(dfuds, sizeof(uint16_t)*((sizeArray[nNodes-deletedNodes] + 3)/4));
    maxNodes = 4*((sizeArray[nNodes-deletedNodes]+3)/4); 
 }

struct nodeInfo 
 {
    uint16_t preorder;
    uint16_t nChildren;
    nodeInfo() {};
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
    subtreeInfo() {};
    subtreeInfo(uint16_t _preorder, uint16_t _subtreeSize) 
     {
        preorder = _preorder;
        subtreeSize = _subtreeSize;
     };
 };


int8_t stack[100];
// index -> Number of children & preorder
nodeInfo stackSS[4096];

// index -> size of subtree & preorder
subtreeInfo subtrees[4096];

// Index -> depth of the node
uint16_t depthVector[4096];

// From paper: the splitting process should traverse T_B to choose the node w such that splitting T_B at w generates two trees whose size difference is minimum. After choosing node w, we carry out the split by generating two blocks, adding the corresponding pointer to the new child block, and adding w as a frontier node (storing its preorder in F_B and its pointer in P_B).
// This function returns node w

treeNode treeBlock::selectSubtree2(uint16_t maxDepth, uint16_t & subTreeSize, uint16_t & depthSelectN)
 { 
    // initialized as the root of the block
    treeNode node(0,0); 
    
    uint16_t depth;
    
    uint16_t curFlag = 0;
    
   //  Corresponds to stackSS, subtrees, depthVector
    uint16_t ssTop=0, subtreeTop = 0, depthTop = 0;    

    // first node & 15; &dfuds -> first 4 nodes; shift 12 get the first node;  
    uint8_t cNodeCod = (*dfuds>>12) & 0x000f;

    // The zeroth node (root) has preorder 0; stackSS[1].nChildrne = numbner of children of the first node  
    stackSS[ssTop].preorder = 0; stackSS[ssTop++].nChildren =  nChildrenT[cNodeCod];    
    
    nextNode(node);
    
    depth = rootDepth + 1;

    int32_t nextFlag;
    
    if (nPtrs == 0 || curFlag >= nPtrs)
       nextFlag = -1;
    else       
       nextFlag = ((blockPtr *)ptr)[curFlag].flag;
    
   //  Start from 1, since we moved on from the first node
   // Iterate over each node
    for (uint16_t i=1; i < nNodes; ++i) {
      //  If we meet a child node
       if (i == nextFlag) {
          ++curFlag;
          if (nPtrs == 0 || curFlag >= nPtrs)
             nextFlag = -1;
          else
             nextFlag = ((blockPtr *)ptr)[curFlag].flag;
         // Decrement the number of children since we just go past a child node
          --stackSS[ssTop-1].nChildren;
       }
      //  Go to a new node and start searching for its children
       else if (depth < maxDepth) {
          stackSS[ssTop].preorder = i; 
          cNodeCod = (dfuds[i>>2]>>shiftT[i & 0x3]) & 0x000f;         
          stackSS[ssTop++].nChildren = nChildrenT[cNodeCod];          
          depth++;
        }
      //   at the maxDepth levle
        else --stackSS[ssTop-1].nChildren;

       while (ssTop > 0 && stackSS[ssTop-1].nChildren == 0) {
         
         //  Update the preorder and subtreeSize of that subtree
          subtrees[subtreeTop].preorder = stackSS[ssTop-1].preorder;
          subtrees[subtreeTop++].subtreeSize = i-stackSS[ssTop-1].preorder+1;
                    
          --ssTop;
           // depthVector maps node index to depth
          depthVector[depthTop++] = --depth;          
         
          if (ssTop == 0) break;         
          else stackSS[ssTop-1].nChildren--;
       }
                 
    }

    // Now I have to go through the subtrees vector to choose the proper subtree
    int16_t  nodemin, min, posmin;
    
    uint16_t leftmost = MAX_UINT_16;

   //  From paper: Our splitting criterion, then, tries first to separate the leftmost node in the block whose subtree size is 25%–75% of the total block size.
    uint16_t lowerB = 0.25*nNodes, upperB = 0.75*nNodes;

   // Try to find the leftmost node where its subtree sizes is 25%-75% of total number of nodes
   // posmin: position of the node whose subtree satisfies the requirement  
    for (uint16_t i = 0; i < subtreeTop; ++i) {

       if (((float)nNodes/4) <= subtrees[i].subtreeSize && subtrees[i].subtreeSize <= ((float)3*nNodes/4) && subtrees[i].preorder < leftmost) {
          leftmost = nodemin = subtrees[i].preorder;
          posmin = i;
       }
    }
    
   //  If the above split cannot be found 
    if (leftmost == MAX_UINT_16) {
       int16_t diff; 
       
       nodemin = subtrees[0].preorder, 
      //  I don't get this part
       min = nNodes - 2*subtrees[0].subtreeSize;
       posmin = 0;
    	 
       for (uint16_t i = 1; i < subtreeTop; ++i) {
          diff = nNodes - 2*subtrees[i].subtreeSize;
          if (diff < 0) diff = -diff;
       
          if (diff < min) {
             min = diff;
             nodemin = subtrees[i].preorder;
             posmin = i;
          }
       }    	
    }

    subTreeSize = subtrees[posmin].subtreeSize;
    
    depthSelectN = depthVector[posmin]; 
   //  Return the splitting node, update the values of subTreeSize and depthSelectN
   // The returned node will be turned into a frontier node
    return treeNode(nodemin >> 2,nodemin & 0x3);
 }


// Note: This is not used! Hence, deleted
// I think treeBlock::selectSubtree2 is an improved version of treeBlock::selectSubtree
// treeNode treeBlock::selectSubtree(uint16_t maxDepth, uint16_t & subTreeSize, uint16_t & depthSelectN)

treeNode dummyRootBlockNode(0,0);


// insert str in the subtree of node x
void treeBlock::insert(treeNode node, uint8_t str[], uint64_t length, uint16_t level, 
                       uint64_t maxDepth, uint16_t curFlag)
 {
    treeNode nodeAux = node;
    
    if (rootDepth </*=*/ L1) Nt = S1;
    else if (rootDepth <= L2) Nt = S2;
    else Nt = S3;
    
   //  node is a frontier node
    if (ptr!=NULL && curFlag < nPtrs && absolutePosition(node) == ((blockPtr *)ptr)[curFlag].flag) {
       // insertion must be carried out in the root of a child block
        uint8_t cNodeCod = (dfuds[node.first]>>shiftT[node.second]) & 0x000f;
        
      //  equivalent to aux = shiftT[node.second]
        register uint64_t aux = 4*(3-node.second);    

      //   0xF -> 15
      // If node.second = 0, remove the first 4 bits in dfuds[node.first]
      // If node.second = 1, remove the 5-8 bits in dfuds[node.first]
        dfuds[node.first] = dfuds[node.first] & ~(0xF << aux); 

      // Replace that gap with insertT[cNodeCod][str[0]]; keep nNodeCod and add str[0] position
        dfuds[node.first] = dfuds[node.first] | (insertT[cNodeCod][str[0]] << aux);
        
        ((blockPtr *)ptr)[curFlag].P->insert(dummyRootBlockNode, str, length, level, maxDepth, 0);

        return;
    }
    else
    if (length == 1) {
      //  If there is only one character left
      // Insert that character into the correct position

       uint8_t cNodeCod = (dfuds[node.first]>>shiftT[node.second]) & 0x000f;
       
       register uint64_t aux = 4*(3-node.second);       
       
       dfuds[node.first] = dfuds[node.first] & ~(0xF << aux); 
          
       dfuds[node.first] = dfuds[node.first] | (insertT[cNodeCod][str[0]] << aux);
       return;            
    }
    else 
    if (nNodes + length - 1 <= maxNodes) {
       // there is room in current block for new nodes
                 
       // The position under node where the new str[0] will be inserted
       node = skipChildrenSubtree(node, str[0], level, maxDepth, curFlag);
       
       treeNode origNode, destNode;
       
       --length; 
       
      //  destNode: last node after including the whole string
       destNode.first =  (nNodes + length - 1)/4;
       destNode.second = (nNodes + length - 1)%4;

      //  origNode: last node before including the whole string 
       origNode.first = (nNodes - 1)/4;
       origNode.second = (nNodes - 1)%4;
       
       uint16_t preorderNode = absolutePosition(node);       
       uint16_t preorderOrigNode = absolutePosition(origNode);
       uint16_t preorderDestNode =  absolutePosition(destNode);     

      //  In this while loop, we are making space for str
      //  By shifting nodes to the right of str[i] by len(str) spots
       while (preorderOrigNode >= preorderNode) {
          register uint64_t aux = 4*(3-(preorderDestNode & 0x3) /*destNode.second*/);
          register uint64_t aux2 = preorderDestNode >> 2;
          
         //  Copy OrigNode to the destNode
          dfuds[aux2/*destNode.first*/] = dfuds[aux2/*destNode.first*/] & ~(0xF << aux);
                                 
          dfuds[aux2/*destNode.first*/] = dfuds[aux2/*destNode.first*/] 
                                 | (((dfuds[preorderOrigNode>>2/*origNode.first*/] >> 4*(3-(preorderOrigNode & 0x3)/*origNode.second*/)) 
                                 & 0x000F) << aux) ;            
          --preorderDestNode;          
          --preorderOrigNode;
       }

      // Insert that character (str[0]) into the correct position
       uint8_t cNodeCod = (dfuds[nodeAux.first]>>shiftT[nodeAux.second]) & 0x000f;

       dfuds[nodeAux.first] = dfuds[nodeAux.first]
                              & ~(0xF << 4*(3-nodeAux.second)); 

       dfuds[nodeAux.first] = dfuds[nodeAux.first] 
                             | (insertT[cNodeCod][str[0]] << 4*(3-nodeAux.second));

      //  Here preorderOrigNode = preorderNode
       ++preorderOrigNode;
    
      //  Insert all remaining characters 
       for (uint16_t i = 1; i <= length; i++) {
          register uint64_t aux = 4*(3-(preorderOrigNode & 0x3)/*origNode.second*/);
          register uint64_t aux2 = preorderOrigNode >> 2;

          dfuds[aux2/*origNode.first*/] = dfuds[aux2/*origNode.first*/] & ~(0xF << aux); 
          
         //  symbol2NodeT: given a symbol, returns a unary node representing that symbol
          dfuds[aux2/*origNode.first*/] = dfuds[aux2/*origNode.first*/] 
                                 | (symbol2NodeT[str[i]] << aux);
          ++nNodes;          
          ++preorderOrigNode;
       }
    
       // Now updates the flags, as new nodes have been added,
       // shift the flags by length since all nodes have been shifted by that amount
       if (ptr)
          for (uint16_t i = curFlag; i < nPtrs; ++i)
             ((blockPtr *)ptr)[i].flag += length;
    }
    else {
       // there is no room for the new nodes

       // block can still grow
       if (nNodes + length - 1 <= Nt) { 
          // if the block can still grow, grow it.
          grow(length-1);
          // After growing, recursively inserts the node 
          insert(node, str, length, level, maxDepth, curFlag);
       
       }
       else { 
          // block cannot grow
         //  We must first split the treeBlock to make room

          treeNode selectedNode, originalSelectedNode;
          
          uint16_t subTreeSize, depthSelectedNode;
          
         //  selectSubtree2 returns the node we split
          originalSelectedNode = selectedNode = selectSubtree2(maxDepth, subTreeSize, depthSelectedNode);
                   
          // Create the new dfuds to copy the selected subtree over
          uint16_t *new_dfuds = (uint16_t *)calloc((sizeArray[subTreeSize]+4-1)/4, sizeof(uint16_t));
          
         //  Root node for the new block
          treeNode destNode(0,0);
          
          uint16_t copiedNodes=0, copiedFlags = 0;
                    
          bool insertionInNewBlock = false;
          
          treeNode insertionNode = node;
          
          uint16_t flag, auxFlag=0, flagSelectedNode;
          
          uint16_t preorderSelectedNode = absolutePosition(selectedNode);    
          
         //  Find the first flag > preorderSelectedNode
          for (flag = 0; flag < nPtrs; ++flag)
             if (((blockPtr *)ptr)[flag].flag > preorderSelectedNode) break;
          
          flagSelectedNode = flag; // stores the first flag which is > to the selected node
                                   // this is to know later where to put the flag of the selected node
          
          blockPtr *new_ptr;
         //  Create a new blockptr. 
          if (nPtrs > 0) new_ptr = (blockPtr *)malloc(sizeof(blockPtr)*nPtrs);
          else new_ptr = NULL;          
          
          // flag in the new block, in case the insertion continues in that block  
          uint16_t curFlagNewBlock;        
             
          bool isInRoot = false;          
          
          uint16_t preorderDestNode = absolutePosition(destNode);          
         //  Copy all nodes of the subtree to the new block
          while (copiedNodes < subTreeSize) {

            //  When the node (one we want to make it into a frontier node) is the current node
             if (selectedNode == node) {
                // the original node where insertion must be carried out
                // is stored in the child (new) block
                insertionInNewBlock = true;
                
                if (destNode != treeNode(0,0)) 
                   // this will be the node where insertion must be carried out in the new block             
                   insertionNode = destNode; 

                  // Otherwise, it will be inserted in the root
                else { insertionNode = node; isInRoot = true;}              
                
                curFlagNewBlock = copiedFlags;
             
             }

             // Copy pointer to the new block
             if (ptr!=NULL && flag < nPtrs && preorderSelectedNode == ((blockPtr *)ptr)[flag].flag) {
                new_ptr[auxFlag].flag = preorderDestNode;
                new_ptr[auxFlag].P = ((blockPtr *)ptr)[flag].P;
                ++flag;
                ++auxFlag;
                ++copiedFlags;
             }
             
             register uint64_t aux = 4*(3-selectedNode.second);             
             
            //  Copy selectedNode to destNode
             new_dfuds[destNode.first] = new_dfuds[destNode.first] 
                                       | (((dfuds[selectedNode.first] >> aux/*4*(3-selectedNode.second)*/)
                                       & 0x000F) << 4*(3-destNode.second));  

            // Remove selected Nodes except for the frontier node
             if (selectedNode != originalSelectedNode)
                dfuds[selectedNode.first] =  dfuds[selectedNode.first]
                                            & ~(0xF << aux/*4*(3-selectedNode.second)*/);             
                    
             nextNode(destNode);
             ++preorderDestNode;
             nextNode(selectedNode);
             ++preorderSelectedNode;
             ++copiedNodes;
          }
          
          bool insertionBeforeSelectedTree = true;
          
          if (!insertionInNewBlock && flag <= curFlag)
          	 // the insertion point is after the selected subtree
             insertionBeforeSelectedTree = false;   

          // malloc a new block; 
          treeBlock *new_block = (treeBlock *)malloc(sizeof(treeBlock));
          
          new_block->nNodes = subTreeSize;
          new_block->maxNodes = sizeArray[subTreeSize]; 
          new_block->dfuds = new_dfuds; 
          new_block->rootDepth = depthSelectedNode;
          
         //  If no pointer is copied to the new block
          if (auxFlag == 0) {
             if (new_ptr != NULL) free((void *)new_ptr);
             new_block->ptr = NULL;
             new_block->nPtrs = 0;

             // there is a new pointer in the current block
             ptr = realloc(ptr, sizeof(blockPtr)*(nPtrs+1));  
             
             // Shift right one spot to move the pointers from flagSelectedNode + 1 to nPtrs  
             for (uint16_t i = nPtrs; i > flagSelectedNode; --i) {
                ((blockPtr *)ptr)[i].P = ((blockPtr *)ptr)[i-1].P;
                ((blockPtr *)ptr)[i].flag = ((blockPtr *)ptr)[i-1].flag - subTreeSize + 1;
             }
            //  Insert that new pointer
             ((blockPtr *)ptr)[flagSelectedNode].flag = absolutePosition(originalSelectedNode);
             ((blockPtr *)ptr)[flagSelectedNode].P = new_block;  // pointer to the new child block
             
             nPtrs++;
          }
          else {
            //  If there are pointers copied to the new block
             new_ptr = (blockPtr *)realloc(new_ptr, sizeof(blockPtr)*auxFlag);
          
             new_block->ptr = new_ptr; 
    
             new_block->nPtrs = auxFlag;
             
             ((blockPtr *)ptr)[flagSelectedNode].flag = absolutePosition(originalSelectedNode);
             
             ((blockPtr *)ptr)[flagSelectedNode].P = new_block;
            // Shift pointers to fill out the gap left by the copied pointers
             for (uint16_t i = flagSelectedNode+1; flag < nPtrs; ++i, ++flag) {
                ((blockPtr *)ptr)[i].P = ((blockPtr *)ptr)[flag].P;
                ((blockPtr *)ptr)[i].flag = ((blockPtr *)ptr)[flag].flag - subTreeSize + 1;
             }
             
             nPtrs = nPtrs - copiedFlags + 1;
             ptr = realloc(ptr, sizeof(blockPtr)*nPtrs); // resize pointer array size 
          }   
       
          // Now you have to delete the subtree copied to the child
          flag = flagSelectedNode+1;
          nextNode(originalSelectedNode);
          preorderSelectedNode = absolutePosition(selectedNode);
                   
          while (preorderSelectedNode/*absolutePosition(selectedNode)*/ < nNodes) {
             register uint64_t auxOriginalS = 4*(3-originalSelectedNode.second);
             register uint64_t auxS = 4*(3-selectedNode.second);             
             
             dfuds[originalSelectedNode.first] =  dfuds[originalSelectedNode.first]
                                    & ~(0xF << auxOriginalS/*4*(3-originalSelectedNode.second)*/); 
             // Copy selectedNode to original SelectedNode
             dfuds[originalSelectedNode.first] = dfuds[originalSelectedNode.first] 
                                       | (((dfuds[selectedNode.first] >> auxS/*4*(3-selectedNode.second)*/)
                                       & 0x000F) << auxOriginalS /*4*(3-originalSelectedNode.second)*/);  

             dfuds[selectedNode.first] =  dfuds[selectedNode.first]
                                    & ~(0xF << auxS/*4*(3-selectedNode.second)*/);             
            
            if (selectedNode == node) insertionNode = originalSelectedNode; 
            
            nextNode(selectedNode);
            nextNode(originalSelectedNode);
            ++preorderSelectedNode;
          }
         // Adjust the size
          if (subTreeSize > length)         
             shrink(subTreeSize-1-length+1);
          else           
             shrink(subTreeSize-1);
          nNodes -= (subTreeSize - 1); 
          
          if (!insertionBeforeSelectedTree)
             curFlag -= copiedFlags;          
          
          if (insertionInNewBlock) { 
             // insertion continues in the new block              
            if (isInRoot)
               insert(insertionNode, str, length, level, maxDepth, curFlag);           
            else              
               new_block->insert(insertionNode, str, length, level, maxDepth, curFlagNewBlock);
          }          
          else
             // insertion continues in the original block
             insert(insertionNode, str, length, level, maxDepth, curFlag);
       }         
    }
 }


treeBlock *treeBlock::getPointer(uint16_t curFlag)
 {
    return ((blockPtr *)ptr)[curFlag].P;
 }


// skip children (and their subtrees) before the node to get to child(node, symbol)
treeNode treeBlock::skipChildrenSubtree(treeNode &node, uint8_t symbol, uint16_t &curLevel,
                                        uint16_t maxLevel, uint16_t &curFlag)
 {
 	 if (curLevel == maxLevel) return node;

    // Top of the stack, -1 stack is empty, 0 stack has one element 
    int16_t sTop = -1;

    uint8_t cNodeCod = (dfuds[node.first]>>shiftT[node.second]) & 0x000f;

   // Number of subtrees of the given node must be skipped (before the symbol)
    uint8_t skipChild = (uint8_t) childSkipT[cNodeCod][symbol];
    
   //  number of children under the given node
    uint8_t nChildren = nChildrenT[cNodeCod];
   
   // Number of children including and to the right of symbol
    uint8_t diff = nChildren - skipChild;

   // We keep a stack S (initially empty) with the number of children not yet traversed of the ancestors of the current node   
    stack[++sTop] = nChildren;
    
    treeNode currNode = node;
    
    //  From paper: We start looking for the desired child by moving to position x + 1, corresponding to the first child of x in preorder
    nextNode(currNode);
    uint16_t curPreorder = absolutePosition(currNode);
   
   // Increment curFlag
   // Since we traverse in preorder, and F_B is sorted, increasing i_f (curFlag) as we traverse T_B is enough to keep i_f (curFlag) up to date. 
   // When the preorder of the current node exceeds F_B[i_f], we increase i_f.

    if (ptr != NULL && curFlag < nPtrs && curPreorder > ((blockPtr *)ptr)[curFlag].flag)
       ++curFlag; 
    
   //  nextFlag stores the absolute position of the next frontier node
    uint16_t nextFlag;
    
    if (nPtrs == 0 || curFlag >= nPtrs)
       nextFlag = -1;
    else       
       nextFlag = ((blockPtr *)ptr)[curFlag].flag;
        
   //  we keep track of the current depth d (curLevel), to know when we arrive to frontier nodes.
   // Since we move down to the first child of node, we increment curLevel
    ++curLevel;
    
    while(curPreorder < nNodes && sTop >= 0 && diff < stack[0])  {

      //  Every time we reach a new node, we push in S its number of children if the node is not of maximum depth (minus 1), and it is not a frontier node. 

      //  Frontier node; skip to the next child of node (x)
       if (curPreorder == nextFlag) {
           ++curFlag;
           if (nPtrs == 0 || curFlag >= nPtrs)
              nextFlag = -1;
           else
              nextFlag = ((blockPtr *)ptr)[curFlag].flag;
           --stack[sTop];
        }
      // Go down at the node
        else if (curLevel < maxLevel) {
        	  cNodeCod = (dfuds[curPreorder >> 2/*currNode.first*/]>>shiftT[curPreorder & 0x3/*currNode.second*/]) & 0x000f;
           stack[++sTop] = nChildrenT[cNodeCod];
           ++curLevel;
        }
      //   Reached the maximum level
        else --stack[sTop];

        ++curPreorder;

      // When the top value becomes 0, it means that a whole subtree has been traversed.
      // In such a case we pop S, decrease the current depth d, and decrease the new value at the top
       while (sTop >= 0 && stack[sTop] == 0) {
          --sTop;          
          --curLevel;
          if (sTop >= 0) --stack[sTop];
       }
    }
    
    currNode.first = curPreorder >> 2; //currNode.first
    currNode.second = curPreorder & 0x3; //currNode.second, 0x3 -> 3
    return currNode;
 }


treeNode NULL_NODE = treeNode((NODE_TYPE)-1, 0);

// curBlock->child(x,i) yields the child of the node x by symbol i (0-3)
treeNode treeBlock::child(treeBlock *&p, treeNode & node, uint8_t symbol, uint16_t &curLevel, uint16_t maxLevel,
                          uint16_t &curFlag)
 {
    uint8_t cNodeCod = (dfuds[node.first]>>shiftT[node.second]) & 0x000f;

   // soughtChild: number of children up to and including nNodeCod
    uint8_t soughtChild = (uint8_t) childT[cNodeCod][symbol];

    if (soughtChild == (uint8_t)-1) return NULL_NODE;
   
   // Reached the maximum level and no need to go down;
    if (curLevel == maxLevel && soughtChild != (uint8_t)-1) return node;
       
    treeNode currNode;    

   //  From paper: To support this checking efficiently, we keep a finger i_f on array F_B, such that i_f is the smallest value for which F_B[i_f] is greater or equal than the preorder of the current node in the traversal.
   //  F_B sorted array storing the preorder number of the frontier nodes
   //  i_f -> curFlag; F_B -> ((blockPtr *)ptr)

    if (ptr != NULL && curFlag < nPtrs && absolutePosition(node) == ((blockPtr *)ptr)[curFlag].flag) {

      //  node is in the frontier of the TreeBlock
      // We go down to block P_B[i_f] or p

       p = ((blockPtr *)ptr)[curFlag].P;
      //  set i_f <- 0; or curFlag = 0
       curFlag = 0;
       treeNode auxNode;

      // start from the root Node (.first = 0, .second = 0)
       auxNode.first = auxNode.second = 0;
       currNode = p->skipChildrenSubtree(auxNode, symbol, curLevel, maxLevel, curFlag);
    }    
    else 
      // x is not a frontier node, and we stay in B. 
       currNode = skipChildrenSubtree(node, symbol, curLevel, maxLevel, curFlag);
    
    return currNode;  
 }

// Insert the remainder of the string into the treeBlock
// str: remaining of the string, length: str's length
// Level: levels traversed in the trie (length of string alraedy processed)
void insertar(treeBlock *root, uint8_t *str, uint64_t length, uint16_t level, uint16_t maxDepth) 
 {
    treeBlock *curBlock = root, *curBlockAux;
    uint64_t i;

    treeNode curNode(0,0), curNodeAux;
    uint16_t /*level = 0,*/ curFlag = 0;
        
    for (i = 0; i < length; ++i) {
       curBlockAux = curBlock;

      // Note string here is the remainder of the string
      // curBlock->child(x,i) yields the child of the node x by symbol i (0-3)
       curNodeAux = curBlock->child(curBlock, curNode, str[i], level, maxDepth, curFlag);

      //  Break when at str[i], there is no child block;
       if (curNodeAux.first == (NODE_TYPE)-1) break;
       else curNode = curNodeAux;
       
      //  absolutePosition Transform a node from pair to position in DFUDS sequence
       if (curBlock->nPtrs > 0 && absolutePosition(curNode) == curFlag) {
          // Goes down to a child block
          curBlock = curBlock->getPointer(curFlag);
          curNode.first = 0;
          curNode.second = 0;
       }
    }  
    // inserts str[i..length-1] starting from the current node
    // The new nodes will be inserted at the curNode position 
    curBlock->insert(curNode, &str[i], length-i, level, maxDepth, curFlag);
    
 }

// Insert a string into the trie, length is the string length, and maxDepth is the max depth of the trie
void insertTrie(trieNode *t, uint8_t *str, uint64_t length, uint16_t maxDepth)
 {
    uint64_t i = 0;
    treeBlock *p;
    
   //  Go down the prefix Tree until next character cannot be found
    while (t->children[str[i]]) 
       t = t->children[str[i++]];
    
    while (i < L1) {

      //  Create another trieNode
       t->children[str[i]] = (trieNode *)malloc(sizeof(trieNode));
       t = t->children[str[i]];
       i++;
       t->children[0] = t->children[1] = t->children[2] = t->children[3] = NULL;
       t->block = NULL;
    }
    
   // Create a treeBlock for the remainder of the string
    if (t->block == NULL) {
       t->block = malloc(sizeof(treeBlock));
       p = (treeBlock*)t->block;       
       p->dfuds = (uint16_t *)calloc(2, sizeof(uint16_t));
       p->rootDepth = L1;    
       p->nNodes = 1;
       p->ptr = NULL;
       p->nPtrs = 0;
       p->maxNodes = 4;
    }
    else 
       p = (treeBlock*)t->block;
    
   //  length - i: remaining length of the string
   //  i current index
   // &str[i]: pointer to the current character
   // p pointer to the treeBlock
    insertar(p, &str[i], length-i, i, maxDepth); 
 }




bool isEdge(treeBlock *root, uint8_t *str, uint64_t length, uint16_t level, uint16_t maxDepth) 
 {
    treeBlock *curBlock = root, *curBlockAux;
    uint64_t i;
    treeNode curNode(0,0), curNodeAux;
    uint16_t /*level = 0,*/ curFlag = 0;
    
    for (i = 0; i < length; ++i) {
       curBlockAux = curBlock;       
       curNodeAux = curBlock->child(curBlock, curNode, str[i], level, maxDepth, curFlag);
       
       if (curNodeAux.first == (NODE_TYPE)-1) break;
       else {
          //if (curBlock != curBlockAux) curFlag = 0;
          curNode = curNodeAux;
       }
       
       if (curBlock->nPtrs > 0 && absolutePosition(curNode) == curFlag) {
          // Goes down to a child block
          curBlock = curBlock->getPointer(curFlag);
          curNode.first = 0;
          curNode.second = 0;
       }
    }  
    
    // inserts str[i..length-1] starting from the current node
    // The new nodes inserted will descend from curNode 
    return length == i;  
 }



bool isEdgeTrie(trieNode *t, uint8_t *str, uint64_t length, uint16_t maxDepth)
 {
    uint64_t i = 0;
    treeBlock *p; 	
 	
    while (t->children[str[i]] && !t->block) 
       t = t->children[str[i++]];
    
    p = (treeBlock*)t->block;
    
    if (p)
       return isEdge(p, &str[i], length-i, i, maxDepth); 
    else
       return false; 
 }


uint64_t totalBlocks = 0, totalNodes = 0;


uint64_t treeBlock::size()
 {
 	
    totalBlocks++;
    
    totalNodes += nNodes;
     	
    uint64_t totalSize = sizeof(uint8_t) + sizeof(uint16_t) + sizeof(uint16_t *) + sizeof(blockPtr *) 
                       + sizeof(uint16_t) + nPtrs*(sizeof(treeBlock *) + sizeof(uint16_t));
    
    totalSize += sizeof(uint16_t)*((maxNodes + 3) / 4); // dfuds array  
    
    for(uint16_t i = 0; i < nPtrs; ++i)
       totalSize += ((blockPtr*)ptr)[i].P->size();    
    
    return totalSize;
 }


uint64_t sizeTrie(trieNode *t)
 {
 	
 	if (!t) return 0;
 	
 	uint64_t totalSize = /*sizeof(void *) +*/ 4*sizeof(uint16_t); // si usamos hasta 8 niveles de punteros, los punteros a hijo del trie se pueden implementar con 16 bits
 	
   if (!t->block) {
       totalSize += sizeTrie(t->children[0]);
       totalSize += sizeTrie(t->children[1]);
       totalSize += sizeTrie(t->children[2]);
       totalSize += sizeTrie(t->children[3]);
   }  	
  	else {
      totalSize += ((treeBlock *)t->block)->size();  
   } 
 
   return totalSize;
 }

int main() 
 {
    treeBlock B;
    treeNode node;
    uint16_t curFlag = 0;
    uint16_t subTreeSize;
    uint16_t level = 0;
    
    trieNode *t = (trieNode *) malloc(sizeof(trieNode));
    t->children[0] = t->children[1] = t->children[2] = t->children[3] = NULL;
    t->block = NULL;


    double alpha = 0.99;
    
    
    N1 = 4;
    
    // Maximum block size, try 96, 128, 256, 512, 1024
    Nt = S3;

    sizeArray = (uint16_t *) malloc(sizeof(uint16_t)*(Nt+1));
    
    for (int i = 0; i <= Nt; ++i) {
       if (i > N1) N1 = 4*(((uint16_t)ceil((double)N1/alpha)+3)/4);
       sizeArray[i] = N1;
      //  printf("%d ", sizeArray[i]);
    }

   // sizeArray= [4,4,4,4,4,8,8,8,8,12,12,12,12,16,16,16,16,20,20,20,20 ... ]
    
    node.first = 0;
    node.second = 0;
        
    uint64_t n, n1, nEdges;
    
    uint8_t str[50];
    
    clock_t start, diff=0;

   //  scanf("%lu %lu %lu\n", &n, &n1, &nEdges); 
    n = 3, n1 = 3, nEdges = 10;

    for (uint64_t i = 0; i < nEdges; ++i) {
       scanf("%s\n", str);

      //  k=2, d=2, Thus there are only 4 leaves
       for (uint8_t j = 0; j < 23; ++j)
          switch(str[j]) {
             case '0': str[j] = 0;
                       break;
             case '1': str[j] = 1;
                       break;
             case '2': str[j] = 2;
                       break;
             case '3': str[j] = 3;
                       break;
          }
          	
       if (i%1000000 == 0) {printf("%lu\n", i); fflush(stdout);}       

       start = clock();       

       insertTrie(t, str, 23, 22);

       diff += clock() - start;

    }
    
    
    uint64_t msec = diff * 1000 / CLOCKS_PER_SEC;
    uint64_t count = 0;
    
    printf("Nodes in the root block: %d\n",B.nNodes);    
    
    printf("Insertion time: %f microseconds per insertion\n", (float)msec*1000/nEdges);

    uint64_t treeSize = sizeTrie(t);//B.size();        

    printf("Total size: %lu bytes\n", treeSize);
    
    printf("Bits per edge of the graph: %f\n", ((float)treeSize*8)/nEdges);

    printf("Number of blocks in the structure: %lu\n", totalBlocks);
    
    printf("Number of internal nodes in the tree: %lu\n", totalNodes);

    bool found = false;
    
    fseek(stdin,0,SEEK_SET);
    scanf("%lu %lu %lu\n", &n, &n1, &nEdges); 

    diff = 0;
    
    for (uint64_t i = 0; i < 100000000/*nEdges*/; ++i) {
       scanf("%s\n", str);
       for (uint8_t j = 0; j < 23; ++j)
          switch(str[j]) {
             case '0': str[j] = 0;
                       break;
             case '1': str[j] = 1;
                       break;
             case '2': str[j] = 2;
                       break;
             case '3': str[j] = 3;
                       break;
          }
          	
       if (i%1000000 == 0) { printf("%lu %lu\n", i, count); fflush(stdout);}       

       start = clock();       

       found = isEdgeTrie(t, str, 23, 22);//isEdge(&B, str, 23, 22);

       diff += clock() - start;
       if (found) count++;
    }
    
    
    // Ahora genero arcos que no necesariamente están en el grafo y los busco
    
    str[0] = str[1] = str[2] = str[3] = str[4] = str[5] = str[6] = str[7] = str[8] = str[9] = str[10] = str[11] = 0;
    str[12] = str[13] = str[14] = str[15] = str[16] = str[17] = str[18] = str[19] = str[20] = str[21] = str[22] = 0;
     
     
    uint8_t j;
    
    uint64_t i;
    
    for (i = 0; i < 100000000; i++) {
     
       for (j = 22; j >= 0; j--) {
          if (str[j] != 3) break;
          str[j] = 0;
       }
       
       str[j]++;

       if (i%1000000 == 0) {printf("%lu %lu\n", i, count); fflush(stdout);}       

       start = clock();       

       found = isEdgeTrie(t, str, 23, 22); //isEdge(&B, str, 23, 22);

       diff += clock() - start;
       
       if (found) count++;
           
    }    

    msec = diff * 1000 / CLOCKS_PER_SEC;
    
    printf("Search time: %f microseconds per query\n", (float)msec*1000/200000000);

    printf("Number of edges found: %lu\n", count);

    
    
    return 0;
 }

