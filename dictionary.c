#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdbool.h>
#include "dictionary.h"

#define MAX_DICTIONARY_WORDS 105000 //dict has 104334
#define MAX_WORD_LENGTH 100

char *words[MAX_DICTIONARY_WORDS]; //store words
int dictSize = 0; //track dict the size of dict

//check if word in dictionary
bool isWordInDictionary(const char *word){
    for(int i = 0; i < dictSize; i++){
        if(strcmp(word, words[i]) == 0){
            return true;
        }
    }
    return false;
}

void loadDictionary(){
    printf("Loading Dictionary\n");
    
    //open dict file
    FILE *file = fopen("/usr/share/dict/words", "r");
    if(file == NULL){
        perror("Error opening dictionary file");
        exit(1);
    }

    char currWord[MAX_WORD_LENGTH]; //buffer to hold each word

    //read each line and overwrite previous currWord contents until eof
    while(fgets(currWord, sizeof(currWord), file) != NULL){
        currWord[strcspn(currWord, "\n")] = 0; //remove \n

        words[dictSize] = malloc(strlen(currWord) + 1);
        if(!words[dictSize]){
            perror("Failed to allocate memory for word");
            fclose(file);
            exit(1);
        }
        //copy new word into 
        strcpy(words[dictSize], currWord);
        dictSize++;
    }

    printf("Loaded Dictionary with %d words\n", dictSize);
    fclose(file);
}

