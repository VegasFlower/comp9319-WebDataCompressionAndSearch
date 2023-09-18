#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Tong Zheng
// z5142003

typedef int bool;
#define true 1
#define false 0

#define DICTIONARY_SIZE 32768
#define MAX_BUFFER_SIZE 4096

// Each entry consist index and symbol, it also points to the next entry in the dict

typedef struct Entry {
    int index;
    char symbol[MAX_BUFFER_SIZE];
    struct Entry* next;
} Entry;

// Dictionary will hold two sets of entries
// One for searching entry using a char, another for searching entry using a index.

typedef struct {
    Entry* entries[DICTIONARY_SIZE];
    Entry* reverseEntries[DICTIONARY_SIZE];
} Dictionary;

//  Initialize the dictionary with two types of entries
void initializeDictionary(Dictionary* dictionary) {
    for (int i = 0; i < DICTIONARY_SIZE; i++) {
        dictionary->entries[i] = NULL;
    }
    for (int i = 0; i < DICTIONARY_SIZE; i++) {
        dictionary->reverseEntries[i] = NULL;
    }
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

    // Directly add new entry to dict for fast accessing

    dictionary->reverseEntries[index] = newEntry;
}

Entry* searchDictionaryByIndex(Dictionary* dictionary, int index) {
    Entry* entry = dictionary->reverseEntries[index];
    if (entry != NULL) {
        return entry;
    }
    return NULL;
}


void ldecode(const char* inputFile, const char* outputFile) {
    FILE* input = fopen(inputFile, "rb");
    FILE* output = fopen(outputFile, "w");
    if (input == NULL) {
        printf("Error: cannot open file %s\n", inputFile);
        return;
    }
    if (output == NULL) {
        printf("Error: cannot open file %s\n", outputFile);
        return;
    }

    // Create and initialize a dictionary
    Dictionary dictionary;
    initializeDictionary(&dictionary);

    int nextIndex = 0;
    int current_index = 0;
    unsigned short temp_symbol = 0;
    unsigned char currentByte;
    char current_symbol[MAX_BUFFER_SIZE];
    char previous_symbol[MAX_BUFFER_SIZE];
    char new_symbol[MAX_BUFFER_SIZE];


    char pc_symbol[MAX_BUFFER_SIZE];

    char input_char[1];
    input_char[0] = fgetc(input);
    input_char[1] = '\0';
    fputs(input_char, output);
    strcpy(previous_symbol, input_char);

    // bool found_in_dict = false;
    bool dict_index = false;

    while (fread(&currentByte, sizeof(char), 1, input) == 1) {
        // case index number
        if (currentByte & 0x80) {
            char nextByte;
            fread(&nextByte, sizeof(char), 1, input);
            // temp_symbol = ((currentByte & 0x7F) << 8) | nextByte;
            unsigned short temp_symbol = (((unsigned char)currentByte & 0x7F) << 8) | (unsigned char)nextByte;
            temp_symbol &= 0x7FFF;
            // obtain symbol(s) from dictionary
            Entry* entry = searchDictionaryByIndex(&dictionary, temp_symbol);
            if (entry != NULL) {
                // dict_index = true;
                strcpy(current_symbol, entry->symbol);
                strcpy(pc_symbol, current_symbol);
                // printf("Handle index %d to find %s in dictionary\n", temp_symbol, pc_symbol);
                // fputs(pc_symbol, output);
                // strcpy(previous_symbol, pc_symbol);
                // continue;
            } else {
                // printf("----->>>>> %d NOT FOUND!!!\n", temp_symbol);

                // Tricky condition, when index presents before added to dictionary

                strcpy(current_symbol, previous_symbol);
                char tmp = previous_symbol[0];
                char first[2] = { tmp, '\0' };
                strcat(current_symbol, &first[0]);
                strcpy(pc_symbol, current_symbol);
                insertDictionary(&dictionary, pc_symbol, nextIndex);
                nextIndex++;
                fputs(pc_symbol, output);
                strcpy(previous_symbol, pc_symbol);
                continue;
                // strcat(current_symbol, previous_symbol);
                // break;
            }
        } else {
            // printf("Handle ASCII character: %c\n", currentByte);
            sprintf(current_symbol, "%c", currentByte);
            strcpy(pc_symbol, current_symbol);

        }

        fputs(pc_symbol, output);

        if (nextIndex < DICTIONARY_SIZE) {
        // strcpy(pc_symbol, previous_symbol);
            
            char tmp = pc_symbol[0];
            char first[2] = { tmp, '\0' };

            strcpy(new_symbol, previous_symbol);
            strcat(new_symbol, &first[0]);

            /// check if new symbol is in dictionary
            Entry* entry = searchDictionary(&dictionary, new_symbol);
            if (entry != NULL) {
                // found_in_dict = true;
                // printf("found %s in dictionary\n", new_symbol);
                
                strcpy(previous_symbol, new_symbol);
                continue;
            } else {
                // printf("not found %s in dictionary\n", new_symbol);
                insertDictionary(&dictionary, new_symbol, nextIndex);
                // printf(">>>>> Saved %s at index %d\n", new_symbol, nextIndex);
                nextIndex++;
            }
            /// end of check
        }
        strcpy(previous_symbol, pc_symbol);
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

    ldecode(inputFile, outputFile);

    return 0;
}
