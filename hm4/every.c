#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define true 1
#define false 0

int main(int argc, char *argv[]) {
	int numOfLines = 1;
	int freq = 1;
	int i;
	int found = false;
	for (i = 1; i < argc; i++) {
		//printf("%s\n", argv[i]);
		char *argument = argv[i];
		int index = 0;
		if (argument[index] == '-' && !found) {
			index++;
			if (argument[index] != ',') {
				freq = 0;
			}
			int *p = &freq;
			//printf("%d", *p);
	                while(argument[index] != '\0') {
				if (argument[index] == ',') {
					numOfLines = 0;
					p = &numOfLines;
					index++;
					continue;
				}
				*p = (*p) * 10 + (int)argument[index] - '0';
				//printf("current = %d\n", *p);
				index++;
			}
			found = true;	
		}
		if (found) break;
	}
	if (!found) {
		char *EVERY = getenv("EVERY");
		if (EVERY == NULL) 
			printf("No $EVERY\n");	
		else {
			//printf("%s\n", EVERY);
			int index = 0;
			//printf("%c\n", EVERY[index]);
			//return 1;
			if (EVERY[index] != ',') freq = 0;
			int *p = &freq;
			//printf("%d\n", *p);
			while (EVERY[index] != '\0') {
				//printf("index %d = %c", index, EVERY[index]);
				if (EVERY[index] == ',') {
					numOfLines = 0;
					p = &numOfLines;
					index++;
					continue;
				}
				*p = (*p) * 10 + (int) EVERY[index] - '0';
				//printf("Index %d: %d\n", index, *p);
				index++;
			}	
		}
	}
	printf("m = %d, n = %d\n", freq, numOfLines);
	for (i = 1; i < argc; i++) {
		if (argv[i][0] != '-') {
			FILE *f = fopen(argv[i], "r");
			if (f == 0) printf("%s: Error: Cannot find file %s", argv[0], argv[i]);
			printf("%d\n", f);
		}
	}
	return 0;
}
