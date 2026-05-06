# CSE321 Project 1 - Implementation and Analysis of B-tree Index Structures

## 1. Overview

This project implements and evaluates three tree-based index structures:
- B-tree
- B*-tree
- B+-tree

The index key is the `Student ID` from the input CSV file.  
The Record Identifier (RID) is the array index of each record after loading the CSV file into memory.

For example, if a record is stored at `records[i]`, then:
```text
key = records[i].student_id
rid = i
```

The project supports:

* Insert
* Point Search
* Range Query
* Delete
* Structural metric collection

The same experimental workloads are executed on all three index structures.

---

## 2. Environment

This project was implemented and tested with:

* Language: C++17
* Compiler: g++ / clang++
* Operating System: macOS
* Dataset path: data/student.csv

---

## 3. Project Structure

```
Project1/
├── include/
│   ├── BTree.h
│   ├── BPlusTree.h
│   ├── BStarTree.h
│   ├── Record.h
│   ├── Utils.h
│   └── Experiment.h
│
├── src/
│   ├── main.cpp
│   ├── BTree.cpp
│   ├── BPlusTree.cpp
│   ├── BStarTree.cpp
│   └── Utils.cpp
│
├── data/
│   └── student.csv
│
└── README.md
```

---

## 4. Dataset

The input CSV file must be placed at:

```
data/student.csv
```

Each record contains:

* Student ID
* Name
* Gender
* GPA
* Height
* Weight

The program loads all records into an in-memory array.  
The Student ID is used as the key, and the array index is used as the RID.

For example:

```
records[0] -> RID 0
records[1] -> RID 1
records[2] -> RID 2
```

---

## 5. Order Parameter

In this implementation, the compile-time parameter `M` is used as the minimum degree parameter.

Each node can contain up to:

```
2M - 1 keys
2M children
```

The experiments are conducted with:

```
M = 3, 5, 10
```

The value of `M` is specified at compile time using the `-DM=<value>` option.

For example:

```
-DM=3
-DM=5
-DM=10
```

---

## 6. Compile

Run all commands from the project root directory.

**M = 3**
```bash
g++ -std=c++17 -O2 -DM=3 -Iinclude src/main.cpp src/BTree.cpp src/BPlusTree.cpp src/BStarTree.cpp src/Utils.cpp -o experiment_m3
```

**M = 5**
```bash
g++ -std=c++17 -O2 -DM=5 -Iinclude src/main.cpp src/BTree.cpp src/BPlusTree.cpp src/BStarTree.cpp src/Utils.cpp -o experiment_m5
```

**M = 10**
```bash
g++ -std=c++17 -O2 -DM=10 -Iinclude src/main.cpp src/BTree.cpp src/BPlusTree.cpp src/BStarTree.cpp src/Utils.cpp -o experiment_m10
```

---

## 7. Run Experiments

After compilation, run:

```bash
./experiment_m3
./experiment_m5
./experiment_m10
```

To save the outputs as CSV files:

```bash
./experiment_m3 > result_m3.csv
./experiment_m5 > result_m5.csv
./experiment_m10 > result_m10.csv
```

---

## 8. Merge Result Files

To merge the three result files into a single CSV file:

```bash
head -n 1 result_m3.csv > results.csv
tail -n +2 result_m3.csv >> results.csv
tail -n +2 result_m5.csv >> results.csv
tail -n +2 result_m10.csv >> results.csv
```

The final result file is:

```
results.csv
```

---

## 9. Output Format

Each experiment outputs the following columns:

```
tree,M,insert_time_ms,search_mean_us,range_time_ms,delete_time_ms,splits,utilization,height,node_count,range_count,male_count,avg_gpa,avg_height
```

| Column | Description |
|---|---|
| tree | Index structure type: BTree, BStarTree, or BPlusTree |
| M | Minimum degree parameter |
| insert_time_ms | Total insertion time for all records in milliseconds |
| search_mean_us | Mean point search time over 10,000 random keys in microseconds |
| range_time_ms | Time for the range query and aggregation in milliseconds |
| delete_time_ms | Total deletion time for 2,000 random keys in milliseconds |
| splits | Total number of node split operations |
| utilization | Average node utilization |
| height | Final tree height |
| node_count | Final number of nodes |
| range_count | Number of records found in the range query |
| male_count | Number of male students in the range query result |
| avg_gpa | Average GPA of selected male students |
| avg_height | Average height of selected male students |

---

## 10. Experimental Workloads

The program performs the following workloads for each tree structure and each value of M.

### 10.1 Insertion

All 100,000 records are inserted into an initially empty tree.

Measured metrics:

* Total insertion time
* Number of node splits
* Final node utilization
* Final tree height
* Final number of nodes

### 10.2 Point Search

10,000 Student IDs are randomly selected from the dataset and searched in the tree.

Measured metric:

* Mean search time per query

### 10.3 Range Query

The same Student ID range query is executed on all three tree structures.

The range query selects records whose Student IDs are in a fixed range and then calculates:

* Number of matching records
* Number of male students
* Average GPA of male students
* Average height of male students

Measured metric:

* Range query and aggregation time

### 10.4 Deletion

2,000 randomly selected records are deleted from each tree.

Measured metric:

* Total deletion time

---

## 11. Implementation Notes

### 11.1 B-tree

The B-tree stores key-RID pairs in its nodes.  
Search may terminate at an internal node when the target key is found.

Implemented operations:

* Insert
* Search
* Delete
* Range Query by recursive traversal

Insertion uses node splitting when a full node is encountered.  
Deletion handles:

* Deletion from leaf nodes
* Deletion from internal nodes
* Borrowing from siblings
* Merging nodes

### 11.2 B*-tree

The B*-tree extends the B-tree insertion policy.

For non-root full nodes, the implementation first attempts redistribution with an adjacent sibling.  
If redistribution is not possible, the implementation performs a 2-to-3 split.

The root node is handled as a special case because it has no sibling for redistribution.  
Therefore, root overflow is handled using a separate root split procedure.

Implemented operations:

* Insert
* Search
* Delete
* Range Query by recursive traversal

### 11.3 B+-tree

The B+-tree stores all RIDs only in leaf nodes.  
Internal nodes contain only keys and child pointers.  
Leaf nodes are connected using linked pointers to support efficient range scans.

Implemented operations:

* Insert
* Search
* Delete
* Range Query through linked leaf traversal

Search always proceeds to a leaf node.

---

## 12. Example Full Execution

The following commands compile and run all experiments.

```bash
g++ -std=c++17 -O2 -DM=3 -Iinclude src/main.cpp src/BTree.cpp src/BPlusTree.cpp src/BStarTree.cpp src/Utils.cpp -o experiment_m3
g++ -std=c++17 -O2 -DM=5 -Iinclude src/main.cpp src/BTree.cpp src/BPlusTree.cpp src/BStarTree.cpp src/Utils.cpp -o experiment_m5
g++ -std=c++17 -O2 -DM=10 -Iinclude src/main.cpp src/BTree.cpp src/BPlusTree.cpp src/BStarTree.cpp src/Utils.cpp -o experiment_m10
./experiment_m3 > result_m3.csv
./experiment_m5 > result_m5.csv
./experiment_m10 > result_m10.csv
head -n 1 result_m3.csv > results.csv
tail -n +2 result_m3.csv >> results.csv
tail -n +2 result_m5.csv >> results.csv
tail -n +2 result_m10.csv >> results.csv
```

---

## 13. Expected Generated Files

After running the experiments, the following files will be generated:

```
result_m3.csv
result_m5.csv
result_m10.csv
results.csv
```

---

## 14. Notes

* The program assumes that `data/student.csv` exists before execution.
* The same random seed is used for point search and deletion workloads to make the experiments reproducible.
* The same range query is executed on all three tree structures.
* `M` is set at compile time using the `-DM` compiler option.
* If the executable cannot find the dataset, check whether `data/student.csv` exists relative to the project root directory.