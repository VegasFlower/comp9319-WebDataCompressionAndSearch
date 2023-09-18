#ifndef INDEX_BWT_H
#define INDEX_BWT_H
#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>



Checkpoint* create_checkpoint(int* occ, int current_file_size, int offset, int rlb_iteration);

void write_checkpoint_to_file(Checkpoint* cp, FILE* fp);


bool checkIteratedBefore(int *saved_iteration, int input, int length);

occRecord getCharFromC_index(int* C, int i);

occRecord find_char_nth_with_iteration(const char *IndexFilePath, int iteration, const char *InputFilePath, int total_size);

int find_iteration_with_char_nth(const char *IndexFilePath, char ascii, int nth, const char* InputFilePath, int file_size);

int extractOccFromCheckpoint(const char* indexFilePath, int iteration, char c, const char* InputFilePath, int bwt_size);

char *find_record_with_index(int *c_table, int first, int file_size, const char *IndexFilePath, const char *rlbFilePath, int *savedIteration, int length);

void find_records_with_index(int *c_table, int first, int last, int file_size, const char *IndexFilePath, const char *rlbFilePath);

int search_with_index(int *c_table, int* C, char *toBeSearched, int search_size, int bwt_size, const char *indexFilePath, const char *inputFilePath);

int check_index_file(int* c_table, int* C, FILE *fp, char *SearchStr, int total_size, long file_size,  const char *IndexFilePath, const char *InputFilePath);

#endif