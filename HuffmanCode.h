//
// Created by LAMM_ on 02/11/2021.
//

#ifndef PRACTICA_4_HUFFMANCODE_H
#define PRACTICA_4_HUFFMANCODE_H

#include <utility>
#include <vector>
#include <string>
#include <chrono>
#include <algorithm>
#include <iostream>
#include <limits>
#include <bitset>
#include <fstream>
#include <sstream>
#include <cmath>
#include <cstdlib>
#include <cmath>
#include <array>

struct CharNode{
    int sum;
    std::string label;
    char letter;
    CharNode *left;
    CharNode * right;
};

struct BitInfo{
    int bit_size;
    std::string bit_set;
    long value;
};

using CharFrequency = std::pair<char, int>;                 //First position contains character and second position their frequency
using TableFrequency = std::vector<CharFrequency>;          //Table of frequency for characters
using Time_Frame = std::chrono::duration<double>;           //Time lapse
using Timer = std::chrono::system_clock;                    //Clock structure
using Checkpoint = std::chrono::system_clock::time_point;   //Stores time checkpoint
using ListPos = TableFrequency::iterator;                   //Iterator (position in array for table frequency)
using NodeList = std::vector<CharNode*>;                    //List of char nodes
using BinaryMapping = std::vector<BitInfo>;                 //Compression codes for characters, positions are equal for table frequency

class HuffmanCode {
    public:
        HuffmanCode() = delete;
        explicit HuffmanCode(std::string&);                     //Constructor
        Time_Frame& getBuildTime();                             //Returns time taken to build tree
        void compressFile(std::string&, std::string&&);         //Inits compression file
        void decompressFile(std::string&&, std::string&&);      //Inits decompression file
        void printList();                                       //Print Table Frequency and binary code for all characters
        void clear();                                           //Release all attributes
        ~HuffmanCode();
    protected:

        static bool isSameLetter(char, CharFrequency&);

        static bool compareAsCharCount(CharFrequency&, CharFrequency&);
        static bool compareAsSumNode(CharNode*, CharNode*);
        static void makeNodeTable(TableFrequency&, NodeList&);
        static void combineNodesAsMinSum(NodeList &, int, int, int);
        static void appendNodes(CharNode*, CharNode*, CharNode*);
        void traceBinaryCode(long, int, CharNode*, TableFrequency&, BinaryMapping&);
        static void releaseTree(CharNode*);
        static std::string numberToExtendedBinaryString(long number, size_t input_size);
        static void setBinaryExpressionZeros(std::string& binary, size_t input_size);
        static unsigned char bitSetToChar(std::string&& bit_chunk);
        static void generateZeros(std::string&, size_t);
    private:
        //Member Methods
        size_t findInFrequencyTable(char);
        void buildTree();                                       //Inits binary tree building and binary code for characters
        //Non-member Methods
        static void eraseFromNodeTable(int, int, NodeList&);    //Erase Node from node list
        //Member Attributes
        TableFrequency mTable_frequency;
        NodeList mNode_table;
        Checkpoint mStart, mFinish;
        Time_Frame mTime_frame;
        CharNode * mTree_root;
        BinaryMapping mCode_Table;
        std::string original_file_content;
};


#endif //PRACTICA_4_HUFFMANCODE_H
