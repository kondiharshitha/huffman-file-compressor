#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <array>
#include <queue>
#include <cstdint>

using namespace std;

class Node {
public:
    unsigned char ch;
    uint64_t freq;
    Node* left;
    Node* right;

    Node(unsigned char c, uint64_t f) : ch(c), freq(f), left(nullptr), right(nullptr) {}
    Node(Node* l, Node* r) : ch(0), freq(l->freq + r->freq), left(l), right(r) {}

    bool isLeaf() const {
        return left == nullptr && right == nullptr;
    }
};

struct Compare {
    bool operator()(Node* a, Node* b) {
        if (a->freq != b->freq) return a->freq > b->freq;
        return a->ch > b->ch;
    }
};

void deleteTree(Node* root) {
    if (!root) return;
    deleteTree(root->left);
    deleteTree(root->right);
    delete root;
}

void buildCodes(Node* root, string code, array<string, 256>& codes) {
    if (!root) return;

    if (root->isLeaf()) {
        codes[root->ch] = code.empty() ? "0" : code;
        return;
    }

    buildCodes(root->left, code + "0", codes);
    buildCodes(root->right, code + "1", codes);
}

Node* buildTree(const array<uint64_t, 256>& freq) {
    priority_queue<Node*, vector<Node*>, Compare> heap;

    for (int i = 0; i < 256; i++)
        if (freq[i] > 0)
            heap.push(new Node(static_cast<unsigned char>(i), freq[i]));

    if (heap.empty()) return nullptr;

    while (heap.size() > 1) {
        Node* left = heap.top();
        heap.pop();

        Node* right = heap.top();
        heap.pop();

        heap.push(new Node(left, right));
    }

    return heap.top();
}

bool compressFile(const string& inputFile, const string& outputFile) {
    ifstream input(inputFile, ios::binary);

    if (!input) {
        cerr << "Could not open input file\n";
        return false;
    }

    array<uint64_t, 256> freq{};
    uint64_t originalSize = 0;
    unsigned char byte;

    while (input.read(reinterpret_cast<char*>(&byte), 1)) {
        freq[byte]++;
        originalSize++;
    }

    input.close();

    Node* root = buildTree(freq);

    ofstream output(outputFile, ios::binary);

    if (!output) {
        deleteTree(root);
        cerr << "Could not create output file\n";
        return false;
    }

    output.write("HUF1", 4);
    output.write(reinterpret_cast<char*>(&originalSize), sizeof(originalSize));

    uint16_t uniqueSymbols = 0;

    for (int i = 0; i < 256; i++)
        if (freq[i] > 0)
            uniqueSymbols++;

    output.write(reinterpret_cast<char*>(&uniqueSymbols), sizeof(uniqueSymbols));

    for (int i = 0; i < 256; i++) {
        if (freq[i] > 0) {
            unsigned char symbol = static_cast<unsigned char>(i);
            output.write(reinterpret_cast<char*>(&symbol), 1);
            output.write(reinterpret_cast<char*>(&freq[i]), sizeof(freq[i]));
        }
    }

    if (!root) {
        output.close();
        return true;
    }

    array<string, 256> codes;
    buildCodes(root, "", codes);

    input.open(inputFile, ios::binary);

    unsigned char outputByte = 0;
    int bitCount = 0;

    while (input.read(reinterpret_cast<char*>(&byte), 1)) {
        for (char bit : codes[byte]) {
            outputByte = (outputByte << 1) | (bit == '1');
            bitCount++;

            if (bitCount == 8) {
                output.write(reinterpret_cast<char*>(&outputByte), 1);
                outputByte = 0;
                bitCount = 0;
            }
        }
    }

    if (bitCount > 0) {
        outputByte <<= (8 - bitCount);
        output.write(reinterpret_cast<char*>(&outputByte), 1);
    }

    input.close();
    output.close();
    deleteTree(root);

    return true;
}

bool decompressFile(const string& inputFile, const string& outputFile) {
    ifstream input(inputFile, ios::binary);

    if (!input) {
        cerr << "Could not open compressed file\n";
        return false;
    }

    char magic[4];
    input.read(magic, 4);

    if (string(magic, 4) != "HUF1") {
        cerr << "Invalid Huffman file\n";
        return false;
    }

    uint64_t originalSize;
    uint16_t uniqueSymbols;

    input.read(reinterpret_cast<char*>(&originalSize), sizeof(originalSize));
    input.read(reinterpret_cast<char*>(&uniqueSymbols), sizeof(uniqueSymbols));

    array<uint64_t, 256> freq{};

    for (int i = 0; i < uniqueSymbols; i++) {
        unsigned char symbol;
        uint64_t frequency;

        input.read(reinterpret_cast<char*>(&symbol), 1);
        input.read(reinterpret_cast<char*>(&frequency), sizeof(frequency));

        freq[symbol] = frequency;
    }

    Node* root = buildTree(freq);

    ofstream output(outputFile, ios::binary);

    if (!output) {
        deleteTree(root);
        return false;
    }

    if (originalSize == 0) {
        output.close();
        input.close();
        deleteTree(root);
        return true;
    }

    if (root->isLeaf()) {
        for (uint64_t i = 0; i < originalSize; i++)
            output.write(reinterpret_cast<char*>(&root->ch), 1);

        output.close();
        input.close();
        deleteTree(root);
        return true;
    }

    Node* current = root;
    uint64_t decoded = 0;
    unsigned char byte;

    while (input.read(reinterpret_cast<char*>(&byte), 1) && decoded < originalSize) {
        for (int bit = 7; bit >= 0 && decoded < originalSize; bit--) {
            bool value = (byte >> bit) & 1;
            current = value ? current->right : current->left;

            if (current->isLeaf()) {
                output.write(reinterpret_cast<char*>(&current->ch), 1);
                decoded++;
                current = root;
            }
        }
    }

    output.close();
    input.close();
    deleteTree(root);

    return decoded == originalSize;
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        cout << "Usage: ./huffman compress <input> <output>\n";
        cout << "       ./huffman decompress <input> <output>\n";
        return 1;
    }

    string operation = argv[1];
    string inputFile = argv[2];
    string outputFile = argv[3];

    if (operation == "compress")
        return compressFile(inputFile, outputFile) ? 0 : 1;

    if (operation == "decompress")
        return decompressFile(inputFile, outputFile) ? 0 : 1;

    return 1;
}
