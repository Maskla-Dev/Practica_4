//
// Created by LAMM_ on 07/11/2021.
//

#include "BitMapCompressor.h"

BitMapCompressor::BitMapCompressor(std::string&& file_name, int32_t height, int32_t width) {
    mOut_file.open(file_name);
    generateRandomImage(height, width);
    mOut_file.close();
    initFrequencyTable();
    mTree_root = nullptr;
    buildTree();

}

void BitMapCompressor::generateRandomImage(int32_t height, int32_t width) {
    mHeader.sizeOfBitmapFile = 54 + (height * width * 3);
    mInfo_header.height = height;
    mInfo_header.width = width;
    saveHeaders();
    srand(time(nullptr));
    auto *tmp_pixel = new Pixel;
    for(size_t i = 0; i < height * width; ++i){
        tmp_pixel->red = tmp_pixel->green = tmp_pixel->blue = rand() % 256;
        mImage.push_back(*tmp_pixel);
        savePixel(*tmp_pixel);
    }
}

void BitMapCompressor::saveHeaders() {
    //Header
    mOut_file.write(mHeader.bitmapSignatureBytes, 2);
    mOut_file.write(reinterpret_cast<char *>(&mHeader.sizeOfBitmapFile), sizeof(uint32_t));
    mOut_file.write(reinterpret_cast<char *>(&mHeader.reservedBytes), sizeof(uint32_t));
    mOut_file.write(reinterpret_cast<char *>(&mHeader.pixelDataOffset), sizeof(uint32_t));
    //InfoHeader
    mOut_file.write(reinterpret_cast<char *>(&mInfo_header.sizeOfThisHeader), sizeof(uint32_t));
    mOut_file.write(reinterpret_cast<char *>(&mInfo_header.width), sizeof(int32_t));
    mOut_file.write(reinterpret_cast<char *>(&mInfo_header.height), sizeof(int32_t));
    mOut_file.write(reinterpret_cast<char *>(&mInfo_header.numberOfColorPlanes), sizeof(uint16_t));
    mOut_file.write(reinterpret_cast<char *>(&mInfo_header.colorDepth), sizeof(uint16_t));
    mOut_file.write(reinterpret_cast<char *>(&mInfo_header.compressionMethod), sizeof(uint32_t));
    mOut_file.write(reinterpret_cast<char *>(&mInfo_header.rawBitmapDataSize), sizeof(uint32_t));
    mOut_file.write(reinterpret_cast<char *>(&mInfo_header.horizontalResolution), sizeof(int32_t));
    mOut_file.write(reinterpret_cast<char *>(&mInfo_header.verticalResolution), sizeof(int32_t));
    mOut_file.write(reinterpret_cast<char *>(&mInfo_header.colorTableEntries), sizeof(uint32_t));
    mOut_file.write(reinterpret_cast<char *>(&mInfo_header.importantColors), sizeof(uint32_t));
}

void BitMapCompressor::savePixel(Pixel& pixel) {
    mOut_file.write(reinterpret_cast<char *>(&pixel.blue), sizeof(pixel.blue));
    mOut_file.write(reinterpret_cast<char *>(&pixel.green), sizeof(pixel.green));
    mOut_file.write(reinterpret_cast<char *>(&pixel.red), sizeof(pixel.red));
}

void BitMapCompressor::initFrequencyTable() {
    size_t in_list_pos = 0;
    for(auto i : mImage){
        in_list_pos = findInFrequencyTable(i.blue, i.green, i.red);
        if(in_list_pos != std::numeric_limits<size_t>::max())
            ++mFrequency_table[in_list_pos].second;
        else
            mFrequency_table.push_back(PixelFrequency(i, 1));
    }
    std::sort(mFrequency_table.begin(), mFrequency_table.end(), compareAsPixelCount);
}

size_t BitMapCompressor::findInFrequencyTable(int32_t blue, int32_t green, int32_t red) {
    for(int i = 0; i < mFrequency_table.size(); ++i)
        if(mFrequency_table[i].first.red == red && mFrequency_table[i].first.green == green && mFrequency_table[i].first.blue == blue)
            return i;
    return std::numeric_limits<size_t>::max();
}

bool BitMapCompressor::compareAsPixelCount(PixelFrequency &i, PixelFrequency &j) {
    return i.second < j.second;
}

void BitMapCompressor::buildTree() {
    makeNodeList();
    while(mNode_list.size() > 2){
        combineNodesAsSum(0, 1);
    }
    combineNodesAsSum(0, 1);
    mTree_root = mNode_list[0];
    mCode_list = BinaryMapping(mFrequency_table.size());
    traceBinaryCode(0b00, 0, mTree_root, mFrequency_table, mCode_list);
}

void BitMapCompressor::makeNodeList() {
    PixelNode* node;
    for(int i = 0; i < mFrequency_table.size(); ++i){
        node = new PixelNode;
        node->sum = mFrequency_table[i].second;
        node->color = mFrequency_table[i].first;
        node->is_leaf_node = true;
        node->in_list_pos = i;
        mNode_list.push_back(node);
    }
}

void BitMapCompressor::combineNodesAsSum(size_t i, size_t j) {
    auto* parent_node = new PixelNode;
    appendNodes(parent_node, mNode_list[i], mNode_list[j]);
    eraseFromNodeList(i , j);
    mNode_list.push_back(parent_node);
    std::sort(mNode_list.begin(), mNode_list.end(), compareAsNodeSum);
}

void BitMapCompressor::appendNodes(PixelNode *parent, PixelNode *left_child, PixelNode *right_child) {
    parent->left = left_child;
    parent->right = right_child;
    parent->sum = left_child->sum + right_child->sum;
    /*std::cout   << "[" << left_child->color.blue << "," << left_child->color.green << "," << left_child->color.red
                << "] with [" << right_child->color.blue << "," << right_child->color.green << "," << right_child->color.red << "] : " << parent->sum << std::endl;*/
}

void BitMapCompressor::eraseFromNodeList(size_t i, size_t j) {
    mNode_list.erase(mNode_list.begin() + i);
    mNode_list.erase(mNode_list.begin() + j - 1);
}

bool BitMapCompressor::compareAsNodeSum(PixelNode *i, PixelNode *j) {
    return i->sum < j->sum;
}

void BitMapCompressor::traceBinaryCode(long code, int size, PixelNode* node, PixelFrequencyTable& frequency_table, BinaryMapping& code_map) {
    size_t pos;
    if(node != nullptr){
        if(node->is_leaf_node){
            code_map[node->in_list_pos].bit_size = size;
            code_map[node->in_list_pos].value = code;
            code_map[node->in_list_pos].bit_set = HuffmanCode::numberToExtendedBinaryString(code_map[node->in_list_pos].value, code_map[node->in_list_pos].bit_size);
        }
        else{
            traceBinaryCode((code << 1) | 0b00, size + 1, node->left, frequency_table, code_map);
            traceBinaryCode((code << 1) | 0b01, size + 1, node->right, frequency_table, code_map);
        }
    }
}

void BitMapCompressor::compressImage(std::string &&out_file_name) {
    mOut_file.open(out_file_name, std::ios::out | std::ios::binary);
    std::string bit_set_to_write;
    unsigned char character;
    size_t freq;
    size_t pos = mFrequency_table.size();
    saveHeaders();
    mOut_file.write(reinterpret_cast<const char *>(&pos), sizeof(size_t));
    for(auto& i : mFrequency_table){
        savePixel(i.first);
        freq = i.second;
        mOut_file.write(reinterpret_cast<const char*>(&freq), sizeof(freq));
    }
    for(auto i : mImage){
        pos = findInFrequencyTable(i.blue, i.green, i.red);
        if(pos != std::numeric_limits<size_t>::max()){
            bit_set_to_write.append(mCode_list[pos].bit_set);
        }
    }
    int repeat_times = static_cast<int>(bit_set_to_write.size() / 8);
    for(int i = 0; i < repeat_times; ++i){
        character = HuffmanCode::bitSetToChar(bit_set_to_write.substr(0,8));
        mOut_file.write(reinterpret_cast<const char*>(&character), sizeof(character));
        bit_set_to_write.erase(0, 8);
    }
    //Completes residual chunk
    if(!bit_set_to_write.empty()){
        HuffmanCode::generateZeros(bit_set_to_write, 8 - bit_set_to_write.size());
        character = HuffmanCode::bitSetToChar(std::move(bit_set_to_write));
        mOut_file.write(reinterpret_cast<const char*>(&character), sizeof(character));
    }
    mOut_file.close();
}

void BitMapCompressor::clear() {
    releaseTree(mTree_root);
    mFrequency_table.clear();
    mCode_list.clear();
    mNode_list.clear();
    mImage.clear();
}

void BitMapCompressor::releaseTree(PixelNode *node) {
    if(node->left != nullptr)
        releaseTree(node->left);
    if(node->right != nullptr)
        releaseTree(node->right);
    delete node;
}

void BitMapCompressor::decompressImage(std::string && in_file_name, std::string&& out_file_name) {
    mIn_file.open(in_file_name, std::ios::binary | std::ios::in);
    mOut_file.open(out_file_name, std::ios::binary | std::ios::out);
    size_t size;
    Pixel tmp_pix;
    size_t freq;
    char byte_chunk;
    loadHeaders();
    mIn_file.read(reinterpret_cast<char *>(&size), sizeof(size));
    for(int i = 0; i < size; ++i){
        loadPixel(tmp_pix);
        mIn_file.read(reinterpret_cast<char *>(&freq), sizeof(freq));
        mFrequency_table.push_back(PixelFrequency(tmp_pix, freq));
    }
    std::sort(mFrequency_table.begin(), mFrequency_table.end(), compareAsPixelCount);
    buildTree();
    saveHeaders();
    PixelNode *current_node = mTree_root;
    size_t pixels_writen = 0;
    while(pixels_writen <= (mInfo_header.width * mInfo_header.height)){
        mIn_file.read(reinterpret_cast<char*>(&byte_chunk), sizeof(byte_chunk));
        for(int i = 0, head = 0b1000'0000; i < 8; ++i){
            if((byte_chunk & head) == head)
                current_node = current_node->right;
            else
                current_node = current_node->left;
            if(current_node == nullptr)
                current_node = mTree_root;
            if(current_node->is_leaf_node){
                ++pixels_writen;
                if(pixels_writen <= (mInfo_header.width * mInfo_header.height)){
                    savePixel(current_node->color);
                    current_node = mTree_root;
                }
            }
            head >>= 1;
        }
    }
}

void BitMapCompressor::loadHeaders() {
    //Header
    mIn_file.read(mHeader.bitmapSignatureBytes, 2);
    mIn_file.read(reinterpret_cast<char *>(&mHeader.sizeOfBitmapFile), sizeof(uint32_t));
    mIn_file.read(reinterpret_cast<char *>(&mHeader.reservedBytes), sizeof(uint32_t));
    mIn_file.read(reinterpret_cast<char *>(&mHeader.pixelDataOffset), sizeof(uint32_t));
    //InfoHeader
    mIn_file.read(reinterpret_cast<char *>(&mInfo_header.sizeOfThisHeader), sizeof(uint32_t));
    mIn_file.read(reinterpret_cast<char *>(&mInfo_header.width), sizeof(int32_t));
    mIn_file.read(reinterpret_cast<char *>(&mInfo_header.height), sizeof(int32_t));
    mIn_file.read(reinterpret_cast<char *>(&mInfo_header.numberOfColorPlanes), sizeof(uint16_t));
    mIn_file.read(reinterpret_cast<char *>(&mInfo_header.colorDepth), sizeof(uint16_t));
    mIn_file.read(reinterpret_cast<char *>(&mInfo_header.compressionMethod), sizeof(uint32_t));
    mIn_file.read(reinterpret_cast<char *>(&mInfo_header.rawBitmapDataSize), sizeof(uint32_t));
    mIn_file.read(reinterpret_cast<char *>(&mInfo_header.horizontalResolution), sizeof(int32_t));
    mIn_file.read(reinterpret_cast<char *>(&mInfo_header.verticalResolution), sizeof(int32_t));
    mIn_file.read(reinterpret_cast<char *>(&mInfo_header.colorTableEntries), sizeof(uint32_t));
    mIn_file.read(reinterpret_cast<char *>(&mInfo_header.importantColors), sizeof(uint32_t));
}

void BitMapCompressor::loadPixel(Pixel& pixel) {
    mIn_file.read(reinterpret_cast<char *>(&pixel.blue), sizeof(pixel.blue));
    mIn_file.read(reinterpret_cast<char *>(&pixel.green), sizeof(pixel.green));
    mIn_file.read(reinterpret_cast<char *>(&pixel.red), sizeof(pixel.red));
}

void BitMapCompressor::printList() {
    std::cout << "|----------Char count----------|" << std::endl;
    for(int i = 0; i < mFrequency_table.size(); ++i){
        std::cout << "[";
        std::cout << mFrequency_table[i].first.red << "," << mFrequency_table[i].first.green << "," << mFrequency_table[i].first.blue;
        std::cout << "] : " << mFrequency_table[i].second << " - " << mCode_list[i].value << "|" << mCode_list[i].bit_size << std::endl;
    }
    std::cout << "|-------------- ---------------|" << std::endl;
}