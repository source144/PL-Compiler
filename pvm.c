// Gonen Matias
// COP 3402, Spring 2020
// NID: go658748
// Version: 03/31/2020 12:38AM

// FIXME Memory Management
// TODO: Check for underflow?

// NOTE: All of the constants, structs and
// their methods are defined in pvm.h
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
	"GEQ",
	"REG_L",
	"REG_R",
	"REG_B"
};

#define FORMAT_HDR1(...)		printf("Line   OP     R  L  M\n")
#define FORMAT_HDR2(...)		printf("                         PC   BP    SP    REGISTERS[%d]\n", NUM_REGISTERS)
#define FORMAT_INIT(...)		printf("Initial values           %-3d   %-2d    %-2d   ", __VA_ARGS__)
#define FORMAT_DATA(...)		printf("   %-3d   %-2d    %-2d   ", __VA_ARGS__)
#define FORMAT_INST(...)		printf("%-3d  %-6s  %d  %d  %-3d", __VA_ARGS__)
#define FORMAT_INST_LNG(...)	printf("%-4d   %-5s  %d  %d  %-3d", __VA_ARGS__)

#define _FORMAT__PTRS	"   %-3d   %-2d    %-2d   "
#define _FORMAT__INST	"%-4d   %-5s  %d  %d  %-3d"
#define _FORMAT__INST1	"%-3d  %-6s  %d  %d  %-3d"

char *__ptrs;
char *__instrct;
int __sz;
int __sp;
int __bp;
int _indicator = 0;

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
	int i, buf;

	// Append PC, BP and SP to line string
	sprintf(__ptrs, "   %-3d   %-2d    %-2d   ",  vm->pc, vm->bp, vm->sp);

	if (_indicator)
	{
		// Indicator buffer spaces
		for (i = 0; __ptrs[i]; i++)
			printf(" ");

		for (i = 0; __instrct[i]; i++)
			printf(" ");

		// Get total distance until indicator
		for (buf = i = 0; i < vm->__baseReg; i++)
			buf += numDigits(vm->rf[i]) + 2;
			
		for (i = 0; i < buf; i++)
			printf(" ");
		printf(",\n");
	}
	
	// Print the decoded instructions
	// and SP, BP... etc.
	printf("%s", __instrct);
	printf("%s", __ptrs);

	// Continue printing state
	printRegisters(vm->rf);
	printStack(vm);

	// Actually print the stack
	printf("%s\n", __ptrs);

	// TODO:
	// Print SP & BP indicators
	if (_indicator)
	{
		for (i = 0; i < __sp; i++)
			printf("%c", i == __bp ? '^' : ' ');
		printf("%s\n", __sp == __bp ? "^;" : ";");
	}

	printf("\n");
}

// Prints the register file of a vm
// Assumes the register file is from a valid vm
void printRegisters(int rf[NUM_REGISTERS])
{
	int i, end = NUM_REGISTERS - 1;

	// Print registers	
	for (i = 0; i < end; i++)
		printf("%d  ", rf[i]);
	printf("%d\n", rf[end]);
}

// Prints the stack of a vm
// Assumes vm in its entirety is valid 
void printStack(vm_t *vm)
{
	int i, j, end = vm->sp - 1, _digits, _i;
	sprintf(__ptrs, "Stack: ");

	// printf("Stack: ");

	if (!vm->sp)
	{
		// printf("\n       ;\n");
		return;
	}

	j = 0;
	_i = 7;
	sprintf(__ptrs + _i, "%d ", vm->stack[0]);

	// printf("\n%s\n",__ptrs);
	// printf("%d ", vm->stack[0]);
	// printf("\nNum digits: %d\n", _i);

	// Init SP & BP Indicator buffer
	_digits = numDigits(vm->stack[0]);
	__bp = __sp = 9 + _digits;
	_i += _digits + 1;
	
	// printf("\nstrlen = %d\n", strlen(__ptrs));
	// printf("_i = %d\n", _i);

	for (i = 1; i < end; i++)
	{
		// TODO:
		if (_i + 3 >= __sz)
			resizeIndicator();

		// Get number of digits
		_digits = numDigits(vm->stack[i]);
		
		if (vm->__AR[j] && vm->__AR[j] == i && ++j)
		{
			// printf("|");

			// TODO:
			sprintf(__ptrs + _i, "|");
			
			// printf("\nstrlen = %d\n", strlen(__ptrs));
			// printf("i = %d\n", _i);

			// Increment SP & BP buffers
			if (i < vm->sp)
				__sp++;
			if (i <= vm->bp)
				__bp++;
			_i++;
		}
		// printf(" %d ", vm->stack[i]);

		// TODO
		sprintf(__ptrs + _i, " %d ", vm->stack[i]);



		// Update SP & BP buffers
		if (i < vm->sp)
			__sp += _digits + 2;
		if (i < vm->bp)
			__bp += _digits + 2;
		_i += _digits + 2;

		// printf("\nstrlen = %d\n", strlen(__ptrs));
		// printf("_i = %d\n", _i);
	}

	
	// Get number of digits
	_digits = numDigits(vm->stack[end]);

	// Last element
	if (vm->__AR[j++] == end && ++__sp)
	{
		sprintf(__ptrs + _i, "|");

		// Increment SP & BP buffers
		if (i < vm->sp)
			__sp++;
		if (i <= vm->bp)
			__bp++;
		_i++;
	}

	sprintf(__ptrs + _i, " %d\n", vm->stack[end]);
	__ptrs[_i += _digits + 1] = '\0';

	// Update SP & BP buffers
	if (i < vm->sp)
		__sp += _digits + 1;
	if (i < vm->bp)
		__bp += _digits + 1;
}

// Prints the current instruction that
// that is executed by the vm
// Assumes vm in its entirety is valid 
void printInstruction(vm_t *vm)
{
	// Format into str
	sprintf(__instrct, _FORMAT__INST1, vm->pc, getOPStr(vm->code[vm->ir]->op),
			vm->code[vm->ir]->r, vm->code[vm->ir]->l, vm->code[vm->ir]->m);
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
		if (opcode < 28)	return OPSTRINGS[opcode - 3];
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
	int base, op, r, l, m, shouldPrint = 1;

	// Readability purposes:
	op = vm->code[vm->ir]->op;
	r = vm->code[vm->ir]->r;
	l = vm->code[vm->ir]->l;
	m = vm->code[vm->ir]->m;

	// Adjust base reg dynamically
	r += vm->__baseReg;
	
	if (op == OP_NEG
		|| op == OP_ADD
		|| op == OP_SUB
		|| op == OP_MUL
		|| op == OP_DIV
		|| op == OP_ODD
		|| op == OP_MOD
		|| op == OP_EQL
		|| op == OP_NEQ
		|| op == OP_LSS
		|| op == OP_LEQ
		|| op == OP_GTR
		|| op == OP_GEQ)
	{
		l += vm->__baseReg;
		m += vm->__baseReg;
	}

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
			WARN("%d\n", vm->rf[r]);
			
			// Output to stdout because 
			// stdout might be directed to a file
			if (vmFlag)
				OUT("%d\n", vm->rf[r]);


			return CONTINUE;
		
		case OP_RIN:
			if (willOverflowRegister(r))		return HALT_REG;


			// Display promts in stderr
			fflush(stdin);
			WARN("%s", "");
			fprintf(stderr, "Input integer = ");

			// Capture
			fscanf(stdin, "%d", vm->rf + r);
			fflush(stdin);
			fprintf(stderr, "\n");

			// Output to stdout because 
			// stdout might be directed to a file
			if (vmFlag)
			{
				// Change for print
				vm->pc--;

				// Print out captured value and prompt to stdin
				OUT("Input integer = %d\n", vm->rf[r]);
				printInstruction(vm);

				// Restore PC
				vm->pc++;
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

		// Set base register from literal
		case REG_L:
			if (willOverflowRegister(m))
				return HALT_REG;

			shouldPrint = 0;
			if (vmFlag) printState(vm);

			// __baseRegister = m + __baseRegister
			vm->__baseReg = m + vm->__baseReg;
			break;

		// Set base from register to the
		// previous base register (0 if none)
		// TODO: throw error if no previous base register?
		case REG_R:
			if (willOverflowRegister(r))
				return HALT_REG;
			
			shouldPrint = 0;
			if (vmFlag) printState(vm);

			// __baseRegister = REG[__baseRegister - 1] (or 0 if no previous)
			vm->__baseReg = vm->__baseReg ? vm->rf[vm->__baseReg - 1] : 0;
			break;
		
		// Load base to register operation
		case REG_B:
			if (willOverflowRegister(r))
				return HALT_REG;
			
			shouldPrint = 0;
			if (vmFlag) printState(vm);

			// REG[r] = __baseRegister
			vm->rf[r] = vm->__baseReg;
			break;

		default: return HALT_OP;
	}

	// Print vm data after execution
	if (shouldPrint && vmFlag) printState(vm);

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

void initIndicator(void)
{
	__sz = LINE_BUFFER;
	__instrct = calloc(__sz, sizeof(char));
	__ptrs = calloc(__sz, sizeof(char));
}

void resizeIndicator(void)
{
	realloc(__ptrs, (__sz *= 2) * sizeof(char));
	realloc(__instrct, __sz * sizeof(char));
}

int main(int argc, char *argv[])
{
	int halt, i, a, v, loaded;
	char *in;
	vm_t *vm;

	// Init VM
	if (!(vm = initVM()))
		DIE("%s", "\nEXITED - Initialization of VM failed (mem allocation)");

	// Init to 0 (NULL)
	loaded = v = a = 0;

	// Process args and directives
	for (i = 1; i < argc; i++)
	{
		// Process Directives
		if (argv[i][0] == FLAG)
		{
			switch (argv[i][1])
			{
				// -l  -  Print lexemes (stdout)
				case FLAG_VM:
					v = 1;
					break;
					
				// -a  -  Print machinecode (stdout)
				case FLAG_ASMBLY:
					a = 1;
					break;

				// -i  -  Print indicators (SP, BP, RP) (stdout)
				case FLAG_IDCTR:
					_indicator = 1;
					break;

				default:
					// Free memory if needed
					if (vm)
						destroyVM(vm);
					// "Supported Dircetives:"
					// "-v			Prints VM State on each cycle to STDOUT
					// "-a			Prints Decoded Machine Code Instructions to STDOUT
					DIE("\nEXITED - Unkown Directive \"%s\"!\n\n%s\n%c%c\t\t%s\n%c%c\t\t%s\n", argv[i],
						"Supported Dircetives:", FLAG, FLAG_VM, "Prints VM State on each cycle to STDOUT",
						FLAG, FLAG_ASMBLY, "Prints Decoded Machine Code Instructions to STDOUT");
					break;
			}
		}
		// Load Machine code to VM and exit if an error occurs
		else if (!loaded)
		{
			// Load Machine code
			loadFile(in = argv[i], vm->code);

			// Change loaded state to true
			loaded = 1;
		}
	}
	
	if (!loaded)
		DIE("%s", "\nEXITED - Missing Machine Code argument!");
	
	// Print Decoded Machine Code 
	if (a)
	{
		// Print Header
		printf("\nDecoded Machine Instructions:\n");
		printf("(\"%s\")\n", in);
		
		// Print Brackets (-)...
		for (i = -4; i < 0 || in[i]; i++)
			printf("-");

		// Print Decoded Instructions
		printf("\n");
		printDecodedInstructions(vm);
		printf("\n");
	}

	// Print initial VM State
	if (v)
	{
		initIndicator();
		printf("\n");
		printf("\nVM State Log:\n");
		printf("-------------\n");
		printf("\nBase Pointer (SP):       ^");
		printf("\nStack Pointer (SP):      ;");
		printf("\nRegister Pointer (RP):   ,");
		printf("\n");
		initialPrint(vm);
	}

	// Run virtual machine
	do {
		// Fetch Instruction
		fetchOp(vm, v);

		// Execute
		halt = executeOp(vm, v);
	} while (!halt);

	// Check if exited prematurely
	if (halt - 1)	printHaltReason(halt);
	else if (v) 	printState(vm);

	// Cleanup
	destroyVM(vm);
	free(__instrct);
	free(__ptrs);

	// Exit properly
	return 0;
}

#pragma region "DEPRECATED"
// // DEPRECATED // //
// void run(instruction_t *code[], int printFlag)
// {
// 	vm_t *vm;
// 	int halt;

// 	if (!(vm = initVMWithCode(code)))
// 		DIE("%s", "initialization failed (mem allocation)");

// 	if (printFlag)
// 	{
// 		printDecodedInstructions(vm);
// 		initialPrint(vm);
// 	}

// 	// Run virtual machine
// 	do {
// 		// Fetch Instruction
// 		fetchOp(vm, printFlag);

// 		// Execute
// 		halt = executeOp(vm, printFlag);
// 	} while (!halt);

// 	// Check if exited prematurely
// 	if (halt - 1)		printHaltReason(halt);
// 	else if (printFlag)	printState(vm);

// 	// Cleanup
// 	destroyVM(vm);
// }
#pragma endregion