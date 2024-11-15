#include "word_list.h"

#define BLOCK_SIZE 1024

char* env(char* str){
	if(strcmp(str, "$HOME") == 0) return getenv("HOME");
	if(strcmp(str, "$SHELL") == 0) return getenv("SHELL");
	if(strcmp(str, "$USER") == 0) return getenv("USER");
	if(strcmp(str, "$EUID") == 0) return getenv("EUID");
	return str;
}

void initArray(Array *a, size_t initialSize) {
  a->array = malloc(initialSize * sizeof(char*));
  a->used = 0;
  a->size = initialSize;
}

void insertArray(Array *a, char* element) { 
  if (a->used == a->size) {
    a->size *= 2;
    a->array = realloc(a->array, a->size * sizeof(char*));
  }
  a->array[a->used++] = element;
}

void freeArray(Array *a) {
  int i;
  for (i = 0; i < a->used; i++){
    free(a->array[i]);
  }
  free(a->array);
  a->array = NULL;
  a->used = a->size = 0;
}

// Function to determine if a character is special
int isSpecialChar(char c) {
    char specialChars[] = {'|', '&', ';', '>', '<', '(', ')', '\0'};
    for (int i = 0; specialChars[i] != '\0'; i++) {
        if (c == specialChars[i]) {
            return 1; // true
        }
    }
    return 0; // false
}

// Function to split a string into an array of words
char **splitString(const char *input, size_t *wordCount) {
    char **words = NULL;
    size_t count = 0;
    size_t length = strlen(input);
    char currentWord[BLOCK_SIZE] = "";

    for (size_t i = 0; i <= length; i++) {
        char currentChar = input[i];

        if (currentChar == ' ' || currentChar == '\t' || currentChar == '\n' || currentChar == '\0' || isSpecialChar(currentChar)) {
            if (strlen(currentWord) > 0) {
                // Add the current word to the array
                words = realloc(words, (count + 1) * sizeof(char *));
                words[count] = malloc(strlen(currentWord) + 1);
                strcpy(words[count], currentWord);
                count++;
                currentWord[0] = '\0';
            }

            if (isSpecialChar(currentChar)) {
                // Add the current special character and the next character (if it exists and is not part of a two-character combination) to the array
                if (isSpecialChar(input[i + 1]) && (currentChar == '&' || currentChar == '|' || currentChar == '>' || currentChar == '<') && currentChar == input[i + 1]) {
                    char specialPair[3] = {currentChar, input[i + 1], '\0'};
                    i++; // Skip the next character
                    words = realloc(words, (count + 1) * sizeof(char *));
                    words[count] = malloc(strlen(specialPair) + 1);
                    strcpy(words[count], specialPair);
                    count++;
                } else {
                    // Add the current special character to the array
                    char specialChar[2] = {currentChar, '\0'};
                    words = realloc(words, (count + 1) * sizeof(char *));
                    words[count] = malloc(strlen(specialChar) + 1);
                    strcpy(words[count], specialChar);
                    count++;
                }
            }
        } else {
            // Add the character to the current word
            strncat(currentWord, &currentChar, 1);
        }
    }

    *wordCount = count;
    return words;
}

// Function to free memory allocated for an array of words
void freeWords(char **words, size_t wordCount) {
    for (size_t i = 0; i < wordCount; i++) {
        free(words[i]);
    }
    free(words);
}

// Function to get the next character from the string
char getsym(FILE* file) { 
    char c;
    c = fgetc(stdin);
    // Return the next character from the buffer
    return c;
}

// Function to sort an array of words
void sortWordList(Array* list) {
    size_t i, j;
    for (i = 0; i < list->used - 1; i++) {
        for (j = 0; j < list->used- i - 1; j++) {
            if (strcmp(list->array[j], list->array[j + 1]) > 0) {
                char* temp = list->array[j];
                list->array[j] = list->array[j + 1];
                list->array[j + 1] = temp;
            }
        }
    }
}

// Function to print an array of words
void printWordList(Array* wordList) {
    int i;
    char* c;
    for (i = 0; i < wordList->used; i++)
    {   
        printf("%s\n", wordList->array[i]);    
    }
}

// Function to read words from a file stream
void readWords(FILE* stream, Array *wordList) {
    char currentWord[BLOCK_SIZE];
    char* tmp;
    char c;
    const char BACKSLASH = '\\';
    const char QUOT = '"';    
    const char HASHTAG = '#';    
    
    int slash = 0, quotation = 0, endQuote = 0;
    size_t curLen = 0; // currentWordLength

    while (1) {
        
        if (!quotation) c = getchar();

        if (c == BACKSLASH){
            slash = 1;
        } else {
            slash = 0;
        }

        if (slash) c = getchar(); // If a backslash is open, read the next character

        if (c == HASHTAG && slash == 0){
            while(1){
                c = getchar();
                if (c == '\0' || c == EOF || c == '\n') break;
            }
        }

        // If a quotation mark is open, read everything as one word
        while (quotation == 1){
            char c = getchar();
            
            if (c == BACKSLASH){ // Backslash inside quotation marks
                slash = 1;
            } else {
                slash = 0;
            }

            if (slash) c = getchar();

            if (c == EOF){
                quotation = 0;
                break;
            }
            if (c == QUOT && slash == 0){
                quotation = 0;
                c = '\0';
                endQuote = 1;
                break;
            } else{
                currentWord[curLen++] = c;
            }
        }

        // If a quotation mark is opened, start reading into quotes
        if((c == QUOT && slash == 0)){
            quotation = 1;
            c = '\0';
        }
        
        if (c == '\0' || c == EOF || c == '\n') {
            if (curLen > 0) {
                currentWord[curLen] = '\0';

                if(!endQuote){
                    size_t wordCount;
                    char **result = splitString(currentWord, &wordCount);
                    for (size_t i = 0; i < wordCount; i++) {

                        if (result[i][0] == '$'){ // Environment variable
                            tmp = strdup(env(result[i]));
                        } else{
                            tmp = strdup(result[i]);
                        }
                        
                        if (tmp[0] != ' '){
                            insertArray(wordList, tmp); // Add the word to the array
                        }                                  
                    }
                    freeWords(result, wordCount);
                
                } else {  //endQuote
                    tmp = strdup(currentWord);
                    insertArray(wordList, tmp);
                    endQuote = 0;
                }

                curLen = 0;
            }
        } else {
            currentWord[curLen++] = c;
        }
        
        if (c == '\n') {
            break;
        }
        if (c == EOF){
            freeArray(wordList);
            printf("\n");
            exit(0);
        }
    }
}
