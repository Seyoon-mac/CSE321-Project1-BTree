#include <vector>
using namespace std;

class IndexTree {
public:
    virtual void insert(int key, int rid) = 0;
    virtual int search(int key) const = 0;
    virtual bool remove(int key) = 0;
    virtual vector<int> rangeQuery(int startKey, int endKey) const = 0;

    virtual int getHeight() const = 0;
    virtual int getNodeCount() const = 0;
    virtual int getSplitCount() const = 0;
    virtual double getNodeUtilization() const = 0;
    virtual bool validate() const = 0;

    virtual ~IndexTree() = default;
};

