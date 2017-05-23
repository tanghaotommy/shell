#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "parser.h"

int runCommand(struct command cmd) {
	// if (!cmd.script){
	// 	//printf("Error: No excutable file found!\n");
	// 	return 1;
	// }

	// char **args = malloc((cmd.nArgs + 2) * sizeof(char*));
	// int i;
	// args[0] = cmd.script;
	// for (i = 0; i < cmd.nArgs; ++i)
	// {
	// 	args[i + 1] = cmd.args[i];
	// }
	// args[i + 1] = NULL;

	// pid_t p = fork();
	// if (p == -1)
	// {
	// 	perror("fork failed!");
	// 	return EXIT_FAILURE;
	// } else if (p != 0) {
	// 	//Parent
	// 	int status;
	// 	if ( waitpid(p, &status, 0) == -1 ) {
	//         perror("waitpid failed");
	//         return EXIT_FAILURE;
 //    	}
	// 	if(WIFEXITED(status)) {
	// 		printf("child exited with = %d\n",WEXITSTATUS(status));
	// 		switch(WEXITSTATUS(status)) {
	// 			case ENOENT:
	// 				printf("-nsh: %s: command not found\n", cmd.script);
	// 				break;
	// 		}
	// 	}
	// } else {
	// 	//Child
	// 	//printf("Receiving arguments: %s\n", cmd.args[0]);
	// 	execvp(cmd.script, args);
	// 	exit(errno);
	// }
	// return 0;

	if (!cmd.script){
		//printf("Error: No excutable file found!\n");
		return 1;
	}

	char **args = malloc((cmd.nArgs + 2) * sizeof(char*));
	int i;
	args[0] = cmd.script;
	for (i = 0; i < cmd.nArgs; ++i)
	{
		args[i + 1] = cmd.args[i];
	}
	args[i + 1] = NULL;

	execvp(cmd.script, args);
	exit(errno);
	return 0;
}

//If there is multiple commads, run pipeline
int runCommands(struct command *cmds) {
	pid_t p = fork();
	if (p == -1)
	{
		perror("fork failed!");
		return EXIT_FAILURE;
	} else if (p != 0) {
		//Parent
		int status;
		// if ( waitpid(p, &status, 0) == -1 ) {
	 //        perror("waitpid failed");
	 //        return EXIT_FAILURE;
  //   	}
		// if (wait(&status) == -1)
		// {
		// 	perror("wait failed");
		// 	return EXIT_FAILURE;
		// }
		while (wait(&status) > 0) {
			if(WIFEXITED(status)) {
				printf("child exited with = %d\n",WEXITSTATUS(status));
				switch(WEXITSTATUS(status)) {
					case ENOENT:
						printf("-nsh: command not found\n");
						break;
				}
			}
		}
	} else {
		//Child
		//printf("Receiving arguments: %s\n", cmd.args[0]);
		printf("I am child\n");
		int fd[2];
		pipe(fd); // populates both fd[0] and fd[1]
		if( fork() != 0 ) { // Parent
			dup2(fd[1], fileno(stdout));
			close(fd[0]); 
			close(fd[1]); // DON’T FORGET THIS! 
			// perror("I finished!\n");
			// char **args = malloc(2 * sizeof(char *));
			// args[0] = "ls";
			// args[1] = NULL;
			// execvp("ls", args); 
			runCommand(cmds[0]);
			// perror("I finished 2!\n");
			exit(errno);
		} else { // child
			printf("I am the second child!\n");
			dup2( fd[0], fileno(stdin) );
			close( fd[0] ); 
			close( fd[1] );
			// char **args = malloc(2 * sizeof(char *));
			// args[0] = "wc";
			// args[1] = NULL;
			// execvp("wc", args); 

			runCommand(cmds[1]);
			// execl( "/user/bin/wc", "wc", (char *) 0); 
			printf("I am here\n");
			exit(errno);
		}
		//execvp(cmd.script, args);
	}
	return 0;
}

//Read the command
void readCommand(FILE* stdin) {
	while(1) {
		printf("$ ");
		char *buffer = NULL;
		int read;
		unsigned int len;
		read = getline(&buffer, &len, stdin);
		if (-1 == read) {
			printf("\n");
			exit(0);
		}
	    	//printf("No line read...\n");

	    int nCmds;
	    struct command *cmds = parse(buffer, read, &nCmds);

	    //printf("number of command %d\n", nCmds);
	    printCommands(cmds, nCmds);
	    //runCommand(cmds[0]);
	    runCommands(cmds);
		//printf("Size read: %d\n Len: %d\n", read, len);
	    free(buffer);
	}
}

int main(int argc, char **argv) {
	if (argc > 1)
		printf("TO-DO\n");
	else
		readCommand(stdin);
	return 0;
}