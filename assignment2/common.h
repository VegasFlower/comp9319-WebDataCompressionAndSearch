#ifndef COMMON_H
#define COMMON_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>



#define MAX_RECORD_LENGTH 5000

struct RecordWithIndex {
    int index;
    char record[MAX_RECORD_LENGTH + 50]; // +50
};

typedef struct {
    char ascii;
    int nth;
} occRecord;

typedef struct {
    int bwt_size;       // BWT file size before this checkpoint
    int offset_size;        // Offset size this checkpoint contains
    int rlb_size;       // RLB file size until this checkpoint
    int occ[128];
} Checkpoint;

void updateTable(int *C, unsigned char previousByte);

int compareRecords(const void *a, const void *b);

int getIndexFromPair(int* C, occRecord record);

#endif