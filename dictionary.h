#ifndef DICTIONARY_H
#define DICTIONARY_H

#include <stdbool.h>

#define MAX_DICTIONARY_WORDS 105000
#define MAX_WORD_LENGTH 100

extern char *dictionaryWords[MAX_DICTIONARY_WORDS];

bool isWordInDictionary(const char *word);

void loadDictionary();

#endif 

