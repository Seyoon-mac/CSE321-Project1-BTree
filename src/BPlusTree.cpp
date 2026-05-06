#include "BPlusTree.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <vector>

namespace BPlusTreeImpl {

struct BPlusNode* root = NULL;
int split_count = 0;

// 새 노드 생성 및 초기화
struct BPlusNode* createBPlusNode(bool leaf){
    BPlusNode* node = new BPlusNode;
    node->leaf = leaf;
    node->cnt_key = 0;
    node->cnt_child = 0;
    node->next = NULL;

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
bool searchNode(BPlusNode* node, int target, int* pos){
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

// 현재 노드에서 따라 내려가면서 leaf 찾기
// separator key와 같을 때 오른쪽 child로 가야 함 (>= 기준)
BPlusNode* findLeaf(BPlusNode* node, int target){
    if (node == NULL){
        return NULL;
    }
    if (node->leaf){
        return node;
    }
    int pos = 0;
    while (pos < node->cnt_key && target >= node->keys[pos]){
        pos++;
    }
    return findLeaf(node->child[pos], target);
}

// 전체 트리 검색 wrapper (leaf node 반환)
struct BPlusNode* searchNode_root(int key){
    return findLeaf(root, key);
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
void freeTree(BPlusNode* node){
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

// key로 RID 검색, 못 찾으면 -1 반환
int searchRID(int key){
    BPlusNode* leaf = findLeaf(root, key);
    if (leaf == NULL){
        return -1;
    }
    for (int i = 0; i < leaf->cnt_key; i++){
        if (leaf->keys[i] == key){
            return leaf->rids[i];
        }
    }
    return -1;
}

// low ~ high 범위의 RID 전부 반환
std::vector<int> searchRange(int low, int high){
    std::vector<int> result;

    BPlusNode* leaf = findLeaf(root, low);
    if (leaf == NULL){
        return result;
    }

    // low보다 작은 key가 있을 수 있으므로 첫 번째 유효 위치부터 시작
    while (leaf != NULL){
        for (int i = 0; i < leaf->cnt_key; i++){
            if (leaf->keys[i] > high){
                return result;
            }
            if (leaf->keys[i] >= low){
                result.push_back(leaf->rids[i]);
            }
        }
        leaf = leaf->next;
    }
    return result;
}

// 부모 노드 parent의 i번째 자식이 꽉 찼을 때, 그 자식을 둘로 나누고 가운데 key를 부모로 올린다
void splitChild(BPlusNode* parent, int i){
    BPlusNode* fullChild = parent->child[i];
    BPlusNode* newChild = createBPlusNode(fullChild->leaf);
    split_count++;

    if (fullChild->leaf){
        // leaf split - 중간 key를 복사해서 부모로 올림 (leaf에는 그대로 유지)
        int mid = min_keys;

        newChild->cnt_key = fullChild->cnt_key - mid;
        for (int j = 0; j < newChild->cnt_key; j++){
            newChild->keys[j] = fullChild->keys[mid +j];
            newChild->rids[j] = fullChild->rids[mid +j];
        }

        fullChild->cnt_key = mid;

        // leaf linked list 연결
        newChild->next = fullChild->next;
        fullChild->next = newChild;

        // 부모로 올릴 key는 newChild의 첫번째 key
        int middleKey = newChild->keys[0];

        // parent child 한 칸 뒤로 밀기
        for (int j = parent->cnt_key; j >= i +1; j--){
            parent->child[j+1] = parent->child[j];
        }
        parent->child[i+1] = newChild;

        // parent key 한 칸 뒤로 밀기
        for (int j = parent->cnt_key -1; j >= i; j--){
            parent->keys[j+1] = parent->keys[j];
        }
        parent->keys[i] = middleKey;
        parent->cnt_key++;
        parent->cnt_child = parent->cnt_key +1;
    }
    else{
        // 내부 노드 split - 중간 key를 부모로 올리고 제거
        int middleKey = fullChild->keys[min_keys];

        newChild->cnt_key = fullChild->cnt_key - min_keys -1;
        for (int j = 0; j < newChild->cnt_key; j++){
            newChild->keys[j] = fullChild->keys[min_keys +1 +j];
        }

        for (int j = 0; j <= newChild->cnt_key; j++){
            newChild->child[j] = fullChild->child[min_keys +1 +j];
        }
        newChild->cnt_child = newChild->cnt_key +1;

        fullChild->cnt_key = min_keys;
        fullChild->cnt_child = fullChild->cnt_key +1;

        for (int j = parent->cnt_key; j >= i +1; j--){
            parent->child[j+1] = parent->child[j];
        }
        parent->child[i+1] = newChild;

        for (int j = parent->cnt_key -1; j >= i; j--){
            parent->keys[j+1] = parent->keys[j];
        }
        parent->keys[i] = middleKey;
        parent->cnt_key++;
        parent->cnt_child = parent->cnt_key +1;
    }
}

// 꽉 차지 않은 node를 시작점으로 key를 B+ 규칙에 맞게 삽입
void insertNonFull(BPlusNode* node, int key, int rid){
    if (node->leaf){
        int i = node->cnt_key -1;
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
        int i = node->cnt_key -1;
        while (i >= 0 && key < node->keys[i]){
            i--;
        }
        i++;
        // separator key와 같을 때 오른쪽 child로 가도록 >= 기준 재확인
        // i가 separator를 가리키면 오른쪽으로 이동
        while (i < node->cnt_key && key >= node->keys[i]){
            i++;
        }

        if (node->child[i]->cnt_key == max_keys){
            splitChild(node, i);
            // split 후 새 separator 기준으로 재계산
            if (key >= node->keys[i]){
                i++;
            }
        }
        insertNonFull(node->child[i], key, rid);
    }
}

// 트리 전체에 key, rid 삽입
void insertKey(int key, int rid){
    if (searchRID(key) != -1){
        return;
    }
    if (root == NULL){
        root = createBPlusNode(true);
        root->keys[0] = key;
        root->rids[0] = rid;
        root->cnt_key = 1;
        return;
    }
    if (root->cnt_key == max_keys){
        BPlusNode* newRoot = createBPlusNode(false);
        newRoot->child[0] = root;
        newRoot->cnt_child = 1;
        splitChild(newRoot, 0);
        int i = 0;
        if (key >= newRoot->keys[0]){
            i++;
        }
        insertNonFull(newRoot->child[i], key, rid);
        root = newRoot;
    }
    else{
        insertNonFull(root, key, rid);
    }
}

// 현재 노드 안에서 key 위치 또는 내려갈 child index 찾기
int findKeyIndex(BPlusNode* node, int key){
    int idx = 0;
    while (idx < node->cnt_key && node->keys[idx] < key){
        idx++;
    }
    return idx;
}

// leaf 노드에서 idx 위치의 key, rid 삭제
void removeFromLeaf(BPlusNode* node, int idx){
    for (int i = idx +1; i < node->cnt_key; i++){
        node->keys[i-1] = node->keys[i];
        node->rids[i-1] = node->rids[i];
    }
    node->cnt_key--;
    node->keys[node->cnt_key] = 0;
    node->rids[node->cnt_key] = -1;
}

// 두 leaf child와 부모 key 하나를 합치기
/*
node->child[idx]
node->keys[idx]  (routing key, leaf merge시 부모에서만 제거)
node->child[idx+1]
합치기
*/
void mergeLeaf(BPlusNode* node, int idx){
    BPlusNode* child = node->child[idx];
    BPlusNode* sibling = node->child[idx +1];

    for (int i = 0; i < sibling->cnt_key; i++){
        child->keys[child->cnt_key +i] = sibling->keys[i];
        child->rids[child->cnt_key +i] = sibling->rids[i];
    }
    child->cnt_key = child->cnt_key + sibling->cnt_key;
    child->next = sibling->next;

    for (int i = idx +1; i < node->cnt_key; i++){
        node->keys[i-1] = node->keys[i];
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

// 두 내부 노드 child와 부모 key 하나를 합치기
/*
node->child[idx]
node->keys[idx]
node->child[idx+1]
합치기
*/
void mergeInternal(BPlusNode* node, int idx){
    BPlusNode* child = node->child[idx];
    BPlusNode* sibling = node->child[idx +1];

    child->keys[child->cnt_key] = node->keys[idx];

    for (int i = 0; i < sibling->cnt_key; i++){
        child->keys[child->cnt_key +1 +i] = sibling->keys[i];
    }
    for (int i = 0; i <= sibling->cnt_key; i++){
        child->child[child->cnt_key +1 +i] = sibling->child[i];
    }

    child->cnt_key = child->cnt_key + sibling->cnt_key +1;
    child->cnt_child = child->cnt_key +1;

    for (int i = idx +1; i < node->cnt_key; i++){
        node->keys[i-1] = node->keys[i];
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
void borrowFromLeft(BPlusNode* node, int idx){
    BPlusNode* child = node->child[idx];
    BPlusNode* sibling = node->child[idx -1];

    for (int i = child->cnt_key -1; i >= 0; i--){
        child->keys[i+1] = child->keys[i];
    }

    if (child->leaf){
        // leaf면 rids도 같이 밀고 sibling 마지막 key/rid를 child 앞으로
        for (int i = child->cnt_key -1; i >= 0; i--){
            child->rids[i+1] = child->rids[i];
        }
        child->keys[0] = sibling->keys[sibling->cnt_key -1];
        child->rids[0] = sibling->rids[sibling->cnt_key -1];
        sibling->keys[sibling->cnt_key -1] = 0;
        sibling->rids[sibling->cnt_key -1] = -1;
        // routing key 갱신: 부모의 해당 key를 child의 새 첫 key로
        node->keys[idx -1] = child->keys[0];
    }
    else{
        // 내부 노드면 부모 key를 내리고 sibling 마지막 key를 부모로 올림
        for (int i = child->cnt_child -1; i >= 0; i--){
            child->child[i+1] = child->child[i];
        }
        child->keys[0] = node->keys[idx -1];
        child->child[0] = sibling->child[sibling->cnt_key];
        sibling->child[sibling->cnt_key] = NULL;
        node->keys[idx -1] = sibling->keys[sibling->cnt_key -1];
        sibling->keys[sibling->cnt_key -1] = 0;
    }

    child->cnt_key++;
    sibling->cnt_key--;

    if (!child->leaf){
        child->cnt_child = child->cnt_key +1;
        sibling->cnt_child = sibling->cnt_key +1;
    }
}

// child가 key 부족일 때 오른쪽 sibling에서 key 하나 빌려오기
void borrowFromRight(BPlusNode* node, int idx){
    BPlusNode* child = node->child[idx];
    BPlusNode* sibling = node->child[idx +1];

    if (child->leaf){
        // leaf면 rids도 같이 처리
        child->keys[child->cnt_key] = sibling->keys[0];
        child->rids[child->cnt_key] = sibling->rids[0];

        for (int i = 1; i < sibling->cnt_key; i++){
            sibling->keys[i-1] = sibling->keys[i];
            sibling->rids[i-1] = sibling->rids[i];
        }
        sibling->keys[sibling->cnt_key -1] = 0;
        sibling->rids[sibling->cnt_key -1] = -1;
        // routing key 갱신: 부모의 해당 key를 sibling의 새 첫 key로
        node->keys[idx] = sibling->keys[0];
    }
    else{
        // 내부 노드면 부모 key를 내리고 sibling 첫 key를 부모로 올림
        child->keys[child->cnt_key] = node->keys[idx];
        child->child[child->cnt_child] = sibling->child[0];
        node->keys[idx] = sibling->keys[0];

        for (int i = 1; i < sibling->cnt_key; i++){
            sibling->keys[i-1] = sibling->keys[i];
        }
        sibling->keys[sibling->cnt_key -1] = 0;

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
void fillChild(BPlusNode* node, int idx){
    if (idx != 0 && node->child[idx -1]->cnt_key > min_keys){
        borrowFromLeft(node, idx);
    }
    else if (idx != node->cnt_key && node->child[idx +1]->cnt_key > min_keys){
        borrowFromRight(node, idx);
    }
    else{
        if (idx != node->cnt_key){
            if (node->child[idx]->leaf){
                mergeLeaf(node, idx);
            }
            else{
                mergeInternal(node, idx);
            }
        }
        else{
            if (node->child[idx -1]->leaf){
                mergeLeaf(node, idx -1);
            }
            else{
                mergeInternal(node, idx -1);
            }
        }
    }
}

void deleteFromNode(BPlusNode* node, int key);

// 노드에서 key 찾아 삭제, 필요시 fillChild로 균형 유지
void deleteFromNode(BPlusNode* node, int key){
    int idx = findKeyIndex(node, key);

    if (node->leaf){
        if (idx < node->cnt_key && node->keys[idx] == key){
            removeFromLeaf(node, idx);
        }
        return;
    }

    if (idx < node->cnt_key && node->keys[idx] == key){
        // routing key와 일치해도 실제 데이터는 오른쪽 subtree leaf에 있음
        // 오른쪽 child로 내려가서 삭제
        deleteFromNode(node->child[idx +1], key);

        // underflow 처리 먼저
        if (node->child[idx +1]->cnt_key < min_keys){
            fillChild(node, idx +1);
        }

        // fillChild 후 idx+1이 merge되어 없어질 수 있으므로 범위 체크 후 갱신
        if (idx < node->cnt_key){
            BPlusNode* leftmost = node->child[idx +1];
            while (!leftmost->leaf){
                leftmost = leftmost->child[0];
            }
            if (leftmost->cnt_key > 0){
                node->keys[idx] = leftmost->keys[0];
            }
        }
    }
    else{
        // routing key와 다름 - idx child로 내려감
        deleteFromNode(node->child[idx], key);

        // underflow 처리
        if (idx <= node->cnt_key && node->child[idx]->cnt_key < min_keys){
            fillChild(node, idx);
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
        BPlusNode* oldRoot = root;
        if (root->leaf){
            root = NULL;
        }
        else{
            root = root->child[0];
        }
        delete oldRoot;
    }
}

// 가장 왼쪽 child만 따라가서 높이 계산
int getHeight(BPlusNode* node){
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
int getNodeCount(BPlusNode* node){
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

// 전체 key 수 재귀적으로 카운트 (leaf만)
int getKeyCount(BPlusNode* node){
    if (node == NULL){
        return 0;
    }
    if (node->leaf){
        return node->cnt_key;
    }
    int count = 0;
    for (int i = 0; i <= node->cnt_key; i++){
        count += getKeyCount(node->child[i]);
    }
    return count;
}

// 노드 평균 사용률 = leaf key 수 / (노드 수 * max_keys)
double getNodeUtilization(){
    int nodeCount = getNodeCount(root);
    if (nodeCount == 0){
        return 0.0;
    }
    int keyCount = getKeyCount(root);
    return (double)keyCount / ((double)nodeCount * max_keys);
}

// 오름차순 순회 (내부 노드 기준)
void traverse(BPlusNode* node){
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

// leaf linked list 따라가며 순서대로 출력
void printLeaves(){
    BPlusNode* cur = root;
    if (cur == NULL){
        return;
    }
    while (!cur->leaf){
        cur = cur->child[0];
    }
    while (cur != NULL){
        for (int i = 0; i < cur->cnt_key; i++){
            printf("(%d,%d) ", cur->keys[i], cur->rids[i]);
        }
        cur = cur->next;
    }
    printf("\n");
}

} // namespace BPlusTreeImpl
