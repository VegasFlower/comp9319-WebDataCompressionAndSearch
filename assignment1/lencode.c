#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// Tong Zheng
// z5142003

#define DICTIONARY_SIZE 32768
#define MAX_BUFFER_SIZE 4096

typedef struct Entry {
    int index;
    char symbol[MAX_BUFFER_SIZE];
    struct Entry* next;
} Entry;

typedef struct {
    Entry* entries[DICTIONARY_SIZE];
} Dictionary;

void initializeDictionary(Dictionary* dictionary) {
    for (int i = 0; i < DICTIONARY_SIZE; i++) {
        dictionary->entries[i] = NULL;
    }
}

unsigned short int convertIndexToBinary(int index) {
    unsigned short int int_result = 1 << 15;  // Set the leftmost bit to '1'
    int_result |= (unsigned short int)index;
    return int_result;
}

unsigned int hash(const char* str) {
    unsigned int hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }
    return hash % DICTIONARY_SIZE;
}

Entry* searchDictionary(Dictionary* dictionary, const char* key) {
    unsigned int index = hash(key);
    Entry* entry = dictionary->entries[index];

    while (entry != NULL) {
        if (strcmp(entry->symbol, key) == 0) {
            return entry;
        }
        entry = entry->next;
    }

    return NULL;
}

void insertDictionary(Dictionary* dictionary, const char* key, int index) {
    unsigned int hashIndex = hash(key);

    Entry* newEntry = (Entry*)malloc(sizeof(Entry));
    newEntry->index = index;
    strcpy(newEntry->symbol, key);
    newEntry->next = NULL;

    if (dictionary->entries[hashIndex] == NULL) {
        dictionary->entries[hashIndex] = newEntry;
    } else {
        Entry* current = dictionary->entries[hashIndex];
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = newEntry;
    }
}

void lencode(const char* inputFile, const char* outputFile) {    
    FILE* input = fopen(inputFile, "r");
    FILE* output = fopen(outputFile, "wb");
    if (input == NULL) {
        printf("Error: cannot open file %s\n", inputFile);
        return;
    }
    if (output == NULL) {
        printf("Error: cannot open file %s\n", outputFile);
        return;
    }

    Dictionary dictionary;
    initializeDictionary(&dictionary);

    int nextIndex = 0; // Starting index for new entries
    char buffer[MAX_BUFFER_SIZE];
    size_t bytesRead;

    int currentSymbol;
    int previousSymbol = fgetc(input);
    char currentSequence[256];
    sprintf(currentSequence, "%c", previousSymbol);

    int last_index = -1;

    while ((currentSymbol = fgetc(input)) != EOF) {
        char nextSequence[256];

        sprintf(nextSequence, "%s%c", currentSequence, currentSymbol);

        Entry* entry = searchDictionary(&dictionary, nextSequence);

        if (entry != NULL) {
            // printf("Found %s at index %d\n", nextSequence, entry->index);
            sprintf(currentSequence, "%s%c", currentSequence, currentSymbol);
            last_index = entry->index;
        } else {
            if (strlen(currentSequence) >= 3) {
                unsigned short int index_address = convertIndexToBinary(last_index);
                unsigned char bytes[2]; // Array to store the two bytes
    
                // Extract the two bytes from the integer
                bytes[0] = (index_address >> 8) & 0xFF; // Most significant byte
                bytes[1] = index_address & 0xFF; // Least significant byte
                fwrite(&bytes[0], sizeof(unsigned char), 1, output);
                fwrite(&bytes[1], sizeof(unsigned char), 1, output);
            } else {
                fwrite(&currentSequence, sizeof(char), strlen(currentSequence), output);
            }

            if (nextIndex < DICTIONARY_SIZE) {
                insertDictionary(&dictionary, nextSequence, nextIndex);
                // printf(">>>>> Saved %s at index %d\n", nextSequence, nextIndex);
                nextIndex++;
            }

            sprintf(currentSequence, "%c", currentSymbol);
            last_index = -1;
        }
    }

    int last_char_index = -1;
    Entry* lastEntry = searchDictionary(&dictionary, currentSequence);
    if (lastEntry != NULL) {
        last_char_index = lastEntry->index;
    }

    if (last_char_index != -1) {
        if (strlen(currentSequence) >= 3) {
            unsigned short int index_address = convertIndexToBinary(last_char_index);
            unsigned char bytes[2]; // Array to store the two bytes
            bytes[0] = (index_address >> 8) & 0xFF; // Most significant byte
            bytes[1] = index_address & 0xFF; // Least significant byte
            fwrite(&bytes[0], sizeof(unsigned char), 1, output);
            fwrite(&bytes[1], sizeof(unsigned char), 1, output);
        } else {
            fwrite(&currentSequence, sizeof(char), strlen(currentSequence), output);
        }
    } else {
        fwrite(&currentSequence, sizeof(char), strlen(currentSequence), output);
    }

    fclose(input);
    fclose(output);
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Usage: %s <input_file> <output_file>\n", argv[0]);
        return 1;
    }

    const char* inputFile = argv[1];
    const char* outputFile = argv[2];
    // printf("Input file: %s\n", inputFile);
    // printf("Output file: %s\n", outputFile);

    lencode(inputFile, outputFile);
    return 0;
}
