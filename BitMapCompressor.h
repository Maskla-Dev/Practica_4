//
// Created by LAMM_ on 07/11/2021.
//

#ifndef PRACTICA_4_BITMAPCOMPRESSOR_H
#define PRACTICA_4_BITMAPCOMPRESSOR_H

#include <cstdlib>
#include "HuffmanCode.h"

struct BmpHeader {
    char bitmapSignatureBytes[2] = {'B', 'M'};
    uint32_t sizeOfBitmapFile = 54 + 786432;
    uint32_t reservedBytes = 0;
    uint32_t pixelDataOffset = 54;
};

struct BmpInfoHeader {
    uint32_t sizeOfThisHeader = 40;
    int32_t width = 512; // in pixels
    int32_t height = 512; // in pixels
    uint16_t numberOfColorPlanes = 1; // must be 1
    uint16_t colorDepth = 24;
    uint32_t compressionMethod = 0;
    uint32_t rawBitmapDataSize = 0; // generally ignored
    int32_t horizontalResolution = 3780; // in pixel per meter
    int32_t verticalResolution = 3780; // in pixel per meter
    uint32_t colorTableEntries = 0;
    uint32_t importantColors = 0;
};

struct Pixel {
    uint8_t blue = 0;
    uint8_t green = 0;
    uint8_t red = 0;
};

struct PixelNode{
    long long sum = 0;
    Pixel color;
    PixelNode* left = nullptr;
    PixelNode* right = nullptr;
    bool is_leaf_node = false;
    size_t in_list_pos = std::numeric_limits<size_t>::max();
};

using Image = std::vector<Pixel>;
using PixelFrequency = std::pair<Pixel, size_t>;
using PixelFrequencyTable = std::vector<PixelFrequency>;
using PixelNodeList = std::vector<PixelNode*>;

class BitMapCompressor {
    public:
        BitMapCompressor() = delete;
        explicit BitMapCompressor(std::string&&, int32_t, int32_t);
        void compressImage(std::string&&);
        void decompressImage(std::string&&, std::string&&);
        void printList();
        void clear();
    private:
        void generateRandomImage(int32_t, int32_t);
        void saveHeaders();
        void savePixel(Pixel&);
        void initFrequencyTable();
        size_t findInFrequencyTable(int32_t, int32_t, int32_t);
        void buildTree();
        void makeNodeList();
        void combineNodesAsSum(size_t i, size_t j);
        void eraseFromNodeList(size_t i, size_t j);
        void traceBinaryCode(long, int, PixelNode*, PixelFrequencyTable&, BinaryMapping&);
        void loadPixel(Pixel&);
        void loadHeaders();
        //Non-Member Methods
        static bool compareAsPixelCount(PixelFrequency&, PixelFrequency&);
        static void appendNodes(PixelNode*, PixelNode*, PixelNode*);
        static bool compareAsNodeSum(PixelNode*, PixelNode*);
        static void releaseTree(PixelNode*);
        //Attributes
        std::string mFile_Name;
        std::ofstream mOut_file;
        std::ifstream mIn_file;
        Image mImage;
        BmpHeader mHeader;
        BmpInfoHeader mInfo_header;
        PixelFrequencyTable mFrequency_table;
        PixelNodeList mNode_list;
        PixelNode* mTree_root;
        BinaryMapping mCode_list;
};

#endif //PRACTICA_4_BITMAPCOMPRESSOR_H
