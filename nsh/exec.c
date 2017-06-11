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

//Run single command
int runCommand(struct command cmd) {
	//If there is IO redirection
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

	//If no executable given
	if (!cmd.script){
		exit(0);
	}
	
	
	//Pass all the arguments to the executable
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

	exit(1);
}

//If there is multiple commads, run pipeline
int runCommands(struct command *cmds, int nCmds) {
	int LAST_EXIT = 0;
	
	if (nCmds == 0)
		return 0;
	//If there is only one command, then no need to create pipes
	if (nCmds == 1)
	{
		//Only one or less command, no need for pipes;

		struct command cmd = cmds[0];
		
		//If there is a cd command, then set the environment variable of PWD to change the current 
		//directory
		if (cmd.script && strcmp(cmd.script, "cd") == 0 ){
			char* directory;
			if (cmd.nArgs < 1) {
				directory = getenv("HOME");
			} else {
				directory = cmd.args[0];
			}
			int ret = chdir (directory);
			if (ret != 0)
				perror("cd");
			return ret;
		}
		
		pid_t p = fork();
		if (p == -1)
		{
			perror("fork failed!");
			return EXIT_FAILURE;
		} else if (p != 0) {
			//Parent
			int status;
			
			if (cmd.isBackground)
				return 0;
			
			//Wait for the child to finish
			if ( waitpid(p, &status, 0) == -1 ) {
		        	perror("waitpid failed");
		       		return EXIT_FAILURE;
	    		}

	    		//Judge according to different status
			if(WIFEXITED(status)) {
				LAST_EXIT = WEXITSTATUS(status);
				switch(WEXITSTATUS(status)) {
					case ENOENT:
						break;
				}
			}
		} else {
			//Child
			runCommand(cmd);
			exit(errno);
		}
		return LAST_EXIT;
	}


	int fds[2*(nCmds - 1)];
	int i;
	for (i = 0; i < nCmds - 1; ++i)
	{
		if (pipe(fds + 2 * i) < 0)
		{
			perror("pipe1");
			return EXIT_FAILURE;
		}
	}

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
				if (dup2(fds[i/2*2], fileno(stdin)) < 0) {
					perror("pipe");
					exit(EXIT_FAILURE);					
				}	

				if (dup2(fds[(i/2+1)*2+1], fileno(stdout)) < 0) {
					perror("pipe");
					exit(EXIT_FAILURE);
				}		
			}
			//Close all the pipes
			int j;
			for(j = 0; j < 2 * (nCmds - 1); j++){
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
	
	//Judge whether there is background
	
	if (cmds[nCmds - 1].isBackground)
		return 0;
	//Wait for all the children finshed processing
	int status;
	while (wait(&status) > 0) {
		if(WIFEXITED(status)) {
			LAST_EXIT = WIFEXITED(status);
			//printf("child exited with = %d\n",WEXITSTATUS(status));
			switch(WEXITSTATUS(status)) {
				case ENOENT:
					//printf("-nsh: command not found\n");
					break;
			}
		}
	}
	return LAST_EXIT;
}

//Run the script when given the scipt file
int runScript(char* filename) {
	int LAST_EXIT = 0;

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
		//If this line is a comment
		if (len > 0 && line[0] == '#')
			continue;

		//If exit
		if (strcmp(line, "exit\n") == 0 ) {
			printf("Exit with: %d\n", LAST_EXIT);
			exit(LAST_EXIT);
		}

		int nCmds;
		struct command *cmds = parse(line, read, &nCmds);
		//printCommands(cmds, nCmds);
		LAST_EXIT = runCommands(cmds, nCmds);
	}

	free(line);
	exit(LAST_EXIT);
}
