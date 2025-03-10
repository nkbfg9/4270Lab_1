#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

#include "mu-riscv.h"

int did_branch = 0;

/***************************************************************/
/* Print out a list of commands available                                                                  */
/***************************************************************/
void help() {        
	printf("------------------------------------------------------------------\n\n");
	printf("\t**********MU-RISCV Help MENU**********\n\n");
	printf("sim\t-- simulate program to completion \n");
	printf("run <n>\t-- simulate program for <n> instructions\n");
	printf("rdump\t-- dump register values\n");
	printf("reset\t-- clears all registers/memory and re-loads the program\n");
	printf("input <reg> <val>\t-- set GPR <reg> to <val>\n");
	printf("mdump <start> <stop>\t-- dump memory from <start> to <stop> address\n");
	printf("high <val>\t-- set the HI register to <val>\n");
	printf("low <val>\t-- set the LO register to <val>\n");
	printf("print\t-- print the program loaded into memory\n");
	printf("?\t-- display help menu\n");
	printf("quit\t-- exit the simulator\n\n");
	printf("------------------------------------------------------------------\n\n");
}

/***************************************************************/
/* Turn a byte to a word                                                                          */
/***************************************************************/
uint32_t byte_to_word(uint8_t byte)
{
    return (byte & 0x80) ? (byte | 0xffffff80) : byte;
}

/***************************************************************/
/* Turn a halfword to a word                                                                          */
/***************************************************************/
uint32_t half_to_word(uint16_t half)
{
    return (half & 0x8000) ? (half | 0xffff8000) : half;
}

/***************************************************************/
/* Read a 32-bit word from memory                                                                            */
/***************************************************************/
uint32_t mem_read_32(uint32_t address)
{
	int i;
	for (i = 0; i < NUM_MEM_REGION; i++) {
		if ( (address >= MEM_REGIONS[i].begin) &&  ( address <= MEM_REGIONS[i].end) ) {
			uint32_t offset = address - MEM_REGIONS[i].begin;
			return (MEM_REGIONS[i].mem[offset+3] << 24) |
					(MEM_REGIONS[i].mem[offset+2] << 16) |
					(MEM_REGIONS[i].mem[offset+1] <<  8) |
					(MEM_REGIONS[i].mem[offset+0] <<  0);
		}
	}
	return 0;
}

/***************************************************************/
/* Write a 32-bit word to memory                                                                                */
/***************************************************************/
void mem_write_32(uint32_t address, uint32_t value){
	int i;
	uint32_t offset;
	for (i = 0; i < NUM_MEM_REGION; i++) {
		if ( (address >= MEM_REGIONS[i].begin) && (address <= MEM_REGIONS[i].end) ) {
			offset = address - MEM_REGIONS[i].begin;

			MEM_REGIONS[i].mem[offset+3] = (value >> 24) & 0xFF;
			MEM_REGIONS[i].mem[offset+2] = (value >> 16) & 0xFF;
			MEM_REGIONS[i].mem[offset+1] = (value >>  8) & 0xFF;
			MEM_REGIONS[i].mem[offset+0] = (value >>  0) & 0xFF;
		}
	}
}

/***************************************************************/
/* Execute one cycle                                                                                                              */
/***************************************************************/
void cycle() {                                                
	handle_instruction();
	CURRENT_STATE = NEXT_STATE;
	INSTRUCTION_COUNT++;
}

/***************************************************************/
/* Simulate RISCV for n cycles                                                                                       */
/***************************************************************/
void run(int num_cycles) {                                      
	
	if (RUN_FLAG == FALSE) {
		printf("Simulation Stopped\n\n");
		return;
	}

	printf("Running simulator for %d cycles...\n\n", num_cycles);
	int i;
	for (i = 0; i < num_cycles; i++) {
		if (RUN_FLAG == FALSE) {
			printf("Simulation Stopped.\n\n");
			break;
		}
		cycle();
	}
}

/**************************************************************rdump*/
/* simulate to completion                                                                                               */
/***************************************************************/
void runAll() {                                                     
	if (RUN_FLAG == FALSE) {
		printf("Simulation Stopped.\n\n");
		return;
	}

	printf("Simulation Started...\n\n");
	while (RUN_FLAG){
		cycle();
	}
	printf("Simulation Finished.\n\n");
}

/***************************************************************/ 
/* Dump a word-aligned region of memory to the terminal                              */
/***************************************************************/
void mdump(uint32_t start, uint32_t stop) {          
	uint32_t address;

	printf("-------------------------------------------------------------\n");
	printf("Memory content [0x%08x..0x%08x] :\n", start, stop);
	printf("-------------------------------------------------------------\n");
	printf("\t[Address in Hex (Dec) ]\t[Value]\n");
	for (address = start; address <= stop; address += 4){
		printf("\t0x%08x (%d) :\t0x%08x\n", address, address, mem_read_32(address));
	}
	printf("\n");
}

/***************************************************************/
/* Dump current values of registers to the teminal                                              */   
/***************************************************************/
void rdump() {                               
	int i; 
	printf("-------------------------------------\n");
	printf("Dumping Register Content\n");
	printf("-------------------------------------\n");
	printf("# Instructions Executed\t: %u\n", INSTRUCTION_COUNT);
	printf("PC\t: 0x%08x\n", CURRENT_STATE.PC);
	printf("-------------------------------------\n");
	printf("[Register]\t[Value]\n");
	printf("-------------------------------------\n");
	for (i = 0; i < RISCV_REGS; i++){
		printf("[R%d]\t: 0x%08x\n", i, CURRENT_STATE.REGS[i]);
	}
	printf("-------------------------------------\n");
	printf("[HI]\t: 0x%08x\n", CURRENT_STATE.HI);
	printf("[LO]\t: 0x%08x\n", CURRENT_STATE.LO);
	printf("-------------------------------------\n");
}

/***************************************************************/
/* Read a command from standard input.                                                               */  
/***************************************************************/
void handle_command() {                         
	char buffer[20];
	uint32_t start, stop, cycles;
	uint32_t register_no;
	int register_value;
	int hi_reg_value, lo_reg_value;

	printf("MU-RISCV SIM:> ");

	if (scanf("%s", buffer) == EOF){
		exit(0);
	}

	switch(buffer[0]) {
		case 'S':
		case 's':
			runAll(); 
			break;
		case 'M':
		case 'm':
			if (scanf("%x %x", &start, &stop) != 2){
				break;
			}
			mdump(start, stop);
			break;
		case '?':
			help();
			break;
		case 'Q':
		case 'q':
			printf("**************************\n");
			printf("Exiting MU-RISCV! Good Bye...\n");
			printf("**************************\n");
			exit(0);
		case 'R':
		case 'r':
			if (buffer[1] == 'd' || buffer[1] == 'D'){
				rdump();
			}else if(buffer[1] == 'e' || buffer[1] == 'E'){
				reset();
			}
			else {
				if (scanf("%d", &cycles) != 1) {
					break;
				}
				run(cycles);
			}
			break;
		case 'I':
		case 'i':
			if (scanf("%u %i", &register_no, &register_value) != 2){
				break;
			}
			CURRENT_STATE.REGS[register_no] = register_value;
			NEXT_STATE.REGS[register_no] = register_value;
			break;
		case 'H':
		case 'h':
			if (scanf("%i", &hi_reg_value) != 1){
				break;
			}
			CURRENT_STATE.HI = hi_reg_value; 
			NEXT_STATE.HI = hi_reg_value; 
			break;
		case 'L':
		case 'l':
			if (scanf("%i", &lo_reg_value) != 1){
				break;
			}
			CURRENT_STATE.LO = lo_reg_value;
			NEXT_STATE.LO = lo_reg_value;
			break;
		case 'P':
		case 'p':
			print_program(); 
			break;
		default:
			printf("Invalid Command.\n");
			break;
	}
}

/***************************************************************/
/* reset registers/memory and reload program                                                    */
/***************************************************************/
void reset() {   
	int i;
	/*reset registers*/
	for (i = 0; i < RISCV_REGS; i++){
		CURRENT_STATE.REGS[i] = 0;
	}
	CURRENT_STATE.HI = 0;
	CURRENT_STATE.LO = 0;
	
	for (i = 0; i < NUM_MEM_REGION; i++) {
		uint32_t region_size = MEM_REGIONS[i].end - MEM_REGIONS[i].begin + 1;
		memset(MEM_REGIONS[i].mem, 0, region_size);
	}
	
	/*load program*/
	load_program();
	
	/*reset PC*/
	INSTRUCTION_COUNT = 0;
	CURRENT_STATE.PC =  MEM_TEXT_BEGIN;
	NEXT_STATE = CURRENT_STATE;
	RUN_FLAG = TRUE;
}

/***************************************************************/
/* Allocate and set memory to zero                                                                            */
/***************************************************************/
void init_memory() {                                           
	int i;
	for (i = 0; i < NUM_MEM_REGION; i++) {
		uint32_t region_size = MEM_REGIONS[i].end - MEM_REGIONS[i].begin + 1;
		MEM_REGIONS[i].mem = malloc(region_size);
		memset(MEM_REGIONS[i].mem, 0, region_size);
	}
}

/**************************************************************/
/* load program into memory                                                                                      */
/**************************************************************/
void load_program() {                   
	FILE * fp;
	int i, word;
	uint32_t address;
	/* Open program file. */
	fp = fopen(prog_file, "r");
	if (fp == NULL) {
		printf("Error: Can't open program file %s\n", prog_file);
		exit(-1);
	}

	/* Read in the program. */

	i = 0;
	while( fscanf(fp, "%x\n", &word) != EOF ) {
		address = MEM_TEXT_BEGIN + i;
		mem_write_32(address, word);
		printf("writing 0x%08x into address 0x%08x (%d)\n", word, address, address);
		i += 4;
	}
	PROGRAM_SIZE = i/4;
	printf("Program loaded into memory.\n%d words written into memory.\n\n", PROGRAM_SIZE);
	fclose(fp);
}

void R_Processing(uint32_t rd, uint32_t f3, uint32_t rs1, uint32_t rs2, uint32_t f7) {
	switch(f3){
		case 0:
			switch(f7){
				case 0:		//add
					NEXT_STATE.REGS[rd] = NEXT_STATE.REGS[rs1] + NEXT_STATE.REGS[rs2];
					break;
				case 32:	//sub
					NEXT_STATE.REGS[rd] = NEXT_STATE.REGS[rs1] - NEXT_STATE.REGS[rs2];
					break;
				default:
					RUN_FLAG = FALSE;
					break;
				}	
			break;
		case 1:				//sll
			NEXT_STATE.REGS[rd] = NEXT_STATE.REGS[rs1] << NEXT_STATE.REGS[rs2];
			break;
		case 2:				//slt | shouldn't we be doing something to make this signed?
			NEXT_STATE.REGS[rd] = ((int32_t) NEXT_STATE.REGS[rs1] < (int32_t) NEXT_STATE.REGS[rs2])?1:0;
		case 3:				//sltu 
			NEXT_STATE.REGS[rd] = (NEXT_STATE.REGS[rs1] < NEXT_STATE.REGS[rs2])?1:0;
		case 4:
			NEXT_STATE.REGS[rd] = NEXT_STATE.REGS[rs1] ^ NEXT_STATE.REGS[rs2];
		case 5:
			switch(f7){
				case 0:		//srl
					NEXT_STATE.REGS[rd] = NEXT_STATE.REGS[rs1] >> NEXT_STATE.REGS[rs2];
					break;
				case 32:	//sra
					//uint8_t msb = NEXT_STATE.REGS[rs1] >> 31 & 0b1;
					if(NEXT_STATE.REGS[rs1] >> 31 & 0b1){
						for(int i = 0; i < NEXT_STATE.REGS[rs2]; i++) {
							NEXT_STATE.REGS[rd] = NEXT_STATE.REGS[rs1] >> 1 | 1 << 31;
						}
					}
					else {
						NEXT_STATE.REGS[rd] = NEXT_STATE.REGS[rs1] >> NEXT_STATE.REGS[rs2];
					}
					break;
				default:
					RUN_FLAG = FALSE;
					break;
			}
			break;
		case 6: 			//or
			NEXT_STATE.REGS[rd] = (NEXT_STATE.REGS[rs1] | NEXT_STATE.REGS[rs2]);
			break;
		case 7:				//and
			NEXT_STATE.REGS[rd] = (NEXT_STATE.REGS[rs1] & NEXT_STATE.REGS[rs2]);
			break;
		default:
			RUN_FLAG = FALSE;
			break;
	} 			
}

void ILoad_Processing(uint32_t rd, uint32_t f3, uint32_t rs1, uint32_t imm) {
	// if imm isn't set to be padded with msb, then we will have to do that here, right?
	switch (f3)
	{
	case 0: //lb
		NEXT_STATE.REGS[rd] = byte_to_word((mem_read_32(NEXT_STATE.REGS[rs1] + imm)) & 0xFF);
		break;

	case 1: //lh
		NEXT_STATE.REGS[rd] = half_to_word((mem_read_32(NEXT_STATE.REGS[rs1] + imm)) & 0xFFFF);
		break;

	case 2: //lw
		NEXT_STATE.REGS[rd] = mem_read_32(NEXT_STATE.REGS[rs1] + imm);
		break;
	
	default:
		printf("Invalid instruction");
		RUN_FLAG = FALSE;
		break;
	}
}

void Iimm_Processing(uint32_t rd, uint32_t f3, uint32_t rs1, uint32_t imm) {
	uint32_t imm0_4 = (imm << 7) >> 7;
	uint32_t imm5_11 = imm >> 5;
	switch (f3)
	{
	case 0: //addi | if imm isn't set to be padded with msb, we'll have to do that ourselves, right?
		NEXT_STATE.REGS[rd] = NEXT_STATE.REGS[rs1] + imm;
		break;

	case 4: //xori
		NEXT_STATE.REGS[rd] = NEXT_STATE.REGS[rs1] ^ imm;
		break;
	
	case 6: //ori
		NEXT_STATE.REGS[rd] = NEXT_STATE.REGS[rs1] | imm;
		break;
	
	case 7: //andi
		NEXT_STATE.REGS[rd] = NEXT_STATE.REGS[rs1] & imm;
		break;
	
	case 1: //slli
		NEXT_STATE.REGS[rd] = NEXT_STATE.REGS[rs1] << imm0_4;
		break;
	
	case 5: //srli and srai
		switch (imm5_11)
		{
			case 0: //srli
				NEXT_STATE.REGS[rd] = NEXT_STATE.REGS[rs1] >> imm0_4;
				break;

			case 32: //srai
				//NEXT_STATE.REGS[rd] = NEXT_STATE.REGS[rs1] >> imm0_4;
				break;
			
			default:
				RUN_FLAG = FALSE;
				break;
		}
		break;
	
	case 2:
		break;

	case 3:
		break;

	default:
		printf("Invalid instruction");
		RUN_FLAG = FALSE;
		break;
	}
}

void S_Processing(uint32_t imm4, uint32_t f3, uint32_t rs1, uint32_t rs2, uint32_t imm11) {
	// Recombine immediate
	// isn't this supposed to be signed?
	uint32_t imm = (imm11 << 5) + imm4;

	switch (f3)
	{
	case 0: //sb
		mem_write_32((NEXT_STATE.REGS[rs1] + imm), NEXT_STATE.REGS[rs2]);
		break;
	
	case 1: //sh
		mem_write_32((NEXT_STATE.REGS[rs1] + imm), NEXT_STATE.REGS[rs2]);
		break;

	case 2: //sw
		mem_write_32((NEXT_STATE.REGS[rs1] + imm), NEXT_STATE.REGS[rs2]);
		break;

	default:
		printf("Invalid instruction");
		RUN_FLAG = FALSE;
		break;
	}
}

void B_Processing(uint32_t imm4, uint32_t f3, uint32_t rs1, uint32_t rs2, uint32_t imm7) {
	// Recombine immediate
	uint32_t imm = ((imm7 & 0b1000000) << 6) + ((imm4 & 0b00001 << 11)) + ((imm7 & 0b0111111) << 5) + (imm4 & 0b11110);
	// this pads it with msb
	(imm & 0x800) ? imm = (imm | 0xfffff800) : imm;

	switch (f3) 
	{
		case 0: // beq
			if((int32_t) NEXT_STATE.REGS[rs1] == (int32_t) NEXT_STATE.REGS[rs2]) {
				NEXT_STATE.PC += imm;
				did_branch = 1;
			}
			break;

		case 1: // bne
			if((int32_t) NEXT_STATE.REGS[rs1] != (int32_t) NEXT_STATE.REGS[rs2]) {
				NEXT_STATE.PC += imm;
				did_branch = 1;
			}
			break;

		case 4: // blt
			if((int32_t) NEXT_STATE.REGS[rs1] < (int32_t) NEXT_STATE.REGS[rs2]) {
				NEXT_STATE.PC += imm;
				did_branch = 1;
			}
			break;

		case 5: // bge
			if((int32_t) NEXT_STATE.REGS[rs1] >= (int32_t) NEXT_STATE.REGS[rs2]) {
				NEXT_STATE.PC += imm;
				did_branch = 1;
			}
			break;

		case 6: // bltu
		if(NEXT_STATE.REGS[rs1] < NEXT_STATE.REGS[rs2]) {
				NEXT_STATE.PC += imm;
				did_branch = 1;
			}
			break;

		case 7: // bgeu
		if(NEXT_STATE.REGS[rs1] >= NEXT_STATE.REGS[rs2]) {
				NEXT_STATE.PC += imm;
				did_branch = 1;
			}
			break;

		default:
			printf("Invalid instruction");
			RUN_FLAG = FALSE;
			break;
	}
}

void J_Processing(uint32_t rd, uint32_t imm20) {
	// Recombine immediate
	uint32_t imm = (imm20 & 0x80000) + ((imm20 & 0x7fe00) >> 9) + ((imm20 & 0x100) << 2) + ((imm20 & 0xff) << 11);
	// this pads it with msb
	(imm & 0x80000) ? imm = (imm | 0xfff80000) : imm;

	NEXT_STATE.REGS[rd] = NEXT_STATE.PC + 4;
	NEXT_STATE.PC += imm;
	did_branch = 1;
}

void U_Processing() {
	// hi
}

void print_number_as_binary(unsigned int n) {
	if (n >> 1) {
		print_number_as_binary(n >> 1);
	}
	putc((n & 1) ? '1' : '0', stdout);
}

/************************************************************/
/* decode and execute instruction                                                                     */ 
/************************************************************/
void handle_instruction()
{
	
	//printf("instruction #%d: " , INSTRUCTION_COUNT);
	if(did_branch == 1) {
		did_branch = 0;
	}
	else NEXT_STATE.PC += 4;
	if(INSTRUCTION_COUNT>= PROGRAM_SIZE-1){
		RUN_FLAG = FALSE;
	}
	

	uint32_t bincmd = mem_read_32(CURRENT_STATE.PC);//instruction and its opcode
	OPCODE current_type;
	current_type.type = get_opcode_type(bincmd);
	current_type.code = bincmd & BIT_MASK_7;


	uint8_t rd;
	uint8_t funct3;
	uint8_t rs1;
	uint8_t rs2;
	uint8_t funct7;
	uint8_t imm4;
	uint8_t imm11;
	uint8_t imm;

	


	switch(current_type.type){
		case R:
			rd =  bincmd >> 7 & BIT_MASK_5;
			funct3 = bincmd >> 12 & BIT_MASK_3;
			rs1 = bincmd >> 15 & BIT_MASK_5;
			rs2 = bincmd >> 20 & BIT_MASK_5;
			funct7 = bincmd >> 25 & BIT_MASK_7;
			handle_r_print(bincmd);
			printf("\n");
			R_Processing( rd, funct3, rs1, rs2, funct7);
			//print("register #%d before: %d\nand after: %d\n\n",rd,CURRENT_STATE.REGS[rd],NEXT_STATE.REGS[rd]);
			break;
		case I:
			rd = bincmd >> 7 & BIT_MASK_5;
			funct3 = bincmd >> 12 & BIT_MASK_3;
			rs1 = bincmd >> 15 & BIT_MASK_5;
			imm = bincmd >> 20 & (BIT_MASK_12);
			handle_i_print(bincmd);
			printf("\n");
			if(current_type.code == 0b0000011){ ILoad_Processing(rd, funct3 ,rs1 ,imm); }
			else{ Iimm_Processing(rd, funct3 ,rs1 ,imm); }

			break;

		case S:
			imm4 = bincmd >> 7 & BIT_MASK_5;
			funct3 = bincmd >> 12 & BIT_MASK_3;
			rs1 = bincmd >> 15 & BIT_MASK_5;
			rs2 = bincmd >> 20 & BIT_MASK_5;
			imm11 = bincmd >> 25 & BIT_MASK_7;
			imm = (imm11 | imm4);
			handle_s_print(bincmd);
			printf("\n");
			S_Processing(imm4, funct3, rs1, rs2, imm11);
			break;
		case B:
			imm4 = bincmd >> 7 & BIT_MASK_5;
			funct3 = bincmd >> 12 & BIT_MASK_3;
			rs1 = bincmd >> 15 & BIT_MASK_5;
			rs2 = bincmd >> 20 & BIT_MASK_5;
			imm11 = bincmd >> 25 & BIT_MASK_7;
			imm = (imm11 | imm4);
			// we're gonna have to make a command to print b commands
			handle_s_print(bincmd);
			printf("\n");
			B_Processing(imm4, funct3, rs1, rs2, imm11);
			break;
		case U:
		case J:
			rd = bincmd >> 7 & BIT_MASK_5;
			uint32_t imm20 = bincmd >> 12 & 0xffff;
			// we're also gonna need to make a handle_j_print()
			handle_s_print(bincmd);
			printf("\n");
			J_Processing(rd, imm20);
		default:
			printf("These types of instructions are not implemented\n");
	}

	
	

	

}
/************************************************************/
/* Initialize Memory                                                                                                    */ 
/************************************************************/
void initialize() { 
	init_memory();
	CURRENT_STATE.PC = MEM_TEXT_BEGIN;
	CURRENT_STATE.REGS[2] = MEM_STACK_BEGIN;
	NEXT_STATE = CURRENT_STATE;
	RUN_FLAG = TRUE;
}

/************************************************************/
/* Print the program loaded into memory (in RISCV assembly format)    */ 
/************************************************************/
void print_program(){
	uint32_t instr_address = MEM_TEXT_BEGIN;
	uint32_t bincmd;
	OPCODE current_type;

	for(uint32_t i=0; i < PROGRAM_SIZE; i++){
		bincmd = mem_read_32(instr_address+(i*4));
		current_type.type = get_opcode_type(bincmd);
		switch(current_type.type){
			case R:
				handle_r_print(bincmd);
				printf("\n");
				break;
			case I:
				handle_i_print(bincmd);
				printf("\n");
				break;

			case S:
				handle_s_print(bincmd);
				printf("\n");
				break;
			case B:
			case U:
			case J:
			default:
				printf("These types of instructions are not implemented\n");
		}
	}
}

/************************************************************/
/* Print the instruction at given memory address (in RISCV assembly format)    */
/************************************************************/
void print_command(uint32_t bincmd) {
    enum OPCODE_TYPE cmd_type = get_opcode_type(bincmd);
    switch (cmd_type) {
        case R:
            handle_r_print(bincmd);
            break;
		case S:
			handle_s_print(bincmd);
			break;
		case I:
			handle_i_print(bincmd);
			break;
		default:
			printf("Unknown command!");
			break;
    }
	printf("\n");
}

void handle_r_print(uint32_t bincmd) {
	uint8_t rd = bincmd >> 7 & BIT_MASK_5;
	uint8_t funct3 = bincmd >> 12 & BIT_MASK_3;
	uint8_t rs1 = bincmd >> 15 & BIT_MASK_5;
	uint8_t rs2 = bincmd >> 20 & BIT_MASK_5;
	uint8_t funct7 = bincmd >> 25 & BIT_MASK_7;
	switch(funct3) {
		case ADD_SUB:
			switch(funct7){
				case ADD:
					print_r_cmd("add", rd, rs1, rs2);
					break;
				case SUB:
					print_r_cmd("sub", rd, rs1, rs2);
					break;
				default:
					printf("No funct7(%d) for funct3(%d) found for R-type.", funct7, funct3);
					break;
			}
			break;
		case SLL:
			print_r_cmd("sll", rd, rs1, rs2);
			break;
		case SLT:
			print_r_cmd("slt", rd, rs1, rs2);
			break;
		case SLTU:
			print_r_cmd("sltu", rd, rs1, rs2);
			break;
		case XOR:
			print_r_cmd("xor", rd, rs1, rs2);
			break;
		case SRL_SRA:
			switch(funct7){
				case SRL:
					print_r_cmd("srl", rd, rs1, rs2);
					break;
				case SRA:
					print_r_cmd("sra", rd, rs1, rs2);
					break;
				default:
					printf("No funct7(%d) for funct3(%d) found for R-type.", funct7, funct3);
					break;
			}
			break;
		case OR:
			print_r_cmd("or", rd, rs1, rs2);
			break;
		case AND:
			print_r_cmd("and", rd, rs1, rs2);
			break;
		default:
			printf("Unknown funct3(%d) in R-type", funct3);
			break;
	}
}

void handle_s_print(uint32_t bincmd) {
	uint8_t imm4 = bincmd >> 7 & BIT_MASK_5;
	uint8_t f3 = bincmd >> 12 & BIT_MASK_3;
	uint8_t rs1 = bincmd >> 15 & BIT_MASK_5;
	uint8_t rs2 = bincmd >> 20 & BIT_MASK_5;
	uint8_t imm11 = bincmd >> 25 & BIT_MASK_7;
	uint16_t imm = (imm11 | imm4);
	switch(f3) {
		case SB:
			print_s_cmd("sb", rs2, imm, rs1);
			break;
		case SH:
			print_s_cmd("sh", rs2, imm, rs1);
			break;
		case SW:
			print_s_cmd("sw", rs2, imm, rs1);
			break;
		default:
			printf("Unknown funct3(%d) in S type", f3);
			break;
	}
}

void handle_i_print(uint32_t bincmd) {

	uint8_t opcode = bincmd & BIT_MASK_7;
	uint8_t rd = bincmd >> 7 & BIT_MASK_5;
	uint8_t funct3 = bincmd >> 12 & BIT_MASK_3;
	uint8_t rs1 = bincmd >> 15 & BIT_MASK_5;
	uint8_t imm = bincmd >> 20 & (BIT_MASK_12);
	switch(opcode) {
		case 0b0010011:
			switch(funct3) {
				case 0x0: 
					print_i_type1_cmd("addi", rd, rs1, imm);
					break;
				case 0x1:
					print_i_type1_cmd("slli", rd, rs1, imm);
					break;
				case 0x2:
					print_i_type1_cmd("slti", rd, rs1, imm);
					break;
				case 0x3:
					print_i_type1_cmd("sltiu", rd, rs1, imm);
					break;
				case 0x4:
					print_i_type1_cmd("xori", rd, rs1, imm);
					break;
				case 0x5:
					//uint8_t imm5 = imm >> 5;
					switch(imm >> 5){
						case 0:
							print_i_type1_cmd("srli", rd, rs1, imm);
							break;
						case 0x20:
							print_i_type1_cmd("srai", rd, rs1, imm);
							break;
						default:
							printf("Invalid imm[11:5](%d) for I-Type opcode(%d) funct3(%d)", (imm >> 5), opcode, funct3);
							break;
					}
					break;
				case 0x6:
					print_i_type1_cmd("ori", rd, rs1, imm);
					break;
				case 0x7:
					print_i_type1_cmd("andi", rd, rs1, imm);
					break;
				default:
					printf("Invalid funct3(%d) for I-type opcode(%d)", funct3, opcode);
					break;
			}
			break;
		case 0b0000011:
			switch(funct3){
				case 0x0:
					print_i_type2_cmd("lb", rd, rs1, imm);
					break;
				case 0x1:
					print_i_type2_cmd("lh", rd, rs1, imm);
					break;
				case 0x2:
					print_i_type2_cmd("lw", rd, rs1, imm);
					break;
				case 0x4:
					print_i_type2_cmd("lbu", rd, rs1, imm);
					break;
				case 0x5:
					print_i_type2_cmd("lhu", rd, rs1, imm);
					break;
				default:
					printf("Unknown funct3(%d) for I-type opcode(%d).", funct3, opcode);
					break;
			}
			break;
		default:
			printf("Unknown opcode(%d) for I-Type.", opcode);
			break;
	}

}

void print_r_cmd(char* cmd_name, uint8_t rd, uint8_t rs1, uint8_t rs2) {
    printf("%s x%d, x%d, x%d", cmd_name, rd, rs1, rs2);
}

void print_s_cmd(char* cmd_name, uint8_t rs2, uint8_t offset, uint8_t rs1) {
    printf("%s x%d, %d(x%d)", cmd_name, rs2, offset, rs1);
}

void print_i_type1_cmd(char* cmd_name, uint8_t rd, uint8_t rs1, uint16_t imm) {
    printf("%s x%d, x%d, %d", cmd_name, rd, rs1, imm);
}

void print_i_type2_cmd(char* cmd_name, uint8_t rd, uint8_t rs1, uint16_t imm) {
    printf("%s x%d, %d(x%d)", cmd_name, rd, imm, rs1);
}


enum OPCODE_TYPE get_opcode_type(uint32_t cmd) {
	enum OPCODE_TYPE retVal = ERROR;
    for(int i = 0; i < NUM_CODES; i++) {
        // opcode only 7 bits, so only compare those 7 bits 
            // (by forcing a zero into the 8th bit, the opcodes have a 0 zero there by default)
        uint8_t cmp = cmd & BIT_MASK_7;
        if(cmp == opcodes[i].code) {
            retVal = opcodes[i].type;
			break;
        }
    }
	return retVal;
}


/***************************************************************/
/* main                                                                                                                                   */
/***************************************************************/
int main(int argc, char *argv[]) {                              
	printf("\n**************************\n");
	printf("Welcome to MU-RISCV SIM...\n");
	printf("**************************\n\n");
	
	if (argc < 2) {
		printf("Error: You should provide input file.\nUsage: %s <input program> \n\n",  argv[0]);
		exit(1);
	}

	strcpy(prog_file, argv[1]);
	initialize();
	load_program();
	help();
	while (1){
		handle_command();
	}
	return 0;
}
