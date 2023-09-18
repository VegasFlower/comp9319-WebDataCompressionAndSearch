#include "common.h"
#include "common_bwt.h"
#include "index_bwt.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
// #include <malloc.h>


int main(int argc, char* argv[]) {
    if (argc != 4) {
        printf("Usage: %s <input_file> <output_file>\n", argv[0]);
        return 1;
    }

    const char* InputFilePath = argv[1];
    const char* IndexFilePath = argv[2];
    char* SearchStr = argv[3];


    // open input file
    FILE *fp = fopen(InputFilePath, "rb");
    if (fp == NULL) {
        printf("Error: cannot open file %s\n", InputFilePath);
        return -1;
    }
    // get rlb file size
    fseek(fp , 0 , SEEK_END);
	long file_size = ftell(fp);
	rewind(fp);

    int total_size = 0;
    char c;

    int C[128];
    for(int i = 0; i < 128; i++){
        C[i] = 0;
    }

    unsigned char previousByte;
    unsigned char currentByte;
    unsigned char nextByte;
    unsigned char *bufferByte = NULL;

    while(fread(&currentByte, sizeof(char), 1, fp) == 1){
        if(currentByte & 0x80){
            char successiveBytes[5];
            successiveBytes[0] = currentByte;
            int successiveBytesIndex = 1;

            while (fread(&nextByte, sizeof(char), 1, fp) == 1) {
                if(!(nextByte & 0x80)){
                    bufferByte = &nextByte;
                    break;
                }else{
                    successiveBytes[successiveBytesIndex] = nextByte;
                    successiveBytesIndex++;
                }
            }
            int length = successiveBytesIndex;
            unsigned int originalNumber = 0;
            if (length >= 0) {
                originalNumber = successiveBytes[length - 1] & 0x7F;
                for (int i = length - 2; i >= 0; i--) {
                    originalNumber = (originalNumber << 7) | (successiveBytes[i] & 0x7F);
                }
            }
            for (int i = 0; i < originalNumber+2; i++) {
                updateTable(C, previousByte);
                total_size++ ;
            }
            if (bufferByte != NULL){
                long currentPos = ftell(fp);
                if(currentPos == file_size){
                    break;
                }
                updateTable(C, *bufferByte);
                total_size++ ;
            }
            previousByte = *bufferByte;
            continue;
        }else{
            updateTable(C, currentByte);
            total_size++;
        }
        previousByte = currentByte;
    }

    int c_table[128]; // assuming C has size 128
    for (int i = 0; i < 128; i++) {
        c_table[i] = 0;
    }

    for (int i = 0; i < 128; i++) {
        for (int j = 0; j < i; j++) {
            c_table[i] += C[j];
        }
    }
    rewind(fp);

    int search_count = 0;

    if (total_size < 10000) {
        // printf("---------- << BWT size < 10000 >> ----------\n");
        search_count = normal_bwt_search(fp, SearchStr, total_size, c_table);
    } else {
        // printf("---------- << BWT size > 100000 >> ----------\n");
        search_count = check_index_file(c_table, C, fp, SearchStr, total_size, file_size, IndexFilePath, InputFilePath);
        struct stat st;
        stat(IndexFilePath, &st);
        // printf("---------- Index file size:         %d\n", st.st_size);
    }
    // printf("---------- Input RLB size:          %ld\n", file_size);
    // printf("---------- Input BWT size:          %ld\n", total_size);


    // printf("---------- Searching Result Count:  %d\n", search_count);

    // struct mallinfo mi = mallinfo();
    // printf("Memory usage: %d\n", mi.uordblks);
    return 0;
}