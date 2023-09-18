#include "common.h"
#include "common_bwt.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


occRecord getCharFromC(int* C, int i) {
    occRecord result;
    int nth = 0;
    for (int j = 0; j < 128; j++) {
        int count = C[j+1] - C[j];
        if(count > 0) {
            if(i >= C[j] && i < C[j+1]){
                nth = i - C[j] + 1;
                result.ascii = (char)j;
                result.nth = nth;
                return result;
            }else{
                continue;
            }
        }else{
            continue;
        }
    }
}

occRecord findIncreasedChar(int** occ, int iteration) {
    occRecord result;

    for (int j = 0; j < 128; j++) {
        if(iteration !=0 ){
            if (occ[iteration][j] - occ[iteration - 1][j] == 1) {
                result.ascii = (char)j;
                result.nth = occ[iteration][j];
                break;
            }
        } else{
            if (occ[iteration][j] == 1) {
                result.ascii = (char)j;
                result.nth = occ[iteration][j];
                break;
            }
        }
    }
    return result;
}

// int getIndexFromPair(int* C, occRecord record) {
//     char c = record.ascii;
//     int c_int = (int)c;
//     int nth = record.nth;
//     int base = C[c_int];
//     // printf("%d\n", base);
//     int result = base + nth;
//     return result;
// }

int getIndexFromPair2(int** occ, occRecord record, int file_size) {
    int c = (int)record.ascii;
    for(int i = 0; i < file_size; i++){
        if(occ[i][c] == record.nth){
            // printf("index: %d\n", i+1);
            return i+1;
        }
    }
}

char *findRecord(int *c_table, int** occ, int first, int last, int file_size){
    char c;
    int nth = 0;
    // initate two empty strings that will be used to concate chars to them
    char *headString = malloc(sizeof(char) * 1000);
    char *tailString = malloc(sizeof(char) * 1000);
    char *resultString = malloc(sizeof(char) * 3000);
    
    int i = 0;
    int j = 0;
    int iteration = first+1;
    occRecord record = getCharFromC(c_table, first);
    headString[i] = record.ascii;
    i++;
    while(record.ascii != '['){
        record = findIncreasedChar(occ, iteration-1);
        iteration = getIndexFromPair(c_table, record);
        headString[i] = record.ascii;
        i++;
    }

    record = getCharFromC(c_table, first);
    iteration = first+1;
    while(record.ascii != '['){
        iteration = getIndexFromPair2(occ, record, file_size);
        record = getCharFromC(c_table, iteration-1);
        tailString[j] = record.ascii;
        j++;
    }

    for(int k = 0; k < i; k++){
        resultString[k] = headString[i-k-1];
    }
    for(int k = 0; k < j-1; k++){
        resultString[i+k] = tailString[k];
    }
    return resultString;
}

void findRecords(int *c_table, int** occ, int first, int last, int file_size){
    struct RecordWithIndex *recordsArray = malloc(sizeof(struct RecordWithIndex) * (last - first + 1));
    // create a array to store index integer of the records
    int *indexArray = malloc(sizeof(int) * (last-first+1));
    int index = 0;
    int repeat;
    char *record = malloc(sizeof(char) * 5050);



    for(int i = first; i <= last; i++){
        repeat = 0;
        record = findRecord(c_table, occ, i, last, file_size);

        int j = 0;
        while(record[j] != ']'){
            j++;
        }
        char *indexString = malloc(sizeof(char) * j);
        for(int k = 1; k < j; k++){
            indexString[k-1] = record[k];
        }
        index = atoi(indexString);
        char *newline = strchr(record, '\n');
        if (newline != NULL) {
            continue;
        }
        // if found index is already in the array, skip the record
        for(int k = 0; k < index; k++){
            if(indexArray[k] == index){
                repeat = 1;
                // printf("Record repeat: %d\n", index);
                break;
            }
        }
        if(repeat == 0){
            indexArray[i-first] = index;
            recordsArray[i - first].index = index;
            strncpy(recordsArray[i - first].record, record, MAX_RECORD_LENGTH);
            recordsArray[i - first].record[MAX_RECORD_LENGTH] = '\0'; // Ensure null-termination
            // add the record to the result string

            // printf("%s\n", record);
        }
        free(indexString);
        free(record);

    }
    // Sort the recordsArray based on the index
    qsort(recordsArray, last - first + 1, sizeof(struct RecordWithIndex), compareRecords);

    // Print the sorted records
    for (int i = 0; i < last - first + 1; i++)
    {
        // only print the record if it is not empty
        if(recordsArray[i].record[0] != '\0'){
            printf("%s\n", recordsArray[i].record);
        }
    }
    free(recordsArray);
    free(indexArray);
}

int bwtSearch(int *c_table, int** occ, char *toBeSearched, int input_size, int file_size){

    // impletementation of bwt algorithm from the lecture.
    int i = input_size - 1;
    char c = toBeSearched[i];
    int first = c_table[(int)c];
    int last = c_table[(int)(c + 1)] - 1;
    if (last < 0)
    {
        last = file_size - 1;
        return 0;
    }

    while ((first <= last) && (i >= 1))
    {
        c = toBeSearched[i - 1];
        int int_c = (int)c;

        if(first <= 1){
            first = 1;
            first = c_table[(int)c] + occ[first-1][int_c] - 1;
            last = c_table[(int)c] + occ[last][int_c];
        }else{
            first = c_table[(int)c] + occ[first-1][int_c];
            last = c_table[(int)c] + occ[last][int_c] - 1;
        }

        i = i - 1;
    }
    if (last < first)
    {
        return 0;
    }
    else
    {
        // printf("first: %d, last: %d\n", first, last);
        findRecords(c_table, occ, first, last, file_size);
        return (last - first + 1);
    }
}

int normal_bwt_search(FILE *fp, char *SearchStr, int total_size, int* c_table){
    unsigned char previousByte;
    unsigned char currentByte;
    unsigned char nextByte;
    unsigned char *bufferByte = NULL;

    // construct data structure that hold file_size * 128 integer in 2D array
    int **occ = (int**)malloc((int)total_size * sizeof(int*) * 128);
    for (int i = 0; i < total_size; i++) {
        occ[i] = (int*)malloc(sizeof(int) * 128);
    }
    // initialize the occ array
    for (int i = 0; i < total_size; i++) {
        for (int j = 0; j < 128; j++) {
            occ[i][j] = 0;
        }
    }


    int iteration = 0;

    while (fread(&currentByte, sizeof(char), 1, fp) == 1) {
        // check if the byte's first bit is 1
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
                int ascii = previousByte;
                occ[iteration][ascii]++;
				iteration ++ ;
            }
            // add the bufferByte to outputStr
            if (bufferByte != NULL){
                int ascii = *bufferByte;
                occ[iteration][ascii]++;
				iteration ++ ;
            }
            previousByte = *bufferByte;
            continue;
        }else{
            int ascii = currentByte;
            occ[iteration][ascii]++;
            iteration ++ ;
        }
        previousByte = currentByte;
    }

    // print occ table
    for (int i = 0; i < total_size; i++) {
        for (int j = 0; j < 128; j++) {
            if(i != 0){
                occ[i][j] =  occ[i][j] + occ[i-1][j];
            }
        }
    }

    int length = strlen(SearchStr);
    int count = bwtSearch(c_table, occ, SearchStr, length, total_size);
    return count;
}