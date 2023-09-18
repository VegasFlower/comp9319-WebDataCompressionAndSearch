# comp9319-Web Data Compression and Search
UNSW 23T2

## Course Schedule
- **Week 1:** Introduction, basic information theory, basic compression
- **Week 2:** More basic compression algorithms
- **Week 3:** Adaptive Huffman; Overview of BWT
- **Week 4:** Pattern matching and regular expression
- **Week 5:** FM index, backward search, compressed BWT
- **Week 7:** Suffix tree, suffix array, the linear time algorithm
- **Week 8:** XML overview; XML compression
- **Week 9:** Graph compression; Distributed Web query processing
- **Week 10:** Optional advanced topics; Course Revision
- 

## Assignment 1: LZW Encoding & Decoding

### Overview
Assignment 1 focuses on LZW (Lempel-Ziv-Welch) encoding and decoding. LZW is a popular data compression algorithm that works well with various types of data.

### Instructions
To compile the LZW encoder and decoder, use the following commands:

```shell
gcc -o lencode lencode.c
gcc -o ldecode ldecode.c
```
Usage:
To encode a file, run the following command:
```shell
./lencode originalFile encodedFile
```
To decode an encoded file, use the following command:
```shell
./ldecode encodedFile decodedFile
```
Make sure to replace originalFile, encodedFile, and decodedFile with the appropriate file names.


## Assignment 2: BWT Backward Search

### Overview

Assignment 2 involves BWT (Burrows-Wheeler Transform) backward search. BWT is a data transformation technique commonly used in data compression and indexing.

### Instructions
To compile the BWT search program, you can use the provided Makefile. Simply run:
```
make
```
Usage:
To perform a BWT backward search, follow these steps:

Run the BWT search program with the following command:
```
./bwtsearch inputFile.rlb indexFile.idx 'searchingString'
```
Replace inputFile.rlb, indexFile.idx, and 'searchingString' with your specific input file, index file, and the string you want to search for.

The program will generate an index file if it doesn't exist.

You can then use this index file to complete the backward searching on the specified string.





