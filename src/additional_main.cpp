#include <iostream>
#include <vector>
#include <chrono>
#include <random>
#include <algorithm>
#include <fstream>
#include <string>
#include "Record.h"
#include "Utils.h"
#include "BTree.h"
#include "BPlusTree.h"
#include "BStarTree.h"

//additional experiment

const int SEARCH_QUERIES = 10000;

struct RangeCase {
    const char* name;
    int low;
    int high;
};

RangeCase rangeCases[] = {
    {"Small",  202000000, 202020000},
    {"Medium", 202000000, 202100000},
    {"Large",  202000000, 202300000}
};

struct RangeResult {
    int range_count;
    int male_count;
    double avg_gpa;
    double avg_height;
};

std::vector<int> makeSearchKeys(const std::vector<int>& keys){
    std::mt19937 rng(321);
    std::vector<int> result = keys;
    std::shuffle(result.begin(), result.end(), rng);
    result.resize(std::min(SEARCH_QUERIES, (int)result.size()));
    return result;
}

std::vector<int> makeMissingKeys(){
    std::vector<int> result;
    result.reserve(SEARCH_QUERIES);

    for (int i = 0; i < SEARCH_QUERIES; i++){
        result.push_back(203000000 + i);
    }

    return result;
}

RangeResult calculateRangeResult(const std::vector<Record>& records,
                                 const std::vector<int>& rangeRids){
    double sumGpa = 0.0;
    double sumHeight = 0.0;
    int maleCount = 0;

    for (int rid : rangeRids){
        if (rid >= 0 && rid < (int)records.size() && records[rid].gender == "Male"){
            sumGpa += records[rid].gpa;
            sumHeight += records[rid].height;
            maleCount++;
        }
    }

    RangeResult result;
    result.range_count = (int)rangeRids.size();
    result.male_count = maleCount;
    result.avg_gpa = maleCount ? sumGpa / maleCount : 0.0;
    result.avg_height = maleCount ? sumHeight / maleCount : 0.0;

    return result;
}

void buildBTree(const std::vector<Record>& records){
    BTreeImpl::resetTree();

    for (int i = 0; i < (int)records.size(); i++){
        BTreeImpl::insertKey(records[i].student_id, i);
    }
}

void buildBStarTree(const std::vector<Record>& records){
    BStarTreeImpl::resetTree();

    for (int i = 0; i < (int)records.size(); i++){
        BStarTreeImpl::insertKey(records[i].student_id, i);
    }
}

void buildBPlusTree(const std::vector<Record>& records){
    BPlusTreeImpl::resetTree();

    for (int i = 0; i < (int)records.size(); i++){
        BPlusTreeImpl::insertKey(records[i].student_id, i);
    }
}

void runRangeExperimentBTree(const std::vector<Record>& records,
                             std::ofstream& out){
    buildBTree(records);

    for (RangeCase rc : rangeCases){
        auto start = std::chrono::high_resolution_clock::now();

        std::vector<int> rangeRids = BTreeImpl::searchRange(rc.low, rc.high);
        RangeResult result = calculateRangeResult(records, rangeRids);

        auto end = std::chrono::high_resolution_clock::now();
        double timeMs = std::chrono::duration<double, std::milli>(end - start).count();

        out << "BTree,"
            << M << ","
            << rc.name << ","
            << rc.low << ","
            << rc.high << ","
            << result.range_count << ","
            << result.male_count << ","
            << result.avg_gpa << ","
            << result.avg_height << ","
            << timeMs
            << "\n";
    }
}

void runRangeExperimentBStarTree(const std::vector<Record>& records,
                                 std::ofstream& out){
    buildBStarTree(records);

    for (RangeCase rc : rangeCases){
        auto start = std::chrono::high_resolution_clock::now();

        std::vector<int> rangeRids = BStarTreeImpl::searchRange(rc.low, rc.high);
        RangeResult result = calculateRangeResult(records, rangeRids);

        auto end = std::chrono::high_resolution_clock::now();
        double timeMs = std::chrono::duration<double, std::milli>(end - start).count();

        out << "BStarTree,"
            << M << ","
            << rc.name << ","
            << rc.low << ","
            << rc.high << ","
            << result.range_count << ","
            << result.male_count << ","
            << result.avg_gpa << ","
            << result.avg_height << ","
            << timeMs
            << "\n";
    }
}

void runRangeExperimentBPlusTree(const std::vector<Record>& records,
                                 std::ofstream& out){
    buildBPlusTree(records);

    for (RangeCase rc : rangeCases){
        auto start = std::chrono::high_resolution_clock::now();

        std::vector<int> rangeRids = BPlusTreeImpl::searchRange(rc.low, rc.high);
        RangeResult result = calculateRangeResult(records, rangeRids);

        auto end = std::chrono::high_resolution_clock::now();
        double timeMs = std::chrono::duration<double, std::milli>(end - start).count();

        out << "BPlusTree,"
            << M << ","
            << rc.name << ","
            << rc.low << ","
            << rc.high << ","
            << result.range_count << ","
            << result.male_count << ","
            << result.avg_gpa << ","
            << result.avg_height << ","
            << timeMs
            << "\n";
    }
}

void runSearchExperimentBTree(const std::vector<Record>& records,
                              const std::vector<int>& successKeys,
                              const std::vector<int>& missingKeys,
                              std::ofstream& out){
    buildBTree(records);

    volatile int checksum = 0;
    int foundCount = 0;

    auto successStart = std::chrono::high_resolution_clock::now();
    for (int key : successKeys){
        int rid = BTreeImpl::searchRID(key);
        checksum += rid;
        if (rid != -1){
            foundCount++;
        }
    }
    auto successEnd = std::chrono::high_resolution_clock::now();

    double successMeanUs =
        std::chrono::duration<double, std::micro>(successEnd - successStart).count()
        / (double)successKeys.size();

    out << "BTree,"
        << M << ","
        << "Successful,"
        << successMeanUs << ","
        << foundCount
        << "\n";

    foundCount = 0;

    auto missingStart = std::chrono::high_resolution_clock::now();
    for (int key : missingKeys){
        int rid = BTreeImpl::searchRID(key);
        checksum += rid;
        if (rid != -1){
            foundCount++;
        }
    }
    auto missingEnd = std::chrono::high_resolution_clock::now();

    double missingMeanUs =
        std::chrono::duration<double, std::micro>(missingEnd - missingStart).count()
        / (double)missingKeys.size();

    out << "BTree,"
        << M << ","
        << "Unsuccessful,"
        << missingMeanUs << ","
        << foundCount
        << "\n";

    (void)checksum;
}

void runSearchExperimentBStarTree(const std::vector<Record>& records,
                                  const std::vector<int>& successKeys,
                                  const std::vector<int>& missingKeys,
                                  std::ofstream& out){
    buildBStarTree(records);

    volatile int checksum = 0;
    int foundCount = 0;

    auto successStart = std::chrono::high_resolution_clock::now();
    for (int key : successKeys){
        int rid = BStarTreeImpl::searchRID(key);
        checksum += rid;
        if (rid != -1){
            foundCount++;
        }
    }
    auto successEnd = std::chrono::high_resolution_clock::now();

    double successMeanUs =
        std::chrono::duration<double, std::micro>(successEnd - successStart).count()
        / (double)successKeys.size();

    out << "BStarTree,"
        << M << ","
        << "Successful,"
        << successMeanUs << ","
        << foundCount
        << "\n";

    foundCount = 0;

    auto missingStart = std::chrono::high_resolution_clock::now();
    for (int key : missingKeys){
        int rid = BStarTreeImpl::searchRID(key);
        checksum += rid;
        if (rid != -1){
            foundCount++;
        }
    }
    auto missingEnd = std::chrono::high_resolution_clock::now();

    double missingMeanUs =
        std::chrono::duration<double, std::micro>(missingEnd - missingStart).count()
        / (double)missingKeys.size();

    out << "BStarTree,"
        << M << ","
        << "Unsuccessful,"
        << missingMeanUs << ","
        << foundCount
        << "\n";

    (void)checksum;
}

void runSearchExperimentBPlusTree(const std::vector<Record>& records,
                                  const std::vector<int>& successKeys,
                                  const std::vector<int>& missingKeys,
                                  std::ofstream& out){
    buildBPlusTree(records);

    volatile int checksum = 0;
    int foundCount = 0;

    auto successStart = std::chrono::high_resolution_clock::now();
    for (int key : successKeys){
        int rid = BPlusTreeImpl::searchRID(key);
        checksum += rid;
        if (rid != -1){
            foundCount++;
        }
    }
    auto successEnd = std::chrono::high_resolution_clock::now();

    double successMeanUs =
        std::chrono::duration<double, std::micro>(successEnd - successStart).count()
        / (double)successKeys.size();

    out << "BPlusTree,"
        << M << ","
        << "Successful,"
        << successMeanUs << ","
        << foundCount
        << "\n";

    foundCount = 0;

    auto missingStart = std::chrono::high_resolution_clock::now();
    for (int key : missingKeys){
        int rid = BPlusTreeImpl::searchRID(key);
        checksum += rid;
        if (rid != -1){
            foundCount++;
        }
    }
    auto missingEnd = std::chrono::high_resolution_clock::now();

    double missingMeanUs =
        std::chrono::duration<double, std::micro>(missingEnd - missingStart).count()
        / (double)missingKeys.size();

    out << "BPlusTree,"
        << M << ","
        << "Unsuccessful,"
        << missingMeanUs << ","
        << foundCount
        << "\n";

    (void)checksum;
}

int main(){
    std::vector<Record> records = loadCSV("student.csv");

    if (records.empty()){
        std::cerr << "데이터 로드 실패\n";
        return 1;
    }

    std::vector<int> keys;
    keys.reserve(records.size());

    for (int i = 0; i < (int)records.size(); i++){
        keys.push_back(records[i].student_id);
    }

    std::vector<int> successKeys = makeSearchKeys(keys);
    std::vector<int> missingKeys = makeMissingKeys();

    std::string rangeFileName = "additional_range_results_m" + std::to_string(M) + ".csv";
    std::string searchFileName = "additional_search_results_m" + std::to_string(M) + ".csv";

    std::ofstream rangeOut(rangeFileName);
    std::ofstream searchOut(searchFileName);

    if (!rangeOut.is_open() || !searchOut.is_open()){
        std::cerr << "추가 실험 파일 생성 실패\n";
        return 1;
    }

    rangeOut << "tree,M,range_type,low,high,range_count,male_count,avg_gpa,avg_height,range_time_ms\n";
    searchOut << "tree,M,query_type,search_mean_us,found_count\n";

    runRangeExperimentBTree(records, rangeOut);
    runRangeExperimentBStarTree(records, rangeOut);
    runRangeExperimentBPlusTree(records, rangeOut);

    runSearchExperimentBTree(records, successKeys, missingKeys, searchOut);
    runSearchExperimentBStarTree(records, successKeys, missingKeys, searchOut);
    runSearchExperimentBPlusTree(records, successKeys, missingKeys, searchOut);

    rangeOut.close();
    searchOut.close();

    std::cout << "Additional experiments finished for M=" << M << "\n";
    std::cout << "Saved: " << rangeFileName << "\n";
    std::cout << "Saved: " << searchFileName << "\n";

    return 0;
}