#ifndef BPLUSTREE_H
#define BPLUSTREE_H

#include <stdbool.h>
#include <math.h>
#include <vector>

#ifndef M
#define M 5
#endif

#define min_child M
#define min_keys (M - 1)
#define max_children (2 * M)
#define max_keys (2 * M - 1)

struct BPlusNode {
    bool leaf;
    int keys[max_keys + 1];
    int rids[max_keys + 1];
    int cnt_key;
    struct BPlusNode* child[max_children + 1];
    int cnt_child;
    struct BPlusNode* next;
};

namespace BPlusTreeImpl {
    extern struct BPlusNode* root;
    extern int split_count;

    struct BPlusNode* createBPlusNode(bool leaf);
    struct BPlusNode* searchNode_root(int key);
    int searchRID(int key);
    std::vector<int> searchRange(int low, int high);
    void insertKey(int key, int rid);
    void deleteKey(int key);
    int getTreeHeight();
    int getTreeNodeCount();
    double getNodeUtilization();
    void resetSplitCount();
    int getSplitCount();
    void resetTree();
    void printTree();
    void printLeaves();
}

#endif