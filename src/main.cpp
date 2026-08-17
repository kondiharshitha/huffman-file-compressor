#include "Huffman.cpp"
#include <iostream>
#include <string>
int main() {
    HuffmanCoding compressor;
    std::cout << "====================================\n";
    std::cout << "       HUFFMAN FILE COMPRESSOR\n";
    std::cout << "====================================\n";
    std::cout << "\n1. Compress a file\n";
    std::cout << "2. Decompress a file\n";
    std::cout << "3. Exit\n";
    int choice;
    std::cout << "\nEnter your choice: ";
    std::cin >> choice;
    if (choice == 3) {
        std::cout << "\nExiting...\n";
        return 0;
    }
    if (choice != 1 && choice != 2) {
        std::cout << "\nInvalid choice.\n";
        return 1;
    }
    std::string inputFile;
    std::string outputFile;
    std::cout << "\nEnter input file name: ";
    std::cin >> inputFile;
    std::cout << "Enter output file name: ";
    std::cin >> outputFile;
    bool success;
    if (choice == 1) success = compressor.compressFile(inputFile, outputFile);
    else success = compressor.decompressFile(inputFile, outputFile);
    if (success) std::cout << "\nOperation completed successfully!\n";
    else std::cout << "\nOperation failed.\n";
    return success ? 0 : 1;
}
