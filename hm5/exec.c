#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#ifndef PARSER_H
#define PARSER_H
#include "parser.h"
#endif

#ifndef EXEC_H
#define EXEC_H
#include "exec.h"
#endif

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

	//printf("hasOutputRedirect: %d, hasInputRedirect: %d\n", cmd.hasOutputRedirect, cmd.hasInputRedirect);

	if (cmd.hasInputRedirect)
	{
		FILE *fp = fopen(cmd.inputRedirect, "r");
		if (fp == NULL)
		{
			perror("open file");
			exit(errno);
		}
		dup2(fileno(fp), fileno(stdin)); 
		fclose(fp);
	}

	if (cmd.hasOutputRedirect)
	{
		FILE *fp;
		if (cmd.typeOfOutputRedirect == OUTPUT_WRITE)
		{
			fp = fopen(cmd.outputRedirect, "w");
			if (fp == NULL)
			{
				perror("open file");
				exit(errno);
			}
		} else if (cmd.typeOfOutputRedirect == OUTPUT_APPEND)
		{
			fp = fopen(cmd.outputRedirect, "a");
			if (fp == NULL)
			{
				perror("open file");
				exit(errno);
			}			
		}
		dup2(fileno(fp), fileno(stdout)); 
		fclose(fp);
	}

	if (!cmd.script){
		//printf("Error: No excutable file found!\n");
		exit(0);
	}

	//printf("Running command %s\n", cmd.script);

	char **args = malloc((cmd.nArgs + 2) * sizeof(char*));
	int i;
	args[0] = cmd.script;
	for (i = 0; i < cmd.nArgs; ++i)
	{
		args[i + 1] = cmd.args[i];
	}
	args[i + 1] = NULL;

	if (execvp(cmd.script, args) < 0) {
		perror(cmd.script);
		exit(EXIT_FAILURE);
	}

	exit(errno);
	return 0;
}

//If there is multiple commads, run pipeline
int runCommands(struct command *cmds, int nCmds) {
	if (nCmds < 2)
	{
		//Only one or less command, no need for pipes;

		struct command cmd = cmds[0];
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

		pid_t p = fork();
		if (p == -1)
		{
			perror("fork failed!");
			return EXIT_FAILURE;
		} else if (p != 0) {
			//Parent
			int status;
			if ( waitpid(p, &status, 0) == -1 ) {
		        perror("waitpid failed");
		        return EXIT_FAILURE;
	    	}
			if(WIFEXITED(status)) {
				printf("child exited with = %d\n",WEXITSTATUS(status));
				switch(WEXITSTATUS(status)) {
					case ENOENT:
						//printf("-nsh: %s: command not found\n", cmd.script);
						break;
				}
			}
		} else {
			//Child
			//printf("Receiving arguments: %s\n", cmd.args[0]);
			runCommand(cmd);
			exit(errno);
		}
		return 0;
	}

	//printf("2 or more commands\n");

	int fds[2*(nCmds - 1)];
	for (int i = 0; i < nCmds - 1; ++i)
	{
		//printf("Assgining pipe %d\n", 2 * i);
		if (pipe(fds + 2 * i) < 0)
		{
			perror("pipe1");
			return EXIT_FAILURE;
		}
	}

	//printf("Finished assigning pipe\n");

	int i;
	for (i = 0; i < nCmds; i++) {
		int pid = fork();
		if( pid == 0 ){
			//I am a child
			if (i == 0)
			{
				//The first command only need to pipe stdout
				if (dup2(fds[1], fileno(stdout)) < 0) {
					perror("pipe");
					exit(EXIT_FAILURE);
				}
			} else if (i == nCmds - 1)
			{
				//The last command only need to pipe stdin, the second to the last one
				if (dup2(fds[2 * (nCmds - 1) - 2], fileno(stdin)) < 0) {
					perror("pipe");
					exit(EXIT_FAILURE);					
				}
			} else {
				//Command in between, which use the stdin of the previous pipe, and the stdout of the next pipe
				// command: 0  -  1  -  2  -  3
				// pipe:      0-1   2-3   4-5
				//printf("Redirecting stdin to pipe %d \n", i/2*2);
				if (dup2(fds[i/2*2], fileno(stdin)) < 0) {
					perror("pipe");
					exit(EXIT_FAILURE);					
				}	

				//printf("Redirecting stdout to pipe %d \n", (i/2+1)*2+1);
				if (dup2(fds[(i/2+1)*2+1], fileno(stdout)) < 0) {
					perror("pipe");
					exit(EXIT_FAILURE);
				}		
			}

			// if (i != 0)
			// {
			// 	//For reading the pipe
			// 	printf("ith command: %d:", i);
			// 	printf("Redirecting stdin to pipe %d \n", i/2*2);
			// 	if (dup2(fds[i/2*2], fileno(stdin)) < 0) {
			// 		perror("pipe2");
			// 		exit(EXIT_FAILURE);
			// 	}
			// }
			// if (i != nCmds - 1)
			// {
			// 	printf("ith command: %d:", i);
			// 	printf("Redirecting stdout to pipe %d \n", i/2*2+1);
			// 	//For writing the pipe, the last do not need to 
			// 	if (dup2(fds[i/2*2+1], fileno(stdout)) < 0) {
			// 		perror("pipe3");
			// 		exit(EXIT_FAILURE);
			// 	}
			// }
			int j;
			for(j = 0; j < 2 * (nCmds - 1); j++){
				//printf("Closing pipe %d\n", j);
        		close(fds[j]);
    		}
    		runCommand(cmds[i]);
    		exit(0);
		}
	}

	for(i = 0; i < 2 * (nCmds - 1); i++){
		//printf("Parent Closing pipe %d\n", i);
		close(fds[i]);
	}
	//Wait for all the children finshed processing
	int status;
	while (wait(&status) > 0) {
		if(WIFEXITED(status)) {
			//printf("child exited with = %d\n",WEXITSTATUS(status));
			switch(WEXITSTATUS(status)) {
				case ENOENT:
					//printf("-nsh: command not found\n");
					break;
			}
		}
	}
	return 0;
}

int runScript(char* filename) {
	FILE *fp = fopen(filename, "r");
	if (fp == NULL)
	{
		perror("open file");
		exit(errno);
	}

	size_t len = 0;
	ssize_t read;
	char *line = NULL;

	while ((read = getline(&line, &len, fp)) != -1) {
		if (len > 0 && line[0] == '#')
			continue;
		int nCmds;
		struct command *cmds = parse(line, read, &nCmds);
		printCommands(cmds, nCmds);
		runCommands(cmds, nCmds);
	}

	free(line);
	return 0;
}
