#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include "tree.h"
#include "exec.h"
#include "printTree.c"

#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define YEL   "\x1B[33m"
#define BLU   "\x1B[34m"
#define MAG   "\x1B[35m"
#define CYN   "\x1B[36m"
#define WHT   "\x1B[37m"
#define RESET "\x1B[0m"

char * WordListError[] = 
{
	"", // 0
    "expected command but found end of string", // 1
	"expected command but found delimiter", // 2
	"expected file name but found end of string", // 3
	"expected file name but found delimiter", // 4
	"expected file name", // 5
	"syntax error near unexpected token '('", // 6
	"syntax error near unexpected token ')'", // 7
	"syntax error near unexpected token '&'", // 8
	"incorrect number of brackets", // 9
	"unexpected command after ')'", // 10
	""
};

void handInt(){
	printf("\n");
	char buf[1024];
	getcwd(buf, sizeof(buf));
	printf("\x1B[1;32m%s\x1B[0;37m:\x1B[1;31m%s\x1B[0;37m$ ", env("$USER"), buf);
	fflush(stdout);
}

void curDir(){
	char buf[1024];
	getcwd(buf, sizeof(buf));
	printf("%s\n", buf);
}

int main(){
	signal(SIGINT, SIG_IGN);
	
	Array wordList;
	initArray(&wordList, 1);
	int* newst = NULL;
	int errnum, status = 0;
	tree t = NULL;
	char buf[1024];
	
	pid_t exec_pid;

	while (1)
	{	
		freeTree(t);	
		setjmp(notree);
		freeArray(&wordList);
		char buf[1024];
		getcwd(buf, sizeof(buf));
		printf("\x1B[1;32m%s\x1B[0;37m:\x1B[1;31m%s\x1B[0;37m$ ", env("$USER"), buf);
		fflush(stdout);

		initArray(&wordList, 1);
		readWords(stdin, &wordList);
		if (wordList.used == 0) longjmp(notree, 1);

		if(strcmp(wordList.array[0], "exit") == 0){
			freeArray(&wordList);
            exit(0);
		}
		
		if(strcmp(wordList.array[0], "cd") == 0){
			if (wordList.used == 1) chdir(getenv("HOME"));				
			if (wordList.used == 2) chdir(wordList.array[1]);				
			if (wordList.used > 2) printf("\x1B[33mError\x1B[0m: too many arguments");					
			longjmp(notree, 1);
		}

		if (errnum = checkwordList(&wordList)){
			error(WordListError[errnum]);
		}
		
		newst = (int*) malloc(sizeof(int)); // Tree
		t = listToNode(t, &wordList, 0, newst);
		free(newst);
		freeArray(&wordList);

		exec_pid = fork(); // exec
		if (exec_pid < 0) perror("\x1B[33mError\x1B[0m");
		if (!exec_pid){

			signal(SIGINT, SIG_DFL);
			
			intlist fg;

			fg = NULL;
			status = exec_main(t, 0, 1, &fg, t);
			while (fg != NULL){
			
			waitpid((fg)->pid, NULL, 0);
			intlist tmp = (fg)->next;
			free(fg);
			(fg) = tmp;
		}
			freeIntList(fg);
			freeTree(t);
			exit(status);
		}
		waitpid(exec_pid, &status, 0);
	}	
}