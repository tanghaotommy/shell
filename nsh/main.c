#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

#ifndef EXEC_H
#define EXEC_H
#include "exec.h"
#endif

#ifndef PARSER_H
#define PARSER_H
#include "parser.h"
#endif

static int LAST_EXIT;

//Ignore Ctrl - C
void ignore(int sigNum) {
	return;
}
//Read the command
void readCommand(FILE* stdin) {
	signal(SIGINT, ignore);
	//LAST_EXIT = 0;

	while(1) {
		printf("? ");
		char *buffer = NULL;
		int read;
		unsigned int len;
		read = getline(&buffer, &len, stdin);
		if (-1 == read) {
			// Crl-D
			printf("\n");
			exit(0);
		}

		if (strcmp(buffer, "exit\n") == 0 ) {
			printf("Exit with: %d\n", LAST_EXIT);
			exit(LAST_EXIT);
		}

	    int nCmds;
	    struct command *cmds = parse(buffer, read, &nCmds);

	    //printCommands(cmds, nCmds);
	    LAST_EXIT = runCommands(cmds, nCmds);
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
