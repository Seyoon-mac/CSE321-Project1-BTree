#include <iostream>
#include <vector>
#include <chrono>
#include <random>
#include <algorithm>
#include "Record.h"
#include "Utils.h"
#include "BTree.h"
#include "BPlusTree.h"
#include "BStarTree.h"

const int SEARCH_QUERIES = 10000;
const int DELETE_QUERIES = 2000;
const int RANGE_LOW  = 202000000;
const int RANGE_HIGH = 202100000;

std::vector<int> makeSearchKeys(const std::vector<int>& keys){
    std::mt19937 rng(321);
    std::vector<int> result = keys;
    std::shuffle(result.begin(), result.end(), rng);
    result.resize(std::min(SEARCH_QUERIES, (int)result.size()));
    return result;
}

std::vector<int> makeDeleteKeys(const std::vector<int>& keys){
    std::mt19937 rng(42);
    std::vector<int> result = keys;
    std::shuffle(result.begin(), result.end(), rng);
    result.resize(std::min(DELETE_QUERIES, (int)result.size()));
    return result;
}

void runBTree(const std::vector<Record>& records, const std::vector<int>& keys){
    BTreeImpl::resetTree();

    std::vector<int> searchKeys = makeSearchKeys(keys);
    std::vector<int> deleteKeys = makeDeleteKeys(keys);

    // insert 실험
    auto insertStart = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < (int)records.size(); i++){
        BTreeImpl::insertKey(records[i].student_id, i);
    }

auto insertEnd = std::chrono::high_resolution_clock::now();

    // search 실험
    volatile int checksum = 0;
    auto searchStart = std::chrono::high_resolution_clock::now();
    for (int key : searchKeys){
        checksum += BTreeImpl::searchRID(key);
    }
    auto searchEnd = std::chrono::high_resolution_clock::now();

    // range query + 평균 GPA/height 계산까지 포함해서 측정
    auto rangeStart = std::chrono::high_resolution_clock::now();
    std::vector<int> rangeRids = BTreeImpl::searchRange(RANGE_LOW, RANGE_HIGH);

    double sumGpa    = 0.0;
    double sumHeight = 0.0;
    int    maleCount = 0;
    for (int rid : rangeRids){
        if (rid >= 0 && rid < (int)records.size() && records[rid].gender == "Male"){
            sumGpa    += records[rid].gpa;
            sumHeight += records[rid].height;
            maleCount++;
        }
    }
    double avgGpa    = maleCount ? sumGpa    / maleCount : 0.0;
    double avgHeight = maleCount ? sumHeight / maleCount : 0.0;
    auto rangeEnd = std::chrono::high_resolution_clock::now();

    // delete 실험
    auto deleteStart = std::chrono::high_resolution_clock::now();
    for (int key : deleteKeys){
        BTreeImpl::deleteKey(key);
    }
    auto deleteEnd = std::chrono::high_resolution_clock::now();

    double insertMs     = std::chrono::duration<double, std::milli>(insertEnd   - insertStart).count();
    double searchMeanUs = std::chrono::duration<double, std::micro>(searchEnd   - searchStart).count()
                          / (double)searchKeys.size();
    double rangeMs      = std::chrono::duration<double, std::milli>(rangeEnd    - rangeStart).count();
    double deleteMs     = std::chrono::duration<double, std::milli>(deleteEnd   - deleteStart).count();

    std::cout << "BTree,"
              << M << ","
              << insertMs << ","
              << searchMeanUs << ","
              << rangeMs << ","
              << deleteMs << ","
              << BTreeImpl::getSplitCount() << ","
              << BTreeImpl::getNodeUtilization() << ","
              << BTreeImpl::getTreeHeight() << ","
              << BTreeImpl::getTreeNodeCount() << ","
              << rangeRids.size() << ","
              << maleCount << ","
              << avgGpa << ","
              << avgHeight
              << "\n";
    (void)checksum;
}

void runBStarTree(const std::vector<Record>& records, const std::vector<int>& keys){
    BStarTreeImpl::resetTree();

    std::vector<int> searchKeys = makeSearchKeys(keys);
    std::vector<int> deleteKeys = makeDeleteKeys(keys);

    // insert 실험
    auto insertStart = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < (int)records.size(); i++){
        BStarTreeImpl::insertKey(records[i].student_id, i);
    }
    auto insertEnd = std::chrono::high_resolution_clock::now();

    // search 실험
    volatile int checksum = 0;
    auto searchStart = std::chrono::high_resolution_clock::now();
    for (int key : searchKeys){
        checksum += BStarTreeImpl::searchRID(key);
    }
    auto searchEnd = std::chrono::high_resolution_clock::now();

    // range query + 평균 GPA/height 계산까지 포함해서 측정
    auto rangeStart = std::chrono::high_resolution_clock::now();
    std::vector<int> rangeRids = BStarTreeImpl::searchRange(RANGE_LOW, RANGE_HIGH);

    double sumGpa    = 0.0;
    double sumHeight = 0.0;
    int    maleCount = 0;
    for (int rid : rangeRids){
        if (rid >= 0 && rid < (int)records.size() && records[rid].gender == "Male"){
            sumGpa    += records[rid].gpa;
            sumHeight += records[rid].height;
            maleCount++;
        }
    }
    double avgGpa    = maleCount ? sumGpa    / maleCount : 0.0;
    double avgHeight = maleCount ? sumHeight / maleCount : 0.0;
    auto rangeEnd = std::chrono::high_resolution_clock::now();

    // delete 실험
    auto deleteStart = std::chrono::high_resolution_clock::now();
    for (int key : deleteKeys){
        BStarTreeImpl::deleteKey(key);
    }
    auto deleteEnd = std::chrono::high_resolution_clock::now();

    double insertMs     = std::chrono::duration<double, std::milli>(insertEnd   - insertStart).count();
    double searchMeanUs = std::chrono::duration<double, std::micro>(searchEnd   - searchStart).count()
                          / (double)searchKeys.size();
    double rangeMs      = std::chrono::duration<double, std::milli>(rangeEnd    - rangeStart).count();
    double deleteMs     = std::chrono::duration<double, std::milli>(deleteEnd   - deleteStart).count();

    std::cout << "BStarTree,"
              << M << ","
              << insertMs << ","
              << searchMeanUs << ","
              << rangeMs << ","
              << deleteMs << ","
              << BStarTreeImpl::getSplitCount() << ","
              << BStarTreeImpl::getNodeUtilization() << ","
              << BStarTreeImpl::getTreeHeight() << ","
              << BStarTreeImpl::getTreeNodeCount() << ","
              << rangeRids.size() << ","
              << maleCount << ","
              << avgGpa << ","
              << avgHeight
              << "\n";
    (void)checksum;
}

void runBPlusTree(const std::vector<Record>& records, const std::vector<int>& keys){
    BPlusTreeImpl::resetTree();

    std::vector<int> searchKeys = makeSearchKeys(keys);
    std::vector<int> deleteKeys = makeDeleteKeys(keys);

    // insert 실험
    auto insertStart = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < (int)records.size(); i++){
        BPlusTreeImpl::insertKey(records[i].student_id, i);
    }
    auto insertEnd = std::chrono::high_resolution_clock::now();

    // search 실험
    volatile int checksum = 0;
    auto searchStart = std::chrono::high_resolution_clock::now();
    for (int key : searchKeys){
        checksum += BPlusTreeImpl::searchRID(key);
    }
    auto searchEnd = std::chrono::high_resolution_clock::now();

    // range query + 평균 GPA/height 계산까지 포함해서 측정
    auto rangeStart = std::chrono::high_resolution_clock::now();
    std::vector<int> rangeRids = BPlusTreeImpl::searchRange(RANGE_LOW, RANGE_HIGH);

    double sumGpa    = 0.0;
    double sumHeight = 0.0;
    int    maleCount = 0;
    for (int rid : rangeRids){
        if (rid >= 0 && rid < (int)records.size() && records[rid].gender == "Male"){
            sumGpa    += records[rid].gpa;
            sumHeight += records[rid].height;
            maleCount++;
        }
    }
    double avgGpa    = maleCount ? sumGpa    / maleCount : 0.0;
    double avgHeight = maleCount ? sumHeight / maleCount : 0.0;
    auto rangeEnd = std::chrono::high_resolution_clock::now();

    // delete 실험
    auto deleteStart = std::chrono::high_resolution_clock::now();
    for (int key : deleteKeys){
        BPlusTreeImpl::deleteKey(key);
    }
    auto deleteEnd = std::chrono::high_resolution_clock::now();

    double insertMs     = std::chrono::duration<double, std::milli>(insertEnd   - insertStart).count();
    double searchMeanUs = std::chrono::duration<double, std::micro>(searchEnd   - searchStart).count()
                          / (double)searchKeys.size();
    double rangeMs      = std::chrono::duration<double, std::milli>(rangeEnd    - rangeStart).count();
    double deleteMs     = std::chrono::duration<double, std::milli>(deleteEnd   - deleteStart).count();

    std::cout << "BPlusTree,"
              << M << ","
              << insertMs << ","
              << searchMeanUs << ","
              << rangeMs << ","
              << deleteMs << ","
              << BPlusTreeImpl::getSplitCount() << ","
              << BPlusTreeImpl::getNodeUtilization() << ","
              << BPlusTreeImpl::getTreeHeight() << ","
              << BPlusTreeImpl::getTreeNodeCount() << ","
              << rangeRids.size() << ","
              << maleCount << ","
              << avgGpa << ","
              << avgHeight
              << "\n";
    (void)checksum;
}

int main(){
    std::vector<Record> records = loadCSV("student.csv");

    if (records.empty()){
        std::cerr << "data load fail\n";
        return 1;
    }

    std::vector<int> keys;
    keys.reserve(records.size());
    for (int i = 0; i < (int)records.size(); i++){
        keys.push_back(records[i].student_id);
    }

    std::cout << "tree,M,insert_time_ms,search_mean_us,range_time_ms,delete_time_ms,"
                 "splits,utilization,height,node_count,range_count,male_count,avg_gpa,avg_height\n";

    runBTree(records, keys);
    runBStarTree(records, keys);
    runBPlusTree(records, keys);

    return 0;
}