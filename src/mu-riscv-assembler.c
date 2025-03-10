#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

char prog_file[32];
char *prog_tokens[256][4];
int num_lines = 0;
const char *delimeter = " \n,";
char *temp;

//-return -1 for failure, 1 for beq, 2 for bne, 3 for blt, 4 for bge, 5 for bltu, 6 for bgeu
int determine_branch(char *line[4])
{
    if (!line || !line[3])
    {
        return -1;
    }

    if (strncmp("beq", line[0], strlen("beq")) == 0)
        return 1;
    if (strncmp("bne", line[0], strlen("bne")) == 0)
        return 2;
    if (strncmp("blt", line[0], strlen("blt")) == 0)
        return 3;
    if (strncmp("bge", line[0], strlen("bge")) == 0)
        return 4;
    if (strncmp("bltu", line[0], strlen("bltu")) == 0)
        return 5;
    if (strncmp("bgeu", line[0], strlen("bgeu")) == 0)
        return 6;
    return -1;
}

// return -1 for failure, index of line containing label for success
int label_search(const char *label)
{
    if (!label)
        return -1;

    for (int i = 0; i < num_lines; ++i)
    {
        if (strncmp(prog_tokens[i][0], label, strlen(label)) == 0)
        {
            return i;
        }
    }
    return -1;
}

// return 0 upon failure, labels should never have 0 distance, returns label distance in bytes
int label_distance(char *line[4], int line_index)
{
    if (!line)
    {
        return 0;
    }
    int label_index = -1;
    if (strncmp("jal", line[0], strlen("jal")) == 0)
        label_index = 2;
    if (determine_branch(line) > -1)
        label_index = 3;
    if (label_index == 2 || label_index == 3)
    {
        char buffer[50];
        if (!line[2])
            return 0;
        snprintf(buffer, strlen(line[label_index]) + 2, "%s:", line[label_index]);
        int label_line = label_search(buffer);

        return 4 * (label_line - line_index);
    }
    return 0;
}

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
        if (strncmp(line, "\n", 2) == 0) // skip the newlines
            continue;

        int tk = 0;
        temp = strtok(line, delimeter);
        while (temp != NULL)
        {
            if (tk > 3 || strncmp(temp, "#", 1) == 0)
            {
                break;
            }

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
    printf("%d\n", label_distance(prog_tokens[2], 2));
}