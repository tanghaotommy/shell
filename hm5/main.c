#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#ifndef EXEC_H
#define EXEC_H
#include "exec.h"
#endif

#ifndef PARSER_H
#define PARSER_H
#include "parser.h"
#endif

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
	    runCommands(cmds, nCmds);
		//printf("Size read: %d\n Len: %d\n", read, len);
		free(cmds);
	    free(buffer);
	}
}

int main(int argc, char **argv) {
	if (argc > 1) {
		runScript(argv[1]);
	}
	else
		readCommand(stdin);
	return 0;
}