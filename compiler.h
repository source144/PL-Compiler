#ifndef PVM_COMPILER_H
#define PVM_COMPILER_H

// Constants
#define PLMC_OUT			"a.plmc"
#define MAX_IDENT_LEN		11
#define MAX_DECIM_LEN		5
#define MAX_SYMBL_LEN		2
#define MAX_TABLE_SIZE		99
#define SEPARATOR			'|'
#define SYMBOLS				";:=+-*/'\",()<>."
#define NUM_RESERVED		15
#define FRAME_SIZE			4
#define MAIN				0
#define CHECK_LINE			1

// Reserve regs before this (0 if not)
#define REG					0

char RESERVED[NUM_RESERVED][MAX_IDENT_LEN] = {
	"const",
	"var",
	"procedure",
	"call",
	"begin",
	"end",
	"if",
	"then",
	"else",
	"while",
	"do",
	"read",
	"write",
	"odd",
	"return"
};

#define CONST		1
#define VAR			2
#define PROC		3
#define nulsym			1
#define identsym		2
#define numbersym		3
#define plussym			4
#define minussym		5
#define multsym			6
#define slashsym		7
#define oddsym			8
#define eqlsym			9
#define neqsym			10
#define lessym			11
#define leqsym			12
#define gtrsym			13
#define geqsym			14
#define lparentsym		15
#define rparentsym		16
#define commasym		17
#define semicolonsym	18
#define periodsym		19
#define becomessym		20
#define beginsym		21
#define endsym			22
#define ifsym			23
#define thensym			24
#define whilesym		25
#define dosym			26
#define callsym			27
#define constsym		28
#define varsym			29
#define procsym			30
#define writesym		31
#define readsym			32
#define elsesym			33
#define returnsym		34


#pragma region "Struct defenitions"

typedef struct
{
	char *charAt;
	int length;
} string_t;

typedef struct node
{
	int __line;
	int __col;
	int __idx;
	int token;
	string_t *data;
	struct node *next;
} node_t;

typedef struct
{
	node_t *head;
	node_t *tail;
	int size;
} list_t;

typedef struct
{
	int kind; 		// const = 1
	char name[MAX_IDENT_LEN + 1];	// name up to 11 chars
	int val; 		// number (ASCII value)
	int level; 		// L level
	int addr; 		// M address
	int _argc;		// Number of args expected (procedure)
} symbol_t;

#pragma endregion

#pragma region "Function Prototypes"

// List Methods
list_t *createList(void);
list_t *destroyList(list_t *list);
void add(list_t *l, int line, int col, int idx, string_t *data, int token);

// String Methods
string_t *destroyString(string_t *str);

// Node Methods
node_t *createNode(string_t *data, int line, int col, int idx, int token);

// Scanner Helpers
int getReservedToken(int reservedIdx);
int getSymbolToken(string_t *symbol);
int getIdentifierToken(string_t *identifier);
int isValidSymbol(char c);
void newLine(int *ln, int *col);

// Scanner Methods
string_t *readIdentifier(string_t *content, int *idx, int *ln, int *col, int *lnIdx);
string_t *readNumber(string_t *content, int *idx, int *ln, int *col, int *lnIdx);
string_t *readSymbol(string_t *content, int *idx, int *ln, int *col, int *lnIdx);
void addIdentifier(list_t *l, int line, int col, int idx, string_t *identifier);
void addNumber(list_t *l, int line, int col, int idx, string_t *number);
void addSymbol(list_t *l, int line, int col, int idx, string_t *symbol);
string_t *readFile(char *filename);
void printLeximTable(list_t *leximList);
void printLeximList(list_t *leximList);
void process(string_t *content, list_t *leximsList);

// Parser Helpers
void lineError(char *title, char *msg, int idx, int lineIdx, int lineNum, int colNum);
void printIndicator(int lineIdx, int lineNum, int colNum);
void error(node_t *token, char *msg, int e);
int numDigits(int n);
node_t *nextToken(node_t *token);
node_t *alginForError(node_t *token, int checkLine);

// Symbol Table Methods
int tableInsert(int kind, char *name, int *idx, int lvl, int *addr, int val, symbol_t *tbl);
int lookup(char *name, int lvl, int *tblIdx, symbol_t *tbl);

#pragma endregion

#pragma region "Token Helpers"

int getReservedToken(int reservedIdx)
{
	switch (reservedIdx)
	{
		case 0:		return constsym;
		case 1:		return varsym;
		case 2:		return procsym;// FIXME:	
		case 3:		return callsym;
		case 4:		return beginsym;
		case 5:		return endsym;
		case 6:		return ifsym;
		case 7:		return thensym;
		case 8:		return elsesym;
		case 9:		return whilesym;
		case 10:	return dosym;
		case 11:	return readsym;
		case 12:	return writesym;
		case 13:	return oddsym;
		case 14:	return returnsym;
		default:	return -1;
	}
}

int getSymbolToken(string_t *symbol)
{
	switch (symbol->charAt[0])
	{
		case '+':							return plussym;
		case '-':							return minussym;
		case '*':							return multsym;// FIXME:	
		case '/':							return slashsym;
		case '(':							return lparentsym;
		case ')':							return rparentsym;
		case '=':							return eqlsym;
		case ',':							return commasym;
		case '.':							return periodsym;
		case ';':							return semicolonsym;
		case '<':
			if (symbol->length == 1)		return lessym;
			if (symbol->charAt[1] == '>')	return neqsym;
			if (symbol->charAt[1] == '=')	return leqsym;
			else return -1;

		case '>':
			if (symbol->length == 1)		return gtrsym;
			if (symbol->charAt[1] == '=')	return geqsym;
			else return -1;

		case ':':
			if (symbol->length < 2 || symbol->charAt[1] != '=')
											return -1;
			else 							return becomessym;

		default:		return -1;
	}
}

#pragma endregion

#endif