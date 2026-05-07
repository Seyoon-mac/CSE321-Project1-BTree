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
