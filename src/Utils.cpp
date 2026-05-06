#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include "Record.h"

using namespace std;

// 문자열 split
vector<string> split(const string& line, char delimiter) {
    vector<string> tokens;
    stringstream ss(line);
    string item;

    while (getline(ss, item, delimiter)) {
        tokens.push_back(item);
    }

    return tokens;
}

// 한 줄 → Record
Record parseRecord(const string& line) {
    vector<string> tokens = split(line, ',');

    Record r;

    if (tokens.size() < 6) {
        cerr << "잘못된 라인: " << line << endl;
        return r;
    }

    r.student_id = stoi(tokens[0]);
    r.name       = tokens[1];
    r.gender     = tokens[2];
    r.gpa        = stod(tokens[3]);
    r.height     = stod(tokens[4]);
    r.weight     = stod(tokens[5]);

    return r;
}

// CSV 전체 로딩
vector<Record> loadCSV(const string& filename) {
    vector<Record> records;
    ifstream file(filename);

    if (!file.is_open()) {
        cerr << "파일 열기 실패: " << filename << endl;
        return records;
    }

    string line;

    // header skip
    getline(file, line);

    while (getline(file, line)) {
        if (line.empty()) continue;

        Record r = parseRecord(line);
        records.push_back(r);
    }

    file.close();
    return records;
}

// key-RID 생성
vector<pair<int, int>> buildKeyRID(const vector<Record>& records) {
    vector<pair<int, int>> result;

    for (int i = 0; i < records.size(); i++) {
        int key = records[i].student_id;
        int rid = i;

        result.push_back({key, rid});
    }

    return result;
}