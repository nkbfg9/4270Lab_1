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
int num_lines = 0;
const char *delimeter = " ";
char *temp;


void toLowerCase(char *str) {
    for (int i = 0; str[i]; i++) {
        if (str[i] >= 'A' && str[i] <= 'Z') {
            str[i] += 'a' - 'A';
        }
    }
}

uint32_t getOpcode(char * tokens[]) {
    uint32_t value = 0;
    char * name = tokens[0];
    toLowerCase(name);
    if(strcmp(name, "add") || strcmp(name, "sub") || strcmp(name, "xor") || strcmp(name, "or") || strcmp(name, "and") || strcmp(name, "sll")
        || strcmp(name, "srl")  || strcmp(name, "sll") || strcmp(name, "sra") || strcmp(name, "slt") || strcmp(name, "sltu")) {
            value += 0b0110011;
            return value;
    }
    else if(strcmp(name, "addi") || strcmp(name, "xori") || strcmp(name, "ori") || strcmp(name, "andi") || strcmp(name, "slli") || strcmp(name, "srli")
        || strcmp(name, "srai") || strcmp(name, "slti") || strcmp(name, "sltiu")) {
            value += 0b0010011;
            return value + handle_i_type(tokens);
        }
    else if(strcmp(name, "lb") || strcmp(name, "lh") || strcmp(name, "lw") || strcmp(name, "lbu") || strcmp(name, "lhu")) {
        value += 0b0000011;
        return value;
    }
    else if(strcmp(name, "sb") || strcmp(name, "sh") || strcmp(name, "sw")) {
        value += 0b0100011;
        return value;
    }
    else if(strcmp(name, "beq") || strcmp(name, "bne") || strcmp(name, "blt") || strcmp(name, "bge") || strcmp(name, "bltu") || strcmp(name, "bgeu")) {
        value += 0b1100011;
        return value;
    }
    else if(strcmp(name, "jal")) {
        value += 0b1101111;
        return value;
    }
    else if(strcmp(name, "jalr")) {
        value += 0b1100111;
        return value;
    }
    else return 0;
}

//takes token and returns an integer[must be null terminating]
uint32_t char_to_int(char * token){
    uint32_t length = strlen(token);
    uint32_t blah =0;
    for(uint32_t i=0;i < length;i++){
        blah += token[i] * (pow(10,length-(i+1)));//10 ^ length-i for tens positions
    }
    return blah;
}


uint32_t handle_r_type(char * tokens[]){

    uint32_t value =0;
    uint32_t registers =0;
    //rs2
    if(strcmp(tokens[1], "zero")){
        registers = char_to_int(tokens[1] + sizeof(char));
        value += registers << 20;
    }
    
    //rs1
    if(strcmp(tokens[2], "zero")){
        registers = char_to_int(tokens[2]  + sizeof(char));
        value += registers << 15;
    }
    //rd
    registers = char_to_int(tokens[3] + sizeof(char));
    value += registers << 7;

    //funct7
    //only sub and sra != 0x00
    if( !(  strcmp("sub", tokens[0])  ) ||  !(  strcmp("sra",tokens[0]) )  ){
        registers = 32;
        value += registers << 25;
    }

    //funct3 i hate elifs
    if(!(  strcmp("sub", tokens[0])  ) ||  !(  strcmp("add",tokens[0]) )){
        registers =0;
    }
    else if(  !(  strcmp("srl", tokens[0])  ) ||  !(  strcmp("sra",tokens[0]) )  ){
        registers = 5;
    }
    else if(!(  strcmp("xor", tokens[0])  )){
        registers = 4;
    }
    else if(!(  strcmp("or", tokens[0])  )){
        registers = 6;
    }
    else if(!(  strcmp("and", tokens[0])  )){
        registers = 7;
    }
    else if(!(  strcmp("sll", tokens[0])  )){
        registers = 1;
    }
    else if(!(  strcmp("slt", tokens[0])  )){
        registers = 2;
    }
    else if(!(  strcmp("sltu", tokens[0])  )){
        registers = 3;
    }
    value += registers << 12;


    return value;

}

uint32_t handle_i_type(char * tokens[]) {
    uint32_t value = 0;
    char * name = tokens[0];
    char * rd = tokens[1];
    char * rs1 = tokens[2];
    char * imm = tokens[3];
    toLowerCase(name);
    if(strcmp(name, "addi")) {

    }
    if(strcmp(name, "xori")) {
        value += 0x4000;
    }
    if(strcmp(name, "ori")) {
        value += 0x6000;
    }
    if(strcmp(name, "andi")) {
        value += 0x7000;
    }
    if(strcmp(name, "slli")) {
        value += 0x1000;
    }
    if(strcmp(name, "srli")) {
        value += 0x5000;
    }
    if(strcmp(name, "srai")) {
        value += 0x5000;
        value += 0x20000000;
    }
    if(strcmp(name, "slti")) {
        value += 0x2000;
    }
    if(strcmp(name, "sltiu")) {
        value += 0x3000;
    }

    value += char_to_int(rd) << 6;
    value += char_to_int(rs1) << 15;
    value += char_to_int(imm) << 20;
    return value;
}

uint32_t handle_s_type(char * tokens[]){
    uint32_t value,registers =0;
    uint32_t immediate_mask = 0b11111110000000000000111110000000;

    //rs2
    if(strcmp(tokens[1], "zero")){
        registers = char_to_int(tokens[1] + sizeof(char));
        value += registers << 20;
    }
    
    //separating rs1 and immediate to get null terminated tokens
    char * offset_and_register = malloc(sizeof(tokens[2]));
    strcpy(offset_and_register, tokens[2]);
    char * offset, reg = NULL;

    offset = strtok(offset_and_register,"()");//immediate
    reg = strtok(offset_and_register,"()");//rs1

    
    //rs1
    if(strcmp(reg, "zero")){
        registers = char_to_int(reg + sizeof(char));
        value += registers << 15;
    }



    //immediate
    registers = char_to_int(offset);

    value += (immediate_mask & (registers << 7));//only 12 bits so dont need to worry about overlap
    value += (immediate_mask & (registers << 20));//shift 20 to get last 7 bits

    //no mem leak
    free(offset_and_register);
    offset = NULL;
    reg = NULL;
    offset_and_register = NULL;



    //funct3 code
    //sw < sh < sb in strcmp
    int funct3 = strcmp("sh", tokens[0]);
    if(funct3 == 0){//sh
        registers = 1;
    }
    else if(funct3 <0){//sw
        registers = 2;

    }
    else{//sb greater than 0
        registers = 0;
    }

    value += registers << 12;


    return value;

}

uint32_t handle_b_type(char * tokens[]){
    uint32_t value,registers =0;

    //rs2
    if(strcmp(tokens[1], "zero")){
        registers = char_to_int(tokens[1] + sizeof(char));
        value += registers << 20;
    }
    
    //rs1
    if(strcmp(tokens[2], "zero")){
        registers = char_to_int(tokens[2]  + sizeof(char));
        value += registers << 15;
    }



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
    uint32_t instruction;
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

    for(int i=0; i< num_lines;i++){
        instruction = getOpcode(prog_tokens[i]);

    }



}