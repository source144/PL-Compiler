// Gonen Matias
// COP 3402, Spring 2020
// NID: go658748
// Version: 01/29/2020 01:09AM

// FIXME Memory Management
// TODO: Check for underflow?

// NOTE: All of the constants, structs and
// their methods are defined in pvm.h
#include <stdio.h>
#include <stdlib.h>
#include "pvm.h"

// IO Defines
#define WARN(format, ...) fprintf(stderr, "PL/0 VM: " format "\n", __VA_ARGS__)
#define OUT(format, ...) fprintf(stdout, "PL/0 VM: " format "\n", __VA_ARGS__)
#define DIE(format, ...) WARN(format, __VA_ARGS__), exit(EXIT_FAILURE)

// OPSTRINGS lookup array
const char *OPSTRINGS[] = {
	"LIT",
	"RTN",
	"LOD",
	"STO",
	"CAL",
	"INC",
	"JMP",
	"JPC",
	"SIO",
	"NEG",
	"ADD",
	"SUB",
	"MUL",
	"DIV",
	"ODD",
	"MOD",
	"EQL",
	"NEQ",
	"LSS",
	"LEQ",
	"GTR",
	"GEQ"
};


#define FORMAT_HDR1(...)		printf("Line   OP   R  L  M\n")
#define FORMAT_HDR2(...)		printf("                      PC   BP    SP    REGISTERS\n")
#define FORMAT_INIT(...)		printf("Initial values        %-3d   %-2d    %-2d   ", __VA_ARGS__)
#define FORMAT_DATA(...)		printf("   %-3d   %-2d    %-2d   ", __VA_ARGS__)
#define FORMAT_INST(...)		printf("%-3d  %-3s  %d  %d  %-3d", __VA_ARGS__)
#define FORMAT_INST_LNG(...)	printf("%-4d   %-3s  %d  %d  %-3d", __VA_ARGS__)

#pragma region "Function Prototypes"
vm_t *initVMWithCode(instruction_t **code);
vm_t *initVM(void);
vm_t *destroyVM(vm_t *vm);
instruction_t *createInstruction(int op, int r, int l, int m);
instruction_t *destroyInstruction(instruction_t *instr);
void printDecodedInstructions(vm_t *vm);
void initialPrint(vm_t *vm);
void printState(vm_t *vm);
void printRegisters(int rf[NUM_REGISTERS]);
void printStack(vm_t *vm);
void printInstruction(vm_t *vm);
void printHaltReason(int haltCode);
int willOverflowStack(int nextIdx);
int willOverflowRegister(int nextIdx);
int willOverflowAR(int nextIdx);
const char *getOPStr(int opcode);
int b(int l, int bp, int stack[]);
void loadFile(char *filename, instruction_t *code[]);
int executeOp(vm_t *vm, int printFlag);
void fetchOp(vm_t *vm, int printFlag);
#pragma endregion

#pragma region "VM struct functions"

// Initializes a VM struct
// and returns a pointer to it
vm_t *initVM(void)
{
	vm_t *vm;

	// Inits all to 0
	if (!(vm = calloc(1, sizeof(vm_t))))
		return NULL;

	if (!(vm->code = calloc(MAX_CODE_LENGTH, sizeof(instruction_t))))
		return destroyVM(vm);
		
	// Init BP to 1
	vm->bp = 1;

	return vm;
}

// Initializes a VM struct with a given
// code, and returns a pointer to it
vm_t *initVMWithCode(instruction_t **code)
{
	vm_t *vm;

	// Check code and init all to 0
	if (!code || !(vm = calloc(1, sizeof(vm_t))))
		return NULL;

	// Set VM Code
	vm->code = code;

	// Init BP to 1
	vm->bp = 1;

	return vm;
}

vm_t *destroyVM(vm_t *vm)
{
	if (!vm) return NULL;

	if (vm->code)
		for (int i = 0; i < MAX_CODE_LENGTH; i++)
			destroyInstruction(vm->code[i]);

	free(vm->code);
	free(vm);

	return NULL;
}

#pragma endregion

#pragma region "Instruction struct functions"

// Create a new instruction
// and returns a pointer to it
instruction_t *createInstruction(int op, int r, int l, int m)
{
	instruction_t *instr;

	if (!(instr = malloc(sizeof(instruction_t))))
		return NULL;

	instr->op = op;
	instr->r = r;
	instr->l = l;
	instr->m = m;

	return instr;
}

instruction_t *destroyInstruction(instruction_t *instr)
{
	if (!instr) return NULL;
	free(instr);
	
	return NULL;
}

#pragma endregion

#pragma region "Print Functions"
// Prints the decoded input file
// Assumes vm in its entirety is valid 
void printDecodedInstructions(vm_t *vm)
{
	FORMAT_HDR1();
	for (int i = 0; vm->code[i]; i++)
	{
		FORMAT_INST_LNG(i, getOPStr(vm->code[i]->op),
			vm->code[i]->r, vm->code[i]->l, vm->code[i]->m);
		// printRegisters(vm->rf);
		printf("\n");
	}

	fflush(stdout);
}
// Print initial state of a vm
// Assumes vm in its entirety is valid 
void initialPrint(vm_t *vm)
{
	FORMAT_HDR2();
	FORMAT_INIT(vm->pc, vm->bp, vm->sp);
	printRegisters(vm->rf);
	printStack(vm);
}

// Print current state of a vm
// Assumes vm in its entirety is valid 
void printState(vm_t *vm)
{
	FORMAT_DATA(vm->pc, vm->bp, vm->sp);
	printRegisters(vm->rf);
	printStack(vm);
}

// Prints the register file of a vm
// Assumes the register file is from a valid vm
void printRegisters(int rf[NUM_REGISTERS])
{
	int i, end = NUM_REGISTERS - 1;

	for (i = 0; i < end; i++)
		printf("%d  ", rf[i]);
	printf("%d\n", rf[end]);
}

// Prints the stack of a vm
// Assumes vm in its entirety is valid 
void printStack(vm_t *vm)
{
	int i, j, end = vm->sp - 1;
	printf("Stack: ");

	if (!vm->sp)
	{
		printf("\n\n");
		return;
	}

	j = 0;
	printf("%d ", vm->stack[0]);
	for (i = 1; i < end; i++)
	{
		if (vm->__AR[j] && vm->__AR[j] == i && ++j)
			printf("|");
		printf(" %d ", vm->stack[i]);
	}

	// Last element
	if (vm->__AR[j++] == end)
		printf("|");
	printf(" %d\n\n", vm->stack[end]);
}

// Prints the current instruction that
// that is executed by the vm
// Assumes vm in its entirety is valid 
void printInstruction(vm_t *vm)
{
	FORMAT_INST(vm->pc, getOPStr(vm->code[vm->ir]->op),
			vm->code[vm->ir]->r, vm->code[vm->ir]->l, vm->code[vm->ir]->m);
	fflush(stdout);
}

void printHaltReason(int haltCode)
{
	switch (haltCode)
	{
		case HALT_STACK:
			WARN("%s", "the stack overflowed!");
			break;

		case HALT_REG:
			WARN("%s", "the register overflowed!");
			break;

		case HALT_AR:
			WARN("%s", "activation record level exceeded!");
			break;

		case HALT_OP:
			WARN("%s", "invalid operation!");
			break;
	}
}
#pragma endregion

#pragma region "Overflow Validations"

// Determines whether the VM's stack will overflow
int willOverflowStack(int nextIdx)
{
	return nextIdx >= MAX_STACK_HEIGHT;
}

// Determines whether the VM's register will overflow
int willOverflowRegister(int nextIdx)
{
	return nextIdx >= NUM_REGISTERS;
}

// Determines whether the VM's register will overflow
int willOverflowAR(int nextIdx)
{
	return nextIdx >= MAX_LEXI_LEVELS;
}

#pragma endregion

#pragma region "Helpers"
// Returns a decoded opcode (operation name) as a string
const char *getOPStr(int opcode)
{
	if (opcode > 0)
	{
		if (opcode < 9)		return OPSTRINGS[opcode - 1];
		if (opcode < 12)	return OPSTRINGS[8];
		if (opcode < 25)	return OPSTRINGS[opcode - 3];
		else return "ERR";
	}
	else return "ERR";
}

// Returns the base pointer to the Activation record
// Activation record (AR) l levels down in the stack
int b(int l, int bp, int stack[])
{
	for (int i = l; i > 0; i--)
		bp = stack[bp + 2];

	return bp;
	// return bp == 1 ? 0 : bp;
}

#pragma endregion

// Loads an input file into a vm
void loadFile(char *filename, instruction_t *code[])
{
	int i, op, r, l, m;
	char line[LINE_BUFFER];
	FILE *fp;

	if (!(fp = fopen(filename, "r")))
		DIE("%s (%s)", "unable to read input file", filename);

	// Read File
	for (i = 0; fgets(line, LINE_BUFFER, fp); i++)
	{
		// Validate code length
		if (i >= MAX_CODE_LENGTH)
		{
			fclose(fp);
			DIE("%s exceeds %d lines", filename, MAX_CODE_LENGTH);
		}

		// Validate current line's arguments
		if (sscanf(line, " %d %d %d %d ", &op, &r, &l, &m) != 4)
		{
			fclose(fp);
			WARN("%s (%d) %s", filename, i + 1, line);
			DIE("%s (%d) invalid/missing argument(s)", filename, i + 1);
		}

		// Add to instruction set
		code[i] = createInstruction(op, r, l, m);
	}

	fclose(fp);  
}

int executeOp(vm_t *vm, int vmFlag)
{
	int base, op, r, l, m;

	// Readability purposes:
	op = vm->code[vm->ir]->op;
	r = vm->code[vm->ir]->r;
	l = vm->code[vm->ir]->l;
	m = vm->code[vm->ir]->m;
	
	// Exec instruction
	switch (op)
	{
		case OP_LIT:
			if (willOverflowRegister(r))		return HALT_REG;

			vm->rf[r] = m;
			break;
		
		case OP_RTN:
			vm->sp = vm->bp;
			vm->bp = vm->stack[vm->sp + 2];
			vm->pc = vm->stack[vm->sp + 3];
			vm->__AR[--vm->__AR_CNT] = 0;
			break;

		case OP_LOD:
			base = b(l, vm->bp, vm->stack);
			if (base == 1) base = 0;

			if (willOverflowStack(base + m))	return HALT_STACK;
			if (willOverflowRegister(r))		return HALT_REG;

			vm->rf[r] = vm->stack[base + m];
			break;

		case OP_STO:
			base = b(l, vm->bp, vm->stack);
			if (base == 1) base = 0;

			if (willOverflowStack(base + m))	return HALT_STACK;
			if (willOverflowRegister(r))		return HALT_REG;

			vm->stack[base + m] = vm->rf[r];
			break;

		case OP_CAL:
			base = b(l, vm->bp, vm->stack);
			
			if (willOverflowStack(vm->sp + 4))	return HALT_STACK;
			if (willOverflowAR(vm->__AR_CNT))	return HALT_AR;

			vm->stack[vm->sp] = 0;				// RV
			vm->stack[vm->sp + 1] = base;		// SL
			vm->stack[vm->sp + 2] = vm->bp;		// DL
			vm->stack[vm->sp + 3] = vm->pc;		// RA
			
			vm->pc = m;							// Set Program Counter
			vm->bp = vm->sp;					// Set Base Pointer
			vm->__AR[vm->__AR_CNT++] = vm->sp;	// Track AR
			break;
		
		case OP_INC:
			if (willOverflowStack((vm->sp += m)))
				return HALT_STACK;
			break;
		
		case OP_JMP:
			vm->pc = m;
			break;

		case OP_JPC:
			if (willOverflowRegister(r))		return HALT_REG;
			if (!vm->rf[r])						vm->pc = m;
			break;

		case OP_OUT:
			if (willOverflowRegister(r))		return HALT_REG;
			
			// Print vm data before SIO OUT
			if (vmFlag) printState(vm);

			// SIO OUT
			OUT("%d\n", vm->rf[r]);
			
			return CONTINUE;
		
		case OP_RIN:
			if (willOverflowRegister(r))		return HALT_REG;

			// Output to stdout
			if (vmFlag)
			{
				// Change for print
				vm->pc--;

				// Prompt and capture
				OUT("%s", "");
				fprintf(stdout, "Input integer = ");
				fscanf(stdin, "%d", vm->rf + r);
				fflush(stdin);
				fprintf(stdout, "\n");

				// Print the Instruction
				printInstruction(vm);

				// Restore PC
				vm->pc++;
			}

			// Output to stderr
			else
			{
				WARN("%s", "");
				fprintf(stderr, "Input integer = ");
				fscanf(stdin, "%d", vm->rf + r);
				fflush(stdin);
				fprintf(stderr, "\n");
			}
			break;
		
		case OP_EXT:
			return HALT_EXIT;

		case OP_NEG:
			if (willOverflowRegister(r))		return HALT_REG;
			
			vm->rf[r] *= -1;
			break;
		
		case OP_ADD:
			if (willOverflowRegister(r) ||
				willOverflowRegister(l) ||
				willOverflowRegister(m))
				return HALT_REG;

			vm->rf[r] = vm->rf[l] + vm->rf[m];
			break;

		case OP_SUB:
			if (willOverflowRegister(r) ||
				willOverflowRegister(l) ||
				willOverflowRegister(m))
				return HALT_REG;
				
			vm->rf[r] = vm->rf[l] - vm->rf[m];
			break;
			
		case OP_MUL:
			if (willOverflowRegister(r) ||
				willOverflowRegister(l) ||
				willOverflowRegister(m))
				return HALT_REG;
				
			vm->rf[r] = vm->rf[l] * vm->rf[m];
			break;

		case OP_DIV:
			if (willOverflowRegister(r) ||
				willOverflowRegister(l) ||
				willOverflowRegister(m))
				return HALT_REG;
				
			vm->rf[r] = vm->rf[l] / vm->rf[m];
			break;

		case OP_ODD:
			vm->rf[r] %= 2;
			break;

		case OP_MOD:
			if (willOverflowRegister(r) ||
				willOverflowRegister(l) ||
				willOverflowRegister(m))
				return HALT_REG;
				
			vm->rf[r] = vm->rf[l] % vm->rf[m];
			break;

		case OP_EQL:
			if (willOverflowRegister(r) ||
				willOverflowRegister(l) ||
				willOverflowRegister(m))
				return HALT_REG;
				
			vm->rf[r] = vm->rf[l] == vm->rf[m];
			break;
		case OP_NEQ:
			if (willOverflowRegister(r) ||
				willOverflowRegister(l) ||
				willOverflowRegister(m))
				return HALT_REG;
				
			vm->rf[r] = vm->rf[l] != vm->rf[m];
			break;
			
		case OP_LSS:
			if (willOverflowRegister(r) ||
				willOverflowRegister(l) ||
				willOverflowRegister(m))
				return HALT_REG;
				
			vm->rf[r] = vm->rf[l] < vm->rf[m];
			break;
			
		case OP_LEQ:
			if (willOverflowRegister(r) ||
				willOverflowRegister(l) ||
				willOverflowRegister(m))
				return HALT_REG;
				
			vm->rf[r] = vm->rf[l] <= vm->rf[m];
			break;
			
		case OP_GTR:
			if (willOverflowRegister(r) ||
				willOverflowRegister(l) ||
				willOverflowRegister(m))
				return HALT_REG;
				
			vm->rf[r] = vm->rf[l] > vm->rf[m];
			break;
			
		case OP_GEQ:
			if (willOverflowRegister(r) ||
				willOverflowRegister(l) ||
				willOverflowRegister(m))
				return HALT_REG;
				
			vm->rf[r] = vm->rf[l] >= vm->rf[m];
			break;

		default: return HALT_OP;
	}

	// Print vm data after execution
	if (vmFlag) printState(vm);

	return CONTINUE;
}

// Fetches the next op in the vm
// Assumes vm in its entirety is valid 
void fetchOp(vm_t *vm, int printFlag)
{
	// Fetch
	vm->ir = vm->pc;

	// Print instruction
	// [PC , op, l, m]
	if (printFlag && vm->code[vm->ir]->op != OP_RIN)
		printInstruction(vm);

	// Increment PC after print
	vm->pc++;
}

void run(instruction_t *code[], int printFlag)
{
	vm_t *vm;
	int halt;

	if (!(vm = initVMWithCode(code)))
		DIE("%s", "initialization failed (mem allocation)");

	if (printFlag)
	{
		printDecodedInstructions(vm);
		initialPrint(vm);
	}

	// Run virtual machine
	do {
		// Fetch Instruction
		fetchOp(vm, printFlag);

		// Execute
		halt = executeOp(vm, printFlag);
	} while (!halt);

	// Check if exited prematurely
	if (halt - 1)		printHaltReason(halt);
	else if (printFlag)	printState(vm);

	// Cleanup
	destroyVM(vm);
}

// int main(int argc, char *argv[])
// {
// 	vm_t *vm;
// 	int halt;

// 	if (argc < 2)
// 		DIE("%s", "missing input file execution argument");

// 	if (!(vm = initVM()))
// 		DIE("%s", "initialization failed (mem allocation)");

// 	loadFile(argv[1], vm->code);
// 	printDecodedInstructions(vm);
// 	initialPrint(vm);

// 	// Run virtual machine
// 	do {
// 		// Fetch Instruction
// 		fetchOp(vm, 1);

// 		// Execute
// 		halt = executeOp(vm, 1);
// 	} while (!halt);

// 	// Check if exited prematurely
// 	if (halt - 1)	printHaltReason(halt);
// 	else 			printState(vm);

// 	// Cleanup
// 	destroyVM(vm);

// 	// Exit properly
// 	return 0;
// }