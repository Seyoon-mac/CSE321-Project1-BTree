#ifndef BSTARTREE_H
#define BSTARTREE_H

#include <stdbool.h>
#include <math.h>
#include <vector>

#ifndef M
#define M 5
#endif

#define max_children (2 * M)
#define max_keys (2 * M - 1)

#define min_child_bstar (int)(ceil(2.0 * max_children / 3.0))
#define min_keys_bstar (min_child_bstar - 1)

#define temp_size (max_keys * 2 + 3)

struct BStarNode {
    bool leaf;
    int keys[max_keys + 2];
    int rids[max_keys + 2];
    int cnt_key;
    struct BStarNode* child[max_children + 2];
    int cnt_child;
};

namespace BStarTreeImpl {
    extern struct BStarNode* root;
    extern int split_count;

    struct BStarNode* createBStarNode(bool leaf);
    struct BStarNode* searchNode_root(int key);
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
