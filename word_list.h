// word_list.h
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#ifndef WORD_LIST_H
#define WORD_LIST_H

typedef struct WordNode {
    char* word;
    struct WordNode* next;
} WordNode;

typedef struct WordList {
    WordNode* head;
} WordList;

typedef struct {
  char* *array;
  size_t used;
  size_t size;
} Array;

WordList* createWordList();
void appendWord(WordList* list, const char* word);
void freeWordList(WordList* list);
void freeArray(Array *a);
void initArray(Array *a, size_t initialSize);
void insertArray(Array *a, char* element);
int isSpecialChar(char c);
char **splitString(const char *input, size_t *wordCount);
void freeWords(char **words, size_t wordCount);
char getsym(FILE* file);
void printHelper(Array* wordList);
void printWordList(Array* wordList);
void readWords(FILE* stream, Array *wordList);
char* env(char* str);

#endif // WORD_LIST_H