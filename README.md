# Huffman File Compressor

A lossless file compression and decompression application implemented using Huffman Coding in C++.

## Features

- Huffman Tree construction
- Custom Min-Heap using priority queue
- Lossless compression
- Lossless decompression
- Binary file handling
- Web interface for uploading and downloading files

## Technologies

- C++
- HTML
- CSS
- JavaScript
- Python HTTP server

## How It Works

The application calculates character frequencies, constructs a Huffman Tree, generates variable-length prefix codes, and stores the encoded data in a compressed binary file.

The web interface allows users to upload a file, select compression or decompression, and download the resulting file.

## Run

Compile the Huffman compressor:

    g++ Huffman.cpp -o huffman

Start the web server:

    python3 server.py

Open the forwarded port 8000 in the browser.

## Project Structure

    Huffman.cpp
    server.py
    index.html
    style.css
    script.js
    sample.txt
    README.md
