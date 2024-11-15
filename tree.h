#ifndef TREE_H
#define TREE_H
#include <stdio.h>
#include <unistd.h>
#include <setjmp.h>
#include <string.h>
#include "word_list.h"

extern jmp_buf notree;

typedef enum {NXT, AND, OR} next_type;

struct Tree {
	int argc;
    char **argv;
    char *infile;
    char *outfile;
    int backgrnd;
    next_type type;
    int append;
    struct Tree *psubcmd;
    struct Tree *pipe;
    struct Tree *next;
};

typedef struct Tree *tree;

void add_argv(tree Node, char* str);
tree listToNode(tree node, Array *wordList, int start, int* newstart);
tree parseWordlist(tree Node, Array *wordList, int start, int opened);
tree createNode(tree Node);
void error(char *str);
void freeTree(tree t);
void freeTreeNoSub(tree t);
void print_tree(tree t, int shift);
int checkBrackets(Array* wordList);
int checkwordList(Array *wordList);

#endif