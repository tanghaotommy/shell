//For running single command
int runCommand(struct command);

//For running command with multiple pipes
int runCommands(struct command *, int);


//If given a script, then read the script and execute it
int runScript(char*);
