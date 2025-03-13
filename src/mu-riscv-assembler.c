#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <math.h>

#define MAX_INSTR 256
#define MAX_TOKENS 4
char prog_file[32];
char *prog_tokens[MAX_INSTR][MAX_TOKENS];
uint32_t prog_instr[MAX_INSTR];
int num_lines = 0;
const char *delimeter = " \n,";
char *temp;

// covert string to lower case------------------------------------------------------------------------------------------------------------------------------
void toLowerCase(char *str)
{
    for (int i = 0; str[i]; i++)
    {
        if (str[i] >= 'A' && str[i] <= 'Z')
        {
            str[i] += 'a' - 'A';
        }
    }
}

// takes token and returns an integer[must be null terminating]--------------------------------------------------------------------------------------------
uint32_t char_to_int(const char *token)
{

    uint32_t length = strlen(token);
    uint32_t blah = 0;
    for (uint32_t i = 0; i < length; i++)
    {
        blah += (token[i] - '0') * (pow(10, length - (i + 1))); // 10 ^ length-i for tens positions
    }
    // printf("token: %s\ninteger: %d\n",token,blah);
    return blah;
}
// helllo how are you, i changed one line

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
int label_search( char *label)
{
    if (!label)
        return -1;

    toLowerCase(label);

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
int32_t label_distance(char *line[4], int line_index)
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
        toLowerCase(buffer);
        int label_line = label_search(buffer);
        if (label_line == -1)
            return 0;

        int value = 0;
        if (label_line > line_index)
        {
            for (int i = line_index; i < label_line; ++i)
            {
                if (!(prog_tokens[i][0] && !prog_tokens[i][1] && !prog_tokens[i][1] && !prog_tokens[i][2]))
                    value += 4;
            }
        }
        else if (line_index > label_line)
        {
            for (int i = line_index - 1; i >= label_line; --i)
            {
                if (!(prog_tokens[i][0] && !prog_tokens[i][1] && !prog_tokens[i][1] && !prog_tokens[i][2]))
                    value -= 4;
            }
        }

        return value;
    }
    return 0;
}

// returns uint32_t with binary for R-instructiongiven by tokens EXCLUDING OPCODE-------------------------------------------------------------------------------------------
uint32_t handle_r_type(char *tokens[])
{
    uint32_t value = 0;
    uint32_t registers = 0;
    // rs2
    if (strcmp(tokens[1], "zero") != 0)
    {
        registers = char_to_int(tokens[1] + sizeof(char));
        value += registers << 7;
    }

    // rs1
    if (strcmp(tokens[2], "zero") != 0)
    {
        registers = char_to_int(tokens[2] + sizeof(char));
        value += registers << 15;
    }
    // rd
    registers = char_to_int(tokens[3] + sizeof(char));
    value += registers << 20;
    // funct7
    // only sub and sra != 0x00
    if (!(strcmp("sub", tokens[0])) || !(strcmp("sra", tokens[0])))
    {
        registers = 32;
        value += registers << 25;
    }

    // funct3 i hate elifs
    if (!(strcmp("sub", tokens[0])) || !(strcmp("add", tokens[0])))
    {
        registers = 0;
    }
    else if (!(strcmp("srl", tokens[0])) || !(strcmp("sra", tokens[0])))
    {
        registers = 5;
    }
    else if (!(strcmp("xor", tokens[0])))
    {
        registers = 4;
    }
    else if (!(strcmp("or", tokens[0])))
    {
        registers = 6;
    }
    else if (!(strcmp("and", tokens[0])))
    {
        registers = 7;
    }
    else if (!(strcmp("sll", tokens[0])))
    {
        registers = 1;
    }
    else if (!(strcmp("slt", tokens[0])))
    {
        registers = 2;
    }
    else if (!(strcmp("sltu", tokens[0])))
    {
        registers = 3;
    }
    value += registers << 12;

    return value;
}

uint32_t handle_other_i_type(char *tokens[])
{
    uint32_t value = 0;
    uint32_t registers = 0;

    // rd
    if (strcmp(tokens[1], "zero") != 0)
    {
        registers = char_to_int(tokens[1] + sizeof(char));
        value += registers << 7;
    }

    // separating rs1 and immediate to get null terminated tokens
    char *offset_and_register = malloc(sizeof(tokens[2]));
    strcpy(offset_and_register, tokens[2]);
    char *offset = NULL;
    char *reg = NULL;

    offset = strtok(offset_and_register, "()"); // immediate
    reg = strtok(NULL, "())");                  // rs1

    // rs1
    if (strcmp(reg, "zero") != 0)
    {
        registers = char_to_int(reg + sizeof(char));
        value += registers << 15;
    }

    // immediate
    registers = char_to_int(offset);

    value += (registers << 20); // only 12 bits so dont need to worry about overlap

    // no mem leak
    free(offset_and_register);
    offset = NULL;
    reg = NULL;
    offset_and_register = NULL;

    // func3
    if (strcmp(tokens[0], "lh") == 0)
    {
        value += 0x1000;
    }
    else if (strcmp(tokens[0], "lb") == 0)
    {
    }
    else if (strcmp(tokens[0], "lw") == 0)
    {
        value += 0x2000;
    }

    return value;
}
// returns uint32_t with binary for I-instructiongiven by tokens EXCLUDING OPCODE-------------------------------------------------------------------------------------------
uint32_t handle_i_type(char *tokens[])
{
    uint32_t value = 0;
    char * name = tokens[0];
    char * rd = tokens[1];
    char * rs1 = tokens[2];
    char * imm = tokens[3];
    if(strcmp(name, "addi")==0) {


    }
    else if (strcmp(name, "xori") == 0)
    {
        value += 4 << 12;
    }
    else if (strcmp(name, "ori") == 0)
    {
        value += 6 << 12;
    }
    else if (strcmp(name, "andi") == 0)
    {
        value += 7 << 12;
    }
    else if (strcmp(name, "slli") == 0)
    {
        value += 1 << 12;
    }
    else if (strcmp(name, "srli") == 0)
    {
        value += 5 << 12;
    }
    else if (strcmp(name, "srai") == 0)
    {
        value += 5 << 12;
        value += 32 << 20;
    }
    else if (strcmp(name, "slti") == 0)
    {
        value += 2 << 12;
    }
    else if (strcmp(name, "sltiu") == 0)
    {
        value += 3 << 12;
    }

    if (strcmp(rd, "zero") != 0)
    {
        value += (char_to_int(rd + sizeof(char)) << 7);
    }
    if (strcmp(rs1, "zero") != 0)
    {
        value += char_to_int(rs1 + sizeof(char)) << 15;
    }

    value += char_to_int(imm) << 20;
    return value;
}

// returns uint32_t with binary for S-instructiongiven by tokens EXCLUDING OPCODE-------------------------------------------------------------------------------------------
uint32_t handle_s_type(char *tokens[])
{
    uint32_t value = 0;
    uint32_t registers = 0;
    uint32_t immediate_mask = 0b11111110000000000000111110000000;

    // rs2
    if (strcmp(tokens[1], "zero") != 0)
    {
        registers = char_to_int(tokens[1] + sizeof(char));
        value += registers << 20;
    }

    // separating rs1 and immediate to get null terminated tokens
    char *offset_and_register = malloc(sizeof(tokens[2]));
    strcpy(offset_and_register, tokens[2]);
    char *offset = NULL;
    char *reg = NULL;

    offset = strtok(offset_and_register, "()"); // immediate
    reg = strtok(NULL, "())");                  // rs1

    // rs1
    if (strcmp(reg, "zero") != 0)
    {
        registers = char_to_int(reg + sizeof(char));
        value += registers << 15;
    }

    // immediate
    registers = char_to_int(offset);

    value += (immediate_mask & (registers << 7));  // only 12 bits so dont need to worry about overlap
    value += (immediate_mask & (registers << 20)); // shift 20 to get last 7 bits

    // no mem leak
    free(offset_and_register);
    offset = NULL;
    reg = NULL;
    offset_and_register = NULL;

    // funct3 code
    // sw < sh < sb in strcmp
    int funct3 = strcmp("sh", tokens[0]);
    if (funct3 == 0)
    { // sh
        registers = 1;
    }
    else if (funct3 < 0)
    { // sw
        registers = 2;
    }
    else
    { // sb greater than 0
        registers = 0;
    }

    value += registers << 12;

    return value;
}

// returns uint32_t with binary for B-instructiongiven by tokens EXCLUDING OPCODE-------------------------------------------------------------------------------------------
uint32_t handle_b_type(char *tokens[], int i)
{
    uint32_t value = 0;
    uint32_t registers = 0;
    // rs1
    if (strcmp(tokens[1], "zero") != 0)
    {
        registers = char_to_int(tokens[1] + sizeof(char));
        value += registers << 15;
    }

    // rs2
    if (strcmp(tokens[2], "zero") != 0)
    {
        registers = char_to_int(tokens[2] + sizeof(char));
        value += registers << 20;
    }

    // funct3

    if (strncmp("beq", tokens[0], strlen("beq")) == 0)
        registers = 0;
    else if (strncmp("bne", tokens[0], strlen("bne")) == 0)
        registers = 1;
    else if (strncmp("blt", tokens[0], strlen("blt")) == 0)
        registers = 4;
    else if (strncmp("bge", tokens[0], strlen("bge")) == 0)
        registers = 5;
    else if (strncmp("bltu", tokens[0], strlen("bltu")) == 0)
        registers = 6;
    else
        registers = 7;
    value += registers << 12;

    // offset
    int32_t label_offset = label_distance(tokens, i);
    uint32_t *unsigned_offset = (uint32_t *)&label_offset;
    // cuts out middle 1's
    // imm[12|1-5]
    uint32_t mask = 0b10000000000000000000000000000000;
    value += mask & (*unsigned_offset); // sign bit good
    mask = 0b01111110000000000000000000000000;
    value += mask & (*unsigned_offset << 20);

    // imm[4-1|11]
    mask = 0b00000000000000000000111100000000;
    value += mask & (*unsigned_offset << 7); //
    mask = 0b00000000000000000000000010000000;
    value += mask & (*unsigned_offset >> 4);

    return value;
}

uint32_t handle_j_type(char *tokens[], int i)
{
    uint32_t value = 0;
    uint32_t registers = 0;

    // rd
    registers = char_to_int(tokens[1] + sizeof(char));
    value += registers << 20;

    // offset
    int32_t label_offset = label_distance(tokens, i);
    uint32_t *unsigned_offset = (uint32_t *)&label_offset;

    // immediate
    uint32_t mask = 0b10000000000000000000000000000000;
    value += mask & (*unsigned_offset); // sign bit good
    // 11 bit
    mask = mask >> 11;
    value += mask & (*unsigned_offset << 10);
    // 10-1
    mask = 0b01111111111000000000000000000000;
    value += mask & (*unsigned_offset << 20);
    // 19-12
    mask = 0b00000000000011111111000000000000;
    value += mask & (*unsigned_offset);

    return value;
}
// return uint23_t containing binary for instruction given by tokens---------------------------------------------------------------------------------------
uint32_t getOpcode(char *tokens[], int i)
{
    uint32_t value = 0;
    char *name = tokens[0];
    toLowerCase(name);
    if (strcmp(name, "add") == 0 || strcmp(name, "sub") == 0 || strcmp(name, "xor") == 0 || strcmp(name, "or") == 0 || strcmp(name, "and") == 0 || strcmp(name, "sll") == 0 || strcmp(name, "srl") == 0 || strcmp(name, "sll") == 0 || strcmp(name, "sra") == 0 || strcmp(name, "slt") == 0 || strcmp(name, "sltu") == 0)
    {
        value += 0b0110011;
        return value + handle_r_type(tokens);
    }
    else if (strcmp(name, "addi") == 0 || strcmp(name, "xori") == 0 || strcmp(name, "ori") == 0 || strcmp(name, "andi") == 0 || strcmp(name, "slli") == 0 || strcmp(name, "srli") == 0 || strcmp(name, "srai") == 0 || strcmp(name, "slti") == 0 || strcmp(name, "sltiu") == 0)
    {
        value += 0b0010011;
        return value + handle_i_type(tokens);
    }
    else if (strcmp(name, "lb") == 0 || strcmp(name, "lh") == 0 || strcmp(name, "lw") == 0 || strcmp(name, "lbu") == 0 || strcmp(name, "lhu") == 0)
    {
        value += 0b0000011;
        return value + handle_other_i_type(tokens);
    }
    else if (strcmp(name, "sb") == 0 || strcmp(name, "sh") == 0 || strcmp(name, "sw") == 0)
    {
        value += 0b0100011;
        return value + handle_s_type(tokens);
    }
    else if (strcmp(name, "beq") == 0 || strcmp(name, "bne") == 0 || strcmp(name, "blt") == 0 || strcmp(name, "bge") == 0 || strcmp(name, "bltu") == 0 || strcmp(name, "bgeu") == 0)
    {
        value += 0b1100011;
        return value + handle_b_type(tokens, i);
    }
    else if (strcmp(name, "jal") == 0)
    {
        value += 0b1101111;
        return value + handle_j_type(tokens, i);
    }
    else if (strcmp(name, "jalr") == 0)
    {
        value += 0b1100111;
        return value + handle_i_type(tokens);
    }
    else
    {
        return 0;
    }
}

void load_program()
{
    FILE *fp;
    char *line = malloc(300 * sizeof(char));
    // size_t len = 0;
    // size_t read;

    /* Open program file. */
    fp = fopen(prog_file, "r");
    if (fp == NULL)
    {
        printf("Error: Can't open program file %s\n", prog_file);
        exit(-1);
    }

    /* Read in the program. */
    // while ((read = getline(&line, &len, fp)) != -1)
    while (fgets(line, 300, fp) != NULL)
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

void write_instr()
{
    printf("writing to file\n");
    FILE *fp;
    fp = fopen(prog_file, "w");
    if (fp == NULL)
    {
        printf("Error: Can't open program file %s\n", prog_file);
        exit(-1);
    }
    for (int i = 0; i < num_lines; i++)
    {
        if (prog_instr[i] != 0)
        {
            fprintf(fp, "%08x\n", prog_instr[i]);
        }
    }
    printf("wrote to the file\n");
    fclose(fp);
}
int main(int argc, char *argv[])
{
    printf("\n**************************\n");
    printf("Welcome to MU-RISCV SIM...\n");
    printf("**************************\n\n");

    if (argc < 3)
    {
        printf("Error: You should provide input and output file.\nUsage: %s <input program> \n\n", argv[0]);
        exit(1);
    }
    // uint32_t instruction;
    strcpy(prog_file, argv[1]);
    load_program();

    for (int i = 0; i < num_lines; i++)
    {
        printf("%d ", i);
        for (int j = 0; j < 4; j++)
        {
            if (prog_tokens[i][j])
            {
                printf("%s ", prog_tokens[i][j]);
            }
        }
        printf("\n");
        prog_instr[i] = getOpcode(prog_tokens[i], i);
        printf("instruction %d: %x\n", i, prog_instr[i]);
    }
    //printf("%d\n", label_distance(prog_tokens[48], 48));
    /*for(int i=0; i< num_lines;i++){

        prog_instr[i] = getOpcode(prog_tokens[i],i);
        printf("instruction %d: %u\n\n\n",i,prog_instr[i]);

    }*/
    strcpy(prog_file, argv[2]);
    write_instr();
}