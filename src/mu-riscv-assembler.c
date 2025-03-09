#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

char prog_file[32];
char * prog_tokens[1000];
int num_tokens = 0;
const char * delimeter = " ";
char * temp;

void load_program() {
    FILE * fp;
	char * line = NULL;
    size_t len = 0;
    size_t read;

	/* Open program file. */
	fp = fopen(prog_file, "r");
	if (fp == NULL) {
		printf("Error: Can't open program file %s\n", prog_file);
		exit(-1);
	}

	/* Read in the program. */
    while ((read = getline(&line, &len, fp)) != -1) {
        temp = strtok(line, delimeter);
        while(temp != NULL) {
            prog_tokens[num_tokens] = (char *) malloc(sizeof(char) * strlen(temp));
            strcpy(prog_tokens[num_tokens], temp);
            num_tokens++;
            temp = strtok(NULL, delimeter);
        }
    }

    fclose(fp);
    if (line)
        free(line);
    return;
}



int main(int argc, char *argv[]) {
    printf("\n**************************\n");
	printf("Welcome to MU-RISCV SIM...\n");
	printf("**************************\n\n");
	
	if (argc < 2) {
		printf("Error: You should provide input file.\nUsage: %s <input program> \n\n",  argv[0]);
		exit(1);
	}

    strcpy(prog_file, argv[1]);
    load_program(prog_file);

    for(int i = 0; i < num_tokens; i++) {
        printf("%s ", prog_tokens[i]);
    }

}