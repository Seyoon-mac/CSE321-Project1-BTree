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

The project supports:

* Insert
* Point Search
* Range Query
* Delete
* Structural metric collection

The same experimental workloads are executed on all three index structures.

⸻

2. Environment

This project was implemented and tested with:

* Language: C++17
* Compiler: g++ / clang++
* Operating System: macOS
* Dataset path: data/student.csv

Python is only used for plotting result figures.

* Python version: Python 3
* Required Python packages:
    * pandas
    * matplotlib

⸻

3. Project Structure

Project1/
├── include/
│   ├── BTree.h
│   ├── BPlusTree.h
│   ├── BStarTree.h
│   ├── Record.h
│   └── Utils.h
│
├── src/
│   ├── main.cpp
│   ├── additional_main.cpp
│   ├── BTree.cpp
│   ├── BPlusTree.cpp
│   ├── BStarTree.cpp
│   └── Utils.cpp
│
├── data/
│   └── student.csv
│
├── result_m3.csv
├── result_m5.csv
├── result_m10.csv
├── results.csv
│
├── additional_range_results_m3.csv
├── additional_range_results_m5.csv
├── additional_range_results_m10.csv
├── additional_search_results_m3.csv
├── additional_search_results_m5.csv
├── additional_search_results_m10.csv
├── additional_range_results.csv
├── additional_search_results.csv
│
├── plot_results.py
├── plot_additional_results.py
│
└── README.md

⸻

4. Dataset

The input CSV file must be placed at:

data/student.csv

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

records[0] -> RID 0
records[1] -> RID 1
records[2] -> RID 2

⸻

5. Order Parameter

In this implementation, the compile-time parameter M is used as the minimum degree parameter.

Each node can contain up to:

2M - 1 keys
2M children

The experiments are conducted with:

M = 3, 5, 10

The value of M is specified at compile time using the -DM=<value> option.

For example:

-DM=3
-DM=5
-DM=10

⸻

6. Compile Basic Experiments

Run all commands from the project root directory.

M = 3

g++ -std=c++17 -O2 -DM=3 -Iinclude src/main.cpp src/BTree.cpp src/BPlusTree.cpp src/BStarTree.cpp src/Utils.cpp -o experiment_m3

M = 5

g++ -std=c++17 -O2 -DM=5 -Iinclude src/main.cpp src/BTree.cpp src/BPlusTree.cpp src/BStarTree.cpp src/Utils.cpp -o experiment_m5

M = 10

g++ -std=c++17 -O2 -DM=10 -Iinclude src/main.cpp src/BTree.cpp src/BPlusTree.cpp src/BStarTree.cpp src/Utils.cpp -o experiment_m10

⸻

7. Run Basic Experiments

After compilation, run:

./experiment_m3
./experiment_m5
./experiment_m10

To save the outputs as CSV files:

./experiment_m3 > result_m3.csv
./experiment_m5 > result_m5.csv
./experiment_m10 > result_m10.csv

⸻

8. Merge Basic Result Files

To merge the three result files into a single CSV file:

head -n 1 result_m3.csv > results.csv
tail -n +2 result_m3.csv >> results.csv
tail -n +2 result_m5.csv >> results.csv
tail -n +2 result_m10.csv >> results.csv

The final basic experiment result file is:

results.csv

⸻

9. Output Format

Each basic experiment outputs the following columns:

tree,M,insert_time_ms,search_mean_us,range_time_ms,delete_time_ms,splits,utilization,height,node_count,range_count,male_count,avg_gpa,avg_height

Column	Description
tree	Index structure type: BTree, BStarTree, or BPlusTree
M	Minimum degree parameter
insert_time_ms	Total insertion time for all records in milliseconds
search_mean_us	Mean point search time over 10,000 random keys in microseconds
range_time_ms	Time for the range query and aggregation in milliseconds
delete_time_ms	Total deletion time for 2,000 random keys in milliseconds
splits	Total number of node split operations
utilization	Average node utilization
height	Final tree height
node_count	Final number of nodes
range_count	Number of records found in the range query
male_count	Number of male students in the range query result
avg_gpa	Average GPA of selected male students
avg_height	Average height of selected male students

⸻

10. Basic Experimental Workloads

The program performs the following workloads for each tree structure and each value of M.

10.1 Insertion

All 100,000 records are inserted into an initially empty tree.

Measured metrics:

* Total insertion time
* Number of node splits
* Final node utilization
* Final tree height
* Final number of nodes

10.2 Point Search

10,000 Student IDs are randomly selected from the dataset and searched in the tree.

Measured metric:

* Mean search time per query

10.3 Range Query

The same Student ID range query is executed on all three tree structures.

The range query selects records whose Student IDs are in a fixed range and then calculates:

* Number of matching records
* Number of male students
* Average GPA of male students
* Average height of male students

Measured metric:

* Range query and aggregation time

10.4 Deletion

2,000 randomly selected records are deleted from each tree.

Measured metric:

* Total deletion time

⸻

11. Additional Experiments

This project also includes additional experiments used in the report.

The additional experiment source file is:

src/additional_main.cpp

The additional experiments include:

1. Range query selectivity experiment
2. Successful vs unsuccessful point search experiment

11.1 Compile Additional Experiments

Run the following commands from the project root directory.

g++ -std=c++17 -O2 -DM=3 -Iinclude src/additional_main.cpp src/BTree.cpp src/BPlusTree.cpp src/BStarTree.cpp src/Utils.cpp -o additional_m3
g++ -std=c++17 -O2 -DM=5 -Iinclude src/additional_main.cpp src/BTree.cpp src/BPlusTree.cpp src/BStarTree.cpp src/Utils.cpp -o additional_m5
g++ -std=c++17 -O2 -DM=10 -Iinclude src/additional_main.cpp src/BTree.cpp src/BPlusTree.cpp src/BStarTree.cpp src/Utils.cpp -o additional_m10

11.2 Run Additional Experiments

./additional_m3
./additional_m5
./additional_m10

These commands generate:

additional_range_results_m3.csv
additional_range_results_m5.csv
additional_range_results_m10.csv
additional_search_results_m3.csv
additional_search_results_m5.csv
additional_search_results_m10.csv

The merged additional result files are:

additional_range_results.csv
additional_search_results.csv

11.3 Additional Range Query Experiment

This experiment uses three range sizes:

Small  : 202000000 ~ 202020000
Medium : 202000000 ~ 202100000
Large  : 202000000 ~ 202300000

The output columns are:

tree,M,range_type,low,high,range_count,male_count,avg_gpa,avg_height,range_time_ms

11.4 Additional Search Experiment

This experiment compares:

* Successful point search
* Unsuccessful point search

Successful search keys are sampled from existing Student IDs.
Unsuccessful search keys are generated from a key range that does not exist in the dataset.

The output columns are:

tree,M,query_type,search_mean_us,found_count

⸻

12. Plot Result Figures

Python scripts are included to reproduce the figures used in the report.

12.1 Plot Basic Experiment Results

python3 plot_results.py

This script generates:

fig_splits_by_m.png
fig_range_time_by_m.png
fig_utilization_by_m.png

12.2 Plot Additional Experiment Results

python3 plot_additional_results.py

This script generates figure files such as:

fig_additional_range_m3.png
fig_additional_range_m5.png
fig_additional_range_m10.png
fig_additional_search_m3.png
fig_additional_search_m5.png
fig_additional_search_m10.png
fig_additional_search_by_m_Successful.png
fig_additional_search_by_m_Unsuccessful.png

The figures used in the report are mainly:

fig_splits_by_m.png
fig_range_time_by_m.png
fig_utilization_by_m.png
fig_additional_range_m10.png
fig_additional_search_m10.png

⸻

13. Implementation Notes

13.1 B-tree

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

13.2 B*-tree

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

13.3 B+-tree

The B+-tree stores all RIDs only in leaf nodes.
Internal nodes contain only keys and child pointers.
Leaf nodes are connected using linked pointers to support efficient range scans.

Implemented operations:

* Insert
* Search
* Delete
* Range Query through linked leaf traversal

Search always proceeds to a leaf node.

⸻

14. Example Full Execution

The following commands compile and run all basic experiments.

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

To run additional experiments:

g++ -std=c++17 -O2 -DM=3 -Iinclude src/additional_main.cpp src/BTree.cpp src/BPlusTree.cpp src/BStarTree.cpp src/Utils.cpp -o additional_m3
g++ -std=c++17 -O2 -DM=5 -Iinclude src/additional_main.cpp src/BTree.cpp src/BPlusTree.cpp src/BStarTree.cpp src/Utils.cpp -o additional_m5
g++ -std=c++17 -O2 -DM=10 -Iinclude src/additional_main.cpp src/BTree.cpp src/BPlusTree.cpp src/BStarTree.cpp src/Utils.cpp -o additional_m10
./additional_m3
./additional_m5
./additional_m10

⸻

15. Expected Generated Files

After running the basic experiments, the following files will be generated:

result_m3.csv
result_m5.csv
result_m10.csv
results.csv

After running the additional experiments, the following files will be generated:

additional_range_results_m3.csv
additional_range_results_m5.csv
additional_range_results_m10.csv
additional_search_results_m3.csv
additional_search_results_m5.csv
additional_search_results_m10.csv

After running the plotting scripts, figure files such as the following will be generated:

fig_splits_by_m.png
fig_range_time_by_m.png
fig_utilization_by_m.png
fig_additional_range_m10.png
fig_additional_search_m10.png

⸻

16. Notes

* The program assumes that data/student.csv exists before execution.
* The same random seed is used for point search and deletion workloads to make the experiments reproducible.
* The same range query is executed on all three tree structures.
* M is set at compile time using the -DM compiler option.
* Additional experiments are not required for running the basic workloads, but they are included to reproduce the analysis in the report.
* If the executable cannot find the dataset, check whether data/student.csv exists relative to the project root directory.

