#include <stdio.h>
#include <string.h>
#include "run.h"

#define FLAG		'-'
#define FLAG_LEX	'l'
#define FLAG_ASMBLY	'a'
#define FLAG_VM		'v'

void badDirective()
{
	// TODO: print valid directives and die
}

int main(int argc, char *argv[])
{
	int lex, asmbly, vm;
	instruction_t **code;
	char *sourcecode;

	// Init flags
	lex = asmbly = vm = 0;

	// Init sourcecode location
	sourcecode = NULL;

	// Process arguments
	for (int i = 1; i < argc; i++)
	{
		if (argv[i][0] == FLAG)
		{
			if (argv[i][1] == FLAG_LEX)			lex = 1;
			else if (argv[i][1] == FLAG_ASMBLY)	asmbly = 1;
			else if (argv[i][1] == FLAG_VM)		vm = 1;
			else badDirective();
		}
		else if (sourcecode == NULL)
			sourcecode = argv[i];
	}

	// PL/0 Sourcecode not
	// provided in arguments
	if (!sourcecode)
	{
		// TODO: die - no sourcecode provided
		printf("no sourcecode\n");
		return 1;
	}


	// Compile Source Code (Scanner and Parser)
	if (!(code = compile(sourcecode, lex, asmbly)))
	{
		// TODO: die - no code
		printf("no code");
		return 1;
	}

	// Run Machine Code on VM
	run(code, vm);

	// TODO: compile
	// TODO: run
	
	/* code */
	return 0;
}