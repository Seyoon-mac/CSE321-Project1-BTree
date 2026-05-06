#include "BTree.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <vector>

namespace BTreeImpl {

struct BTreeNode* root = NULL;
int split_count = 0;

// 새 노드 생성 및 초기화
struct BTreeNode* createNode(bool leaf){
    BTreeNode* node = new BTreeNode;
    node->leaf = leaf;
    node->cnt_key = 0;
    node->cnt_child = 0;

    for (int i = 0; i < max_keys +1; i++){
        node->keys[i] = 0;
        node->rids[i] = -1;
    }
    for (int i = 0; i < max_children +1; i++){
        node->child[i] = NULL;
    }
    return node;
}

// 현재 노드 안에서만 target 찾기 + 위치 계산
bool searchNode(BTreeNode* node, int target, int* pos){
    int i = 0;
    while (i < node->cnt_key && target > node->keys[i]){
        i++;
    }
    *pos = i;
    if (i < node->cnt_key && target == node->keys[i]){
        return true;
    }
    return false;
}

// 현재 노드에서 따라 내려가면서 찾기
static BTreeNode* search(BTreeNode* node, int target){
    if (node == NULL){
        return NULL;
    }
    int pos;
    if (searchNode(node, target, &pos)){
        return node;
    }
    if (node->leaf){
        return NULL;
    }
    return search(node->child[pos], target);
}

// 전체 트리 검색 wrapper
struct BTreeNode* searchNode_root(int key){
    return search(root, key);
}

// key로 RID 검색, 못 찾으면 -1 반환
int searchRID(int key){
    BTreeNode* node = search(root, key);

    if (node == NULL){
        return -1;
    }

    int pos = 0;
    bool found = searchNode(node, key, &pos);

    if (!found){
        return -1;
    }

    return node->rids[pos];
}

// split count 초기화
void resetSplitCount(){
    split_count = 0;
}

// split count 반환
int getSplitCount(){
    return split_count;
}

// 트리 초기화
void resetTree(){
    root = NULL;
    split_count = 0;
}

// 부모 노드 parent의 i번째 자식이 꽉 찼을 때, 그 자식을 둘로 나누고 가운데 key를 부모로 올린다
void splitChild(BTreeNode* parent, int i){
    BTreeNode* fullChild = parent->child[i];
    BTreeNode* newChild = createNode(fullChild->leaf);
    split_count++;

    int middleKey = fullChild->keys[min_keys];
    int middleRid = fullChild->rids[min_keys];

    newChild->cnt_key = min_keys;
    for (int j = 0; j < newChild->cnt_key; j++){
        newChild->keys[j] = fullChild->keys[(min_keys+1) +j];
        newChild->rids[j] = fullChild->rids[(min_keys+1) +j];
    }
    fullChild->cnt_key = min_keys;

    if (!fullChild->leaf){
        for (int j = 0; j < newChild->cnt_key +1; j++){
            newChild->child[j] = fullChild->child[(min_keys+1) +j];
        }
        newChild->cnt_child = newChild->cnt_key +1;
        fullChild->cnt_child = fullChild->cnt_key +1;
    }

    for (int j = parent->cnt_key; j >= i +1; j--){
        parent->child[j+1] = parent->child[j];
    }
    parent->child[i+1] = newChild;

    for (int j = parent->cnt_key -1; j >= i; j--){
        parent->keys[j+1] = parent->keys[j];
        parent->rids[j+1] = parent->rids[j];
    }
    parent->keys[i] = middleKey;
    parent->rids[i] = middleRid;
    parent->cnt_key++;
    parent->cnt_child = parent->cnt_key +1;
}

// 꽉 차지 않은 node를 시작점으로 해서, key를 B-Tree 규칙에 맞게 삽입
void insertNonFull(BTreeNode* node, int key, int rid){
    int i = node->cnt_key -1;

    if (node->leaf){
        while (i >= 0 && key < node->keys[i]){
            node->keys[i+1] = node->keys[i];
            node->rids[i+1] = node->rids[i];
            i--;
        }
        node->keys[i+1] = key;
        node->rids[i+1] = rid;
        node->cnt_key++;
    }
    else{
        while (i >= 0 && key < node->keys[i]){
            i--;
        }
        i++;
        if (node->child[i]->cnt_key == max_keys){
            splitChild(node, i);
            if (key > node->keys[i]){
                i++;
            }
        }
        insertNonFull(node->child[i], key, rid);
    }
}

// 트리 전체에 key, rid 삽입
void insertKey(int key, int rid){
    if (root == NULL){
        root = createNode(true);
        root->keys[0] = key;
        root->rids[0] = rid;
        root->cnt_key = 1;
        return;
    }

    if (searchRID(key) != -1){
        return;
    }

    if (root->cnt_key == max_keys){
        BTreeNode* newRoot = createNode(false);
        newRoot->child[0] = root;
        newRoot->cnt_child = 1;

        splitChild(newRoot, 0);

        int i = 0;
        if (key > newRoot->keys[0]){
            i++;
        }

        insertNonFull(newRoot->child[i], key, rid);
        root = newRoot;
    }
    else{
        insertNonFull(root, key, rid);
    }
}

// 삽입 결과 확인용 오름차순 순회
void traverse(BTreeNode* node){
    if (node == NULL){
        return;
    }
    int i;
    for (i = 0; i < node->cnt_key; i++){
        if (!node->leaf){
            traverse(node->child[i]);
        }
        printf("%d ", node->keys[i]);
    }
    if (!node->leaf){
        traverse(node->child[i]);
    }
}

void printTree(){
    traverse(root);
    printf("\n");
}

// 가장 왼쪽 child만 따라가서 높이 계산
int getHeight(BTreeNode* node){
    if (node == NULL){
        return 0;
    }
    if (node->leaf){
        return 1;
    }
    return 1 + getHeight(node->child[0]);
}

int getTreeHeight(){
    return getHeight(root);
}

// 전체 노드 수 재귀적으로 카운트
int getNodeCount(BTreeNode* node){
    if (node == NULL){
        return 0;
    }
    int count = 1;
    if (!node->leaf){
        for (int i = 0; i <= node->cnt_key; i++){
            count += getNodeCount(node->child[i]);
        }
    }
    return count;
}

int getTreeNodeCount(){
    return getNodeCount(root);
}

// 전체 key 수 재귀적으로 카운트
int getKeyCount(BTreeNode* node){
    if (node == NULL){
        return 0;
    }
    int count = node->cnt_key;
    if (!node->leaf){
        for (int i = 0; i <= node->cnt_key; i++){
            count += getKeyCount(node->child[i]);
        }
    }
    return count;
}

// 노드 평균 사용률 = 전체 key 수 / (노드 수 * max_keys)
double getNodeUtilization(){
    int nodeCount = getNodeCount(root);
    if (nodeCount == 0){
        return 0.0;
    }
    int keyCount = getKeyCount(root);
    return (double)keyCount / ((double)nodeCount * max_keys);
}

// 현재 노드 안에서 key 위치 또는 내려갈 child index 찾기
int findKeyIndex(BTreeNode* node, int key){
    int idx = 0;
    while (idx < node->cnt_key && node->keys[idx] < key){
        idx++;
    }
    return idx;
}

// leaf 노드에서 idx 위치의 key, rid 삭제
void removeFromLeaf(BTreeNode* node, int idx){
    for (int i = idx +1; i < node->cnt_key; i++){
        node->keys[i-1] = node->keys[i];
        node->rids[i-1] = node->rids[i];
    }
    node->cnt_key--;
    node->keys[node->cnt_key] = 0;
    node->rids[node->cnt_key] = -1;
}

// node->keys[idx] 바로 이전 값, 왼쪽 subtree에서 가장 큰 key 찾기
int getPredecessor(BTreeNode* node, int idx){
    BTreeNode* cur = node->child[idx];
    while (!cur->leaf){
        cur = cur->child[cur->cnt_key];
    }
    return cur->keys[cur->cnt_key -1];
}

// predecessor의 rid 찾기
int getPredecessorRid(BTreeNode* node, int idx){
    BTreeNode* cur = node->child[idx];
    while (!cur->leaf){
        cur = cur->child[cur->cnt_key];
    }
    return cur->rids[cur->cnt_key -1];
}

// node->keys[idx] 바로 다음 값, 오른쪽 subtree에서 가장 작은 key 찾기
int getSuccessor(BTreeNode* node, int idx){
    BTreeNode* cur = node->child[idx +1];
    while (!cur->leaf){
        cur = cur->child[0];
    }
    return cur->keys[0];
}

// successor의 rid 찾기
int getSuccessorRid(BTreeNode* node, int idx){
    BTreeNode* cur = node->child[idx +1];
    while (!cur->leaf){
        cur = cur->child[0];
    }
    return cur->rids[0];
}

// 두 child와 부모 key 하나를 합치기
/*
node->child[idx]
node->keys[idx]
node->child[idx+1]
합치기
*/
void mergeNode(BTreeNode* node, int idx){
    BTreeNode* child = node->child[idx];
    BTreeNode* sibling = node->child[idx +1];

    child->keys[child->cnt_key] = node->keys[idx];
    child->rids[child->cnt_key] = node->rids[idx];

    for (int i = 0; i < sibling->cnt_key; i++){
        child->keys[child->cnt_key +1 +i] = sibling->keys[i];
        child->rids[child->cnt_key +1 +i] = sibling->rids[i];
    }

    if (!child->leaf){
        for (int i = 0; i <= sibling->cnt_key; i++){
            child->child[child->cnt_key +1 +i] = sibling->child[i];
        }
    }

    child->cnt_key = child->cnt_key + sibling->cnt_key +1;

    if (!child->leaf){
        child->cnt_child = child->cnt_key +1;
    }

    for (int i = idx +1; i < node->cnt_key; i++){
        node->keys[i-1] = node->keys[i];
        node->rids[i-1] = node->rids[i];
    }
    for (int i = idx +2; i <= node->cnt_key; i++){
        node->child[i-1] = node->child[i];
    }

    node->cnt_key--;
    if (!node->leaf){
        node->cnt_child = node->cnt_key +1;
    }

    delete sibling;
}

// child가 key 부족일 때 왼쪽 sibling에서 key 하나 빌려오기
/*
node              : parent 노드
node->child[idx]  : key를 보강해야 하는 child
node->child[idx-1]: 왼쪽 sibling
*/
void borrowFromLeft(BTreeNode* node, int idx){
    BTreeNode* child = node->child[idx];
    BTreeNode* sibling = node->child[idx -1];

    for (int i = child->cnt_key -1; i >= 0; i--){
        child->keys[i+1] = child->keys[i];
        child->rids[i+1] = child->rids[i];
    }
    if (!child->leaf){
        for (int i = child->cnt_child -1; i >= 0; i--){
            child->child[i+1] = child->child[i];
        }
    }

    child->keys[0] = node->keys[idx -1];
    child->rids[0] = node->rids[idx -1];

    if (!child->leaf){
        child->child[0] = sibling->child[sibling->cnt_key];
    }

    node->keys[idx -1] = sibling->keys[sibling->cnt_key -1];
    node->rids[idx -1] = sibling->rids[sibling->cnt_key -1];
    sibling->keys[sibling->cnt_key -1] = 0;
    sibling->rids[sibling->cnt_key -1] = -1;
    sibling->child[sibling->cnt_key] = NULL;

    child->cnt_key++;
    sibling->cnt_key--;

    if (!child->leaf){
        child->cnt_child = child->cnt_key +1;
        sibling->cnt_child = sibling->cnt_key +1;
    }
}

// 오른쪽 sibling에서 key 하나 빌려오기
void borrowFromRight(BTreeNode* node, int idx){
    BTreeNode* child = node->child[idx];
    BTreeNode* sibling = node->child[idx +1];

    child->keys[child->cnt_key] = node->keys[idx];
    child->rids[child->cnt_key] = node->rids[idx];

    if (!child->leaf){
        child->child[child->cnt_child] = sibling->child[0];
    }

    node->keys[idx] = sibling->keys[0];
    node->rids[idx] = sibling->rids[0];

    for (int i = 1; i < sibling->cnt_key; i++){
        sibling->keys[i-1] = sibling->keys[i];
        sibling->rids[i-1] = sibling->rids[i];
    }
    sibling->keys[sibling->cnt_key -1] = 0;
    sibling->rids[sibling->cnt_key -1] = -1;

    if (!sibling->leaf){
        for (int i = 1; i <= sibling->cnt_key; i++){
            sibling->child[i-1] = sibling->child[i];
        }
        sibling->child[sibling->cnt_key] = NULL;
    }

    child->cnt_key++;
    sibling->cnt_key--;

    if (!child->leaf){
        child->cnt_child = child->cnt_key +1;
        sibling->cnt_child = sibling->cnt_key +1;
    }
}

// child key 부족 시 borrow 또는 merge로 균형 맞추기
void fillChild(BTreeNode* node, int idx){
    if (idx != 0 && node->child[idx -1]->cnt_key > min_keys){
        borrowFromLeft(node, idx);
    }
    else if (idx != node->cnt_key && node->child[idx +1]->cnt_key > min_keys){
        borrowFromRight(node, idx);
    }
    else{
        if (idx != node->cnt_key){
            mergeNode(node, idx);
        }
        else{
            mergeNode(node, idx -1);
        }
    }
}

void deleteFromNode(BTreeNode* node, int key);

// 내부 노드에서 key 삭제 - predecessor/successor로 대체하거나 merge
void removeFromNonLeaf(BTreeNode* node, int idx){
    int key = node->keys[idx];

    if (node->child[idx]->cnt_key > min_keys){
        int pred = getPredecessor(node, idx);
        int predRid = getPredecessorRid(node, idx);

        node->keys[idx] = pred;
        node->rids[idx] = predRid;

        deleteFromNode(node->child[idx], pred);
    }
    else if (node->child[idx +1]->cnt_key > min_keys){
        int succ = getSuccessor(node, idx);
        int succRid = getSuccessorRid(node, idx);

        node->keys[idx] = succ;
        node->rids[idx] = succRid;

        deleteFromNode(node->child[idx +1], succ);
    }
    else{
        mergeNode(node, idx);
        deleteFromNode(node->child[idx], key);
    }
}

// 노드에서 key 찾아 삭제, 필요시 fillChild로 균형 유지
void deleteFromNode(BTreeNode* node, int key){
    int idx = findKeyIndex(node, key);

    if (idx < node->cnt_key && node->keys[idx] == key){
        if (node->leaf){
            removeFromLeaf(node, idx);
        }
        else{
            removeFromNonLeaf(node, idx);
        }
    }
    else{
        if (node->leaf){
            return;
        }

        bool isLast = (idx == node->cnt_key);

        if (node->child[idx]->cnt_key < min_keys +1){
            fillChild(node, idx);
        }

        // fillChild 후 idx가 바뀔 수 있으므로 재확인
        if (isLast && idx > node->cnt_key){
            deleteFromNode(node->child[idx -1], key);
        }
        else{
            deleteFromNode(node->child[idx], key);
        }
    }
}

// 트리 전체에서 key 삭제
void deleteKey(int key){
    if (root == NULL){
        return;
    }

    deleteFromNode(root, key);

    if (root->cnt_key == 0){
        BTreeNode* oldRoot = root;
        if (root->leaf){
            root = NULL;
        }
        else{
            root = root->child[0];
        }
        delete oldRoot;
    }
}

// range query용 순회
void collectRange(BTreeNode* node, int low, int high, std::vector<int>& result){
    if (node == NULL){
        return;
    }

    int i = 0;

    for (i = 0; i < node->cnt_key; i++){
        if (!node->leaf){
            collectRange(node->child[i], low, high, result);
        }

        if (node->keys[i] >= low && node->keys[i] <= high){
            result.push_back(node->rids[i]);
        }
    }

    if (!node->leaf){
        collectRange(node->child[i], low, high, result);
    }
}

// range query wrapper
std::vector<int> searchRange(int low, int high){
    std::vector<int> result;
    collectRange(root, low, high, result);
    return result;
}

} // namespace BTreeImpl