#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void updateTable(int *C, unsigned char previousByte) {
    int index = (int)previousByte;
    C[index] += 1;
}

int compareRecords(const void *a, const void *b) {
    const struct RecordWithIndex *recordA = (const struct RecordWithIndex *)a;
    const struct RecordWithIndex *recordB = (const struct RecordWithIndex *)b;
    return recordA->index - recordB->index;
}

int getIndexFromPair(int* C, occRecord record){
    // char c = record.ascii;
    int c_int = (int)record.ascii;
    // int nth = record.nth;
    int base = C[c_int];
    // printf("%d\n", base);
    return base + record.nth;
}