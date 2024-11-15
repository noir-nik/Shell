#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "tree.h"
#include "word_list.h"

#define INPUT "<"
#define OUTPUT ">"
#define APPEND ">>"
#define PIPE "|"
#define BACKGROUND "&"
#define NEXT ";"
#define AAND "&&"
#define	OOR "||"
#define OPEN "("
#define CLOSE ")"

jmp_buf jbuf;
jmp_buf notree;

int root_is_set = 0;
tree Root = NULL;

char specialChars[] = {'|', '&', ';', '>', '<', '(', ')'};
char* specialWords[] = {"&", "|", "&&", "||", ";", ">>", ">", "<", "(", ")"};
char* betweens[] = {"&", "|", "&&", "||", ";"};

void error(char *str){
	printf("\x1B[33mError\x1B[0m: %s", str);
	printf("\n");
	longjmp(notree, 1);
}

void error_tree(char *str, tree Node){
	    freeTree(Root); // Cleanup of the partially constructed tree
	printf("\x1B[33mError\x1B[0m: %s", str);
	printf("\n");
	longjmp(jbuf, 1);
}

void freeTreeNoSub(tree t){
	if (t != NULL){
		//freeTree(t->psubcmd);
		freeTree(t->pipe);
		freeTree(t->next);
		for (int i = 0; i < t->argc; i++){
			free(t->argv[i]);
		}
		free(t->infile);
		free(t->outfile);
		free(t->argv);
		free(t);
	}
}

void freeTree(tree t){
	if (t != NULL){
		freeTree(t->psubcmd);
		freeTree(t->pipe);
		freeTree(t->next);
		for (int i = 0; i < t->argc; i++){
			free(t->argv[i]);
		}
		free(t->infile);
		free(t->outfile);
		
		free(t->argv);
		free(t);
	}
}

tree createNode(tree Node) {
    tree newNode = (tree)malloc(sizeof(struct Tree));
	newNode->argc = 0;
	newNode->argv = NULL;
	newNode->infile=NULL;
	newNode->outfile=NULL;
	newNode->backgrnd = 0;
	newNode->type = 0;
	newNode->append=0;
	newNode->psubcmd = NULL;
	newNode->pipe = NULL;
	newNode->next = NULL;
    return newNode;
}

void add_argv(tree Node, char* str){
	Node->argv = realloc(Node->argv, (Node->argc + 1) * (sizeof(char*)));
	if(str != NULL){
		Node->argv[Node->argc] = strdup(str);
	} else {
		Node->argv[Node->argc] = NULL;
	}
	Node->argc++;
}

int checkBrackets(Array* wordList){
	struct nod {
    struct nod* next;
	};
	typedef struct nod* st;
	st stack = NULL, tmp;
	for (int i = 0; i < wordList->used; i++){
		if (strcmp(wordList->array[i], "(") == 0){
			tmp = (st)malloc(sizeof(struct nod));
			if (tmp == NULL) {
        		printf("Stack malloc error");
				exit(-1);
			}
			tmp->next = stack;
			stack = tmp;
		}
		if (strcmp(wordList->array[i], ")") == 0){
			if (stack == NULL)
				return 7;
			tmp = stack;
			stack = stack->next;
			free(tmp);	
		}
	}
	if (stack == NULL){
		return 0;
	}
	while (stack != NULL){
		st tmp = stack;
		stack = stack->next;
		free(tmp);
	}
	return 9;
}

int isSplitter(char * str){
	int j;
	for(j = 0; j < sizeof(betweens) / sizeof(char*); j++){
		if (strcmp(str, betweens[j]) == 0){
			return 1;
		}
	}
	return 0;
}

int isSpecialWord(char * str){
	int j;
	for(j = 0; j < sizeof(specialWords) / sizeof(char*); j++){
		if (strcmp(str, specialWords[j]) == 0){
			return 1;
		}
	}
	return 0;
}

int checkwordList(Array *wordList){
    int i, j, balance;
	char * files[] = {INPUT, OUTPUT, APPEND}; 

	if ((balance = checkBrackets(wordList))) return balance; // Check brackets balance

	for(j = 1; j < sizeof(betweens) / sizeof(char*); j++){ //Check last symbol
		if (strcmp(wordList->array[wordList->used - 1], betweens[j]) == 0){
			return 1;
		}
	}
	for (i = 0; i < wordList->used; i++){		
		
		if (strcmp(wordList->array[i], OPEN) == 0){ // "("
			if (strcmp(wordList->array[i + 1], CLOSE) == 0) return 7; // "()"						
			if(i != 0){
				if (strcmp(wordList->array[i - 1], OPEN) == 0) continue; // "("
				if (!isSplitter(wordList->array[i - 1])) return 6; // "(" only after splitter
			}						
		}

		if (strcmp(wordList->array[i], CLOSE) == 0){ // ")"	
			if (strcmp(wordList->array[i - 1], BACKGROUND) == 0) continue;
			if (isSplitter(wordList->array[i - 1])) return 7; // ")" after splitter
			if (i != wordList->used - 1){
				if (!isSpecialWord(wordList->array[i + 1])) return 10; // unexpected command after ")"
			}		
		}

		for(j = 0; j < sizeof(files) / sizeof(char*); j++){ // FILEs		
			if (strcmp(wordList->array[i], files[j]) == 0){
				if (i == wordList->used - 1){
					return 3;
				} 
				if (isSpecialWord(wordList->array[i+1])){ // only file name
					return 5;
				}
			}
		}

		for(j = 1; j < sizeof(betweens) / sizeof(char*); j++){ // NEXT (splitter)	
			if (strcmp(wordList->array[i], betweens[j]) == 0){
				if (i == 0) return 2; // Splitter at the beggining
				if (i == wordList->used - 1){ // Splitter at the end
					return 1;
				}
				if (isSplitter(wordList->array[i+1])){ // Two splitters in a row
					return 2;
				}
			}
			
		}
		if (strcmp(wordList->array[i], BACKGROUND) == 0){ // "&"
			if (i == 0) return 2; // Splitter at the beggining
			if (i == wordList->used - 1){ // If "&" at the end of the list - Success
				return 0;
			}
			if (isSplitter(wordList->array[i+1])) { // If splitter after "&" 
				return 2;	
			}
		}
	}
	return 0; // List is correct
}
tree listToNode(tree Node, Array *wordList, int start, int* newstart){ // until |
    int i, j;
	
	if (wordList->used == 0) return NULL; // if empty list of words
	if (start == wordList->used) return NULL; // if end of list
	if (strcmp(wordList->array[start], CLOSE) == 0) return NULL; // if "()" together
	
	Node = createNode(Node);

	if (strcmp(wordList->array[start], OPEN) == 0){ // "("
		Node->psubcmd = listToNode(Node->psubcmd, wordList, start + 1, newstart);
		start = *newstart; // Continue after corresponding ")"
	}

	for (i = start; i < wordList->used; i++){		

		if (strcmp(wordList->array[i], CLOSE) == 0){ // ")"
			*newstart = i + 1;
			add_argv(Node, NULL);
			return Node;
		}

		if (strcmp(wordList->array[i], INPUT) == 0){ // "<"
			Node->infile = strdup(wordList->array[++i]);
		} else
		if (strcmp(wordList->array[i], OUTPUT) == 0){ // ">"
			Node->outfile = strdup(wordList->array[++i]);
		} else
		if (strcmp(wordList->array[i], APPEND) == 0){ // ">>"
			Node->outfile = strdup(wordList->array[++i]);
			Node->append = 1; 				
		} else
		if (strcmp(wordList->array[i], NEXT) == 0){ // ";"
			add_argv(Node, NULL);		
			Node->type = 0;
			Node->next = listToNode(Node->next, wordList, i + 1, newstart);
			return Node;		
		} else
		if (strcmp(wordList->array[i], AAND) == 0){ // "&&"
			add_argv(Node, NULL);
			Node->type = 1;
			Node->next = listToNode(Node->next, wordList, i + 1, newstart);
			return Node;			
		} else
		if (strcmp(wordList->array[i], OOR) == 0){ // "||"
			add_argv(Node, NULL);
			Node->type = 2;
			Node->next = listToNode(Node->next, wordList, i + 1, newstart);
			return Node;
		} else
		if (strcmp(wordList->array[i], PIPE) == 0){ // "|"
			add_argv(Node, NULL);
			Node->pipe = listToNode(Node->pipe, wordList, i + 1, newstart);
			return Node;		
		} else
		if (strcmp(wordList->array[i], BACKGROUND) == 0){ // "&"
			add_argv(Node, NULL);
			Node->backgrnd = 1;
			int separates = 1; 
			if (i == wordList->used - 1){ // If "&" at the end of the entire list
				return Node;
			}
			if (strcmp(wordList->array[i + 1], OPEN) == 0) continue; // "("
			if (strcmp(wordList->array[i + 1], CLOSE) == 0) continue; // ")"

			if (!isSplitter(wordList->array[i + 1])){ // If "&" separates commands
				Node->next = listToNode(Node->next, wordList, i + 1, newstart);
				return Node;
			}						
		} else {
			add_argv(Node, wordList->array[i]);		
		}
	}
	add_argv(Node, NULL);
	return Node;
}