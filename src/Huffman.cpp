#include <iostream>
#include <fstream>
#include <iomanip>
#include <array>
#include <vector>
#include <string>
#include <cstdint>
#include <utility>
class HuffmanNode {
public:
    int symbol;
    uint64_t frequency;
    HuffmanNode* left;
    HuffmanNode* right;
    HuffmanNode(int symbol, uint64_t frequency) : symbol(symbol), frequency(frequency), left(nullptr), right(nullptr) {}
    bool isLeaf() const { return left == nullptr && right == nullptr; }
};
class MinHeap {
private:
    std::vector<HuffmanNode*> heap;
    void heapifyUp(int index) {
        while (index > 0) {
            int parent = (index - 1) / 2;
            if (heap[parent]->frequency <= heap[index]->frequency) break;
            std::swap(heap[parent], heap[index]);
            index = parent;
        }
    }
    void heapifyDown(int index) {
        int n = heap.size();
        while (true) {
            int smallest = index;
            int left = 2 * index + 1;
            int right = 2 * index + 2;
            if (left < n && heap[left]->frequency < heap[smallest]->frequency) smallest = left;
            if (right < n && heap[right]->frequency < heap[smallest]->frequency) smallest = right;
            if (smallest == index) break;
            std::swap(heap[index], heap[smallest]);
            index = smallest;
        }
    }
public:
    void insert(HuffmanNode* node) {
        heap.push_back(node);
        heapifyUp(heap.size() - 1);
    }
    HuffmanNode* extractMin() {
        if (heap.empty()) return nullptr;
        HuffmanNode* minimum = heap[0];
        heap[0] = heap.back();
        heap.pop_back();
        if (!heap.empty()) heapifyDown(0);
        return minimum;
    }
    int size() const { return static_cast<int>(heap.size()); }
    bool empty() const { return heap.empty(); }
};
class HuffmanCoding {
private:
    HuffmanNode* root;
    void deleteTree(HuffmanNode* node) {
        if (node == nullptr) return;
        deleteTree(node->left);
        deleteTree(node->right);
        delete node;
    }
    void buildTree(const std::array<uint64_t, 256>& frequencies) {
       MinHeap minHeap;
        for (int i = 0; i < 256; ++i) {
            if (frequencies[i] > 0) minHeap.insert(new HuffmanNode(i, frequencies[i]));
        }
        if (minHeap.empty()) {
            root = nullptr;
            return;
        }
        while (minHeap.size() > 1) {
            HuffmanNode* left = minHeap.extractMin();
            HuffmanNode* right = minHeap.extractMin();
            HuffmanNode* parent = new HuffmanNode(-1, left->frequency + right->frequency);
            parent->left = left;
            parent->right = right;
            minHeap.insert(parent);
        }
        root = minHeap.extractMin();
    }
    void generateCodes(HuffmanNode* node, const std::string& code, std::array<std::string, 256>& codes) {
        if (node == nullptr) return;
        if (node->isLeaf()) {
            codes[node->symbol] = code.empty() ? "0" : code;
            return;
        }
        generateCodes(node->left, code + "0", codes);
        generateCodes(node->right, code + "1", codes);
    }
public:
    HuffmanCoding() : root(nullptr) {}
    ~HuffmanCoding() { deleteTree(root); }
    bool compressFile(const std::string& inputFile, const std::string& outputFile) {
        std::ifstream input(inputFile, std::ios::binary);
        if (!input) {
            std::cerr << "Error: Could not open input file.\n";
            return false;
        }
        std::array<uint64_t, 256> frequencies{};
        unsigned char byte;
        while (input.read(reinterpret_cast<char*>(&byte), sizeof(byte))) frequencies[byte]++;
        input.close();
        buildTree(frequencies);
        std::array<std::string, 256> codes{};
        if (root != nullptr) generateCodes(root, "", codes);
        std::ofstream output(outputFile, std::ios::binary);
        if (!output) {
            std::cerr << "Error: Could not create output file.\n";
            return false;
        }
        const char magic[4] = {'H', 'U', 'F', '1'};
        output.write(magic, sizeof(magic));
        uint64_t originalSize = 0;
        for (uint64_t frequency : frequencies) originalSize += frequency;
        output.write(reinterpret_cast<const char*>(&originalSize), sizeof(originalSize));
        for (uint64_t frequency : frequencies) output.write(reinterpret_cast<const char*>(&frequency), sizeof(frequency));
        if (root != nullptr) {
            input.open(inputFile, std::ios::binary);
            unsigned char outputByte = 0;
            int bitCount = 0;
            while (input.read(reinterpret_cast<char*>(&byte), sizeof(byte))) {
                const std::string& code = codes[byte];
                for (char bit : code) {
                    outputByte <<= 1;
                    if (bit == '1') outputByte |= 1;
                    bitCount++;
                    if (bitCount == 8) {
                        output.put(static_cast<char>(outputByte));
                        outputByte = 0;
                        bitCount = 0;
                    }
                }
            }
            if (bitCount > 0) {
                outputByte <<= (8 - bitCount);
                output.put(static_cast<char>(outputByte));
            }
            input.close();
        }
        output.close();
        std::ifstream compressedFile(outputFile, std::ios::binary | std::ios::ate);
        if (!compressedFile) {
            std::cerr << "Error: Could not read compressed file size.\n";
            return false;
        }
        uint64_t compressedSize = static_cast<uint64_t>(compressedFile.tellg());
        compressedFile.close();
        double compressionRatio = 0.0;
        double spaceSaved = 0.0;
        if (originalSize > 0) {
            compressionRatio = (static_cast<double>(compressedSize) / static_cast<double>(originalSize)) * 100.0;
            spaceSaved = 100.0 - compressionRatio;
        }
        std::cout << "\n";
        std::cout << "====================================\n";
        std::cout << "       COMPRESSION RESULTS\n";
        std::cout << "====================================\n";
        std::cout << std::fixed << std::setprecision(2);
        std::cout << "Original size      : " << originalSize << " bytes\n";
        std::cout << "Compressed size    : " << compressedSize << " bytes\n";
        std::cout << "Compression ratio  : " << compressionRatio << "%\n";
        std::cout << "Space saved        : " << spaceSaved << "%\n";
        std::cout << "Output file        : " << outputFile << "\n";
        std::cout << "====================================\n";
        return true;
    }
    bool decompressFile(const std::string& inputFile, const std::string& outputFile) {
        std::ifstream input(inputFile, std::ios::binary);
        if (!input) {
            std::cerr << "Error: Could not open compressed file.\n";
            return false;
        }
        char magic[4];
        input.read(magic, sizeof(magic));
        if (magic[0] != 'H' || magic[1] != 'U' || magic[2] != 'F' || magic[3] != '1') {
            std::cerr << "Error: Invalid Huffman file.\n";
            input.close();
            return false;
        }
        uint64_t originalSize;
        input.read(reinterpret_cast<char*>(&originalSize), sizeof(originalSize));
        std::array<uint64_t, 256> frequencies{};
        for (uint64_t& frequency : frequencies) input.read(reinterpret_cast<char*>(&frequency), sizeof(frequency));
        buildTree(frequencies);
        std::ofstream output(outputFile, std::ios::binary);
        if (!output) {
            std::cerr << "Error: Could not create output file.\n";
            input.close();
            return false;
        }
        if (originalSize == 0) {
            output.close();
            input.close();
            std::cout << "Decompression completed successfully.\n";
            return true;
        }
        if (root != nullptr && root->isLeaf()) {
            for (uint64_t i = 0; i < originalSize; ++i) output.put(static_cast<char>(root->symbol));
            output.close();
            input.close();
            std::cout << "Decompression completed successfully.\n";
            return true;
        }
        HuffmanNode* current = root;
        uint64_t decodedBytes = 0;
        unsigned char byte;
        while (decodedBytes < originalSize && input.read(reinterpret_cast<char*>(&byte), sizeof(byte))) {
            for (int bit = 7; bit >= 0 && decodedBytes < originalSize; --bit) {
                bool bitValue = (byte >> bit) & 1;
                current = bitValue ? current->right : current->left;
                if (current->isLeaf()) {
                    output.put(static_cast<char>(current->symbol));
                    decodedBytes++;
                    current = root;
                }
            }
        }
        output.close();
        input.close();
        if (decodedBytes != originalSize) {
            std::cerr << "Error: Decompression failed.\n";
            return false;
        }

        std::cout << "Decompression completed successfully.\n";
        return true;
    }
};
