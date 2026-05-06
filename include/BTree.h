#ifndef BTREE_H
#define BTREE_H

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

struct BTreeNode {
    bool leaf;
    int keys[max_keys+1];
    int rids[max_keys+1];
    int cnt_key;
    struct BTreeNode* child[max_children+1];
    int cnt_child;
};

namespace BTreeImpl {
    extern struct BTreeNode* root;
    extern int split_count;

    struct BTreeNode* createNode(bool leaf);
    struct BTreeNode* searchNode_root(int key);
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
}

#endif
