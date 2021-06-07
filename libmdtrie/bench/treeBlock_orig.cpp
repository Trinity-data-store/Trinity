
#include "treeBlock_orig.h"
#include "tqdm.h"
#include <unistd.h>
#include <sys/time.h>
#include <limits.h>
#include <tqdm.h>

// Transforma un nodo en su representación de par, a un nodo en representacion absoluta en la secuencia DFUDS
typedef unsigned long long int TimeStamp;
static TimeStamp GetTimestamp() {
  struct timeval now;
  gettimeofday(&now, NULL);

  return now.tv_usec + (TimeStamp) now.tv_sec * 1000000;
}

uint16_t absolutePosition(treeNode &node)
 {
    return 4*node.first + node.second;
 }


inline void nextNode(treeNode &node)
 {
     /*node.second = (node.second+1) % 4;
     if (!node.second) node.first++;*/     
     
     node.second = (node.second+1) & 0x3;
     node.first += !node.second;
     
     
 }


inline void prevNode(treeNode &node)
 {
/*     if (!node.second) {
        node.second = 3;
        --node.first;
     }
     else --node.second;*/
     
     node.first -= !node.second;
     node.second = (node.second-1) & 0x3;
 }



void treeBlock::freeTreeBlock() 
 {
    free((void *)dfuds);
    for (uint16_t i = 0; i < nPtrs; ++i)
       ((blockPtr *)ptr)[i].P->freeTreeBlock();
    
    free((void*)ptr);
    free(this); 
 }




void treeBlock::grow(uint16_t extraNodes)
 {
    //if ((sizeArray[nNodes+extraNodes] + 3)/4 > (maxNodes + 3)/4)    
       dfuds = (uint16_t *) realloc(dfuds, sizeof(uint16_t)*((sizeArray[nNodes+extraNodes] + 3)/4));
    maxNodes = 4*((sizeArray[nNodes+extraNodes]+3)/4);
 }


void treeBlock::shrink(uint16_t deletedNodes)
 {
    //if ((sizeArray[nNodes-deletedNodes] + 3)/4 < (maxNodes + 3)/4)
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

nodeInfo stackSS[4096];

subtreeInfo subtrees[4096];

uint16_t depthVector[4096];


treeNode treeBlock::selectSubtree2(uint16_t maxDepth, uint16_t & subTreeSize, uint16_t & depthSelectN)
// depth is the depth of the root of the block
 { 
    treeNode node(0,0); // initialized as the root of the block
    
    uint16_t depth;
    
    uint16_t curFlag = 0;
    
    uint16_t ssTop=0, subtreeTop = 0, depthTop = 0;    
            
    uint8_t cNodeCod = (*dfuds>>12) & 0x000f;
        
    stackSS[ssTop].preorder = 0; stackSS[ssTop++].nChildren =  nChildrenT[cNodeCod];    
    
    nextNode(node);
    
    depth = rootDepth + 1;

    int32_t nextFlag;
    
    if (nPtrs == 0 || curFlag >= nPtrs)
       nextFlag = -1;
    else       
       nextFlag = ((blockPtr *)ptr)[curFlag].flag;

    for (uint16_t i=1; i < nNodes; ++i) {
        
       if (i == nextFlag) {
          ++curFlag;
          if (nPtrs == 0 || curFlag >= nPtrs)
             nextFlag = -1;
          else
             nextFlag = ((blockPtr *)ptr)[curFlag].flag;
          --stackSS[ssTop-1].nChildren;
       }
       else if (depth < maxDepth) {
          stackSS[ssTop].preorder = i; 
          cNodeCod = (dfuds[i>>2]>>shiftT[i & 0x3]) & 0x000f;         
          stackSS[ssTop++].nChildren = nChildrenT[cNodeCod];          
          depth++;
        }
        else --stackSS[ssTop-1].nChildren;

       while (ssTop > 0 && stackSS[ssTop-1].nChildren == 0) {
          subtrees[subtreeTop].preorder = stackSS[ssTop-1].preorder;
          subtrees[subtreeTop++].subtreeSize = i-stackSS[ssTop-1].preorder+1;
                    
          --ssTop;          
          depthVector[depthTop++] = --depth;          
         
          if (ssTop == 0) break;         
          else stackSS[ssTop-1].nChildren--;
       }
                 
    }

    // Ahora debo recorrer el vector subtrees para elegir el subarbol adecuado
    int16_t  nodemin, min, posmin;
    
    uint16_t leftmost = MAX_UINT_16;
    // uint16_t lowerB = 0.25*nNodes, upperB = 0.75*nNodes;
           
    for (uint16_t i = 0; i < subtreeTop; ++i) {
//       if (lowerB <= subtrees[i].subtreeSize && subtrees[i].subtreeSize <= upperB && subtrees[i].preorder < leftmost) {

       if (((float)nNodes/4) <= subtrees[i].subtreeSize && subtrees[i].subtreeSize <= ((float)3*nNodes/4) && subtrees[i].preorder < leftmost) {
          leftmost = nodemin = subtrees[i].preorder;
          posmin = i;
       }
    }
    
    if (leftmost == MAX_UINT_16) {
       int16_t diff; 
       
       nodemin = subtrees[0].preorder, 
       min = nNodes - subtrees[0].subtreeSize - subtrees[0].subtreeSize,
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
    
    return treeNode(nodemin >> 2,nodemin & 0x3);
 }





treeNode treeBlock::selectSubtree(uint16_t maxDepth, uint16_t & subTreeSize, uint16_t & depthSelectN)
// depth is the depth of the root of the block
 { 
    treeNode node(0,0); // initialized as the root of the block
    
    uint16_t depth;
    
    uint16_t curFlag = 0;
    
    uint16_t ssTop=0, subtreeTop = 0, depthTop = 0;    
            
    uint8_t cNodeCod = (*dfuds>>12) & 0x000f;
        
    stackSS[ssTop].preorder = 0; stackSS[ssTop++].nChildren =  nChildrenT[cNodeCod]; 
    
    nextNode(node);
    
    depth = rootDepth + 1;

    int32_t nextFlag;
    
    if (nPtrs == 0 || curFlag >= nPtrs)
       nextFlag = -1;
    else       
       nextFlag = ((blockPtr *)ptr)[curFlag].flag;

    for (uint16_t i=1; i < nNodes; ++i) {
        
       if (i == nextFlag) {
          ++curFlag;
          if (nPtrs == 0 || curFlag >= nPtrs)
             nextFlag = -1;
          else
             nextFlag = ((blockPtr *)ptr)[curFlag].flag;
          --stackSS[ssTop-1].nChildren;
       }
       else if (depth < maxDepth) {
          stackSS[ssTop].preorder = i; 
          cNodeCod = (dfuds[i>>2]>>shiftT[i & 0x3]) & 0x000f;         
          stackSS[ssTop++].nChildren = nChildrenT[cNodeCod];          
          depth++;
        }
        else --stackSS[ssTop-1].nChildren;

       while (ssTop > 0 && stackSS[ssTop-1].nChildren == 0) {
          subtrees[subtreeTop].preorder = stackSS[ssTop-1].preorder;
          subtrees[subtreeTop++].subtreeSize = i-stackSS[ssTop-1].preorder+1;
                    
          --ssTop;          
          depthVector[depthTop++] = --depth;          
         
          if (ssTop == 0) break;         
          else stackSS[ssTop-1].nChildren--;
       }
                 
    }

    // Ahora debo recorrer el vector subtrees para elegir el subarbol adecuado
    int16_t diff, nodemin = subtrees[0].preorder, 
            min = nNodes - subtrees[0].subtreeSize - subtrees[0].subtreeSize,
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
    
    subTreeSize = subtrees[posmin].subtreeSize;
    
    depthSelectN = depthVector[posmin]; 
    
    return treeNode(nodemin >> 2,nodemin & 0x3);
 }


treeNode dummyRootBlockNode(0,0);



void treeBlock::insert(treeNode node, uint8_t str[], uint64_t length, uint16_t level, 
                       uint64_t maxDepth, uint16_t curFlag)
 {
    treeNode nodeAux = node;
    if (length == 0)
    {
        return;
    }
    if (rootDepth </*=*/ L1) Nt = S1;
    else if (rootDepth <= L2) Nt = S2;
    else Nt = S3;
    
    if (ptr!=NULL && curFlag < nPtrs && absolutePosition(node) == ((blockPtr *)ptr)[curFlag].flag) {
       // insertion must be carried out in the root of a child block
        uint8_t cNodeCod = (dfuds[node.first]>>shiftT[node.second]) & 0x000f;
        
        register uint64_t aux = 4*(3-node.second);        
        
        dfuds[node.first] = dfuds[node.first] & ~(0xF << aux); 
           
        dfuds[node.first] = dfuds[node.first] | (insertT[cNodeCod][str[0]] << aux);
        
        ((blockPtr *)ptr)[curFlag].P->insert(dummyRootBlockNode, str, length, level, maxDepth, 0);

        return;
    }
    else
    if (length == 1) {
       uint8_t cNodeCod = (dfuds[node.first]>>shiftT[node.second]) & 0x000f;
       
       register uint64_t aux = 4*(3-node.second);       
       
       dfuds[node.first] = dfuds[node.first] & ~(0xF << aux); 
          
       dfuds[node.first] = dfuds[node.first] | (insertT[cNodeCod][str[0]] << aux);
       return;            
    }
    else 
    if (nNodes + length - 1 <= maxNodes) {
       // there is room in current block for new nodes
                 
       // Se coloca en la posicion donde se insertaran los nuevos descendientes de node
       node = skipChildrenSubtree(node, str[0], level, maxDepth, curFlag);
       
       treeNode origNode, destNode;
       
       --length; 
       
       destNode.first =  (nNodes + length - 1)/4;
       destNode.second = (nNodes + length - 1)%4;
             
       origNode.first = (nNodes - 1)/4;
       origNode.second = (nNodes - 1)%4;
       
       uint16_t preorderNode = absolutePosition(node);       
       uint16_t preorderOrigNode = absolutePosition(origNode);
       uint16_t preorderDestNode =  absolutePosition(destNode);
       
       register uint64_t aux, aux2;       
       
       while (preorderOrigNode >= preorderNode) {
          aux = 4*(3-(preorderDestNode & 0x3) /*destNode.second*/);
          aux2 = preorderDestNode >> 2;
         
          dfuds[aux2/*destNode.first*/] = dfuds[aux2/*destNode.first*/] & ~(0xF << aux);
                                 
          dfuds[aux2/*destNode.first*/] = dfuds[aux2/*destNode.first*/] 
                                 | (((dfuds[preorderOrigNode>>2/*origNode.first*/] >> 4*(3-(preorderOrigNode & 0x3)/*origNode.second*/)) 
                                 & 0x000F) << aux) ;            
          //prevNode(destNode);
          --preorderDestNode;          
          //prevNode(origNode);
          --preorderOrigNode;
       }

       uint8_t cNodeCod = (dfuds[nodeAux.first]>>shiftT[nodeAux.second]) & 0x000f;
        
       dfuds[nodeAux.first] = dfuds[nodeAux.first]
                              & ~(0xF << 4*(3-nodeAux.second)); 
         
       dfuds[nodeAux.first] = dfuds[nodeAux.first] 
                             | (insertT[cNodeCod][str[0]] << 4*(3-nodeAux.second));
       
       //nextNode(origNode);
       ++preorderOrigNode;
    
       for (uint16_t i = 1; i <= length; i++) {
          aux = 4*(3-(preorderOrigNode & 0x3)/*origNode.second*/);
          aux2 = preorderOrigNode >> 2;

          dfuds[aux2/*origNode.first*/] = dfuds[aux2/*origNode.first*/] & ~(0xF << aux); 
          
          dfuds[aux2/*origNode.first*/] = dfuds[aux2/*origNode.first*/] 
                                 | (symbol2NodeT[str[i]] << aux);
          ++nNodes;          
          //nextNode(origNode);
          ++preorderOrigNode;
       }
    
       // Now updates the flags, as new nodes have been added
       if (ptr)
          for (uint16_t i = curFlag; i < nPtrs; ++i)
             ((blockPtr *)ptr)[i].flag += length;
    }
    else {
       // there is no room for the new nodes

       if (nNodes + length - 1 <= Nt) { // block can still grow
          // if the block can still grow, grow it.
          grow(length-1);
          // After growing, recursively inserts the node 
          insert(node, str, length, level, maxDepth, curFlag);
       
       }
       else {
          treeNode selectedNode, originalSelectedNode;
          
          uint16_t subTreeSize, depthSelectedNode;
          
          originalSelectedNode = selectedNode = selectSubtree2(maxDepth, subTreeSize, depthSelectedNode);
                   
          // Ahora copio el subarbol seleccionado a un nuevo bloque
          
          uint16_t *new_dfuds = (uint16_t *)calloc((sizeArray[subTreeSize]+4-1)/4, sizeof(uint16_t));
          
          treeNode destNode(0,0);
          
          uint16_t copiedNodes=0, copiedFlags = 0;
                    
          bool insertionInNewBlock = false;
          
          treeNode insertionNode = node;
          
          uint16_t flag, auxFlag=0, flagSelectedNode;
          
          uint16_t preorderSelectedNode = absolutePosition(selectedNode);    
          
          for (flag = 0; flag < nPtrs; ++flag)
             if (((blockPtr *)ptr)[flag].flag > preorderSelectedNode) break;
          
          flagSelectedNode = flag; // almacena el primer flag que es > al selected node
                                   // esto es para saber despues donde poner el flag del selected
                                   // node
          
          blockPtr *new_ptr;
          
          if (nPtrs > 0) new_ptr = (blockPtr *)malloc(sizeof(blockPtr)*nPtrs);
          else new_ptr = NULL;          
          
          uint16_t curFlagNewBlock; // flag en el nuevo bloque, por si la insercion continua en ese bloque          
             
          bool isInRoot = false;          
          
          uint16_t preorderDestNode = absolutePosition(destNode);          
          
          while (copiedNodes < subTreeSize) {
          
             if (selectedNode == node) {
                // the original node where insertion must be carried out
                // is stored in the child (new) block
                insertionInNewBlock = true;
                
                if (destNode != treeNode(0,0))                
                   insertionNode = destNode; // this will be the node where insertion must be
                                             // carried out in the new block
                else { insertionNode = node; isInRoot = true;}              
                
                curFlagNewBlock = copiedFlags;
             
             }
             
             if (ptr!=NULL && flag < nPtrs && preorderSelectedNode == ((blockPtr *)ptr)[flag].flag) {
               //Copiar puntero para el nuevo bloque
                new_ptr[auxFlag].flag = preorderDestNode;
                new_ptr[auxFlag].P = ((blockPtr *)ptr)[flag].P;
                ++flag;
                ++auxFlag;
                ++copiedFlags;
             }
             
             register uint64_t aux = 4*(3-selectedNode.second);             
             
             new_dfuds[destNode.first] = new_dfuds[destNode.first] 
                                       | (((dfuds[selectedNode.first] >> aux/*4*(3-selectedNode.second)*/)
                                       & 0x000F) << 4*(3-destNode.second));  

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
          	 // el punto de insercion esta despues del subarbol seleccionado
             insertionBeforeSelectedTree = false;   
                    
          treeBlock *new_block = (treeBlock *)malloc(sizeof(treeBlock));
          
          new_block->nNodes = subTreeSize;
          new_block->maxNodes = sizeArray[subTreeSize]; // OJO con este valor, definir bien
          new_block->dfuds = new_dfuds; 
          new_block->rootDepth = depthSelectedNode;
          
          if (auxFlag == 0) {
             if (new_ptr != NULL) free((void *)new_ptr);
             new_block->ptr = NULL;
             new_block->nPtrs = 0;
             
             ptr = realloc(ptr, sizeof(blockPtr)*(nPtrs+1)); // there is a new pointer 
                                                             // in the current block
             
             for (uint16_t i = nPtrs; i > flagSelectedNode; --i) {
                ((blockPtr *)ptr)[i].P = ((blockPtr *)ptr)[i-1].P;
                ((blockPtr *)ptr)[i].flag = ((blockPtr *)ptr)[i-1].flag - subTreeSize + 1;
             }
             
             ((blockPtr *)ptr)[flagSelectedNode].flag = absolutePosition(originalSelectedNode);
             
             ((blockPtr *)ptr)[flagSelectedNode].P = new_block;  // pointer to the new child block
             
             nPtrs++;
          }
          else {
             new_ptr = (blockPtr *)realloc(new_ptr, sizeof(blockPtr)*auxFlag);
          
             new_block->ptr = new_ptr; 
    
             new_block->nPtrs = auxFlag;
             
             ((blockPtr *)ptr)[flagSelectedNode].flag = absolutePosition(originalSelectedNode);
             
             ((blockPtr *)ptr)[flagSelectedNode].P = new_block;
            
             for (uint16_t i = flagSelectedNode+1; flag < nPtrs; ++i, ++flag) {
                ((blockPtr *)ptr)[i].P = ((blockPtr *)ptr)[flag].P;
                ((blockPtr *)ptr)[i].flag = ((blockPtr *)ptr)[flag].flag - subTreeSize + 1;
             }
             
             nPtrs = nPtrs - copiedFlags + 1;
             ptr = realloc(ptr, sizeof(blockPtr)*nPtrs); // reajusta tamaño de arreglo de punteros 
          }   
       
          // Ahora hay que borrar el subarbol copiado al hijo
          flag = flagSelectedNode+1;
          
          nextNode(originalSelectedNode);
          
          preorderSelectedNode = absolutePosition(selectedNode);
                   
          
          while (preorderSelectedNode/*absolutePosition(selectedNode)*/ < nNodes) {
             register uint64_t auxOriginalS = 4*(3-originalSelectedNode.second);
             register uint64_t auxS = 4*(3-selectedNode.second);             
             
             dfuds[originalSelectedNode.first] =  dfuds[originalSelectedNode.first]
                                    & ~(0xF << auxOriginalS/*4*(3-originalSelectedNode.second)*/); 
                                    
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



treeNode treeBlock::skipChildrenSubtree(treeNode &node, uint8_t symbol, uint16_t &curLevel,
                                        uint16_t maxLevel, uint16_t &curFlag)
 {
 	 if (curLevel == maxLevel) return node;
 	     
    int16_t sTop = -1;
      
    uint8_t cNodeCod = (dfuds[node.first]>>shiftT[node.second]) & 0x000f;

    uint8_t skipChild = (uint8_t) childSkipT[cNodeCod][symbol];
    
    uint8_t nChildren = nChildrenT[cNodeCod];
        
    stack[++sTop] = nChildren;
    
    treeNode currNode = node;
    
    nextNode(currNode);

    uint16_t curPreorder = absolutePosition(currNode);

    if (ptr != NULL && curFlag < nPtrs && curPreorder > ((blockPtr *)ptr)[curFlag].flag)
       ++curFlag; 
    
    
    uint16_t nextFlag;
    
    if (nPtrs == 0 || curFlag >= nPtrs)
       nextFlag = -1;
    else       
       nextFlag = ((blockPtr *)ptr)[curFlag].flag;
        
    
    ++curLevel;
    
    uint8_t diff = nChildren - skipChild;
    
    while(curPreorder < nNodes && sTop >= 0 && diff < stack[0])  {
       
       if (curPreorder == nextFlag) {//ptr != NULL && curFlag < nPtrs && curPreorder > ((blockPtr *)ptr)[curFlag].flag)
           ++curFlag;
           if (nPtrs == 0 || curFlag >= nPtrs)
              nextFlag = -1;
           else
              nextFlag = ((blockPtr *)ptr)[curFlag].flag;
           --stack[sTop];
        }
        else if (curLevel < maxLevel) {
        	  cNodeCod = (dfuds[curPreorder >> 2/*currNode.first*/]>>shiftT[curPreorder & 0x3/*currNode.second*/]) & 0x000f;
           stack[++sTop] = nChildrenT[cNodeCod];
           ++curLevel;
        }
        else --stack[sTop];

        //nextNode(currNode);
        ++curPreorder;
               
       while (sTop >= 0 && stack[sTop] == 0) {
          --sTop;          
          --curLevel;
          if (sTop >= 0) --stack[sTop];
       }
    }
    
    currNode.first = curPreorder >> 2;
    currNode.second = curPreorder & 0x3;
    return currNode;
 }


treeNode NULL_NODE = treeNode((NODE_TYPE)-1, 0);


treeNode treeBlock::child(treeBlock *&p, treeNode & node, uint8_t symbol, uint16_t &curLevel, uint16_t maxLevel,
                          uint16_t &curFlag)
 {
    
    uint8_t cNodeCod = (dfuds[node.first]>>shiftT[node.second]) & 0x000f;
    
    uint8_t soughtChild = (uint8_t) childT[cNodeCod][symbol];
    
    if (soughtChild == (uint8_t)-1) return NULL_NODE; //treeNode((NODE_TYPE)-1, 0); // No such child to go down
    
    if (curLevel == maxLevel && soughtChild != (uint8_t)-1) return node;
       
    treeNode currNode;    
     
    if (ptr != NULL && curFlag < nPtrs && absolutePosition(node) == ((blockPtr *)ptr)[curFlag].flag) {
       p = ((blockPtr *)ptr)[curFlag].P;
       curFlag = 0;
       treeNode auxNode;
       auxNode.first = auxNode.second = 0;
       currNode = p->skipChildrenSubtree(auxNode, symbol, curLevel, maxLevel, curFlag);
    }    
    else    
       currNode = skipChildrenSubtree(node, symbol, curLevel, maxLevel, curFlag);
    
    return currNode;  
 }



void insertar(treeBlock *root, uint8_t *str, uint64_t length, uint16_t level, uint16_t maxDepth) 
 {
    treeBlock *curBlock = root;
    uint64_t i;
    treeNode curNode(0,0), curNodeAux;
    uint16_t /*level = 0,*/ curFlag = 0;
        
    for (i = 0; i < length; ++i) {
    //    curBlockAux = curBlock;       
       curNodeAux = curBlock->child(curBlock, curNode, str[i], level, maxDepth, curFlag);
       
       if (curNodeAux.first == (NODE_TYPE)-1) break;
       else curNode = curNodeAux;
       
       
       if (curBlock->nPtrs > 0 && absolutePosition(curNode) == curFlag) {
          // Goes down to a child block
          curBlock = curBlock->getPointer(curFlag);
          curNode.first = 0;
          curNode.second = 0;
          curFlag = 0;
       }
    }  
    
    // inserts str[i..length-1] starting from the current node
    // The new nodes inserted will descend from curNode 
    curBlock->insert(curNode, &str[i], length-i, level, maxDepth, curFlag);
    
 }


void insertTrie(trieNode *t, uint8_t *str, uint64_t length, uint16_t maxDepth)
 {
    uint64_t i = 0;
    treeBlock *p;
    
    while (t->children[str[i]]) 
       t = t->children[str[i++]];
    
    while (i < L1) {
       t->children[str[i]] = (trieNode *)malloc(sizeof(trieNode));
       t = t->children[str[i]];
       i++;
       t->children[0] = t->children[1] = t->children[2] = t->children[3] = NULL;
       t->block = NULL;
    }
    
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
    
    insertar(p, &str[i], length-i, i, maxDepth); 
 }




bool isEdge(treeBlock *root, uint8_t *str, uint64_t length, uint16_t level, uint16_t maxDepth) 
 {
    treeBlock *curBlock = root;
    uint64_t i;
    treeNode curNode(0,0), curNodeAux;
    uint16_t /*level = 0,*/ curFlag = 0;
    
    for (i = 0; i < length; ++i) {
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
          curFlag = 0;
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
    // uint16_t curFlag = 0;
    // uint16_t subTreeSize;
    // uint16_t level = 0;
    
    trieNode *t = (trieNode *) malloc(sizeof(trieNode));
    t->children[0] = t->children[1] = t->children[2] = t->children[3] = NULL;
    t->block = NULL;


    double alpha = 0.99;
    
    
    N1 = 4;
    
    // Tamaño maximo del bloque, probar con 96, 128, 256, 512, 1024
    Nt = S3;
        
    sizeArray = (uint16_t *) malloc(sizeof(uint16_t)*(Nt+1));
    
    for (int i = 0; i <= Nt; ++i) {
       if (i > N1) N1 = 4*(((uint16_t)ceil((double)N1/alpha)+3)/4);
       sizeArray[i] = N1;
    }

    
    node.first = 0;
    node.second = 0;
        
    // uint64_t n, n1, nEdges;
    uint64_t nEdges = 14583357;

    uint8_t str[50];
    


    // leaf_config *leaf_point = new leaf_config(dimensions);

    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    FILE *fp = fopen("../libmdtrie/bench/data/sample_shuf.txt", "r");

    // If the file cannot be open
    if (fp == NULL)
    {
        fprintf(stderr, "file not found\n");
        char cwd[PATH_MAX];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("Current working dir: %s\n", cwd);
        } else {
            perror("getcwd() error");
        }
        exit(EXIT_FAILURE);
    }


    tqdm bar;
    int n_points = 0;
    TimeStamp start, diff;
    int max_depth = 32;
    int str_len = max_depth + 1;
    while ((read = getline(&line, &len, fp)) != -1)
    {
        bar.progress(n_points, nEdges);
        // Get the first token
        char *token = strtok(line, " ");
        char *ptr;
        // Skip the second and third token
        for (int i = 0; i < 2; i ++){
            token = strtok(NULL, " ");
        }
        uint64_t coordinates[2];
        for (int i = 0; i < 2; i++){
            token = strtok(NULL, " ");
            coordinates[i] = strtoul(token, &ptr, 10);;
        }
        start = GetTimestamp();
      //   Count in the time to change to Morton code
        for (int i = 0; i < str_len; i ++){
            uint64_t result = 0;
            for (int j = 0; j < 2; j++)
            {
                int coordinate = coordinates[j];
                int bit = (coordinate >> (max_depth - i - 1)) & 1;
                result *= 2;
                result += bit;
            }
            str[i] = result;
        }
        
        insertTrie(t, str, str_len, max_depth);
        diff += GetTimestamp() - start;
        n_points ++;
    }
    bar.finish();

    // scanf("%lu %lu %lu\n", &n, &n1, &nEdges); 
    
    // for (uint64_t i = 0; i < nEdges; ++i) {
    // //    scanf("%s\n", str);
    //    for (uint8_t j = 0; j < 23; ++j)
    //       switch(str[j]) {
    //          case '0': str[j] = 0;
    //                    break;
    //          case '1': str[j] = 1;
    //                    break;
    //          case '2': str[j] = 2;
    //                    break;
    //          case '3': str[j] = 3;
    //                    break;
    //       }
          	
    //    if (i%1000000 == 0) {printf("%lu\n", i); fflush(stdout);}       

    //    start = clock();       

    //    insertTrie(t, str, 23, 22);

    //    diff += clock() - start;

    // }
    
    uint64_t msec = diff * 1000 / CLOCKS_PER_SEC;
    // uint64_t count = 0;
    
    // printf("Nodes in the root block: %d\n",B.nNodes);    
    
    printf("Insertion time: %f microseconds per insertion\n", (float)msec*1000 / nEdges);

    uint64_t treeSize = sizeTrie(t);//B.size();        

    printf("Total size: %lu bytes\n", treeSize);
    
    // printf("Bits per edge of the graph: %f\n", ((float)treeSize*8)/nEdges);

    // printf("Numero de bloques en la estructura: %lu\n", totalBlocks);
    
    // printf("Numero de nodos internos en el arbol: %lu\n", totalNodes);
    
    // return 0;
 }
