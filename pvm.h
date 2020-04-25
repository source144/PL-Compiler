// Gonen Matias
// COP 3402, Spring 2020
// NID: go658748
// Version: 03/31/2020 12:38AM

#ifndef __PVM__
#define __PVM__

#include <stdlib.h>
#include <math.h>

// Debugging
#define DBG_BASE_REG		1

// Constants
#define LINE_BUFFER			1024
#define MAX_STACK_HEIGHT	100
#define MAX_CODE_LENGTH		200
#define MAX_LEXI_LEVELS		30
#define NUM_REGISTERS		20

// Directives
#define FLAG		'-'
#define FLAG_LEX	'l'
#define FLAG_IDCTR	'i'
#define FLAG_ASMBLY	'a'
#define FLAG_VM		'v'
#define FLAG_OUT	'o'

// Halt Codes
#define CONTINUE	0
#define HALT_EXIT	1
#define HALT_STACK	2
#define HALT_REG	3
#define HALT_AR		4
#define HALT_OP		5
// TODO: REG underflow?

// Opcodes
#define OP_LIT	1	// Push constant value (M) into register (R)
#define OP_RTN	2	// Returns from a subroutine
#define OP_LOD	3	// Load into register (R) from stack level (L) offset (M)
#define OP_STO	4	// Store from register (R) to stack level (L) offset (M)
#define OP_CAL	5	// Call procedure at located at Code[M]
#define OP_INC	6	// Allocate (M) locals
#define OP_JMP	7	// Jump to instruction (M)
#define OP_JPC	8	// If value in register (R) is 0, jump to instruction (M)
#define OP_OUT	9	// Write the content in register (R)
#define OP_RIN	10	// Read input and store to register (R)
#define OP_EXT	11	// End program

// ALU Like opcodes
#define OP_NEG	12
#define OP_ADD	13
#define OP_SUB	14
#define OP_MUL	15
#define OP_DIV	16
#define OP_ODD	17
#define OP_MOD	18
#define OP_EQL	19
#define OP_NEQ	20
#define OP_LSS	21
#define OP_LEQ	22
#define OP_GTR	23
#define OP_GEQ	24

// REG Codes
#define REG_L 25	// Set by literal
#define REG_R 26	// Set by reg
#define REG_B 27	// Load Base to reg

#pragma region "Structs"
//	ISA 24 Instructions
//	seperated by spaces
//		OP	R	L	M
typedef struct
{
	int op;
	int r;
	int l;
	int m;
} instruction_t;

typedef struct
{
	int ir;									// Instruction Register (implemented as an index)
	int bp;									// Base Pointer
	int sp;									// Stack Pointer
	int pc;									// Program Counter
	int __baseReg;							// The base register we operate on
	int __AR_CNT;							// Activation Record count
	int rf[NUM_REGISTERS];					// Register File
	int stack[MAX_STACK_HEIGHT];			// The stack of the VM
	int __AR[MAX_LEXI_LEVELS];				// Indexes of records in stack
	instruction_t **code;	// All instructions of the VM
} vm_t;

void initIndicator(void);
void resizeIndicator(void);


int _numDigits(int n)
{
	return n ? floor(log10(abs(n))) + 1 + (n < 0) : 1;
}

#pragma endregion

#endif