#include <stdio.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
	printf("Hello World!\n");
	int i;
	for (i = 0; i < argc; i++) {
		printf("%s\n", argv[i]);
	}
	printf("Looping!\n");
	while(1) {
		sleep(1);
		printf("Continue...\n");
	}
	return 0;
}
