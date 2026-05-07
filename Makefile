CXX = g++
CXXFLAGS = -std=c++17 -Iinclude

TARGET = main

SRC = src/main.cpp \
      src/Record.cpp \
      src/BTree.cpp \
      src/BPlusTree.cpp \
      src/BStarTree.cpp \
      src/Experiment.cpp \
      src/Utils.cpp

all:
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET)