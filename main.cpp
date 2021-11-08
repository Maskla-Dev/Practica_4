/* Luis Antonio Montoya Morales
 * Practice 4 - Huffman Code
 * 7/11/21
*/

#include "HuffmanCode.h"
#include "BitMapCompressor.h"

std::string readFile(std::string&);                 //Returns file content as string
bool compareFiles(std::string&&, std::string&&);    //Compare 2 files and returns true if the content of both are equal, false otherwise

int main() {
    //Bidimensional array.
    //Bidimensional Array Contains file names for testing, first position is original file name, second compress file name and third decompress file name.
    std::array<std::array<std::string, 3>, 4> test_files = {"ADN1.txt", "ADN1_compressed.txt", "ADN1_decompressed.txt",
                                                            "ADN2.txt", "ADN2_compressed.txt", "ADN2_decompressed.txt",
                                                            "ADN3.txt", "ADN3_compressed.txt", "ADN3_decompressed.txt",
                                                            "Texto.txt", "Texto_compressed.txt", "Texto_decompressed.txt"};
    std::string file_content;
    HuffmanCode *compresor;
    //Do 4 test Huffman Code compression
    for(auto & test_file : test_files){
        std::cout << "Start compression for: " << test_file[0] << std::endl;
        file_content = readFile(test_file[0]);
        compresor = new HuffmanCode(file_content);
        std::cout << "Build time: " << compresor->getBuildTime().count() << std::endl;
        compresor->printList();
        compresor->compressFile(file_content, std::move(test_file[1]));
        //Releases compresor attributes with the purpose of do a full test to compressed file
        compresor->clear();
        compresor->decompressFile(std::move(test_file[1]), std::move(test_file[2]));
        if(compareFiles(std::move(test_file[0]), std::move(test_file[2])))
            std::cout << "Successful compression and decompression\n" << std::endl;
        else
            std::cout << "Compression was not successful\n" << std::endl;
        delete compresor;
        file_content.clear();
    }
    //Testing image compression
    BitMapCompressor bitmap("test_image.bmp", 512, 512);
    bitmap.compressImage("test_image.cbmp");
    bitmap.printList();
    bitmap.clear();
    bitmap.decompressImage("test_image.cbmp", "decompressed_test_image.bmp");
    bitmap.printList();
    if(compareFiles("test_image.bmp", "decompressed_test_image.bmp"))
        std::cout << "Successful compression and decompression\n" << std::endl;
    else
        std::cout << "Compression was not successful\n" << std::endl;
    bitmap.clear();
    return 0;
}

bool compareFiles(std::string&& file1_str, std::string&& file2_str){
    std::ifstream file1, file2;
    std::stringstream buffer1, buffer2;
    file1.open(file1_str);
    file2.open(file2_str);
    size_t less_length;
    std::string file_content1, file_content2;
    if(file1.is_open() && file2.is_open()){
        buffer1 << file1.rdbuf();
        buffer2 << file2.rdbuf();
        file_content1 = buffer1.str();
        file_content2 = buffer2.str();
        less_length = file_content1.size() <= file_content2.size() ? file_content1.size() : file_content2.size();
        for(int i = 0; i < less_length; ++i)
            if(file_content1[i] != file_content2[i])
                std::cout << "Differences at " << i << " with " << file_content1[i] << "|" << file_content2[i] << std::endl;
        if(file_content1.size() < file_content2.size())
            std::cout << "File 2 is greater than File 1, overflow with: " << file_content2.substr(file_content1.size(), file_content2.size()) << std::endl;
        else if(file_content1.size() > file_content2.size())
            std::cout << "File 1 is greater than File 2, overflow: with" << file_content1.substr(file_content2.size(), file_content1.size()) << std::endl;
    }
    else
        std::cerr << "Cannot open file" << std::endl;
    return (buffer1.str() == buffer2.str());
}

std::string readFile(std::string& file_name){
    std::ifstream file;
    std::stringstream buffer;
    file.open(file_name);
    if(file.is_open())
        buffer << file.rdbuf();
    else
        std::cerr << "Cannot open file" << std::endl;
    return buffer.str();
}