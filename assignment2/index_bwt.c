#include "index_bwt.h"
#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>



Checkpoint* create_checkpoint(int* occ, int current_file_size, int offset, int rlb_iteration) {
    Checkpoint* cp = malloc(sizeof(Checkpoint));

    cp->bwt_size = current_file_size;
    cp->rlb_size = rlb_iteration;
    cp->offset_size = offset;

    for (int i = 0; i < 128; i++) {
        cp->occ[i] = occ[i];
    }
    return cp;
}

//write_checkpoint_to_file
void write_checkpoint_to_file(Checkpoint* cp, FILE* fp) {
    fwrite(&cp->bwt_size, sizeof(int), 1, fp);
    fwrite(&cp->offset_size, sizeof(int), 1, fp);
    fwrite(&cp->rlb_size, sizeof(int), 1, fp);
    fwrite(cp->occ, sizeof(int), 128, fp);
}

occRecord getCharFromC_index(int* C, int i) {
    occRecord result;
    int nth = 0;
    for (int j = 0; j < 128; j++) {
        int count = C[j+1] - C[j];
        if(count > 0) {
            if(i > C[j] && i <= C[j+1]){
                nth = i - C[j];
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

occRecord find_char_nth_with_iteration(const char *IndexFilePath, int iteration, const char *InputFilePath, int total_size) {   
    occRecord result;
    FILE *index_fp = fopen(IndexFilePath, "rb");
    Checkpoint *cp = malloc(sizeof(Checkpoint));

    while (fread(cp, sizeof(Checkpoint), 1, index_fp) == 1) {
        if(((cp->bwt_size - cp->offset_size) <= iteration && cp->bwt_size > iteration) || iteration == total_size){
            
            fseek(index_fp, -2*sizeof(Checkpoint), SEEK_CUR);
            // read the checkpoint again, find the occ at the iteration
            fread(cp, sizeof(Checkpoint), 1, index_fp);
            // }

            // create a temp occ table
            int *temp_occ = malloc(sizeof(int) * 128);
            for(int i = 0; i < 128; i++){
                temp_occ[i] = cp->occ[i];
            }

            FILE *rlb_fp = fopen(InputFilePath, "rb");
            fseek(rlb_fp, cp->rlb_size * sizeof(char), SEEK_SET);
            unsigned char previousByte;
            unsigned char currentByte;
            unsigned char nextByte;
            unsigned char *bufferByte = NULL;
            // if(previousByte == '\0'){
                // the previous byte is the last char of the previous checkpoint
                // so just move the rlb file pointer to the one previous byte
                // fetch the char, then reset the rlb file pointer
                // printf(">>>>>>>>>>>>>>>> previousByte is null <<<<<<<<<<<<<<<<\n");
            fseek(rlb_fp, -1*sizeof(char), SEEK_CUR);
            fread(&previousByte, sizeof(char), 1, rlb_fp);
            fseek(rlb_fp, cp->rlb_size * sizeof(char), SEEK_SET);
            // printf("previousByte: %c\n", previousByte);
            // }
            int re_iteration = cp->bwt_size;
            if(re_iteration == iteration){
                result.ascii = previousByte;
                int int_c = (int)previousByte;
                result.nth = temp_occ[int_c];
                // result = temp_occ[int_c];
                free(cp);
                free(temp_occ);
                fclose(rlb_fp);
                fclose(index_fp);
                return result;
            }
            while (fread(&currentByte, sizeof(char), 1, rlb_fp) == 1) {
                // printf("currentByte: %c\n", currentByte);
                if(re_iteration == iteration){
                    result.ascii = previousByte;
                    int int_c = (int)previousByte;
                    result.nth = temp_occ[int_c];
                    // result = temp_occ[int_c];
                    free(cp);
                    free(temp_occ);
                    fclose(rlb_fp);
                    fclose(index_fp);
                    return result;
                }
                // check if the byte's first bit is 1
                if(currentByte & 0x80){
                    char successiveBytes[5];
                    successiveBytes[0] = currentByte;
                    int successiveBytesIndex = 1;

                    while (fread(&nextByte, sizeof(char), 1, rlb_fp) == 1) {
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
                        // update rank array
                        int ascii = (int)previousByte;
                        temp_occ[ascii]++;
                        re_iteration++;
                        if(re_iteration == iteration){
                            // int result;
                            result.ascii = previousByte;
                            int int_c = (int)previousByte;
                            result.nth = temp_occ[int_c];
                            free(cp);
                            free(temp_occ);
                            fclose(rlb_fp);
                            fclose(index_fp);
                            return result;
                        }
                    }
                    // add the bufferByte to outputStr
                    if (bufferByte != NULL){
                        int ascii = (int)*bufferByte;
                        temp_occ[ascii]++;
                        re_iteration++;

                        if(re_iteration == iteration){
                            result.ascii = *bufferByte;
                            int int_c = (int)*bufferByte;
                            result.nth = temp_occ[int_c];

                            free(cp);
                            free(temp_occ);
                            fclose(rlb_fp);
                            fclose(index_fp);
                            return result;
                        }
                    }
                    previousByte = *bufferByte;
                    continue;
                }else{
                    int ascii = (int)currentByte;
                    temp_occ[ascii]++;
                    re_iteration++;
                    if(re_iteration == iteration){
                        // int result;
                        result.ascii = currentByte;
                        int int_c = (int)currentByte;
                        result.nth = temp_occ[int_c];

                        free(cp);
                        free(temp_occ);
                        fclose(rlb_fp);
                        fclose(index_fp);
                        return result;
                    }
                }
                previousByte = currentByte;
            }
        }
    }
}

int find_iteration_with_char_nth(const char *IndexFilePath, char ascii, int nth, const char* InputFilePath, int file_size) {
    FILE* index_fp = fopen(IndexFilePath, "rb");
    Checkpoint* prev_cp = malloc(sizeof(Checkpoint));
    fread(prev_cp, sizeof(Checkpoint), 1, index_fp);
    fseek(index_fp, 0, SEEK_SET);
    Checkpoint* cp = malloc(sizeof(Checkpoint));
    int ascii_int = (int)ascii;
    int re_iteration = 0;

    while(fread(cp, sizeof(Checkpoint), 1, index_fp) == 1){
        if(prev_cp->occ[ascii_int] < nth && cp->occ[ascii_int] >= nth){
            fseek(index_fp, -2 * sizeof(Checkpoint), SEEK_CUR);
            fread(cp, sizeof(Checkpoint), 1, index_fp);
            re_iteration = cp->bwt_size;

            int ascii_nth = cp->occ[ascii_int];
            FILE *rlb_fp = fopen(InputFilePath, "rb");
            fseek(rlb_fp, cp->rlb_size * sizeof(char), SEEK_SET);

            unsigned char previousByte;
            unsigned char currentByte;
            unsigned char nextByte;
            unsigned char *bufferByte = NULL;
            fseek(rlb_fp, -1*sizeof(char), SEEK_CUR);
            fread(&previousByte, sizeof(char), 1, rlb_fp);
            fseek(rlb_fp, cp->rlb_size * sizeof(char), SEEK_SET);
            while (fread(&currentByte, sizeof(char), 1, rlb_fp) == 1) {
                if(ascii_nth == nth){
                    free(cp);
                    free(prev_cp);
                    fclose(rlb_fp);
                    fclose(index_fp);
                    return re_iteration;
                }
                if(currentByte & 0x80){
                    char successiveBytes[5];
                    successiveBytes[0] = currentByte;
                    int successiveBytesIndex = 1;

                    while (fread(&nextByte, sizeof(char), 1, rlb_fp) == 1) {
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
                        // originalNumber = generateIntegerFromBytes(successiveBytes, length);
                        originalNumber = successiveBytes[length - 1] & 0x7F;
                        for (int i = length - 2; i >= 0; i--) {
                            originalNumber = (originalNumber << 7) | (successiveBytes[i] & 0x7F);
                        }
                    }

                    for (int i = 0; i < originalNumber+2; i++) {  
                        if(previousByte == ascii){
                            ascii_nth++;
                        }
                        re_iteration++;
                        if(ascii_nth == nth){
                            free(cp);
                            free(prev_cp);
                            fclose(rlb_fp);
                            fclose(index_fp);
                            return re_iteration;
                        }
                    }
                    if (bufferByte != NULL){
                        if(*bufferByte == ascii){
                            ascii_nth++;
                        }
                        re_iteration++;
                        if(ascii_nth == nth){
                            free(cp);
                            free(prev_cp);
                            fclose(rlb_fp);
                            fclose(index_fp);
                            return re_iteration;
                        }
                    }
                    previousByte = *bufferByte;
                    continue;
                }else{
                    if(currentByte == ascii){
                        ascii_nth++;
                    }
                    re_iteration++;
                    if(ascii_nth == nth){
                        free(cp);
                        free(prev_cp);
                        fclose(rlb_fp);
                        fclose(index_fp);
                        return re_iteration;
                    }
                }
                previousByte = currentByte;
            }
        }
        memcpy(prev_cp, cp, sizeof(Checkpoint));
    }
    free(cp);
    free(prev_cp);
    fclose(index_fp);
    return -1;
}

int extractOccFromCheckpoint(const char* indexFilePath, int iteration, char c, const char* InputFilePath, int bwt_size) {
    FILE* index_fp = fopen(indexFilePath, "rb");
    Checkpoint* cp = malloc(sizeof(Checkpoint));

    int int_c = (int)c;
    int result = 0;

    while(fread(cp, sizeof(Checkpoint), 1, index_fp) == 1){
        if(((cp->bwt_size - cp->offset_size) <= iteration && cp->bwt_size > iteration) || iteration == bwt_size){

            if(iteration == bwt_size){
                // printf("----- Edge case : iteration == bwt_size -----\n");
                fseek(index_fp, -sizeof(Checkpoint), SEEK_END);
                fread(cp, sizeof(Checkpoint), 1, index_fp);
                result = cp->occ[int_c];
                free(cp);
                fclose(index_fp);
                return result;
            }
            if(cp->bwt_size - cp->offset_size == 0){
                // printf("----- Edge case : cp->bwt_size - cp->offset_size == 0 -----\n");
            }

            fseek(index_fp, -2*sizeof(Checkpoint), SEEK_CUR);
            fread(cp, sizeof(Checkpoint), 1, index_fp);
            result = cp->occ[int_c];
            // open the rlb file
            FILE *rlb_fp = fopen(InputFilePath, "rb");
            fseek(rlb_fp, cp->rlb_size * sizeof(char), SEEK_SET);

            // printf("Found the nearest checkpoint: %d\n", cp->bwt_size);
            // printf("The occ of %c at %d is %d\n", c, cp->bwt_size, result);

            // now iterate through the rlb file to find the occ at the iteration
            unsigned char previousByte;
            unsigned char currentByte;
            unsigned char nextByte;
            unsigned char *bufferByte = NULL;
            fseek(rlb_fp, -1*sizeof(char), SEEK_CUR);
            fread(&previousByte, sizeof(char), 1, rlb_fp);
            fseek(rlb_fp, cp->rlb_size * sizeof(char), SEEK_SET);

            int re_iteration = cp->bwt_size;
            if(re_iteration == iteration){
                result = cp->occ[int_c];
                free(cp);
                fclose(rlb_fp);
                fclose(index_fp);
                return result;
            }

            while (fread(&currentByte, sizeof(char), 1, rlb_fp) == 1) {
                if(re_iteration == iteration){
                    result = cp->occ[int_c];
                    free(cp);
                    fclose(rlb_fp);
                    fclose(index_fp);
                    return result;
                }
                // check if the byte's first bit is 1
                if(currentByte & 0x80){
                    char successiveBytes[5];
                    successiveBytes[0] = currentByte;
                    int successiveBytesIndex = 1;

                    while (fread(&nextByte, sizeof(char), 1, rlb_fp) == 1) {
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
                        // originalNumber = generateIntegerFromBytes(successiveBytes, length);
                    }

                    for (int i = 0; i < originalNumber+2; i++) {  
                        if(previousByte == c){
                            result++;
                        }

                        re_iteration++;
                        if(re_iteration == iteration){
                            free(cp);
                            fclose(rlb_fp);
                            fclose(index_fp);
                            // printf("The occ of %c at %d is %d\n", c, iteration, result);
                            return result;
                        }
                    }
                    if (bufferByte != NULL){
                        if(*bufferByte == c){
                            result++;
                        }
                        re_iteration++;
                        if(re_iteration == iteration){
                            free(cp);
                            fclose(rlb_fp);
                            fclose(index_fp);
                            // printf("The occ of %c at %d is %d\n", c, iteration, result);
                            return result;
                        }
                    }
                    previousByte = *bufferByte;
                    continue;
                }else{
                    if(currentByte == c){
                        result++;
                    }
                    re_iteration++;
                    if(re_iteration == iteration){
                        free(cp);
                        fclose(rlb_fp);
                        fclose(index_fp);
                        // printf("The occ of %c at %d is %d\n", c, iteration, result);
                        return result;
                    }
                }
                previousByte = currentByte;
            }
        }
    }

    free(cp);
    fclose(index_fp);
    return -1;
}


bool checkIteratedBefore(int *saved_iteration, int input, int length){
    for(int i = 0; i < length; i++){
        // printf("saved_iteration[%d]: %d\n", i, saved_iteration[i]);
        if(saved_iteration[i] == input){
            return true;
        }
    }
    return false;
}

char *find_record_with_index(int *c_table, int first, int file_size, const char *IndexFilePath, const char *rlbFilePath, int *savedIteration, int length) {
    // initiate head string and tail string
    char *head = malloc(sizeof(char) * 5050);
    char *tail = malloc(sizeof(char) * 5050);
    char *result = malloc(sizeof(char) * 5050);

    int i = 0;
    int j = 0;
    int iteration = first;
    occRecord record = getCharFromC_index(c_table, first);
    head[i] = record.ascii;
    i++;

    //
    while(record.ascii != '['){
        // iteration here is 1 based
        record = find_char_nth_with_iteration(IndexFilePath, iteration, rlbFilePath, file_size);
        iteration = getIndexFromPair(c_table, record);
        // printf("%c %d\n", record.ascii, record.nth);
        // printf("iteration: %d\n", iteration);
        if(checkIteratedBefore(savedIteration, iteration, length)){
            // printf("head iterated: %d\n", iteration);
            return NULL;
        }
        head[i] = record.ascii;
        i++;
    }
    // iteration = first;
    record = getCharFromC_index(c_table, first);
    // iteration = first;
    while(record.ascii != '['){
        char ascii = record.ascii;
        int nth = record.nth;
        iteration = find_iteration_with_char_nth(IndexFilePath, ascii, nth, rlbFilePath, file_size);
        if(checkIteratedBefore(savedIteration, iteration, length)){
        // printf("head iterated: %d\n", iteration);
        return NULL;
        }
        record = getCharFromC_index(c_table, iteration);
        tail[j] = record.ascii;
        j++;   
    }

    for(int k = 0; k < i; k++){
        result[k] = head[i-k-1];
    }
    for(int k = 0; k < j-1; k++){
        result[i+k] = tail[k];
    }
    // Add null terminator
    result[i+j-1] = '\0';
    // printf("result: %s\n", result);
    free(head);
    free(tail);

    return result;

}

void find_records_with_index(int *c_table, int first, int last, int file_size, const char *IndexFilePath, const char *rlbFilePath) {
    struct RecordWithIndex *recordsArray = malloc(sizeof(struct RecordWithIndex) * (last - first + 1));
    // create a array to store index integer of the records
    int length = last - first + 1;
    int *indexArray = malloc(sizeof(int) * length);
    int index = 0;
    int repeat;
    char *record = malloc(sizeof(char) * 5050);
    for(int i = 0; i < length; i++){
        indexArray[i] = -1;
    }
    int m = 0;
    int n = 0;
    // saved iteration
    int *saved_iteration = malloc(sizeof(int) * length);
    for(int i = 0; i < length; i++){
        saved_iteration[i] = -1;
    }
    // printf("---------- extracting record ----------\n");

    for(int i = first; i <= last; i++){
        repeat = 0;
        record = find_record_with_index(c_table, i, file_size, IndexFilePath, rlbFilePath, saved_iteration, length);

        if(record == NULL){
            // printf("-------- Skipped --------\n");
            free(record);
            continue;
        }
        saved_iteration[n] = i+1;
        n++;


        int j = 0;
        while(record[j] != ']'){
            j++;
        }
        char *indexString = malloc(sizeof(char) * j);
        for(int k = 1; k < j; k++){
            indexString[k-1] = record[k];
        }
        index = atoi(indexString);
        // printf("index: %d\n", index);
        char *newline = strchr(record, '\n');
        if (newline != NULL) {
            continue;
        }
        // if found index is already in the array, skip the record
        for(int k = 0; k < length; k++){
            // printf("indexArray[%d]: %d\n", k, indexArray[k]);
            if(indexArray[k] == index){
                repeat = 1;
                // printf("Record repeat: %d\n", index);
                break;
            }
        }
        if(repeat == 0){
            // printf("Record %d\n", index);
            indexArray[m] = index;
            recordsArray[i - first].index = index;
            strncpy(recordsArray[i - first].record, record, MAX_RECORD_LENGTH);
            m++;
            // printf("index array: %d\n", indexArray[i-first]);
        }
        free(indexString);
        // free(record);
    }
    // Sort the recordsArray based on the index
    qsort(recordsArray, length, sizeof(struct RecordWithIndex), compareRecords);

    // Print the sorted records
    for (int i = 0; i < length; i++)
    {
        // only print the record if it is not empty
        if(recordsArray[i].record[0] != '\0'){
            printf("%s\n", recordsArray[i].record);
        }
    }
    free(record);
    free(saved_iteration);
    free(recordsArray);
    free(indexArray);
}

int search_with_index(int *c_table, int* C, char *toBeSearched, int search_size, int bwt_size, 
                                    const char *indexFilePath, const char *inputFilePath) {
    
    int i = search_size - 1;
    char c = toBeSearched[i];
    // 1 based index
    int first = c_table[(int)c] + 1;
    // int last = c_table[(int)(c + 1)];
    int last = first + C[(int)c];


    int occ_temp1;
    int occ_temp2;
    // printf("C_table[%c]: %d     first: %d       last: %d\n", c, c_table[(int)c], first, last);
    // last egde case
    if (last <= 0)
    {
        // printf("----- Edge case: last < 0\n");
        last = bwt_size;
        // return 0;
    }

    while ((first <= last) && (i >= 1))
    {
        c = toBeSearched[i - 1];
        int int_c = (int)c;

        // extract the checkpoint from the index file
        // printf("----- find first %d -----:\n", first);
        occ_temp1 = extractOccFromCheckpoint(indexFilePath, first - 1, c, inputFilePath, bwt_size);
        // printf("----------------------:\n");
        // printf("----- find last %d -----:\n", last);
        occ_temp2 = extractOccFromCheckpoint(indexFilePath, last, c, inputFilePath, bwt_size);
        // printf("----------------------:\n");
        if(occ_temp1 == -1 || occ_temp2 == -1){
            // printf("----- Edge case: occ_temp1 or occ_temp2 < 0, extractOccFromCheckpoint failed\n");
            return 0;
        }
        // head -c 1000 large1.bwt | tail -c 1
        first = c_table[(int)c] + occ_temp1 + 1;
        last = c_table[(int)c] + occ_temp2;
        // printf("C_table[%c]: %d      occ1: %d    occ2: %d    first: %d   last: %d\n", c, c_table[(int)c], occ_temp1, occ_temp2, first, last);
        i = i - 1;
    }
    if (last < first)
    {
        // printf("No results found\n");
        return 0;
    }
    else
    {
        // printf(">>>>> There are %d results\n", last- first + 1);
        // printf("first: %d, last: %d\n", first, last);
        // findRecords(c_table, occ, first, last, file_size);
        find_records_with_index(c_table, first, last, bwt_size, indexFilePath, inputFilePath);
        return (last - first + 1);
    }
}

int check_index_file(int* c_table, int* C, FILE *fp, char *SearchStr, int total_size, long file_size,  const char *IndexFilePath, const char *InputFilePath) {
    FILE* file = fopen(IndexFilePath, "r");
    if (file != NULL) {
        // printf("---------- << Index file EXIST >> ----------\n");
        int length = strlen(SearchStr);
        int new_count = search_with_index(c_table, C, SearchStr, length, total_size, IndexFilePath, InputFilePath);
        // printf("Searching Result Count with Index: %d\n", new_count);
        fclose(file);
        return new_count;
    }
    // printf("---------- << Index file NOT EXIST >> ----------\n");
    FILE *fp_index = fopen(IndexFilePath, "wb");
    // 530
    // int max_cp_size = (int)file_size / 550;
    // int chunk_size = (total_size / max_cp_size);
    int max_cp_size = (int)file_size / 550;
    int chunk_size = (total_size / max_cp_size);
    if(chunk_size < 10000){
        chunk_size = chunk_size * 1.2;
    }
    if(chunk_size > 10000 && chunk_size < 50000){
        chunk_size = chunk_size * 1.1;
    }
    if(total_size  >= 10000000){
        chunk_size = chunk_size * 0.85;
    }
    if(total_size  >= 100000000){
        chunk_size = chunk_size * 0.95;
    }

    unsigned char previousByte;
    unsigned char currentByte;
    unsigned char nextByte;
    unsigned char *bufferByte = NULL;

    int *occ_record = malloc(sizeof(int) * 128);
    for (int i = 0; i < 128; i++) {
        occ_record[i] = 0;
    }
    int iteration = 1;
    int prev_iteration = 1;
    int rlb_iteration = 1;
    int offset = 0;

    Checkpoint *checkpoint = create_checkpoint(occ_record, 0, 0, 0);
    write_checkpoint_to_file(checkpoint, fp_index);

    while (fread(&currentByte, sizeof(char), 1, fp) == 1) {
        rlb_iteration++;
        if(currentByte & 0x80){
            char successiveBytes[5];
            successiveBytes[0] = currentByte;
            int successiveBytesIndex = 1;
            // rlb_iteration++;

            while (fread(&nextByte, sizeof(char), 1, fp) == 1) {
                rlb_iteration++;
                if(!(nextByte & 0x80)){
                    bufferByte = &nextByte;
                    break;
                }else{
                    successiveBytes[successiveBytesIndex] = nextByte;
                    successiveBytesIndex++;
                    // rlb_iteration++;
                }
            }
            int length = successiveBytesIndex;
            unsigned int originalNumber = 0;
            if (length >= 0) {
                originalNumber = successiveBytes[length - 1] & 0x7F;
                for (int i = length - 2; i >= 0; i--) {
                    originalNumber = (originalNumber << 7) | (successiveBytes[i] & 0x7F);
                }
                // originalNumber = generateIntegerFromBytes(successiveBytes, length);
            }
            // if(length >= 3){
            //     printf("Error: length >= 3\n");
            //     for (int i = 0; i < length; i++) {
            //         char byte = successiveBytes[i];
            //         for (int bit = 7; bit >= 0; bit--) {
            //             printf("%d", (byte >> bit) & 0x01);
            //         }
            //         printf(" ");
            //     }
            //     printf("\n");
            //     printf("originalNumber: %d\n", originalNumber);
            // }
            for (int i = 0; i < originalNumber+2; i++) {
                int ascii = (int)previousByte;
                occ_record[ascii]++;
                iteration++ ;
                if(iteration > total_size){
                    offset = iteration - prev_iteration;
                    Checkpoint *checkpoint = create_checkpoint(occ_record, iteration - 1, offset, rlb_iteration - 1);
                    write_checkpoint_to_file(checkpoint, fp_index);
                    prev_iteration = iteration;
                    free(checkpoint);
                }
            }
            // add the bufferByte to outputStr
            if (bufferByte != NULL){
                int ascii = *bufferByte;
                occ_record[ascii]++;
                iteration++ ;
                // rlb_iteration++;
                previousByte = *bufferByte;

                offset = iteration - prev_iteration;
                if (offset >= chunk_size && !(previousByte & 0x80)) {
                    Checkpoint *checkpoint = create_checkpoint(occ_record, iteration - 1, offset, rlb_iteration - 1);
                    write_checkpoint_to_file(checkpoint, fp_index);
                    prev_iteration = iteration;
                    free(checkpoint);
                }
                if(iteration > total_size){
                    offset = iteration - prev_iteration;
                    Checkpoint *checkpoint = create_checkpoint(occ_record, iteration - 1, offset, rlb_iteration - 1);
                    write_checkpoint_to_file(checkpoint, fp_index);
                    prev_iteration = iteration;
                    free(checkpoint);
                }
            }
            // previousByte = *bufferByte;
            continue;
        }else{
            int ascii = (int)currentByte;
            occ_record[ascii]++;
            iteration++;
        }
        previousByte = currentByte;

        if(iteration > total_size){
            offset = iteration - prev_iteration;
            Checkpoint *checkpoint = create_checkpoint(occ_record, iteration - 1, offset, rlb_iteration - 1);
            // printf("-------------------------\n");
            write_checkpoint_to_file(checkpoint, fp_index);
            prev_iteration = iteration;
            free(checkpoint);
        }


        offset = iteration - prev_iteration;
        if (offset >= chunk_size && !(previousByte & 0x80)) {
            Checkpoint *checkpoint = create_checkpoint(occ_record, iteration - 1, offset, rlb_iteration - 1);
            write_checkpoint_to_file(checkpoint, fp_index);
            prev_iteration = iteration;
            free(checkpoint);
        }
    }

    fclose(fp_index);
    
    int length = strlen(SearchStr);
    int new_count = search_with_index(c_table, C, SearchStr, length, total_size, IndexFilePath, InputFilePath);
    // printf("Searching Result Count with Index: %d\n", new_count);

    return new_count;
}