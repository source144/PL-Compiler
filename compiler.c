// Gonen Matias
// COP 3402, Spring 2020
// NID: go658748
// Version: 03/27/2020 05:36PM

// TODO: Organize code
// TODO: add global buffers (conserve memory and runtime?)
// TODO: add comments to functions

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "compiler.h"
#include "pvm.h"

#define WARN(format, ...) fprintf(stderr, "PL/0 COMPILER: " format "\n", __VA_ARGS__)
#define OUT(format, ...) fprintf(stdout, "PL/0 COMPILER: " format "\n", __VA_ARGS__)
#define DIE(format, ...) WARN(format, __VA_ARGS__), exit(EXIT_FAILURE)

// Globally Defined
string_t *__content;

#pragma region "Function Prototypes"

long getFileLength(FILE *fp);
instruction_t *_destroyInstruction(instruction_t *instr);
int program(list_t *lexims, symbol_t *tbl, instruction_t *code);
node_t *block(int lvl, int tblIdx, int *codeIdx, node_t *token, symbol_t *tbl, instruction_t *code);
node_t *constdec(int lvl, int *tblIdx, int *m, node_t *token, symbol_t *tbl);
node_t *vardec(int lvl, int *tblIdx, int *m, node_t *token, symbol_t *tbl);
node_t *statement(int lvl, int *tblIdx, int *codeIdx, node_t *token, symbol_t *tbl, instruction_t *code);
node_t *condition(int lvl, int *tblIdx, int *codeIdx, node_t *token, symbol_t *tbl, instruction_t *code);
node_t *expression(int r, int lvl, int *tblIdx, int *codeIdx, node_t *token, symbol_t *tbl, instruction_t *code);
node_t *term(int r, int lvl, int *tblIdx, int *codeIdx, node_t *token, symbol_t *tbl, instruction_t *code);
node_t *factor(int r, int lvl, int *tblIdx, int *codeIdx, node_t *token, symbol_t *tbl, instruction_t *code);
void emitCode(int *idx, int op, int r, int l, int m, instruction_t *code);

#pragma endregion

#pragma region "List Methods (list_t)"

list_t *createList(void)
{
	return calloc(1, sizeof(list_t));
}

list_t *destroyList(list_t *list)
{
	node_t *node, *next;
	if (!list)
		return NULL;

	for (node = list->head; node; node = next)
	{
		next = node->next;
		destroyString(node->data);
		free(node);
	}
	free(list);

	return NULL;
}

void add(list_t *l, int line, int col, int idx, string_t *data, int token)
{
	node_t *node;

	if (!(node = createNode(data, line, col - data->length, idx, token)))
		DIE("%s", "EXITED - Bad memory allocation (add)");

	if (!l->tail)
		l->head = node;
	else l->tail->next = node;
	
	l->tail = node;
	l->size++;
}

#pragma endregion

#pragma region "String Methods (string_t)"

string_t *destroyString(string_t *str)
{
	if (!str)
		return NULL;
		
	free(str->charAt);
	free(str);

	return NULL;
}

#pragma endregion

#pragma region "Node Methods (node_t)"

node_t *createNode(string_t *data, int line, int col, int idx, int token)
{
	node_t *node;

	if (!(node = calloc(1, sizeof(node_t))))
		DIE("%s", "EXITED - Bad memory allocation (add)");

	node->data = data;
	node->token = token;
	node->__line = line;
	node->__col = col;
	node->__idx = idx;

	return node;	
}

#pragma endregion

#pragma region "Token Helpers"

int getIdentifierToken(string_t *identifier)
{
	int i;

	for (i = 0; i < NUM_RESERVED; i++)
		if (!strcmp(identifier->charAt, RESERVED[i]))
			return getReservedToken(i);
		
	return identsym;
}

#pragma endregion

#pragma region "Helpers"

int isValidSymbol(char c)
{
	for (int i = 0; SYMBOLS[i]; i++)
		if (c == SYMBOLS[i])
			return 1;

	return 0;
}

// Handle line and column count on a new line encounter
void newLine(int *ln, int *col)
{
	(*ln)++;
	*col = 1;
}

#pragma endregion

#pragma region "Read Functions/Tokenizers"

string_t *readIdentifier(string_t *content, int *idx, int *ln, int *col, int *lnIdx)
{
	// FIXME: use a global/constant buffer

	int i = 0;
	char buffer[MAX_IDENT_LEN];
	string_t *identifier;
	
	// Read identifier from content
	do 
	{
		if (i >= MAX_IDENT_LEN)
			lineError("SCANNER INTERRUPT", "Identifier too long", *idx - 1, *lnIdx, *ln, *col - i);

		buffer[i++] = content->charAt[*idx];

		// Increment column number
		++(*col);

		// File ends
		if (++(*idx) >= content->length)
			break;
			
	} while (isalnum(content->charAt[*idx]));

	// Allocate memory for identifier
	if (!(identifier = calloc(1, sizeof(string_t))) ||
		!(identifier->charAt = calloc(i + 1, sizeof(char))))
		DIE("%s", "\nSCANNER INTERRUPT - Bad memory allocation (identifier read)");
		
	// Copy identifier from buffer
	identifier->length = i;
	for (i = 0; i < identifier->length; i++)
		identifier->charAt[i] = buffer[i];

	return identifier;
}

string_t *readNumber(string_t *content, int *idx, int *ln, int *col, int *lnIdx)
{
	int i = 0;
	char buffer[MAX_DECIM_LEN];
	string_t *number;
	
	// Read number from content
	do 
	{
		if (i >= MAX_DECIM_LEN)
			lineError("SCANNER INTERRUPT", "Number too long", *idx - 1, *lnIdx, *ln, *col - i);

		buffer[i++] = content->charAt[*idx];

		// Increment column number
		++(*col);

		// File ends
		if (++(*idx) >= content->length)
			break;

	} while (isdigit(content->charAt[*idx]));

	if (isalpha(content->charAt[*idx]))
		lineError("SCANNER INTERRUPT", "Invalid identifier name (begins with a number)",
			*idx - 1, *lnIdx, *ln, *col - i);

	// Allocate memory for number
	if (!(number = calloc(1, sizeof(string_t))) ||
		!(number->charAt = calloc(i + 1, sizeof(char))))
		DIE("%s", "\nSCANNER INTERRUPT - Bad memory allocation (number read)");
		
	// Copy number from buffer
	number->length = i;
	for (i = 0; i < number->length; i++)
		number->charAt[i] = buffer[i];

	return number;
}

string_t *readSymbol(string_t *content, int *idx, int *ln, int *col, int *lnIdx)
{
	int i = 0;
	char buffer[MAX_SYMBL_LEN];
	string_t *symbol;

	// increment column number
	++(*col);

	// Always add first char to buffer
	switch ((buffer[i++] = content->charAt[*idx]))
	{
		case '/':
			// Increment column number
			++(*col);

			if (++(*idx) >= content->length)
				break;
			
			// Check if it's a comment
			if (content->charAt[*idx] == '*')
			{
				// Ignore comment but keep track
				// of line and column number :)
				do
				{
					do
					{
						// Handle line and column count
						if (content->charAt[i] == '\n')
						{
							newLine(ln, col);
							*lnIdx = *idx + 1;
						}
						else ++(*col);
					}
					while (++(*idx) < content->length && content->charAt[*idx] != '*');

					// Must have another character after '*'
					if (++(*idx) >= content->length)
						lineError("SCANNER INTERRUPT", "Source code ends abrubtly", *idx - 1, *lnIdx, *ln, *col);

					// Check if comment ended	
					if (content->charAt[*idx] == '/') break;
				} while (1);

				// Increment column number
				++(*col);

				// Increment idx
				++(*idx);

				// Comments are ignored
				// therefore return NULL
				return NULL;
			}
			break;
		
		case ':':
			// Increment column number
			++(*col);

			if (++(*idx) >= content->length)
				lineError("SCANNER INTERRUPT", "Source code ends abrubtly", *idx - 1, *lnIdx, *ln, *col - 1);
			
			if (content->charAt[*idx] != '=')
				lineError("SCANNER INTERRUPT", "A column (:) must be followed by an equals sign (=)",
					*idx - 1, *lnIdx, *ln, *col - 1);

			// Store in buffer and increment idx
			buffer[i++] = content->charAt[*idx];
			
			break;

		case '<':
		case '>':
			// Increment column number
			++(*col);

			if (++(*idx) >= content->length)
				lineError("SCANNER INTERRUPT", "Source code ends abrubtly", *idx - 1, *lnIdx, *ln, *col - 1);
			
			if ((content->charAt[*idx - 1] != '<' || content->charAt[*idx] != '>') 
				&& content->charAt[*idx] != '=')
			{
				--(*idx);
				--(*col);
			}
			else buffer[i++] = content->charAt[*idx];

			// printf("buffer(%d): \"%s\"\n", i, buffer);
			break;
	}

	// Allocate memory for symbol
	if (!(symbol = calloc(1, sizeof(string_t))) ||
		!(symbol->charAt = calloc(i + 1, sizeof(char))))
		DIE("%s", "\nSCANNER INTERRUPT - Bad memory allocation (symbol read)");

	// Copy symbol from buffer
	symbol->length = i;
	for (i = 0; i < symbol->length; i++)
		symbol->charAt[i] = buffer[i];

	// Increment idx
	++(*idx);

	return symbol;
}

#pragma endregion

#pragma region "Lexime List add helpers"

void addIdentifier(list_t *l, int line, int col, int idx, string_t *identifier)
{
	if (identifier)
		add(l, line, col, idx, identifier, getIdentifierToken(identifier));
}

void addNumber(list_t *l, int line, int col, int idx, string_t *number)
{
	if (number)
		add(l, line, col, idx, number, numbersym);
}

void addSymbol(list_t *l, int line, int col, int idx, string_t *symbol)
{
	if (symbol)
		add(l, line, col, idx, symbol, getSymbolToken(symbol));
}

#pragma endregion

#pragma region "File IO Methods (input)"

// Returns the length of a the contents a file
// NOTE: assumes [*fp] is a valid [FILE] pointer.
long getFileLength(FILE *fp)
{
	int res;

	// Reach EOF
	fseek(fp, 0, SEEK_END);

	// Save position indicator
	res = ftell(fp);
		
	// Reset position indicator
	fseek(fp, 0, SEEK_SET);

	return res;
}

// Stores the contents of a file into a new [string_t] struct.
// Returns a pointer to the new [string_t].
string_t *readFile(char *filename)
{
	FILE *fp;
	string_t *str = calloc(1, sizeof(string_t));
	int fLen;
		
	if (!(fp = fopen(filename, "r")))
		DIE("\nINTERRUPTED - Unable to read source code file \"%s\"!", filename);


	// Get the length of the file
	fLen = getFileLength(fp);

	// Allocate memory to save file into memory
	str->length = fLen;
	str->charAt = calloc(str->length + 1, sizeof(char));

	// Read contents of file into str
	fread(str->charAt, sizeof(char), fLen, fp);

	return str;
}

#pragma endregion

#pragma region "Print Methods (output)"

void printLeximTable(list_t *leximList)
{
	int i;
	node_t *node;

	printf("Lexeme Table:\n");
	printf("lexeme      token type\n");
	for (node = leximList->head; node; node = node->next)
	{
		printf("%s ", node->data->charAt);
		for (i = 0; i < MAX_IDENT_LEN - node->data->length; i++)
			printf(" ");
		printf("%d\n", node->token);
	}
}

void printLeximList(list_t *leximList)
{
	int i;
	node_t *node;

	printf("Lexeme List:\n");
	for (node = leximList->head; node && node != leximList->tail; node = node->next)
	{
		printf("%d ", node->token);
		if (node->token == identsym || node->token == numbersym)
			printf("%s ", node->data->charAt);
		
		printf("%c ", SEPARATOR);
	}

	// Tail node:
	printf("%d", node->token);
	if (node->token == identsym || node->token == numbersym)
		printf(" %s", node->data->charAt);
}

#pragma endregion

void process(string_t *content, list_t *leximsList)
{
	string_t *buffer;
	int i, ln, col, lineIdx, tmp;

	for (col = ln = 1, lineIdx = i = 0; i < content->length && content->charAt[i]; )
	{
		// col = 1;

		// Handle line and column number
		while (isspace(content->charAt[i]))
		{
			// Update col number
			col++;

			if (content->charAt[i] == '\n')
			{
				newLine(&ln, &col);
				lineIdx = i + 1;
			}
			if (++i >= content->length) return;
		}

		if (isalpha(content->charAt[i]))
			addIdentifier(leximsList, ln, col, lineIdx, readIdentifier(content, &i, &ln, &col, &lineIdx));
			
		else if (isdigit(content->charAt[i]))
			addNumber(leximsList, ln, col, lineIdx, readNumber(content, &i, &ln, &col, &lineIdx));

		else if (isValidSymbol(content->charAt[i]))
			addSymbol(leximsList, ln, col, lineIdx, readSymbol(content, &i, &ln, &col, &lineIdx));
		
		// Handle invalid symbols
		else lineError("SCANNER INTERRUPT", "Invalid Symbol", i, lineIdx, ln, col);
	}
}

void lineError(char *title, char *msg, int idx, int lineIdx, int lineNum, int colNum)
{
	// Specific message and exit
	OUT("\n%s - %s", title, msg);
	printIndicator(lineIdx, lineNum, colNum);
	exit(EXIT_FAILURE);
}

void printIndicator(int lineIdx, int lineNum, int colNum)
{
	// Spaces for nice formatting
	// (NOTE: 8 chars by default: LINE X, X: *col* ^ )
	// LINE 5, 5:  x = y + 56;
	int i, j, numSpaces = 8 + numDigits(lineNum) + numDigits(colNum) + colNum;

	// Source code line with error, and spaces for nice formatting (+1 for NUL terminator)
	char line[__content->length - lineIdx + 1], indicator[numSpaces + 2];

	// Set line and space buffer
	for (i = lineIdx; i < __content->length && __content->charAt[i] && __content->charAt[i] != '\n'; i++)
	{
		j = i - lineIdx;
		// Every white space is going to be a simple space
		if (isspace(__content->charAt[i]))
			line[j] = ' ';
		else line[j] = __content->charAt[i];

		if (j < numSpaces)
			indicator[j] = ' ';
	}
	line[j + 1] = '\0'; // Terminate

	// Continue filling spaces buffer if needed
	for (; j < numSpaces; j++)
		indicator[j] = ' ';

	// The indicator (^) itself and termination
	indicator[numSpaces] = '^';
	indicator[numSpaces + 1] = '\0';

	fprintf(stderr, "LINE %d, %d: %s\n%s\n", lineNum, colNum, line, indicator);
}
#pragma region "Parser Helpers"

// Parser Error
void error(node_t *token, char *msg, int e)
{
	if (!token)
	{
		if (msg)	DIE("\nINTERRUPTED - %s", msg);
		else		DIE("\nINTERRUPTED - Unexpected Error (%d)", e);
	}

	// Print Error Header
	OUT("%s", "");
	fprintf(stderr, "INTERRUPTED - %s", msg);

	// Generic Error
	if (!msg)
	{
		if (e == 1)	fprintf(stderr, "Use \"=\" instead of \":=\" (ERR %d)", e);
		else if (e == 2)	fprintf(stderr, "\"=\" must be followed by a number (ERR %d).\n", e);
		else if (e == 3)	fprintf(stderr, "Identifier must be followed by = (ERR %d).\n", e);
		else if (e == 4)	fprintf(stderr, "const, var, procedure must be followed by identifier (ERR %d).\n", e);
		else if (e == 5)	fprintf(stderr, "Semicolon or comma missing (ERR %d).\n", e);
		else if (e == 6)	fprintf(stderr, "Incorrect symbol after procedure declaration (ERR %d).\n", e);
		else if (e == 7)	fprintf(stderr, "Statement expected (ERR %d).\n", e);
		else if (e == 8)	fprintf(stderr, "Incorrect symbol after statement part in block (ERR %d).\n", e);
		else if (e == 9)	fprintf(stderr, "Period expected (ERR %d).\n", e);
		else if (e == 10)	fprintf(stderr, "Semicolon between statements missing (ERR %d).\n", e);
		else if (e == 11)	fprintf(stderr, "Undeclared identifier (ERR %d).\n", e);
		else if (e == 12)	fprintf(stderr, "Assignment to constant or procedure is not allowed (ERR %d).\n", e);
		else if (e == 13)	fprintf(stderr, "Assignment operator \":=\" expected (ERR %d).\n", e);
		else if (e == 14)	fprintf(stderr, "call must be followed by an identifier (ERR %d).\n", e);
		else if (e == 15)	fprintf(stderr, "Call of a constant or variable is meaningless (ERR %d).\n", e);
		else if (e == 16)	fprintf(stderr, "then expected (ERR %d).\n", e);
		else if (e == 17)	fprintf(stderr, "Semicolon or \"}\" expected (ERR %d).\n", e);
		else if (e == 18)	fprintf(stderr, "do expected (ERR %d).\n", e);
		else if (e == 19)	fprintf(stderr, "Incorrect symbol following statement (ERR %d).\n", e);
		else if (e == 20)	fprintf(stderr, "Relational operator expecte (ERR %d).\n", e);
		else if (e == 21)	fprintf(stderr, "Expression must not contain a procedure identifier (ERR %d).\n", e);
		else if (e == 22)	fprintf(stderr, "Right parenthesis missing (ERR %d).\n", e);
		else if (e == 23)	fprintf(stderr, "The preceding factor cannot begin with this symbol (ERR %d).\n", e);
		else if (e == 24)	fprintf(stderr, "An expression cannot begin with this symbol (ERR %d).\n", e);
		else if (e == 25)	fprintf(stderr, "This number is too large (ERR %d).\n", e);
	}
	else if (e > 0)	fprintf(stderr, "%s (ERR %d)\n", msg, e);
	else 			fprintf(stderr, "%s\n", msg);

	// Print Indicator and Exit
	printIndicator(token->__idx, token->__line, token->__col);
	exit(EXIT_FAILURE);
}

node_t *nextToken(node_t *token)
{
	if (!token)
		error(NULL, "Unable to process NULL token", -1);

	if (!token->next)
	{
		// Output will look nice if
		// we exit on missing period
		token->__col += 3;

		// Missing period token
		if (token->token == endsym)
			error(token, NULL, 9);
		
		// End abruptly
		else error(NULL, "Source code ends abrubtly, expected more tokens", -1);
	}
	
	return token->next;
}

int numDigits(int n)
{
	return floor(log10(abs(n))) + 1;
}

#pragma endregion

int program(list_t *lexims, symbol_t *tbl, instruction_t *code)
{
	node_t *token;
	int codeIdx = 0;

	if ((token = block(0, 0, &codeIdx, lexims->head, tbl, code))->token != periodsym)
		error(token, NULL, 9);

	return codeIdx;
}

node_t *block(int lvl, int tblIdx, int *codeIdx, node_t *token, symbol_t *tbl, instruction_t *code)
{
	// Block indexes
	int reserve, initTblIdx, initCodeIdx;

	if (lvl > MAX_LEXI_LEVELS)
	{
		error(token, NULL, 26);
	}

	// Prep Jump instruction
	reserve = 4; //  SP, BP, PC, RV
	tbl[initTblIdx = tblIdx++].addr = *codeIdx;

	// Emit Jump instruction
	emitCode(codeIdx, OP_JMP, 0, 0 ,0, code);

	do
	{
		if (token->token == constsym)
		{
			// const declaration
			do token = nextToken(token);
			while ((token = constdec(lvl, &tblIdx, &reserve, token, tbl))->token == commasym);
				
			if (token->token != semicolonsym)
				error(token, NULL, 5);
		}
		else if (token->token == varsym)
		{
			// var declaration
			do token = nextToken(token);
			while ((token = vardec(lvl, &tblIdx, &reserve, token, tbl))->token == commasym);

			if (token->token != semicolonsym)
				error(token, NULL, 5);
		}
		else
		{
			// TODO: error
			// TODO:	Unexpected Token
			error(token, "Unexpected token!", -1);
		}
	} while ((token = nextToken(token))->token == constsym || token->token == varsym);

	// Modify jump addresses
	code[tbl[initTblIdx].addr].m	= *codeIdx;
	tbl[initTblIdx].addr			= *codeIdx;

	// Emit INC instruction
	emitCode(codeIdx, OP_INC, 0, 0, reserve, code);
	
	// Run statements
	token = statement(lvl, &tblIdx, codeIdx, token, tbl, code);

	// If this is our main we exit
	if (!lvl)	emitCode(codeIdx, OP_EXT, 0, 0, 3, code);

	// Otherwise we return from block/call?
	else		emitCode(codeIdx, OP_RTN, 0, 0, 0, code);

	return token;
}

node_t *constdec(int lvl, int *tblIdx, int *m, node_t *token, symbol_t *tbl)
{
	// Unexpected Token
	if (token->token != identsym)
		error(token, NULL, 4);
	
	// Recall identifier name
	char *name = token->data->charAt;

	// Get next token
	token = nextToken(token);
	
	switch (token->token)
	{
		case eqlsym:
			// Get next token
			token = nextToken(token);

			// Next lexim should be a number
			if (token->token != numbersym)
				error(token, "Expected a number!", -1);

			// Enter to symbol table
			tableInsert(CONST, name, tblIdx, lvl, 0, atoi(token->data->charAt), tbl);
			break;
		
		case becomessym:
			error(token, NULL, 1);
			break;

		default:
			error(token, "Unexpected token!", -1);
			break;
	}

	return nextToken(token);
}

node_t *vardec(int lvl, int *tblIdx, int *m, node_t *token, symbol_t *tbl)
{
		// Unexpected Token
	if (token->token != identsym)
		error(token, NULL, 4);

	// Enter to symbol table
	tableInsert(VAR, token->data->charAt, tblIdx, lvl, m, -1, tbl);

	return nextToken(token);
}

node_t *statement(int lvl, int *tblIdx, int *codeIdx, node_t *token, symbol_t *tbl, instruction_t *code)
{
	int identIdx, thenDoIdx, conditionIdx, val;
	
	switch (token->token)
	{
		case identsym:
			// Lookup identifier
			identIdx = lookup(token->data->charAt, lvl, tblIdx, tbl);

			// Undeclared identifier
			if (identIdx == 0)
				error(token, NULL, 11);

			// Identifier has to be of a VAR
			else if (tbl[identIdx].kind != VAR)
				error(token, NULL, 12);

			// Next token has to be ":=" (Becomes Symbol)
			if ((token = nextToken(token))->token != becomessym)
				error(token, NULL, 13);

			// IDENT ":=" EXPRESSION
			token = expression(0, lvl, tblIdx, codeIdx, nextToken(token), tbl, code);

			// Emit STO instruction
			//! Fix the following:
			// FIXME: fix L and R of the instruction
			emitCode(codeIdx, OP_STO, 0, lvl - tbl[identIdx].level, tbl[identIdx].addr, code);
			
			// Expression already returns last token
			return token;
		
		case beginsym:
			token = statement(lvl, tblIdx, codeIdx, nextToken(token), tbl, code);

			while (token->token == semicolonsym && (token = nextToken(token))->token != endsym)
				token = statement(lvl, tblIdx, codeIdx, token, tbl, code);

			// do token = nextToken(token);
			// while ((token = statement(lvl, tblIdx, codeIdx, token, tbl, code))->token != endsym);

			if (token->token != endsym)
				error(token, NULL, 17);
			break;
		
		case ifsym:
			// Process condition from next token
			token = condition(lvl, tblIdx, codeIdx, nextToken(token), tbl, code);

			// Token after condition has to be a "then"
			if (token->token != thensym)
				error(token, NULL, 16);

			// Remember where JPC is located in the code
			thenDoIdx = *codeIdx;

			// Add JPC to the code
			emitCode(codeIdx, OP_JPC, 0, 0, 0, code);

			// Process (then) statement from next token
			token = statement(lvl, tblIdx, codeIdx, nextToken(token), tbl, code);

			// If condition is false, JPC skips the "then" statement(s)
			code[thenDoIdx].m = *codeIdx;

			// NOTE: If there was an else statement
			// we would add a jmp at the end of the "then" statement(s)
			// to skip the else statement(s)
			// and the original JPC would still skip the "then" statement(s)
			return token;
		
		case whilesym:
			// Renenber where we evaluate the condition
			conditionIdx = *codeIdx;

			// Process condition from next token
			token = condition(lvl, tblIdx, codeIdx, nextToken(token), tbl, code);

			// Token after condition has to be a "do"
			if (token->token != dosym)
				error(token, NULL, 18);

			// Remember where JPC is located in the code
			thenDoIdx = *codeIdx;

			// Add JPC to the code (after condition)
			emitCode(codeIdx, OP_JPC, 0, 0, 0, code);

			// Process (do) statement from next token
			token = statement(lvl, tblIdx, codeIdx, nextToken(token), tbl, code);

			// Add a jump back to the condition evaluation of the loop
			emitCode(codeIdx, OP_JMP, 0, 0, conditionIdx, code);

			// If condition is false, JPC skips the "do" statement(s)
			code[thenDoIdx].m = *codeIdx;

			return token;
		
		case readsym:
			// Get next token (Identifier)
			token = nextToken(token);

			// Identifier expected
			if (token->token != identsym)
				error(token, NULL, 4);

			// Lookup variable and check for Undeclared identifier
			if (!(identIdx = lookup(token->data->charAt, lvl, tblIdx, tbl)))
				error(token, NULL, 11);

			// Assignment allowed only to variables
			if (tbl[identIdx].kind != VAR)
				error(token, NULL, 14);

			// Read input to register 0 (SIO_IN)
			emitCode(codeIdx, OP_RIN, 0, 0, 0, code);

			// Now we need to save the input data
			// from the register into a variable (in the stack)
			emitCode(codeIdx, OP_STO, 0, lvl - tbl[identIdx].level, tbl[identIdx].addr, code);
			break;
		
		case writesym:
			// Get next token (Identifier)
			token = nextToken(token);

			// Identifier expected
			if (token->token != identsym)
				error(token, NULL, 4);

			// Lookup variable and check for Undeclared identifier
			if (!(identIdx = lookup(token->data->charAt, lvl, tblIdx, tbl)))
				error(token, NULL, 11);
			
			// LOD variable into register
			if (tbl[identIdx].kind == VAR)
				emitCode(codeIdx, OP_LOD, 0, lvl - tbl[identIdx].level, tbl[identIdx].addr, code);

			// LIT constant into register
			else if (tbl[identIdx].kind == CONST)
				emitCode(codeIdx, OP_LIT, 0, 0, tbl[identIdx].val, code);

			// Expected const or var
			else
				error(token, NULL, 21);

			// Output contents of register 0
			emitCode(codeIdx, OP_OUT, 0, 0, 0, code);
			break;

		case semicolonsym:
			printf("semicolumn\n");
			break;

		case varsym:
		case constsym:
			error(token, "declarations must be made on a block level", -1);

		default:
			error(token, "Unexpected token", -1);
	}
	return nextToken(token);
}

node_t *condition(int lvl, int *tblIdx, int *codeIdx, node_t *token, symbol_t *tbl, instruction_t *code)
{
	int whichOp, r, l, m;

	if (token->token == oddsym)
	{
		// Process expression from next token
		token = expression(r = l = 0, lvl, tblIdx, codeIdx, nextToken(token), tbl, code);
		
		// Odd: Reg[0] = Reg[0] % 2
		emitCode(codeIdx, OP_ODD, 0, 0, 0, code);
	}
	else
	{
		// Process first expression expression
		token = expression(r = l = 0, lvl, tblIdx, codeIdx, token, tbl, code);

		// Save the Arithmetic operation
		whichOp = token->token;

		// Process second expression expression
		token = expression(m = 1, lvl, tblIdx, codeIdx, nextToken(token), tbl, code);		

		switch (whichOp)
		{
			case eqlsym:
				emitCode(codeIdx, OP_EQL, r, l, m, code);
				break;

			case neqsym:
				emitCode(codeIdx, OP_NEQ, r, l, m, code);
				break;
			
			case lessym:
				emitCode(codeIdx, OP_LSS, r, l, m, code);
				break;

			case leqsym:
				emitCode(codeIdx, OP_LEQ, r, l, m, code);
				break;

			case gtrsym:
				emitCode(codeIdx, OP_GTR, r, l, m, code);
				break;

			case geqsym:
				emitCode(codeIdx, OP_GEQ, r, l, m, code);
				break;

			default:
				// Relational operator expected
				error(token, NULL, 20);
				break;
		}
	}

	return token;
}

node_t *expression(int r, int lvl, int *tblIdx, int *codeIdx, node_t *token, symbol_t *tbl, instruction_t *code)
{
	int l, m, negate;
	
	// TODO: check if r is in register bounds

	// Optional +/-
	if ((negate = (token->token == minussym)) || token->token == plussym)
	{
		// Process term (into register r) and get next token
		token = term(l = r, lvl, tblIdx, codeIdx, nextToken(token), tbl, code);

		// Perform negation if needed
		if (negate)
			// OP_NEG: Reg[R] = -Reg[R]
			emitCode(codeIdx, OP_NEG, r, 0, 0, code);
	}
	
	// Otherwise, just process term and get next token
	else token = term(l = r, lvl, tblIdx, codeIdx, token, tbl, code);

	while ((negate = (token->token == minussym)) || token->token == plussym)
	{
		// Process term (into register r + 1) and get next token
		token = term(m = r + 1, lvl, tblIdx, codeIdx, nextToken(token), tbl, code);
		
		// OP_SUB: Reg[R] = Reg[L] - Reg[M]
		if (negate)
			emitCode(codeIdx, OP_SUB, r, l, m, code);
		
		// OP_ADD: Reg[R] = Reg[L] + Reg[M]
		else
			emitCode(codeIdx, OP_ADD, r, l, m, code);
	}
	
	// We are always returning the
	// token following the last term
	return token;
}

node_t *term(int r, int lvl, int *tblIdx, int *codeIdx, node_t *token, symbol_t *tbl, instruction_t *code)
{
	int l, m, divide;
	node_t *tmpToken;

	// Process factor (into register r) and get next token
	token = factor(l = r, lvl, tblIdx, codeIdx, token, tbl, code);

	// Factor
	while ((divide = (token->token == slashsym)) || token->token == multsym)
	{
		// Process factor (into register r + 1) and get next token
		token = factor(m = r + 1, lvl, tblIdx, codeIdx, nextToken(token), tbl, code);

		if (divide)
			emitCode(codeIdx, OP_DIV, r, l, m, code);
		else
			emitCode(codeIdx, OP_MUL, r, l, m, code);
	}

	// We are always returning the
	// token following the last factor
	return token;
}

node_t *factor(int r, int lvl, int *tblIdx, int *codeIdx, node_t *token, symbol_t *tbl, instruction_t *code)
{
	int identIdx, num;

	switch (token->token)
	{
		case identsym:
			// Lookup variable and check for Undeclared identifier
			if (!(identIdx = lookup(token->data->charAt, lvl, tblIdx, tbl)))
				error(token, NULL, 11);

			// const
			if (tbl[identIdx].kind == CONST)
				emitCode(codeIdx, OP_LIT, r, 0, tbl[identIdx].val, code);				

			// var
			else if (tbl[identIdx].kind == VAR)
				emitCode(codeIdx, OP_LOD, r, lvl - tbl[identIdx].level, tbl[identIdx].addr, code);

			// Expected const or var
			else error(token, NULL, 21);
			break;
		
		case numbersym:
			num = atoi(token->data->charAt);
			if (numDigits(num) > MAX_DECIM_LEN)
				error(token, NULL, 25);

			emitCode(codeIdx, OP_LIT, r, 0, num, code);
			break;
		
		// "(" - (Left Parenthesis)
		case lparentsym:
			// Process expression after "(" (Left Parenthesis)
			token = expression(r, lvl, tblIdx, codeIdx, nextToken(token), tbl, code);

			// Token != ')' - (Right Parenthesis)
			if (token->token != rparentsym)
				error(token, NULL, 22);
			break;

		default:
			error(token, "Expected an identifier/number/expression!", -1);
			break;
	}

	return nextToken(token);
}

void emitCode(int *idx, int op, int r, int l, int m, instruction_t *code)
{
	if (*idx < MAX_CODE_LENGTH)
	{
		code[*idx].op = op;
		code[*idx].r = r;
		code[*idx].l = l;
		code[*idx].m = m;
	}
	else error(NULL, "Exceeded PL\\0's Machine Code line limit", -1);

	// Increment code index
	(*idx)++;
}

void tableInsert(int kind, char *name, int *idx, int lvl, int *addr, int val, symbol_t tbl[MAX_TABLE_SIZE])
{
	// Check if idx overflows symbol table
	if (*idx >= MAX_TABLE_SIZE)
		error(NULL, "Exceeded Symbol Table size", -1);

	// Check if identifier already exists in this level
	if (lookup(name, lvl, idx, tbl))
		DIE("\nINTERRUPTED - redeclaration of '%s'", name);

	// Set symbol data
	switch ((tbl[*idx].kind = kind))
	{
		// Const
		case 1:
			tbl[*idx].val = val;
			break;

		// Var
		case 2:
			tbl[*idx].addr = (*addr)++;
			break;

		// Procedure
		case 3:
			break;
		
		default:
			error(NULL, "Unexpected Symbol kind", -1);
			break;
	}

	// Set symbol name
	strcpy(tbl[*idx].name, name);

	// Set symbol level
	tbl[*idx].level = lvl;

	// Increment table pointer
	(*idx)++;
}

int lookup(char *name, int lvl, int *tblIdx, symbol_t *tbl)
{
	for (int i = *tblIdx - 1; i > 0; i--)
	{
		// Skip this symbol
		if (tbl[i].level > lvl)
			continue;

		// Went down a level (or stayed)
		lvl = tbl[i].level;

		// Is this the symbol we're looking for
		if ((tbl[i].kind == CONST || tbl[i].kind == VAR) && strcmp(name, tbl[i].name) == 0)
			return i;
	}

	return 0;
}

// TODO: the compiler's main
// The following are the available directives
// compiler.exe sourcecode.plc -l -o output.plmc
//   -l  -   Prints lexemes to STDOUT (table and list)
//   -o  -   Followed by a %outputpath% for the produced machine code
//           Without this directives, the source code will be stored on: 
//           "./a.plmc"    (plmc stands for PL/0 Machine Code) 
// %sourcecode% path must be provided as an argument as well.
int main(int argc, char *argv[])
{
	instruction_t code[MAX_CODE_LENGTH];
	symbol_t tbl[MAX_TABLE_SIZE];
	int numInstrctions, i, l, a;
	char *path = PLMC_OUT;
	list_t *lexims;
	FILE *out;
	char *in, *o;

	// Init to 0 (NULL)
	__content = NULL;
	out = NULL;
	o = NULL;
	l = a = 0;

	// Process args and directives
	for (i = 1; i < argc; i++)
	{
		// Process Directives
		if (argv[i][0] == FLAG)
		{
			switch (argv[i][1])
			{
				// -l  -  Print lexemes (stdout)
				case FLAG_LEX:
					l = 1;
					break;
					
				// -a  -  Print machinecode (stdout)
				case FLAG_ASMBLY:
					a = 1;
					break;

				// -o  -  Output machinecode
				case FLAG_OUT:
					// Define output file if it hasn't been defined yet
					// If missing arguments or file can't be writting to exit and display error
					if (!o && i == argc - 1)
					{
						// Free memory if needed
						if (__content)
							destroyString(__content);
						
						// Display error and exit
						DIE("%s","\nEXITED\t-o output directive must be followed by a valid file path!");
					}

					o = argv[++i];
					break;

				default:
					// Free memory if needed
					if (__content)
						destroyString(__content);
					
					// "Supported Dircetives:"
					// "-l			Prints Lexemes (list and table) to STDOUT"
					// "-a			Prints Produced Machine Code to STDOUT, in ADDITION(!) to saving Machine Code to a file"
					// "-o %path%	Writes the Produced Machine Code to \%path\% instead of %s (default path)"
					DIE("\nEXITED - Unkown Directive \"%s\"!\n\n%s\n%c%c\t\t%s\n%c%c\t\t%s\n%c%c %%path%%\t%s%s%s\n", argv[i],
						"Supported Dircetives:", FLAG, FLAG_LEX, "Prints Lexemes (list and table) to STDOUT",
						FLAG, FLAG_ASMBLY, "Prints Produced Machine Code to STDOUT, in ADDITION(!) to saving Machine Code to a file",
						FLAG, FLAG_OUT, "Writes the Produced Machine Code to %path% instead of ", PLMC_OUT, " (default path)");
					break;
			}
		}
		
		// Read Source Code and exit if an error occurs
		else if (!__content)
			__content = readFile(in = argv[i]);
	}

	// Check if sourcecode path was provided
	if (!__content)
		DIE("%s", "\nEXITED - Missing source code argument!");

	// Init Scanner
	if (!(lexims = createList()))
		DIE("%s", "\nINTERRUPTED - Bad memory allocation (internal error)");

	// Run Scanner
	process(__content, lexims);
	
	// Print lexems
	if (l)
	{
		printLeximTable(lexims);
		printf("\n\n");
		printLeximList(lexims);
		printf("\n\n");
	}

	// Run Parser
	numInstrctions = program(lexims, tbl, code);

	// Open default/overriden file to write machine code to
	if (!(out = fopen(o = o ? o : PLMC_OUT, "w")))
		DIE("\nINTERRUPTED - Unable to write machine code to \"%s\"", o);

	// Write Machine Code
	if (numInstrctions > 0)
	{
		// Print header (-a directive)
		if (a) 
		{
			// Print Header
			printf("Generated Machine Code:\n");
			printf("(\"%s\")\n", in);
			
			// Print brackets
			for (i = -4; i < 0 || in[i]; i++)
				printf("-");

			printf("\n");
		}

		for (i = 0; i < numInstrctions - 1; i++)
		{
			// Write to file
			fprintf(out, "%d %d %d %d\n", code[i].op, code[i].r, code[i].l, code[i].m);

			// STDOUT (-a directive)
			if (a) printf("%d %d %d %d\n", code[i].op, code[i].r, code[i].l, code[i].m);
		}
		// Last line:
		// Write to  file
		fprintf(out, "%d %d %d %d", code[i].op, code[i].r, code[i].l, code[i].m);
		
		// STDOUT (-a directive)
		if (a) printf("%d %d %d %d\n", code[i].op, code[i].r, code[i].l, code[i].m);
	}

	// Clean up after yourself ;)
	destroyList(lexims);
	destroyString(__content);
	fclose(out);

	return 0;
}

#pragma region "DEPRECATED"
// // DEPRECATED // //
// instruction_t **compile(char *sourcecodePath, int lexFlag, int asmblyFlag)
// {
// 	instruction_t code[MAX_CODE_LENGTH], **rtnCode;
// 	symbol_t tbl[MAX_TABLE_SIZE];
// 	int numInstrctions, i;
// 	FILE *machinecode;
// 	list_t *lexims;

// 	if (!sourcecodePath)
// 		return 0;

// 	// Memory Allocations
// 	if (!(lexims = createList()) || !(rtnCode = calloc(MAX_CODE_LENGTH, sizeof(instruction_t))))
// 		DIE("%s", "\nINTERRUPTED\tBad memory allocation (internal error)");
	
// 	if (!(__content = readFile(sourcecodePath)))
// 		DIE("%s", "unable to PL/0 sourcecode (IO Error)");

// 	// Scanner
// 	process(__content, lexims);

// 	// Print Scanner output
// 	if (lexFlag)
// 	{
// 		printLeximTable(lexims);
// 		printf("\n\n");
// 		printLeximList(lexims);
// 		printf("\n\n");
// 	}

// 	// Parser
// 	numInstrctions = program(lexims, tbl, code);

// 	if (numInstrctions > 0)
// 	{
// 		for (i = 0; i < numInstrctions - 1; i++)
// 		{
// 			rtnCode[i] = _createInstruction(code[i].op, code[i].r, code[i].l, code[i].m);

// 			// Print Parser code on line i
// 			if (asmblyFlag)
// 				printf("%d %d %d %d\n", code[i].op, code[i].r, code[i].l, code[i].m);
// 		}
// 		rtnCode[i] = _createInstruction(code[i].op, code[i].r, code[i].l, code[i].m);

// 		// Print Parser code on last line
// 		if (asmblyFlag)
// 			printf("%d %d %d %d\n\n", code[i].op, code[i].r, code[i].l, code[i].m);
// 	}

// 	// Clean up after yourself ;)
// 	destroyList(lexims);
// 	destroyString(__content);
// 	fclose(machinecode);

// 	return rtnCode;
// }
#pragma endregion