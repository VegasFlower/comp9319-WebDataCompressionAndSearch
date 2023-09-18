#ifndef COMMON_BWT_H
#define COMMON_BWT_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
// #include <math.h>


occRecord getCharFromC(int* C, int i);

occRecord findIncreasedChar(int** occ, int iteration);

// int getIndexFromPair(int* C, occRecord record);

int getIndexFromPair2(int** occ, occRecord record, int file_size);

char *findRecord(int *c_table, int** occ, int first, int last, int file_size);

void findRecords(int *c_table, int** occ, int first, int last, int file_size);

int bwtSearch(int *c_table, int** occ, char *toBeSearched, int input_size, int file_size);

int normal_bwt_search(FILE *fp, char *SearchStr, int total_size, int* c_table);


#endif