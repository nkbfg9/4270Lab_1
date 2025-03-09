#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

char prog_file[32];
char *prog_tokens[256][4];
int num_lines = 0;
const char *delimeter = " ";
char *temp;

void load_program()
{
    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    size_t read;

    /* Open program file. */
    fp = fopen(prog_file, "r");
    if (fp == NULL)
    {
        printf("Error: Can't open program file %s\n", prog_file);
        exit(-1);
    }

    /* Read in the program. */
    while ((read = getline(&line, &len, fp)) != -1)
    {
        int tk = 0;
        const char comment = '#';
        temp = strtok(line, delimeter);
        while (temp != NULL)
        {
            if (tk > 3 || strncmp(temp, &comment, 1) == 0)
                break;

            prog_tokens[num_lines][tk] = (char *)malloc(sizeof(char) * strlen(temp));
            strcpy(prog_tokens[num_lines][tk], temp);
            tk++;
            temp = strtok(NULL, delimeter);
        }
        num_lines++;
    }

    fclose(fp);
    if (line)
        free(line);
    return;
}

int main(int argc, char *argv[])
{
    printf("\n**************************\n");
    printf("Welcome to MU-RISCV SIM...\n");
    printf("**************************\n\n");

    if (argc < 2)
    {
        printf("Error: You should provide input file.\nUsage: %s <input program> \n\n", argv[0]);
        exit(1);
    }

    strcpy(prog_file, argv[1]);
    load_program();

    for (int i = 0; i < num_lines; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            if (prog_tokens[i][j])
            {
                printf("%s ", prog_tokens[i][j]);
            }
        }
        printf("\n");
    }
}