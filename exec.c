#include "exec.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

intlist createlist(intlist fg){
	intlist tmp;
	return tmp;
}

void freeIntList(intlist fg){
	if (fg == NULL) return;
	intlist tmp = fg->next;
	free(fg);
	freeIntList(tmp);
}

intlist addPid(intlist fg, pid_t pid){
	if (fg == NULL){
		intlist tmp;
		tmp = (intlist)malloc(sizeof(struct backgrndList));
		tmp->pid = pid;
		tmp->next = NULL;
		return tmp;
	} else {
		fg->next = addPid(fg->next, pid);
		return fg;
	}
}

int exec_simple_com(tree Node, int infd, int outfd, int next_in, intlist* fg, tree root){
	int status = 0;
	pid_t pid;

	if(Node->argc == 0) return 0; // No args

	pid = fork();

	if (pid < 0){ // fork() error
		perror("\x1B[33mError1\x1B[0m");
		exit(-1);
		return -1;
	}

	if(!pid){ // Son
		if(next_in != 0){ //close next in
			close(next_in);
		}
		if (Node->backgrnd){ // After fork()
			signal(SIGINT, SIG_IGN);
		}
		if (Node->infile != NULL){
			int infile;
			infile = open(Node->infile, O_RDONLY);
			if (infile == -1){
				perror("\x1B[33mError2\x1B[0m");
				freeTree(root);
				exit(-1);
			} else{
				dup2(infile, 0);
				close(infile);
			}	
		} else if(infd != 0){
			dup2(infd, 0);
			close(infd);
		}
		if (Node->outfile != NULL){
			int outfile;
			if (Node->append){
				outfile = open(Node->outfile, O_WRONLY | O_APPEND | O_CREAT, 0777);	
			} else {
				outfile = open(Node->outfile, O_WRONLY | O_CREAT | O_TRUNC, 0777);			
			}
			if (outfile == -1){
				perror("\x1B[33mError3\x1B[0m");
				freeTree(root);
				exit(-1);
			} else{
				dup2(outfile, 1);
				close(outfile);
			}
		} else if(outfd != 1){
			dup2(outfd, 1);
			close(outfd);
		}
		execvp(Node->argv[0], Node->argv);
		perror("\x1B[33mError\x1B[0m");	
		freeTree(root);

		exit(-1); 
	} 

	if (!Node->backgrnd){
		
		if (Node->pipe == NULL){
			waitpid(pid, &status, 0);
		} else {
			*fg = addPid(*fg, pid);
		}
	}
	return status;
}

int exec_main(tree Node, int infd, int outfd, intlist *pfg,tree root){
	int status = 0;
	int fd[2], in, out, next_in = 0;
	tree psubtmp = Node->psubcmd;
	tree nexttmp = Node->next;
	tree pipetmp = Node->pipe;	

	if (Node->infile != NULL){ // file in
		in = open(Node->infile, O_RDONLY);	
		if (in == -1){
			perror("\x1B[33mError4\x1B[0m");
		}
		
	} else {
		in = infd; //infd
	}

	if (Node->outfile != NULL){ // file out
		if (Node->append){
			out = open(Node->outfile, O_WRONLY | O_APPEND | O_CREAT , 0777);	
		} else {
			out = open(Node->outfile, O_WRONLY | O_CREAT | O_TRUNC, 0777);			
		}
		if (out == -1){
			perror("\x1B[33mError5\x1B[0m");
			return -1;
		}
		
	} else {
		out = outfd; //outfd
	}

	if (Node->pipe != NULL){ // PIPE
		
		pipe(fd);
		out = fd[1];
		next_in = fd[0];

	}
	
	if (Node->psubcmd != NULL){
		pid_t psubpid;
		psubpid = fork();
		if (psubpid < 0){ // fork() error
			perror("\x1B[33mError6\x1B[0m");
			return -1;
		}
		if(!psubpid){ // psubcmd (Son)
			status = exec_main(psubtmp, in, out, pfg, root);

			freeTree(root);
			freeIntList(*pfg);

			exit(status);
		}
		
		if (!Node->backgrnd){
		
		if (Node->pipe == NULL){
			waitpid(psubpid, &status, 0);
		} else {
			*pfg = addPid(*pfg, psubpid);
		}
	}

	} else{
		status = exec_simple_com(Node, in, out, next_in, pfg, root);
		if (in != 0) close(in); 
	}

	if(Node->next != NULL){
		if (Node->type == NXT){
			status = exec_main(nexttmp, infd, outfd, pfg, root);
		}
		if (Node->type == AND){
			if(WIFEXITED(status)){
				if(WEXITSTATUS(status) == 0){
					status = exec_main(nexttmp, infd, outfd, pfg, root);
				}
			}
		}
		if (Node->type == OR){
			if(WIFSIGNALED(status)){
				status = exec_main(nexttmp, infd, outfd, pfg, root);
			} else if(WIFEXITED(status)){
				if(WEXITSTATUS(status) != 0){
					status = exec_main(nexttmp, infd, outfd, pfg, root);
				}
			}
		}
	}

	if (out != 1) close(out); // close out
	if(Node->pipe != NULL){
		if (in != 0) close(in); // close in
		status = exec_main(Node->pipe, next_in, outfd, pfg, root);
	}
	return status;
}