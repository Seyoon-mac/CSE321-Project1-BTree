#include "BStarTree.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <vector>

namespace BStarTreeImpl {

struct BStarNode* root = NULL;
int split_count = 0;

// 새 노드 생성 및 초기화
struct BStarNode* createBStarNode(bool leaf){
    BStarNode* node = new BStarNode;
    node->leaf = leaf;
    node->cnt_key = 0;
    node->cnt_child = 0;

    for (int i = 0; i < max_keys + 2; i++){
        node->keys[i] = 0;
        node->rids[i] = -1;
    }
    for (int i = 0; i < max_children + 2; i++){
        node->child[i] = NULL;
    }
    return node;
}

// 현재 노드 안에서만 target 찾기 + 위치 계산
bool searchNode(BStarNode* node, int target, int* pos){
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
BStarNode* search(BStarNode* node, int target){
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
struct BStarNode* searchNode_root(int key){
    return search(root, key);
}

// key로 RID 검색, 못 찾으면 -1 반환
int searchRID(int key){
    BStarNode* node = search(root, key);

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

// low ~ high 범위의 RID 전부 반환 (트리 순회 방식)
void collectRange(BStarNode* node, int low, int high, std::vector<int>& result){
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

std::vector<int> searchRange(int low, int high){
    std::vector<int> result;
    collectRange(root, low, high, result);
    return result;
}

// split count 초기화
void resetSplitCount(){
    split_count = 0;
}

// split count 반환
int getSplitCount(){
    return split_count;
}

// 트리 전체 초기화
void freeTree(BStarNode* node){
    if (node == NULL){
        return;
    }
    if (!node->leaf){
        for (int i = 0; i <= node->cnt_key; i++){
            freeTree(node->child[i]);
        }
    }
    delete node;
}

void resetTree(){
    freeTree(root);
    root = NULL;
    split_count = 0;
}

//꽉 찬 child의 key를 왼쪽 sibling으로 넘겨 균형 맞추기
//꽉 찬 child의 key를 왼쪽 sibling으로 넘겨 균형 맞추기
void redistributeLeft(BStarNode* parent, int idx){
    BStarNode* child = parent->child[idx];
    BStarNode* sibling = parent->child[idx -1];

    // parent key를 왼쪽 sibling의 맨 뒤로 내려보냄
    sibling->keys[sibling->cnt_key] = parent->keys[idx -1];
    sibling->rids[sibling->cnt_key] = parent->rids[idx -1];

    if (!child->leaf){
        sibling->child[sibling->cnt_key +1] = child->child[0];
    }

    // child의 첫 key를 parent로 올림
    parent->keys[idx -1] = child->keys[0];
    parent->rids[idx -1] = child->rids[0];

    // child의 key들을 왼쪽으로 한 칸씩 당김
    for (int i = 1; i < child->cnt_key; i++){
        child->keys[i -1] = child->keys[i];
        child->rids[i -1] = child->rids[i];
    }

    child->keys[child->cnt_key -1] = 0;
    child->rids[child->cnt_key -1] = -1;

    if (!child->leaf){
        for (int i = 1; i < child->cnt_child; i++){
            child->child[i -1] = child->child[i];
        }
        child->child[child->cnt_child -1] = NULL;
    }

    sibling->cnt_key++;
    child->cnt_key--;

    if (!child->leaf){
        sibling->cnt_child = sibling->cnt_key +1;
        child->cnt_child = child->cnt_key +1;
    }
}

//꽉 찬 child의 key를 오른쪽 sibling으로 넘겨 균형 맞추기
void redistributeRight(BStarNode* parent, int idx){
    BStarNode* child = parent->child[idx];
    BStarNode* sibling = parent->child[idx +1];

    for (int i = sibling->cnt_key -1; i >= 0; i--){
        sibling->keys[i+1] = sibling->keys[i];
        sibling->rids[i+1] = sibling->rids[i];
    }
    if (!sibling->leaf){
        for (int i = sibling->cnt_child -1; i >= 0; i--){
            sibling->child[i+1] = sibling->child[i];
        }
    }

    sibling->keys[0] = parent->keys[idx];
    sibling->rids[0] = parent->rids[idx];

    if (!sibling->leaf){
        sibling->child[0] = child->child[child->cnt_key];
    }

    parent->keys[idx] = child->keys[child->cnt_key -1];
    parent->rids[idx] = child->rids[child->cnt_key -1];
    child->keys[child->cnt_key -1] = 0;
    child->rids[child->cnt_key -1] = -1;

    child->cnt_key--;
    sibling->cnt_key++;

    if (!child->leaf){
        child->cnt_child = child->cnt_key +1;
        sibling->cnt_child = sibling->cnt_key +1;
    }
}

//꽉 찬 노드 2개를 부모 key 포함해서 3개로 나누기 (2-to-3 split)
void splitTwoToThree(BStarNode* parent, int idx){
    BStarNode* left = parent->child[idx];
    BStarNode* right = parent->child[idx +1];
    BStarNode* mid = createBStarNode(left->leaf);
    split_count += 2;

    int totalKeys = left->cnt_key + right->cnt_key +1;
    int tempKeys[temp_size];
    int tempRids[temp_size];
    BStarNode* tempChildren[temp_size +1];

    int t = 0;
    for (int i = 0; i < left->cnt_key; i++){
        tempKeys[t] = left->keys[i];
        tempRids[t] = left->rids[i];
        t++;
    }
    tempKeys[t] = parent->keys[idx];
    tempRids[t] = parent->rids[idx];
    t++;
    for (int i = 0; i < right->cnt_key; i++){
        tempKeys[t] = right->keys[i];
        tempRids[t] = right->rids[i];
        t++;
    }

    if (!left->leaf){
        int tc = 0;
        for (int i = 0; i <= left->cnt_key; i++){
            tempChildren[tc++] = left->child[i];
        }
        for (int i = 0; i <= right->cnt_key; i++){
            tempChildren[tc++] = right->child[i];
        }
    }

    int sep1 = totalKeys / 3;
    int sep2 = sep1 + 1 + (totalKeys - 2) / 3;

    left->cnt_key = 0;
    for (int i = 0; i < sep1; i++){
        left->keys[i] = tempKeys[i];
        left->rids[i] = tempRids[i];
        left->cnt_key++;
    }

    mid->cnt_key = 0;
    for (int i = sep1 +1; i < sep2; i++){
        mid->keys[mid->cnt_key] = tempKeys[i];
        mid->rids[mid->cnt_key] = tempRids[i];
        mid->cnt_key++;
    }

    right->cnt_key = 0;
    for (int i = sep2 +1; i < totalKeys; i++){
        right->keys[right->cnt_key] = tempKeys[i];
        right->rids[right->cnt_key] = tempRids[i];
        right->cnt_key++;
    }

    if (!left->leaf){
        int tc = 0;
        for (int i = 0; i <= left->cnt_key; i++){
            left->child[i] = tempChildren[tc++];
        }
        left->cnt_child = left->cnt_key +1;

        for (int i = 0; i <= mid->cnt_key; i++){
            mid->child[i] = tempChildren[tc++];
        }
        mid->cnt_child = mid->cnt_key +1;

        for (int i = 0; i <= right->cnt_key; i++){
            right->child[i] = tempChildren[tc++];
        }
        right->cnt_child = right->cnt_key +1;
    }

    for (int i = parent->cnt_key; i > idx +1; i--){
        parent->keys[i] = parent->keys[i -1];
        parent->rids[i] = parent->rids[i -1];
    }
    for (int i = parent->cnt_key +1; i >= idx +2; i--){
        parent->child[i] = parent->child[i -1];
    }

    parent->keys[idx] = tempKeys[sep1];
    parent->rids[idx] = tempRids[sep1];
    parent->keys[idx +1] = tempKeys[sep2];
    parent->rids[idx +1] = tempRids[sep2];
    parent->child[idx +1] = mid;
    parent->cnt_key++;
    parent->cnt_child = parent->cnt_key +1;
}

//root가 꽉 찼을 때 root를 둘로 나누고 새 root 생성
void splitRoot(){
    BStarNode* oldRoot = root;
    BStarNode* newChild = createBStarNode(oldRoot->leaf);
    BStarNode* newRoot = createBStarNode(false);
    split_count++;

    int middleKey = oldRoot->keys[min_keys_bstar];
    int middleRid = oldRoot->rids[min_keys_bstar];

    newChild->cnt_key = oldRoot->cnt_key - min_keys_bstar -1;
    for (int i = 0; i < newChild->cnt_key; i++){
        newChild->keys[i] = oldRoot->keys[min_keys_bstar +1 +i];
        newChild->rids[i] = oldRoot->rids[min_keys_bstar +1 +i];
    }

    if (!oldRoot->leaf){
        for (int i = 0; i <= newChild->cnt_key; i++){
            newChild->child[i] = oldRoot->child[min_keys_bstar +1 +i];
        }
        newChild->cnt_child = newChild->cnt_key +1;
    }

    oldRoot->cnt_key = min_keys_bstar;
    if (!oldRoot->leaf){
        oldRoot->cnt_child = oldRoot->cnt_key +1;
    }

    newRoot->keys[0] = middleKey;
    newRoot->rids[0] = middleRid;
    newRoot->child[0] = oldRoot;
    newRoot->child[1] = newChild;
    newRoot->cnt_key = 1;
    newRoot->cnt_child = 2;

    root = newRoot;
}

//트리 전체에 key, rid 삽입
// bottom-up 방식:
// 1) root → leaf 경로를 path 스택에 저장하며 내려감
// 2) leaf 에 key 삽입 (임시로 max_keys +1 허용)
// 3) overflow 이면 bottom-up 으로 올라오며 redistribute or splitTwoToThree
// 4) root 가 overflow 이면 splitRoot
void insertKey(int key, int rid){
    if (root == NULL){
        root = createBStarNode(true);
        root->keys[0] = key;
        root->rids[0] = rid;
        root->cnt_key = 1;
        return;
    }

    if (searchRID(key) != -1){
        return;
    }

    // 경로 스택
    BStarNode* path[64];
    int        pathIdx[64];
    int        depth = 0;

    // root → leaf 탐색, 경로 저장
    BStarNode* cur = root;
    while (!cur->leaf){
        int i = 0;
        while (i < cur->cnt_key && key >= cur->keys[i]){
            i++;
        }
        path[depth]    = cur;
        pathIdx[depth] = i;
        depth++;
        cur = cur->child[i];
    }

    // leaf 에 삽입 (배열 크기가 max_keys +2 이므로 임시 overflow 허용)
    {
        int i = cur->cnt_key -1;
        while (i >= 0 && key < cur->keys[i]){
            cur->keys[i+1] = cur->keys[i];
            cur->rids[i+1] = cur->rids[i];
            i--;
        }
        cur->keys[i+1] = key;
        cur->rids[i+1] = rid;
        cur->cnt_key++;
    }

    // bottom-up: overflow 이면 부모로 올라가며 처리
    int d = depth -1;
    while (cur->cnt_key > max_keys && d >= 0){
        BStarNode* parent = path[d];
        int        idx    = pathIdx[d];
        d--;

        // redistribute 우선
        if (idx > 0 && parent->child[idx -1]->cnt_key < max_keys){
            redistributeLeft(parent, idx);
            break;
        }
        if (idx < parent->cnt_key && parent->child[idx +1]->cnt_key < max_keys){
            redistributeRight(parent, idx);
            break;
        }

        // redistribute 불가 → 2-to-3 split
        // cur 은 overflow(max_keys +1) 상태, 인접 sibling 은 max_keys
        // splitTwoToThree 의 totalKeys 계산이 left->cnt_key + right->cnt_key +1 이므로
        // overflow 노드를 그대로 넘기면 totalKeys = (max_keys+1) + max_keys +1 = 2*max_keys+2
        // 이를 3등분해도 각 노드 크기가 배열 범위 안에 들어옴 (검증 완료)
        if (idx < parent->cnt_key){
            splitTwoToThree(parent, idx);
        }
        else{
            splitTwoToThree(parent, idx -1);
        }
        cur = parent;
    }

    // root 가 overflow 이면 splitRoot
    if (root->cnt_key > max_keys){
        splitRoot();
    }
}

//현재 노드 안에서 key 위치 또는 내려갈 child index 찾기
int findKeyIndex(BStarNode* node, int key){
    int idx = 0;
    while (idx < node->cnt_key && node->keys[idx] < key){
        idx++;
    }
    return idx;
}

//leaf 노드에서 idx 위치의 key, rid 삭제
void removeFromLeaf(BStarNode* node, int idx){
    for (int i = idx +1; i < node->cnt_key; i++){
        node->keys[i-1] = node->keys[i];
        node->rids[i-1] = node->rids[i];
    }
    node->cnt_key--;
    node->keys[node->cnt_key] = 0;
    node->rids[node->cnt_key] = -1;
}

//node->keys[idx] 바로 이전 값, 왼쪽 subtree에서 가장 큰 key 찾기
int getPredecessor(BStarNode* node, int idx){
    BStarNode* cur = node->child[idx];
    while (!cur->leaf){
        cur = cur->child[cur->cnt_key];
    }
    return cur->keys[cur->cnt_key -1];
}

int getPredecessorRid(BStarNode* node, int idx){
    BStarNode* cur = node->child[idx];
    while (!cur->leaf){
        cur = cur->child[cur->cnt_key];
    }
    return cur->rids[cur->cnt_key -1];
}

// node->keys[idx] 바로 다음 값, 오른쪽 subtree에서 가장 작은 key 찾기
int getSuccessor(BStarNode* node, int idx){
    BStarNode* cur = node->child[idx +1];
    while (!cur->leaf){
        cur = cur->child[0];
    }
    return cur->keys[0];
}

int getSuccessorRid(BStarNode* node, int idx){
    BStarNode* cur = node->child[idx +1];
    while (!cur->leaf){
        cur = cur->child[0];
    }
    return cur->rids[0];
}

// B* 트리 delete underflow 처리:
// merge는 B* 트리에서 두 노드 합이 max_keys를 초과하므로 불가
// 대신 인접 sibling과 부모 key를 포함해 재분배
// child[idx] 가 underflow 일 때 왼쪽 sibling과 3-way 재분배
void redistributeWithLeft(BStarNode* node, int idx){
    BStarNode* left  = node->child[idx -1];
    BStarNode* right = node->child[idx];

    int totalKeys = left->cnt_key + right->cnt_key +1;
    int tempKeys[temp_size];
    int tempRids[temp_size];
    BStarNode* tempChildren[temp_size +1];

    int t = 0;
    for (int i = 0; i < left->cnt_key; i++){
        tempKeys[t] = left->keys[i];
        tempRids[t] = left->rids[i];
        t++;
    }
    tempKeys[t] = node->keys[idx -1];
    tempRids[t] = node->rids[idx -1];
    t++;
    for (int i = 0; i < right->cnt_key; i++){
        tempKeys[t] = right->keys[i];
        tempRids[t] = right->rids[i];
        t++;
    }

    if (!left->leaf){
        int tc = 0;
        for (int i = 0; i <= left->cnt_key; i++){
            tempChildren[tc++] = left->child[i];
        }
        for (int i = 0; i <= right->cnt_key; i++){
            tempChildren[tc++] = right->child[i];
        }
    }

    int sep = totalKeys /2;

    left->cnt_key = 0;
    for (int i = 0; i < sep; i++){
        left->keys[i] = tempKeys[i];
        left->rids[i] = tempRids[i];
        left->cnt_key++;
    }
    node->keys[idx -1] = tempKeys[sep];
    node->rids[idx -1] = tempRids[sep];

    right->cnt_key = 0;
    for (int i = sep +1; i < totalKeys; i++){
        right->keys[right->cnt_key] = tempKeys[i];
        right->rids[right->cnt_key] = tempRids[i];
        right->cnt_key++;
    }

    if (!left->leaf){
        int tc = 0;
        for (int i = 0; i <= left->cnt_key; i++){
            left->child[i] = tempChildren[tc++];
        }
        left->cnt_child = left->cnt_key +1;
        for (int i = 0; i <= right->cnt_key; i++){
            right->child[i] = tempChildren[tc++];
        }
        right->cnt_child = right->cnt_key +1;
    }
}

// child[idx] 가 underflow 일 때 오른쪽 sibling과 재분배
void redistributeWithRight(BStarNode* node, int idx){
    BStarNode* left  = node->child[idx];
    BStarNode* right = node->child[idx +1];

    int totalKeys = left->cnt_key + right->cnt_key +1;
    int tempKeys[temp_size];
    int tempRids[temp_size];
    BStarNode* tempChildren[temp_size +1];

    int t = 0;
    for (int i = 0; i < left->cnt_key; i++){
        tempKeys[t] = left->keys[i];
        tempRids[t] = left->rids[i];
        t++;
    }
    tempKeys[t] = node->keys[idx];
    tempRids[t] = node->rids[idx];
    t++;
    for (int i = 0; i < right->cnt_key; i++){
        tempKeys[t] = right->keys[i];
        tempRids[t] = right->rids[i];
        t++;
    }

    if (!left->leaf){
        int tc = 0;
        for (int i = 0; i <= left->cnt_key; i++){
            tempChildren[tc++] = left->child[i];
        }
        for (int i = 0; i <= right->cnt_key; i++){
            tempChildren[tc++] = right->child[i];
        }
    }

    int sep = totalKeys /2;

    left->cnt_key = 0;
    for (int i = 0; i < sep; i++){
        left->keys[i] = tempKeys[i];
        left->rids[i] = tempRids[i];
        left->cnt_key++;
    }
    node->keys[idx] = tempKeys[sep];
    node->rids[idx] = tempRids[sep];

    right->cnt_key = 0;
    for (int i = sep +1; i < totalKeys; i++){
        right->keys[right->cnt_key] = tempKeys[i];
        right->rids[right->cnt_key] = tempRids[i];
        right->cnt_key++;
    }

    if (!left->leaf){
        int tc = 0;
        for (int i = 0; i <= left->cnt_key; i++){
            left->child[i] = tempChildren[tc++];
        }
        left->cnt_child = left->cnt_key +1;
        for (int i = 0; i <= right->cnt_key; i++){
            right->child[i] = tempChildren[tc++];
        }
        right->cnt_child = right->cnt_key +1;
    }
}

// child가 key 부족일 때 왼쪽 sibling에서 key 하나 빌려오기
/*
node              : parent 노드
node->child[idx]  : key를 보강해야 하는 child
node->child[idx-1]: 왼쪽 sibling
*/
void borrowFromLeft(BStarNode* node, int idx){
    BStarNode* child = node->child[idx];
    BStarNode* sibling = node->child[idx -1];

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

//child가 key 부족일 때 오른쪽 sibling에서 key 하나 빌려오기
void borrowFromRight(BStarNode* node, int idx){
    BStarNode* child = node->child[idx];
    BStarNode* sibling = node->child[idx +1];

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


//child key 부족 시 재분배로 균형 맞추기
// B* 트리는 merge 불가이므로 항상 인접 sibling 과 재분배
void fillChild(BStarNode* node, int idx){
    if (idx != 0){
        // 왼쪽 sibling 과 재분배
        redistributeWithLeft(node, idx);
    }
    else{
        // 왼쪽 sibling 없으면 오른쪽 sibling 과 재분배
        redistributeWithRight(node, idx);
    }
}

void deleteFromNode(BStarNode* node, int key);

//내부 노드에서 key 삭제 - predecessor/successor로 대체 후 재귀 삭제
void removeFromNonLeaf(BStarNode* node, int idx){
    int key = node->keys[idx];

    if (node->child[idx]->cnt_key > min_keys_bstar){
        int pred = getPredecessor(node, idx);
        int predRid = getPredecessorRid(node, idx);
        node->keys[idx] = pred;
        node->rids[idx] = predRid;
        deleteFromNode(node->child[idx], pred);
    }
    else if (node->child[idx +1]->cnt_key > min_keys_bstar){
        int succ = getSuccessor(node, idx);
        int succRid = getSuccessorRid(node, idx);
        node->keys[idx] = succ;
        node->rids[idx] = succRid;
        deleteFromNode(node->child[idx +1], succ);
    }
    else{
        // 양쪽 child 모두 min_keys_bstar 이므로
        // 왼쪽 child 에서 predecessor 를 가져와 대체하고
        // 두 child 를 재분배해서 균형 유지
        int pred = getPredecessor(node, idx);
        int predRid = getPredecessorRid(node, idx);
        node->keys[idx] = pred;
        node->rids[idx] = predRid;
        deleteFromNode(node->child[idx], pred);
    }
}

//노드에서 key 찾아 삭제, 필요시 fillChild로 균형 유지
void deleteFromNode(BStarNode* node, int key){
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

        if (node->child[idx]->cnt_key < min_keys_bstar +1){
            fillChild(node, idx);
            // 재분배 후 key 위치 재계산
            idx = findKeyIndex(node, key);
            isLast = (idx == node->cnt_key);
        }

        if (isLast && idx > node->cnt_key){
            deleteFromNode(node->child[idx -1], key);
        }
        else{
            deleteFromNode(node->child[idx], key);
        }
    }
}

//트리 전체에서 key 삭제
void deleteKey(int key){
    if (root == NULL){
        return;
    }

    deleteFromNode(root, key);

    if (root->cnt_key == 0){
        BStarNode* oldRoot = root;
        if (root->leaf){
            root = NULL;
        }
        else{
            root = root->child[0];
        }
        delete oldRoot;
    }
}

//가장 왼쪽 child만 따라가서 높이 계산
int getHeight(BStarNode* node){
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

//전체 노드 수 재귀적으로 카운트
int getNodeCount(BStarNode* node){
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
int getKeyCount(BStarNode* node){
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

// 노드 평균 사용률
double getNodeUtilization(){
    int nodeCount = getNodeCount(root);
    if (nodeCount == 0){
        return 0.0;
    }
    int keyCount = getKeyCount(root);
    return (double)keyCount / ((double)nodeCount * max_keys);
}

//오름차순 순회
void traverse(BStarNode* node){
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

} // namespace BStarTreeImpl