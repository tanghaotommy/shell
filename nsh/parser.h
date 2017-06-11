#define true 1
#define false 0

//Define error message
#define NOSCRIPT 100
#define NOFILEORDIR 101

//Define the type of output redirection
#define OUTPUT_WRITE 1000
#define OUTPUT_APPEND 1001


//Define the command data type which stores the information of one individual command
struct command
{
	int error;
	char *script;
	char **args;
	int nArgs;
	char* inputRedirect;
	char* outputRedirect;
	int typeOfOutputRedirect;
	int hasInputRedirect;
	int hasOutputRedirect;
	int isBackground;
};

//Print the information of one command
void printCommand(struct command*);

//Print the information of all the command
void printCommands(struct command*, int);

//Add space before and after meta operands, like >, <, >>
char * seperateIO(char*, const unsigned int);

//Parse one command
struct command parseCommand(char*, unsigned int);

//Given the command line, split them into each command by '|'
struct command * parse(char*, const unsigned int, int*);

