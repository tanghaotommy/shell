#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#ifndef PARSER_H
#define PARSER_H
#include "parser.h"
#endif

void printCommand(struct command* cmd) {
	if (cmd->hasInputRedirect)
	{
		printf("<'%s' ", cmd->inputRedirect ? cmd->inputRedirect : "");
	}
	if (cmd->script)
		printf("'%s' ", cmd->script);
	int i;
	for (i = 0; i < cmd->nArgs; ++i)
	{
		printf("'%s' ", cmd->args[i]);
	}
	if (cmd->hasOutputRedirect)
	{
		printf("%s'%s' ", (cmd->typeOfOutputRedirect == OUTPUT_WRITE) ? ">" : ">>", 
			cmd->outputRedirect ? cmd->outputRedirect : "");
	}
}

void printCommands(struct command *cmds, int nCmds) {
	int i;
	int count = 0;

	printf("%d: ", nCmds);

	for (i = 0; i < nCmds; ++i)
	{
		printCommand(&cmds[i]);
		if (i != nCmds - 1)
		{
			printf("| ");
		}
	}
	printf("\n");
}

char * seperateIO(char* input, const unsigned int len) {
	//The 1 more is reserved for escape sign
	char *command = malloc((3 * len + 1) * sizeof(char));
	//strcpy(command, input);
	int length = strlen(command);
	int i = 0;
	int indexOfCommand = 0;
	while (input[i] != '\0') {
		switch(input[i]) {
			case '<':
				command[indexOfCommand] = ' ';
				indexOfCommand++;
				command[indexOfCommand] = '<';
				indexOfCommand++;
				command[indexOfCommand] = ' ';
				break;
			case '>':
				if (i + 1 < len && input[i + 1] == '>')
				{
					command[indexOfCommand] = ' ';
					indexOfCommand++;
					command[indexOfCommand] = '>';
					indexOfCommand++;
					command[indexOfCommand] = '>';
					indexOfCommand++;
					command[indexOfCommand] = ' ';
					i++;
				} else {
					command[indexOfCommand] = ' ';
					indexOfCommand++;
					command[indexOfCommand] = '>';
					indexOfCommand++;
					command[indexOfCommand] = ' ';					
				}
				break;
			default:
				command[indexOfCommand] = input[i];
		}
		i++;
		indexOfCommand++;
	}
	// printf("After preprocessing: %s\n", command);
	return command;
}

int tokenize(char* s, int *start, char** tokens) {
	int i;
	*start = 0;
	int nTokens = 0;
	int len = strlen(s);

	while (s[*start] == ' ' && s[*start] != '\0')
		start++;

	// printf("Starting from %d\n", start);

	for (i = *start; i < len; i++) {
		while(s[i] != ' ' && i < len) {
			i++;
		}
		nTokens++;
		while(s[i] == ' ' && i < len) {
			i++;
		}
		//Because in for loop i++, so we need to subtract one in case of missing the start of next token
		i--;
	}

	int indexOfTokens = 0;
	tokens = malloc(nTokens * sizeof(char *));

	for (i = *start; i < len; i++) {
		char *token = malloc((len + 1) * sizeof(char));
		int index = 0;
		while(s[i] != ' ' && i < len) {
			token[index] = s[i];
			index++;
			i++;
		}
		token[index] = NULL;
		// printf("Find token: %s\n", token);
		tokens[indexOfTokens] = token;
		indexOfTokens++;
		while(s[i] == ' ' && i < len) {
			i++;
		}
		//Because in for loop i++, so we need to subtract one in case of missing the start of next token
		i--;
	}
	return nTokens;
}

//Parsing each individual command
struct command parseCommand(char* command, unsigned int len) {
	//Seperate all the I/O redirection
	// printf("String before: %s, size: %d\n", command, strlen(command));
	command = seperateIO(command, len);
	len = strlen(command);
	// printf("String: %s, size: %d\n", command, strlen(command));
	//printf("%s", command);
	//Calculating number of tokens in total
	int i;
	int start = 0;
	int nTokens = 0;

	while (command[start] == ' ' && command[start] != '\0')
		start++;

	// printf("Starting from %d\n", start);

	for (i = start; i < len; i++) {
		while(command[i] != ' ' && i < len) {
			i++;
		}
		nTokens++;
		while(command[i] == ' ' && i < len) {
			i++;
		}
		//Because in for loop i++, so we need to subtract one in case of missing the start of next token
		i--;
	}

	//Allocate enough memeory for all the tokens
	//Tokenize the command
	int indexOfTokens = 0;
	char **tokens = malloc(nTokens * sizeof(char *));
	for (i = start; i < len; i++) {
		char *token = malloc((len + 1) * sizeof(char));
		int index = 0;
		while(command[i] != ' ' && i < len) {
			token[index] = command[i];
			index++;
			i++;
		}
		token[index] = NULL;
		// printf("Find token: %s\n", token);
		tokens[indexOfTokens] = token;
		indexOfTokens++;
		while(command[i] == ' ' && i < len) {
			i++;
		}
		//Because in for loop i++, so we need to subtract one in case of missing the start of next token
		i--;
	}

	//Check tokenization
	// for (int i = 0; i < nTokens; ++i)
	// {
	// 	printf("%d: %s\n", i, tokens[i]);
	// }

	// printf("Number of tokens: %d\n", nTokens);

	//Because there is at least one filename for script
	int nArgs = -1;
	int hasInputRedirect = false;
	int hasOutputRedirect = false;
	char *input = NULL;
	char *output = NULL;
	char **args = NULL;
	char *script = NULL;
	int typeOfOutputRedirect;
	int indexOfScript = 0;

	//Find IO redirection, and number of args
	for (i = 0; i < nTokens; ++i)
	{
		//If there is an IO redirection
		if (strcmp(tokens[i], ">") == 0 || strcmp(tokens[i], ">>") == 0)
		{
			while ((i + 1 < nTokens) && (strcmp(tokens[i + 1], ">") == 0 || strcmp(tokens[i + 1], ">>") == 0))
				i++;
			// printf("Find IO redirection: %s\n", tokens[i]);

			//If there is the redirection file after
			if ((i + 1 < nTokens) && !(strcmp(tokens[i + 1], ">") == 0 || strcmp(tokens[i + 1], ">>") == 0 || strcmp(tokens[i + 1], "<") == 0))
			{
				if (strcmp(tokens[i], ">") == 0)
					typeOfOutputRedirect = OUTPUT_WRITE;
				else
					typeOfOutputRedirect = OUTPUT_APPEND;
				output = malloc(strlen(tokens[i + 1]) + 1);
				strcpy(output, tokens[i + 1]);
				i++;
			} else {
				//Otherwise, just no output file
				if (strcmp(tokens[i], ">") == 0)
					typeOfOutputRedirect = OUTPUT_WRITE;
				else
					typeOfOutputRedirect = OUTPUT_APPEND;
			}
			hasOutputRedirect = true;
			continue;
		} else if (strcmp(tokens[i], "<") == 0 ) {
			//If current is a input redirection
			while ((i + 1 < nTokens) && (strcmp(tokens[i + 1], "<") == 0))
				i++;
			// printf("Current %d\n", i);
			if ((i + 1 < nTokens) && !(strcmp(tokens[i + 1], ">") == 0 || strcmp(tokens[i + 1], ">>") == 0 || strcmp(tokens[i + 1], "<") == 0)) {
				input = malloc(strlen(tokens[i + 1]) + 1);
				strcpy(input, tokens[i + 1]);
				i++;
			}
			hasInputRedirect = true;
			continue;
		} else {
			// printf("Find argument: %s\n", tokens[i]);
			nArgs++;
		}
	}
	// printf("Finished IO redirection check. Number of args: %d, hasOutputRedirect: %d, hasInputRedirect: %d\n", nArgs, hasOutputRedirect, 
	// 	hasInputRedirect);

	//Find the script name
	for (i = 0; i < nTokens; ++i)
	{
		if (strcmp(tokens[i], ">") == 0 || strcmp(tokens[i], "<") == 0 || strcmp(tokens[i], ">>") == 0) {
			while ((i + 1 < nTokens) && (strcmp(tokens[i + 1], ">") == 0 || strcmp(tokens[i + 1], ">>") == 0 || strcmp(tokens[i + 1], "<") == 0))
				i++;
			i++;
			continue;
		}
		else {
			// printf("Find the script!!!! Length: %d", strlen(tokens[i]));
			indexOfScript = i;
			script = malloc(strlen(tokens[i]) + 1);
			strcpy(script, tokens[i]);
			// printf(", script name:%s\n", script);
			break;
		}
	}

	// printf("Find the script name index: %d, filename: %s\n", indexOfScript, script);
	// printf("Number of args: %d\n", nArgs);

	//Add all the arguments
	if (nArgs > 0) {
		// printf("Allocating arguments array, size: %d\n", nArgs);
		args = malloc((nArgs + 1) * sizeof(char *));
	}
	int indexOfArgs = 0;
	for (i = 0; i < nTokens; ++i)
	{
		//If there is an IO redirection
		if (strcmp(tokens[i], ">") == 0 || strcmp(tokens[i], "<") == 0 || strcmp(tokens[i], ">>") == 0)
		{
			while ((i + 1 < nTokens) && (strcmp(tokens[i + 1], ">") == 0 || strcmp(tokens[i + 1], ">>") == 0 || strcmp(tokens[i + 1], "<") == 0))
				i++;
			//If there is the redirection file after, skip the next token
			i++;
			continue;
		} else {
			//And it is not the script name
			if (i != indexOfScript) {
				// printf("Adding argument %s\n", tokens[i]);
				args[indexOfArgs] = tokens[i];
				indexOfArgs++;
			}
		}
	}
	args[indexOfArgs] == NULL;

	// printf("Output redirection: %d\n", hasOutputRedirect);
	// printf("Intput redirection: %d\n", hasInputRedirect);

	struct command cmd;
	cmd.script = script;
	cmd.args = args;
	cmd.hasOutputRedirect = hasOutputRedirect;
	cmd.hasInputRedirect = hasInputRedirect;
	cmd.inputRedirect = input;
	cmd.outputRedirect = output;
	cmd.error = false;
	cmd.nArgs = nArgs;
	cmd.typeOfOutputRedirect = typeOfOutputRedirect;

	//printCommand(&cmd);
	return cmd;
}

struct command * parse(char* input, const unsigned int len, int* nCmds) {
	int nCmd = 0;
	int i = 0;
	//printf("Command length: %d\n", len);
	// for (int i = 0; i < len - 1; ++i)
	// {
	// 	if (input[i] == '|')
	// 	{
	// 		nCmd++;
	// 	}
	// }

	//If there are consecutive |, then there will be no command between ||,
	//which means cmd || cmd will be recognized as only two commands instead of 3.
	char* input_copy = malloc(len);

	//Remove the last \n
	input[len - 1] = '\0';
	strcpy(input_copy, input);
	char *cmd = strtok(input_copy, "|");

	//Find number of sub commands splitted by '|'
    while(cmd) {
    	nCmd++;
        cmd = strtok(NULL, "|");
    }
    // printf("Number of commands: %d\n", nCmd);

    //Declare the commands
    struct command *cmds = malloc(nCmd * sizeof(struct command));
    int indexOfCommand = 0;
    cmd = strtok(input, "|");
    while(cmd) {
        //printf("Command: %s, length: %d\n", cmd, strlen(cmd));
        cmds[indexOfCommand] = parseCommand(cmd, strlen(cmd));
        indexOfCommand++;
        cmd = strtok(NULL, "|");
    }
	
	*nCmds = nCmd;
	//printf("return number of commands: %d\n", *nCmds);
	// cmds[0] = parseCommand(command, len);
	//printCommands(cmds, nCmd);
	return cmds;
	free(input_copy);
}

