#ifndef UTILS_H
#define UTILS_H

#include <vector>
#include <string>
#include "Record.h"

using namespace std;

// 문자열 split
vector<string> split(const string& line, char delimiter);

// 한 줄 → Record
Record parseRecord(const string& line);

// CSV 전체 로딩
vector<Record> loadCSV(const string& filename);

// key-RID 생성
vector<pair<int, int>> buildKeyRID(const vector<Record>& records);

#endif