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
        ~HuffmanCode();                                         //Destructor of the class
    protected:
    private:
        //Member Methods
        size_t findInFrequencyTable(char);
        void buildTree();                                       //Inits binary tree building and binary code for characters
        void makeNodeList();                                    //Builds node table
        void combineNodesAsMinSum(int, int);                    //Combine 2 nodes: i and j. Resulting node is pushed to the Node List and the ith and jth are erased from Node List
        void eraseFromNodeList(int, int);                       //Erase Node from node list
        //Non-member Methods
        static bool compareAsSumNode(CharNode*, CharNode*);                             //Compares 2 nodes, if ith node is minor than jth, returns true
        static void generateZeros(std::string&, size_t);                                //Generate a string with n zeros
        static std::string numberToExtendedBinaryString(long, size_t);                  //Convert long to binary string representation
        static unsigned char bitSetToChar(std::string&&);                               //Convert an 8 bit set (string representation) to unsigned char
        static void releaseTree(CharNode*);                                             //Release memory used by a binary tree
        static bool compareAsCharCount(CharFrequency&, CharFrequency&);                 //Compare 2 char frequency, if ith char frequency is minor than jth, returns true
        static void appendNodes(CharNode*, CharNode*, CharNode*);                       //Append children nodes to a parent node from binary tree
        void traceBinaryCode(long, int, CharNode*, TableFrequency&, BinaryMapping&);    //Fills compression code list from binary tree
        //Member Attributes
        TableFrequency mTable_frequency;
        NodeList mNode_list;
        Checkpoint mStart, mFinish;
        Time_Frame mTime_frame;
        CharNode * mTree_root;
        BinaryMapping mCode_list;
        std::string original_file_content;
};

#endif //PRACTICA_4_HUFFMANCODE_H
