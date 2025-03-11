#include <stdint.h>

#define FALSE 0
#define TRUE 1

/******************************************************************************/
/* RISCV memory layout                                                                                                                                      */
/******************************************************************************/
#define MEM_TEXT_BEGIN 0x00400000
#define MEM_TEXT_END 0x0FFFFFFF
/*Memory address 0x10000000 to 0x1000FFFF access by $gp*/
#define MEM_DATA_BEGIN 0x10010000
#define MEM_DATA_END 0x7FFFFFFF

#define MEM_KTEXT_BEGIN 0x80000000
#define MEM_KTEXT_END 0x8FFFFFFF

#define MEM_KDATA_BEGIN 0x90000000
#define MEM_KDATA_END 0xFFFEFFFF

/*stack and data segments occupy the same memory space. Stack grows backward (from higher address to lower address) */
#define MEM_STACK_BEGIN 0x7FFFFFFF
#define MEM_STACK_END 0x10010000

// OPCODE INFO
#define BIT_MASK_8 0b11111111
#define BIT_MASK_3 BIT_MASK_8 >> 5
#define BIT_MASK_5 BIT_MASK_8 >> 3
#define BIT_MASK_7 BIT_MASK_8 >> 1
#define BIT_MASK_12 (BIT_MASK_8 << 4) | 0b1111

enum R_FUNCT3
{
    ADD_SUB = 0x0,
    SLL,
    SLT,
    SLTU,
    XOR,
    SRL_SRA,
    OR,
    AND
};

enum R_FUNCT7
{
    ADD = 0x0,
    SRL = 0x0,
    SUB = 0x20,
    SRA = 0x20
};

enum S_FUNCT
{
    SB = 0x0,
    SH = 0x1,
    SW = 0x2
};

enum B_FUNCT
{
    BEQ = 0x0,
    BNE = 0x1,
    BLT = 0x4,
    BGE = 0x5,
    BLTU = 0x6,
    BGEU = 0x7,
};

enum OPCODE_TYPE
{
    R = 0,
    I,
    S,
    B,
    J,
    U,
    ERROR
};

typedef struct
{
    enum OPCODE_TYPE type;
    uint8_t code;
} OPCODE;

#define NUM_CODES 7
static OPCODE opcodes[NUM_CODES] = {
    {.type = R, .code = 0b0110011},
    {.type = I, .code = 0b0010011},
    {.type = I, .code = 0b0000011},
    {.type = S, .code = 0b0100011},
    {.type = B, .code = 0b1100011},
    {.type = U, .code = 0b0110111},
    {.type = J, .code = 0b1101111},
};

enum OPCODE_TYPE get_opcode_type(uint32_t cmd);

typedef struct
{
    uint32_t begin, end;
    uint8_t *mem;
} mem_region_t;

/* memory will be dynamically allocated at initialization */
mem_region_t MEM_REGIONS[] = {
    {MEM_TEXT_BEGIN, MEM_TEXT_END, NULL},
    {MEM_DATA_BEGIN, MEM_DATA_END, NULL},
    {MEM_KDATA_BEGIN, MEM_KDATA_END, NULL},
    {MEM_KTEXT_BEGIN, MEM_KTEXT_END, NULL}};

#define NUM_MEM_REGION 4
#define RISCV_REGS 32

typedef struct CPU_State_Struct
{

    uint32_t PC;               /* program counter */
    uint32_t REGS[RISCV_REGS]; /* register file. */
    uint32_t HI, LO;           /* special regs for mult/div. */
} CPU_State;

/***************************************************************/
/* CPU State info.                                                                                                               */
/***************************************************************/

CPU_State CURRENT_STATE, NEXT_STATE;
int RUN_FLAG; /* run flag*/
uint32_t INSTRUCTION_COUNT;
uint32_t PROGRAM_SIZE; /*in words*/

char prog_file[32];

/***************************************************************/
/* Function Declerations.                                                                                                */
/***************************************************************/
void help();
uint32_t mem_read_32(uint32_t address);
void mem_write_32(uint32_t address, uint32_t value);
void cycle();
void run(int num_cycles);
void runAll();
void mdump(uint32_t start, uint32_t stop);
void rdump();
void handle_command();
void reset();
void init_memory();
void load_program();
void handle_instruction(); /*IMPLEMENT THIS*/
void initialize();
void print_program(); /*IMPLEMENT THIS*/
void print_instruction(uint32_t);
void print_command(uint32_t);
void handle_r_print(uint32_t bincmd);
void handle_s_print(uint32_t bincmd);
void handle_b_print(uint32_t bincmd);
void handle_j_print(uint32_t bincmd);
void handle_i_print(uint32_t bincmd);
void print_r_cmd(char *cmd_name, uint8_t rd, uint8_t rs1, uint8_t rs2);
void print_s_cmd(char *cmd_name, uint8_t rs2, uint8_t offset, uint8_t rs1);
void print_b_cmd(char *cmd_name, uint8_t rs1, uint8_t rs2, int16_t imm);
void print_j_cmd(char *cmd_name, uint8_t rd, int32_t imm);
void print_i_type1_cmd(char *cmd_name, uint8_t rd, uint8_t rs1, uint16_t imm);
void print_i_type2_cmd(char *cmd_name, uint8_t rd, uint8_t rs1, uint16_t imm);
