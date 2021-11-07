//
// Created by LAMM_ on 02/11/2021.
//

#include "HuffmanCode.h"

HuffmanCode::HuffmanCode(std::string& raw_input) : mTime_frame(Time_Frame::zero()), mTree_root(nullptr), original_file_content(raw_input) {
    size_t in_list_pos;
    //Count characters from file content and stores in frequency table
    for(auto i : original_file_content){
        in_list_pos = findInFrequencyTable(i);
        if(in_list_pos != std::numeric_limits<size_t>::max())
            ++mTable_frequency[in_list_pos].second;
        else
            mTable_frequency.push_back(CharFrequency(i, 1));
    }
    std::sort(mTable_frequency.begin(), mTable_frequency.end(), compareAsCharCount);
    mStart = Timer::now();
    buildTree();
    mFinish = Timer::now();
    mTime_frame = mFinish - mStart;
}

Time_Frame &HuffmanCode::getBuildTime() {
    return mTime_frame;
}

void HuffmanCode::compressFile(std::string &file_content, std::string &&file_name) {
    std::ofstream file(file_name, std::ios::binary | std::ios::out);
    size_t pos = mTable_frequency.size();
    std::string bit_set_to_write;
    unsigned char character;
    int freq;
    size_t max_size = file_content.size();
    //Saves max size of file
    file.write(reinterpret_cast<const char*>(&max_size), sizeof(max_size));
    if(file.is_open()){
        //Saves frequency table
        file.write(reinterpret_cast<const char *>(&pos), sizeof(size_t));
        for(auto& i : mTable_frequency){
            character = i.first;
            freq = i.second;
            file << character;
            file.write(reinterpret_cast<const char*>(&freq), sizeof(freq));
        }
        //Make bit string
        for(auto i : file_content){
            pos = findInFrequencyTable(i);
            if(pos != std::numeric_limits<size_t>::max()){
                bit_set_to_write.append(mCode_list[pos].bit_set);
            }
        }
        //Cuts string in 8 bits chunks and writes in file as character
        int repeat_times = static_cast<int>(bit_set_to_write.size() / 8);
        for(int i = 0; i < repeat_times; ++i){
            character = bitSetToChar(bit_set_to_write.substr(0,8));
            file.write(reinterpret_cast<const char*>(&character), sizeof(character));
            bit_set_to_write.erase(0, 8);
        }
        //Completes residual chunk
        if(!bit_set_to_write.empty()){
            generateZeros(bit_set_to_write, 8 - bit_set_to_write.size());
            character = bitSetToChar(std::move(bit_set_to_write));
            file.write(reinterpret_cast<const char*>(&character), sizeof(character));
        }
    }
    else{
        std::cerr << "Cannot open file while compress" << std::endl;
    }
    file.close();
}

void HuffmanCode::decompressFile(std::string &&compressed_file, std::string &&output_file) {
    CharNode *current_node;
    std::ifstream in_file(compressed_file, std::ios::in | std::ios::binary);
    std::ofstream out_file(output_file, std::ios::out);
    std::string file_content, tmp_set;
    unsigned char byte_chunk = 0;
    size_t pos;
    //From compressed file, initialize attributes
    if(in_file.is_open()){
        size_t size;
        CharFrequency tmp;
        size_t max_size;
        in_file.read(reinterpret_cast<char*>(&max_size), sizeof(max_size));
        //Remakes tree
        in_file.read(reinterpret_cast<char*>(&size), sizeof(size));
        for(int i = 0; i < size; ++i) {
            in_file.read(&tmp.first, sizeof(tmp.first));
            in_file.read(reinterpret_cast<char*>(&tmp.second), sizeof(tmp.second));
            mTable_frequency.push_back(tmp);
        }
        std::sort(mTable_frequency.begin(), mTable_frequency.end(), compareAsCharCount);
        buildTree();
        //Reprints frequency table, only for comparative purposes
        printList();
        current_node = mTree_root;
        int characters_writen = 0;
        /* Decompression process
         * Data:    byte_chunk - 8 bit chunk
         *          head - Mimics a turing machine head, init binary value is 1000'0000
         *          current_node - current node of binary tree, starts from root
         *          max_size - Max size of file
         *          characters_writen - Characters writen on file
         *          pos - Index from frequency table
         * 1 Reads 8 bit chunks from compressed file
         *  1.1 Do AND operation with byte_chunk and head
         *  1.2 If the operation is equal to 0, move to left branch, otherwise move to right branch
         *  1.3 If node is not empty, determine if is a leaf node, if is the case print character in file, otherwise don't do anything
         *  1.4 Moves head 1 bit to right
         *  1.5 Repeat until reach max_size in characters_writen
         *
         *  For determine if is a leaf node, search the character representation from the current node in frequency table. If exist in table,
         *  current node is a leaf node.
        */
        while(characters_writen < max_size){
            //Reads bit chunks from file
            in_file.read(reinterpret_cast<char*>(&byte_chunk), sizeof(byte_chunk));
            //Maps tree path for all bits
            for(int i = 0, head = 0b1000'0000; i < 8; ++i){
                if((byte_chunk & head) == head)
                    current_node = current_node->right;
                else
                    current_node = current_node->left;
                if(current_node != nullptr)
                    //Determines if is a leaf node
                    pos = findInFrequencyTable(current_node->letter);
                else{
                    pos = std::numeric_limits<size_t>::max();
                    current_node = mTree_root;
                }
                if(pos != std::numeric_limits<size_t>::max()){
                    ++characters_writen;
                    if(characters_writen <= max_size){
                        file_content += mTable_frequency[pos].first;
                        current_node = mTree_root;
                    }
                }
                head >>= 1;
            }
        }
        //Write file_content to file
        if(out_file.is_open()){
            out_file << file_content;
        }
        else{
            std::cerr << "Cannot open file while compress" << std::endl;
        }
    }
    else
        std::cerr << "Cannot open file while compress" << std::endl;
}

void HuffmanCode::printList() {
    std::cout << "|----------Char count----------|" << std::endl;
    for(int i = 0; i < mTable_frequency.size(); ++i){
        std::cout << "[";
        if(mTable_frequency[i].first == '\n')
            std::cout << "LF";
        else
            std::cout << mTable_frequency[i].first;
        std::cout << "] : " << mTable_frequency[i].second << " - " << mCode_list[i].value << "|" << mCode_list[i].bit_size << std::endl;
    }
    std::cout << "|-------------- ---------------|" << std::endl;
}

void HuffmanCode::clear() {
    releaseTree(mTree_root);
    mTable_frequency.clear();
    mCode_list.clear();
    mNode_list.clear();
}

size_t HuffmanCode::findInFrequencyTable(char letter) {
    for(int i = 0; i < mTable_frequency.size(); ++i)
        if(mTable_frequency[i].first == letter)
            return i;
    return std::numeric_limits<size_t>::max();
}

void HuffmanCode::buildTree(){
    makeNodeList();
    while(mNode_list.size() > 2)
        combineNodesAsMinSum(0, 1);
    //Combine residual nodes
    combineNodesAsMinSum(0, 1);
    mTree_root = mNode_list.front();
    mCode_list = BinaryMapping(mTable_frequency.size());
    traceBinaryCode(0b00, 0, mTree_root, mTable_frequency, mCode_list);
}

void HuffmanCode::makeNodeList() {
    CharNode* node;
    for(auto& i : mTable_frequency){
        node = new CharNode;
        node->left = nullptr;
        node->right = nullptr;
        node->label = i.first;
        node->sum = i.second;
        node->letter = i.first;
        mNode_list.push_back(node);
    }
}

void HuffmanCode::combineNodesAsMinSum(int i, int j) {
    auto* combined_node = new CharNode;
    appendNodes(combined_node, mNode_list[i], mNode_list[j]);
    eraseFromNodeList(i, j);
    mNode_list.push_back(combined_node);
    std::sort(mNode_list.begin(), mNode_list.end(), compareAsSumNode);
}

void HuffmanCode::eraseFromNodeList(int i, int j) {
    mNode_list.erase(mNode_list.begin() + i);
    mNode_list.erase(mNode_list.begin() + j - 1);
}

HuffmanCode::~HuffmanCode() {
    clear();
}

//Non-member methods

bool HuffmanCode::compareAsCharCount(CharFrequency &i, CharFrequency &j) {
    return i.second < j.second;
}

void HuffmanCode::appendNodes(CharNode* parent, CharNode* left_child, CharNode* right_child) {
    parent->left = left_child;
    parent->right = right_child;
    parent->sum = left_child->sum + right_child->sum;
    parent->label = left_child->label + right_child->label;
    std::cout << left_child->label << " with " << right_child->label << " : " << parent->sum << std::endl;
}

bool HuffmanCode::compareAsSumNode(CharNode *i, CharNode *j) {
    return i->sum < j->sum;
}

/* Gets binary code compression for leaf nodes, recursively follows:
 * 1 Get code (long value) and current node
 * 2 Search node character value in frequency table,
 *  2.1 If exist node character in table, current node is a leaf node (this is the base case)
 *      2.1.1 Store string binary representation
 *  2.2 Otherwise
 *      2.2.1 Calls the same process appending (moves 1 bit to left before add) 1 to right child
 *      2.2.2 Calls the same process appending (moves 1 bit to left before add) 0 to left child
*/
void HuffmanCode::traceBinaryCode(long code, int size, CharNode *node, TableFrequency& table, BinaryMapping& list_code) {
    size_t pos;
    if(node != nullptr){
        pos = findInFrequencyTable(node->letter);
        if(pos != std::numeric_limits<size_t>::max()){
            list_code[pos].bit_size = size;
            list_code[pos].value = code;
            list_code[pos].bit_set = numberToExtendedBinaryString(list_code[pos].value, list_code[pos].bit_size);
        }
        else{
            traceBinaryCode((code << 1) | 0b00, size + 1, node->left, table, list_code);
            traceBinaryCode((code << 1) | 0b01, size + 1, node->right, table, list_code);
        }
    }
}

void HuffmanCode::generateZeros(std::string& src, size_t size){
    for(int i = 0; i < size; ++i)
        src.append("0");
}

void HuffmanCode::releaseTree(CharNode *node) {
    if(node->left != nullptr)
        releaseTree(node->left);
    if(node->right != nullptr)
        releaseTree(node->right);
    delete node;
}



std::string HuffmanCode::numberToExtendedBinaryString(long number, size_t input_size){
    std::string binary_expression;
    generateZeros(binary_expression, input_size);
    while(number != 0){
        binary_expression[(input_size--) - 1] = static_cast<char>(48 + (number % 2));
        number /= 2;
    }
    return binary_expression;
}

unsigned char HuffmanCode::bitSetToChar(std::string &&bit_chunk) {
    unsigned char character = 0;
    short temp = 1;
    for(long long i = bit_chunk.size() - 1; i >= 0; --i){
        if(bit_chunk[i] == '1')
            character += temp;
        temp = static_cast<short>(temp * 2);
    }
    return character;
}
