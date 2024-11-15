#include "tree.h"
#include <unistd.h>

struct backgrndList {
    pid_t pid;
    struct backgrndList *next;
};
typedef struct backgrndList* intlist;
extern int exit_val;

void freeIntList(intlist fg);
intlist addPid(intlist fg, pid_t pid);
void printList(intlist fg);
int exec_simple_com(tree Node, int infd, int outfd, int next_in, intlist* fg, tree root);
int exec_main(tree Node, int infd, int outfd, intlist* fg, tree root);

