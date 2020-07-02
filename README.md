
  

# PL/0 Virtual Machine and Compiler

  

### Gonen Matias - Spring 2020

A Virtual machine and Compiler for PL/0 language programming language.
The language allows you to evaluate expressions, call procedures, provide arguments for procedures, capture return value of procedures, read input, and output evaluated expressions.

####
---

The compiler compiles PL/0 source code into machine code, which when loaded on the Virtual Machine, runs as syntactically expected. For example:
``` javascript
var n;
procedure fact(n);
	begin
		if n < 0 then return -1;
		if n < 2 then return 1;

		return fact(n - 1) * n;
	end;

procedure abs(n);
	begin
		if n < 0 then return -n;
		return n;
	end;

begin
	read n;
	write fact(abs(n)) + 5 + fact(fact(n * (2 / 2)));
end.
```

Given an input of **3**, the output of this program (`.\pvm.exe a.plmc`) is:
```java
$ ./pvm.exe a.plmc
PL/0 VM: 
Input integer = 3
			// 3! + 5 + ((3 * (2/2))!)!
			// 6 + 5 + ((3 * 1)!)!
			// 11 + (6)!
			// 11 + 720
PL/0 VM: 731		// = 731
```

---
### The EBNF  of PL/0:

    // Base
    (1) program     :=	block ".".
    (2) block       :=	{ constdec | vardec | procdec } statement.
    
    // Declarations
    (3) constdec    :=	"const" ident "=" number {"," ident "=" number } ";".
    (4) vardec      :=	"var" ident {"," ident } ";".
    (5) procdec     :=	procedure ident "(" [ ident {"," ident}] ");" block ";".
    
    // Statements
    (6) statement   :=	[
					ident ":=" expression
					| [ "call" ] ident "(" [ expression {"," expression } ");"
					| "begin" { statement ";" } "end"
					| "if" condition "then" statement [ ";" "else" statement ]
					| "while" condition "do" statement
					| "read" ident
					| "write" expression
					| "return expression
					| ε
				] .
	
	// Conditions and expressions
	(7) condition   :=	"odd" expression
				| expression rel-op expression.
	(8) rel-op      :=	"=" | "<>" | "<" | ">" | "<=" | ">=".
	(9) expression  :=	["+" | "-"] term { ("+" | "-") term }.
	(10) term       :=	factor { ("*" | "/" ) factor }.
	(11) factor     :=	ident | number | "(" expression ")".
	(12) number     :=	digit { digit }.
	(13) ident      :=	letter { letter | digit }
	(14) digit      :=	"0" | "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9".
	(15) letter     :=	"a" | "b" | ... | "y" | "z" | "A" | "B" | ... | "Y" | "Z".

	
	// NOTE:
	// [] Means optional items
	// {} Means repeat 0 or more times
	// A period . indacates the end of a defintion of a syntactic class

# The PL/0 Virtual Machine
***Implemented in C***, the stack machine has **two** memory stores:  
1. The **stack**, which contains **data** to be used by the PM/0 CPU.  
2. The **Code** (or "text"), which contains **instructions** for the VM.    

It has **five** registers to handle the stack and the code/text segments.  
1.  **BP** - Base Pointer  
2.  **SP** - Stack Pointer  
3.  **PC** - Program Counter  
4.  **IR** - Instruction Register (*implemented as pointer to the instruction in memeory*)
5.  **RP** - Register Pointer (Keeps track of the base of the register)
    
This machine also has a Register File (**RF**) with eight (20) registers _(**changeable** in pvm.h)_.  
  
## Running the VM  
To run the **PL/0 VM** you must first compile the **PVM**'s source code to produce an executable.  
_**NOTE** that we must link C's **math library** by using the **-lm directive**_.  
`gcc -lm -g pvm.c -o pvm.exe`  

Once the code is compiled you are able to run the VM provided a machine code **[INPUT_FILE]** as follows:  
`./pvm.exe [INPUT_FILE]`  

Furthermore, the **Virtual Machine** supports the following **Directives**:  
**-a**  `./pvm.exe [INPUT_FILE] -a`  **prints** the **Decoded Instructions Set**  
**-v**  `./pvm.exe [INPUT_FILE] -v`  **prints** the **VM State** on *each cycle*  
**-i**  `./pvm.exe [INPUT_FILE] -i`  The **VM State** _if printed_, will be printed with **indicators**  

### Saving the output  
Both of these Directives write to `stdout`, so it's possible to direct the output to using the output redirection operator ( **>** ).

To **write** both the **Decoded Instructions Set** and the **VM State** to *vmlog.txt*, for example.  
`./pvm.exe [INPUT_FILE] -v -a >vmlog.txt`

Note that you can change *vmlog.txt* to **any filepath** you wish.  
  
#### Checking for memory leaks  
Compile the source code with the following:  
`gcc -lm -g pvm.c -o pvm.exe`

Afterwards, run the program with the following line:  
`valgrind --leak-check=full ./pvm.exe [INPUT_FILE]`
    
## Instruction Set Architecture  
Each instruction contains **four components**:  
(1) **OP** - The operation cod  
(2) **R** - Refers to a register  
(3) **L** - The lexicographical level or a register in arithmetic and relational instructions  
(4) **M** - Depending on the operation, it indicates a **constant**, **program address**, **data address**, or **a register**.    

### Available instructions  
(1) `lit`  **LIT** - **R, 0, M** - Loads a constant value (literal) **M** into Register **R**  
(2) `rtn`  **RTN** 0, **0, 0** - Returns from a subroutine and restore the caller environment  
(3) `lod`  **LOD R, L, M** - Load value into a selected register from the stack location at offset **M** from **L**lexicographical levels down
  
(4) `sto`  **STO** - - Store value from a selected register in the stack location at offset **M** from **L** lexicographicallevels down
  
(5) `cal`  **CAL** - **0, L, M** - Call procedure at code index **M**  
(6) `inc`  **INC** - **0, 0, M** - Allocate **M** locals (increment sp by M). First four are **Functional Value,** **StaticLink (SL)**, **Dynamic Link (DL)**, and **Return Address (RA)**
  
(7) `jmp`  **JMP** - **0, 0, M** - Jump to instruction **M**  
(8) `jpc`  **JPC** - **R, 0, M** - Jump to instruction **M** if **R** = 0  
(9) `out`  **SIO** - **R, 0, 1** - Write a register to the screen  
(10) `rin`  **SIO** - **R, 0, 2** - Read in input from the user and store it in a register  
(11) `ext`  **SIO** - **0, 0, 3** - End of program (program stops running)    

#### Arithmetic instructions  
(12) `neg` - **NEG** - Reg[**R**] = -Reg[**R**]  
(13) `add` - **ADD** - Reg[**R**] = Reg[**L**] + Reg[**M**]  
(14) `sub` - **SUB** - Reg[**R**] = Reg[**L**] - Reg[**M**]  
(15) `mul` - **MUL** - Reg[**R**] = Reg[**L**] * Reg[**M**]  
(16) `div` - **DIV** - Reg[**R**] = Reg[**L**] / Reg[**M**]  
(17) `odd` - **ODD** - Reg[**R**] = Reg[**L**] % 2  
(18) `mod` - **MOD** - Reg[**R**] = Reg[**L**] % Reg[**M**]  
(19) `eql` - **EQL** - Reg[**R**] = Reg[**L**] == Reg[**M**]  
(20) `neq` - **NEQ** - Reg[**R**] = Reg[**L**] != Reg[**M**]  
(21) `lss` - **LSS** - Reg[**R**] = Reg[**L**] < Reg[**M**]  
(22) `leq` - **LEQ** - Reg[**R**] = Reg[**L**] <= Reg[**M**]  
(23) `gtr` - **GTR** - Reg[**R**] = Reg[**L**] > Reg[**M**]  
(24) `geq` - **GEQ** - Reg[**R**] = Reg[**L**] <= Reg[**M**]  
  

#### Register Instructions  
(12) `reg_l` - **REG_L** - **0, 0, M** - **RP** += **M** ------------_(Register Pointer **M** levels "deeper")_  
(13) `reg_r` - **REG_R** - **0, 0, 0** - **RP** = Reg[**RP** - 1] ----_(Register Pointer 1 level "higher")_  
(14) `reg_b` - **REG_B** - **R, 0, 0** - Reg[**R**] = **RP** ---------_(LODs Register Pointer into Reg[**R**])_  
  
# The PL/0 Compiler  
The PL/0 Compiler consists of a **Scanner** and a **Parser** that both run as part of the compilation process.The compiler reads PL/0 **source code** and converts it to **machine code** (an instruction set).  
To run the **PL/0 Compiler** you must first compile the **Compiler**'s source code to produce an executable.  
_**NOTE** that we must link C's **math library** by using the **-lm directive**_.  
`gcc -lm -g compiler.c -o compiler.exe`  
 

Once the code is compiled you are able to run the Compiler provided a PL/0 source code **[INPUT_FILE]** as follows:  
`./compiler.exe [INPUT_FILE]`  

The compiler will output and explain any error that occurs during the compilation process.  
```java  
$ ./compile.exe codes/while05.txt -o compiled_factorial.plmc
PL/0 COMPILER:
INTERRUPTED - Assignment operator ":=" expected (ERR 13).
LINE 15, 14: factorial = i * factorial;
                       ^
```

By default the compiler saves the compiled **machine code** to `a.plmc`  *(PLMC stands for PL/0 Machine Code)*.
  
You can configure the compiler using the following directives:  
**-l**  `./compiler.exe [INPUT_FILE] -l`  **prints** the **lexeme**  *table and list* produced from the source code.  
**-a**  `./compiler.exe [INPUT_FILE] -a`  **prints** the **produced Machine Code** (not decoded)  
**-o**  `./compiler.exe [INPUT_FILE] -o [OUTPUT_FILE]` - Directs the compiler to write the produced machine code to a specific file, which path to is **[OUTPUT_FILE]**.  

For example the Following `./compiler.exe -a factorial.plc -o factorial.plmc` will compile the source code file, **factorial.plc**, and save the produced machine code to **factorial.plmc**  *as well as* write (**print**) that **machine code** to **STDIN**.    

# Compiling and Running PL/0 Code  
Given a **PL/0 source code** file `factorial.plc`, we can compile and run it on the VM by following these steps:  
**NOTE:** You must first compile both the PL/0 Virtual Machine and the PL/0 Compiler (instructed previously).  
  1. Compile the source code: `./compiler.exe factorial.plc -o factorial.plmc`  
  2. Run the produced machine code on the VM: `./pvm.exe factorial.plmc`  

The `factorial.plc` file:
```javascript
var n;
procedure fact(n);
	begin
		if n < 0 then return -1;	/* -1 represents an error */
		if n < 2 then return 1;		/* 0! = 1! = 1 */
		return n * fact(n - 1);		/* n * (n - 1)! */
	end;
begin
	read n;
	write fact(n);
end.
```
The output of running the compiled code on the Virtual Machine _(input 5 from user)_:
```java
$ ./pvm.exe a.plmc -a -v
PL/0 VM:
Input integer = 5

PL/0 VM: 120
```

### And behind the scenes:
The produced `factorial.plmc` machine code: 

```java
Decoded Machine Instructions:
("factorial.plmc")
----------
Line   OP     R  L  M
0      JMP    0  0  32 
1      JMP    0  0  2  
2      INC    0  0  5  
3      LOD    0  0  4  
4      LIT    1  0  0  
5      LSS    0  0  1  
6      JPC    0  0  11 
7      LIT    0  0  1  
8      NEG    0  0  0  
9      STO    0  0  0  
10     RTN    0  0  0  
11     LOD    0  0  4  
12     LIT    1  0  2  
13     LSS    0  0  1  
14     JPC    0  0  18 
15     LIT    0  0  1  
16     STO    0  0  0  
17     RTN    0  0  0  
18     LOD    0  0  4  
19     REG_B  2  0  0  
20     REG_L  0  0  3  
21     LOD    1  0  4  
22     LIT    2  0  1  
23     SUB    1  1  2  
24     STO    1  0  9  
25     CAL    0  1  1  
26     REG_R  2  0  0  
27     LOD    1  0  5  
28     MUL    0  0  1  
29     STO    0  0  0  
30     RTN    0  0  0  
31     RTN    0  0  0  
32     INC    0  0  5  
33     SIO    0  0  0  
34     STO    0  0  4  
35     REG_B  1  0  0  
36     REG_L  0  0  2  
37     LOD    0  0  4  
38     STO    0  0  9  
39     CAL    0  0  1  
40     REG_R  1  0  0  
41     LOD    0  0  5  
42     SIO    0  0  0  
43     SIO    0  0  3  
```
---
The Virtual Machine's State _(with indicators)_ while running the program, with input 5 from the user:
```bash
VM State Log:
------------------------
   BP, SP, RP Indicators
` - Base Pointer      BP
^ - Stack Pointer     SP
, - Register Pointer  RP
------------------------
                         PC   BP    SP    REGISTERS[20]
Initial values           0     1     0    0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0
Stack: 
       `^
                                          ,
0    JMP     0  0  32    32    1     0    0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0
Stack: 
       `^
                                          ,
32   INC     0  0  5     33    1     5    0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  0
          `           ^
PL/0 VM: Input integer = 5
                                          ,
33   SIO     0  0  0     34    1     5    5  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  0
          `           ^
                                          ,
34   STO     0  0  4     35    1     5    5  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5
          `           ^
                                          ,
35   REG_B   1  0  0     36    1     5    5  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5
          `           ^
                                          ,
36   REG_L   0  0  2     37    1     5    5  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5
          `           ^
                                                ,
37   LOD     0  0  4     38    1     5    5  0  5  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5
          `           ^
                                                ,
38   STO     0  0  9     39    1     5    5  0  5  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5
          `           ^
                                                ,
39   CAL     0  0  1     1     5     5    5  0  5  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5
                     `^
                                                ,
1    JMP     0  0  2     2     5     5    5  0  5  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5
                     `^
                                                ,
2    INC     0  0  5     3     5     10   5  0  5  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5
                       `               ^
                                                ,
3    LOD     0  0  4     4     5     10   5  0  5  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5
                       `               ^
                                                ,
4    LIT     1  0  0     5     5     10   5  0  5  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5
                       `               ^
                                                ,
5    LSS     0  0  1     6     5     10   5  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5
                       `               ^
                                                ,
6    JPC     0  0  11    11    5     10   5  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5
                       `               ^
                                                ,
11   LOD     0  0  4     12    5     10   5  0  5  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5
                       `               ^
                                                ,
12   LIT     1  0  2     13    5     10   5  0  5  2  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5
                       `               ^
                                                ,
13   LSS     0  0  1     14    5     10   5  0  0  2  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5
                       `               ^
                                                ,
14   JPC     0  0  18    18    5     10   5  0  0  2  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5
                       `               ^
                                                ,
18   LOD     0  0  4     19    5     10   5  0  5  2  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5
                       `               ^
                                                ,
19   REG_B   2  0  0     20    5     10   5  0  5  2  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5
                       `               ^
                                                ,
20   REG_L   0  0  3     21    5     10   5  0  5  2  2  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5
                       `               ^
                                                         ,
21   LOD     1  0  4     22    5     10   5  0  5  2  2  0  5  0  0  0  0  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5
                       `               ^
                                                         ,
22   LIT     2  0  1     23    5     10   5  0  5  2  2  0  5  1  0  0  0  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5
                       `               ^
                                                         ,
23   SUB     1  1  2     24    5     10   5  0  5  2  2  0  4  1  0  0  0  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5
                       `               ^
                                                         ,
24   STO     1  0  9     25    5     10   5  0  5  2  2  0  4  1  0  0  0  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5
                       `               ^
                                                         ,
25   CAL     0  1  1     1     10    10   5  0  5  2  2  0  4  1  0  0  0  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5
                                       `^
                                                         ,
1    JMP     0  0  2     2     10    10   5  0  5  2  2  0  4  1  0  0  0  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5
                                       `^
                                                         ,
2    INC     0  0  5     3     10    15   5  0  5  2  2  0  4  1  0  0  0  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5 | 0  1  5  26  4
                                        `               ^
                                                         ,
3    LOD     0  0  4     4     10    15   5  0  5  2  2  4  4  1  0  0  0  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5 | 0  1  5  26  4
                                        `               ^
                                                         ,
4    LIT     1  0  0     5     10    15   5  0  5  2  2  4  0  1  0  0  0  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5 | 0  1  5  26  4
                                        `               ^
                                                         ,
5    LSS     0  0  1     6     10    15   5  0  5  2  2  0  0  1  0  0  0  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5 | 0  1  5  26  4
                                        `               ^
                                                         ,
6    JPC     0  0  11    11    10    15   5  0  5  2  2  0  0  1  0  0  0  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5 | 0  1  5  26  4
                                        `               ^
                                                         ,
11   LOD     0  0  4     12    10    15   5  0  5  2  2  4  0  1  0  0  0  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5 | 0  1  5  26  4
                                        `               ^
                                                         ,
12   LIT     1  0  2     13    10    15   5  0  5  2  2  4  2  1  0  0  0  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5 | 0  1  5  26  4
                                        `               ^
                                                         ,
13   LSS     0  0  1     14    10    15   5  0  5  2  2  0  2  1  0  0  0  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5 | 0  1  5  26  4
                                        `               ^
                                                         ,
14   JPC     0  0  18    18    10    15   5  0  5  2  2  0  2  1  0  0  0  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5 | 0  1  5  26  4
                                        `               ^
                                                         ,
18   LOD     0  0  4     19    10    15   5  0  5  2  2  4  2  1  0  0  0  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5 | 0  1  5  26  4
                                        `               ^
                                                         ,
19   REG_B   2  0  0     20    10    15   5  0  5  2  2  4  2  1  0  0  0  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5 | 0  1  5  26  4
                                        `               ^
                                                         ,
20   REG_L   0  0  3     21    10    15   5  0  5  2  2  4  2  5  0  0  0  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5 | 0  1  5  26  4
                                        `               ^
                                                                  ,
21   LOD     1  0  4     22    10    15   5  0  5  2  2  4  2  5  0  4  0  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5 | 0  1  5  26  4
                                        `               ^
                                                                  ,
22   LIT     2  0  1     23    10    15   5  0  5  2  2  4  2  5  0  4  1  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5 | 0  1  5  26  4
                                        `               ^
                                                                  ,
23   SUB     1  1  2     24    10    15   5  0  5  2  2  4  2  5  0  3  1  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5 | 0  1  5  26  4
                                        `               ^
                                                                  ,
24   STO     1  0  9     25    10    15   5  0  5  2  2  4  2  5  0  3  1  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5 | 0  1  5  26  4
                                        `               ^
                                                                  ,
25   CAL     0  1  1     1     15    15   5  0  5  2  2  4  2  5  0  3  1  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5 | 0  1  5  26  4
                                                        `^
                                                                  ,
1    JMP     0  0  2     2     15    15   5  0  5  2  2  4  2  5  0  3  1  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5 | 0  1  5  26  4
                                                        `^
                                                                  ,
2    INC     0  0  5     3     15    20   5  0  5  2  2  4  2  5  0  3  1  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5 | 0  1  5  26  4 | 0  5  10  26  3
                                                         `                ^
                                                                  ,
3    LOD     0  0  4     4     15    20   5  0  5  2  2  4  2  5  3  3  1  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5 | 0  1  5  26  4 | 0  5  10  26  3
                                                         `                ^
                                                                  ,
4    LIT     1  0  0     5     15    20   5  0  5  2  2  4  2  5  3  0  1  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5 | 0  1  5  26  4 | 0  5  10  26  3
                                                         `                ^
                                                                  ,
5    LSS     0  0  1     6     15    20   5  0  5  2  2  4  2  5  0  0  1  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5 | 0  1  5  26  4 | 0  5  10  26  3
                                                         `                ^
                                                                  ,
6    JPC     0  0  11    11    15    20   5  0  5  2  2  4  2  5  0  0  1  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5 | 0  1  5  26  4 | 0  5  10  26  3
                                                         `                ^
                                                                  ,
11   LOD     0  0  4     12    15    20   5  0  5  2  2  4  2  5  3  0  1  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5 | 0  1  5  26  4 | 0  5  10  26  3
                                                         `                ^
                                                                  ,
12   LIT     1  0  2     13    15    20   5  0  5  2  2  4  2  5  3  2  1  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5 | 0  1  5  26  4 | 0  5  10  26  3
                                                         `                ^
                                                                  ,
13   LSS     0  0  1     14    15    20   5  0  5  2  2  4  2  5  0  2  1  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5 | 0  1  5  26  4 | 0  5  10  26  3
                                                         `                ^
                                                                  ,
14   JPC     0  0  18    18    15    20   5  0  5  2  2  4  2  5  0  2  1  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5 | 0  1  5  26  4 | 0  5  10  26  3
                                                         `                ^
                                                                  ,
18   LOD     0  0  4     19    15    20   5  0  5  2  2  4  2  5  3  2  1  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5 | 0  1  5  26  4 | 0  5  10  26  3
                                                         `                ^
                                                                  ,
19   REG_B   2  0  0     20    15    20   5  0  5  2  2  4  2  5  3  2  1  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5 | 0  1  5  26  4 | 0  5  10  26  3
                                                         `                ^
                                                                  ,
20   REG_L   0  0  3     21    15    20   5  0  5  2  2  4  2  5  3  2  8  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5 | 0  1  5  26  4 | 0  5  10  26  3
                                                         `                ^
                                                                           ,
21   LOD     1  0  4     22    15    20   5  0  5  2  2  4  2  5  3  2  8  0  3  0  0  0  0  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5 | 0  1  5  26  4 | 0  5  10  26  3
                                                         `                ^
                                                                           ,
22   LIT     2  0  1     23    15    20   5  0  5  2  2  4  2  5  3  2  8  0  3  1  0  0  0  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5 | 0  1  5  26  4 | 0  5  10  26  3
                                                         `                ^
                                                                           ,
23   SUB     1  1  2     24    15    20   5  0  5  2  2  4  2  5  3  2  8  0  2  1  0  0  0  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5 | 0  1  5  26  4 | 0  5  10  26  3
                                                         `                ^
                                                                           ,
24   STO     1  0  9     25    15    20   5  0  5  2  2  4  2  5  3  2  8  0  2  1  0  0  0  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5 | 0  1  5  26  4 | 0  5  10  26  3
                                                         `                ^
                                                                           ,
25   CAL     0  1  1     1     20    20   5  0  5  2  2  4  2  5  3  2  8  0  2  1  0  0  0  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5 | 0  1  5  26  4 | 0  5  10  26  3
                                                                          `^
                                                                           ,
1    JMP     0  0  2     2     20    20   5  0  5  2  2  4  2  5  3  2  8  0  2  1  0  0  0  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5 | 0  1  5  26  4 | 0  5  10  26  3
                                                                          `^
                                                                           ,
2    INC     0  0  5     3     20    25   5  0  5  2  2  4  2  5  3  2  8  0  2  1  0  0  0  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5 | 0  1  5  26  4 | 0  5  10  26  3 | 0  10  15  26  2
                                                                           `                 ^
                                                                           ,
3    LOD     0  0  4     4     20    25   5  0  5  2  2  4  2  5  3  2  8  2  2  1  0  0  0  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5 | 0  1  5  26  4 | 0  5  10  26  3 | 0  10  15  26  2
                                                                           `                 ^
                                                                           ,
4    LIT     1  0  0     5     20    25   5  0  5  2  2  4  2  5  3  2  8  2  0  1  0  0  0  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5 | 0  1  5  26  4 | 0  5  10  26  3 | 0  10  15  26  2
                                                                           `                 ^
                                                                           ,
5    LSS     0  0  1     6     20    25   5  0  5  2  2  4  2  5  3  2  8  0  0  1  0  0  0  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5 | 0  1  5  26  4 | 0  5  10  26  3 | 0  10  15  26  2
                                                                           `                 ^
                                                                           ,
6    JPC     0  0  11    11    20    25   5  0  5  2  2  4  2  5  3  2  8  0  0  1  0  0  0  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5 | 0  1  5  26  4 | 0  5  10  26  3 | 0  10  15  26  2
                                                                           `                 ^
                                                                           ,
11   LOD     0  0  4     12    20    25   5  0  5  2  2  4  2  5  3  2  8  2  0  1  0  0  0  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5 | 0  1  5  26  4 | 0  5  10  26  3 | 0  10  15  26  2
                                                                           `                 ^
                                                                           ,
12   LIT     1  0  2     13    20    25   5  0  5  2  2  4  2  5  3  2  8  2  2  1  0  0  0  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5 | 0  1  5  26  4 | 0  5  10  26  3 | 0  10  15  26  2
                                                                           `                 ^
                                                                           ,
13   LSS     0  0  1     14    20    25   5  0  5  2  2  4  2  5  3  2  8  0  2  1  0  0  0  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5 | 0  1  5  26  4 | 0  5  10  26  3 | 0  10  15  26  2
                                                                           `                 ^
                                                                           ,
14   JPC     0  0  18    18    20    25   5  0  5  2  2  4  2  5  3  2  8  0  2  1  0  0  0  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5 | 0  1  5  26  4 | 0  5  10  26  3 | 0  10  15  26  2
                                                                           `                 ^
                                                                           ,
18   LOD     0  0  4     19    20    25   5  0  5  2  2  4  2  5  3  2  8  2  2  1  0  0  0  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5 | 0  1  5  26  4 | 0  5  10  26  3 | 0  10  15  26  2
                                                                           `                 ^
                                                                           ,
19   REG_B   2  0  0     20    20    25   5  0  5  2  2  4  2  5  3  2  8  2  2  1  0  0  0  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5 | 0  1  5  26  4 | 0  5  10  26  3 | 0  10  15  26  2
                                                                           `                 ^
                                                                           ,
20   REG_L   0  0  3     21    20    25   5  0  5  2  2  4  2  5  3  2  8  2  2  11  0  0  0  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5 | 0  1  5  26  4 | 0  5  10  26  3 | 0  10  15  26  2
                                                                           `                 ^
                                                                                     ,
21   LOD     1  0  4     22    20    25   5  0  5  2  2  4  2  5  3  2  8  2  2  11  0  2  0  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5 | 0  1  5  26  4 | 0  5  10  26  3 | 0  10  15  26  2
                                                                           `                 ^
                                                                                     ,
22   LIT     2  0  1     23    20    25   5  0  5  2  2  4  2  5  3  2  8  2  2  11  0  2  1  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5 | 0  1  5  26  4 | 0  5  10  26  3 | 0  10  15  26  2
                                                                           `                 ^
                                                                                     ,
23   SUB     1  1  2     24    20    25   5  0  5  2  2  4  2  5  3  2  8  2  2  11  0  1  1  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5 | 0  1  5  26  4 | 0  5  10  26  3 | 0  10  15  26  2
                                                                           `                 ^
                                                                                     ,
24   STO     1  0  9     25    20    25   5  0  5  2  2  4  2  5  3  2  8  2  2  11  0  1  1  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5 | 0  1  5  26  4 | 0  5  10  26  3 | 0  10  15  26  2
                                                                           `                 ^
                                                                                     ,
25   CAL     0  1  1     1     25    25   5  0  5  2  2  4  2  5  3  2  8  2  2  11  0  1  1  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5 | 0  1  5  26  4 | 0  5  10  26  3 | 0  10  15  26  2
                                                                                             `^
                                                                                     ,
1    JMP     0  0  2     2     25    25   5  0  5  2  2  4  2  5  3  2  8  2  2  11  0  1  1  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5 | 0  1  5  26  4 | 0  5  10  26  3 | 0  10  15  26  2
                                                                                             `^
                                                                                     ,
2    INC     0  0  5     3     25    30   5  0  5  2  2  4  2  5  3  2  8  2  2  11  0  1  1  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5 | 0  1  5  26  4 | 0  5  10  26  3 | 0  10  15  26  2 | 0  15  20  26  1
                                                                                              `                 ^
                                                                                     ,
3    LOD     0  0  4     4     25    30   5  0  5  2  2  4  2  5  3  2  8  2  2  11  1  1  1  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5 | 0  1  5  26  4 | 0  5  10  26  3 | 0  10  15  26  2 | 0  15  20  26  1
                                                                                              `                 ^
                                                                                     ,
4    LIT     1  0  0     5     25    30   5  0  5  2  2  4  2  5  3  2  8  2  2  11  1  0  1  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5 | 0  1  5  26  4 | 0  5  10  26  3 | 0  10  15  26  2 | 0  15  20  26  1
                                                                                              `                 ^
                                                                                     ,
5    LSS     0  0  1     6     25    30   5  0  5  2  2  4  2  5  3  2  8  2  2  11  0  0  1  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5 | 0  1  5  26  4 | 0  5  10  26  3 | 0  10  15  26  2 | 0  15  20  26  1
                                                                                              `                 ^
                                                                                     ,
6    JPC     0  0  11    11    25    30   5  0  5  2  2  4  2  5  3  2  8  2  2  11  0  0  1  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5 | 0  1  5  26  4 | 0  5  10  26  3 | 0  10  15  26  2 | 0  15  20  26  1
                                                                                              `                 ^
                                                                                     ,
11   LOD     0  0  4     12    25    30   5  0  5  2  2  4  2  5  3  2  8  2  2  11  1  0  1  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5 | 0  1  5  26  4 | 0  5  10  26  3 | 0  10  15  26  2 | 0  15  20  26  1
                                                                                              `                 ^
                                                                                     ,
12   LIT     1  0  2     13    25    30   5  0  5  2  2  4  2  5  3  2  8  2  2  11  1  2  1  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5 | 0  1  5  26  4 | 0  5  10  26  3 | 0  10  15  26  2 | 0  15  20  26  1
                                                                                              `                 ^
                                                                                     ,
13   LSS     0  0  1     14    25    30   5  0  5  2  2  4  2  5  3  2  8  2  2  11  1  2  1  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5 | 0  1  5  26  4 | 0  5  10  26  3 | 0  10  15  26  2 | 0  15  20  26  1
                                                                                              `                 ^
                                                                                     ,
14   JPC     0  0  18    15    25    30   5  0  5  2  2  4  2  5  3  2  8  2  2  11  1  2  1  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5 | 0  1  5  26  4 | 0  5  10  26  3 | 0  10  15  26  2 | 0  15  20  26  1
                                                                                              `                 ^
                                                                                     ,
15   LIT     0  0  1     16    25    30   5  0  5  2  2  4  2  5  3  2  8  2  2  11  1  2  1  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5 | 0  1  5  26  4 | 0  5  10  26  3 | 0  10  15  26  2 | 0  15  20  26  1
                                                                                              `                 ^
                                                                                     ,
16   STO     0  0  0     17    25    30   5  0  5  2  2  4  2  5  3  2  8  2  2  11  1  2  1  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5 | 0  1  5  26  4 | 0  5  10  26  3 | 0  10  15  26  2 | 1  15  20  26  1
                                                                                              `                 ^
                                                                                     ,
17   RTN     0  0  0     26    20    25   5  0  5  2  2  4  2  5  3  2  8  2  2  11  1  2  1  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5 | 0  1  5  26  4 | 0  5  10  26  3 | 0  10  15  26  2
                                                                           `                 ^
                                                                                     ,
26   REG_R   2  0  0     27    20    25   5  0  5  2  2  4  2  5  3  2  8  2  2  11  1  2  1  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5 | 0  1  5  26  4 | 0  5  10  26  3 | 0  10  15  26  2
                                                                           `                 ^
                                                                           ,
27   LOD     1  0  5     28    20    25   5  0  5  2  2  4  2  5  3  2  8  2  1  11  1  2  1  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5 | 0  1  5  26  4 | 0  5  10  26  3 | 0  10  15  26  2
                                                                           `                 ^
                                                                           ,
28   MUL     0  0  1     29    20    25   5  0  5  2  2  4  2  5  3  2  8  2  1  11  1  2  1  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5 | 0  1  5  26  4 | 0  5  10  26  3 | 0  10  15  26  2
                                                                           `                 ^
                                                                           ,
29   STO     0  0  0     30    20    25   5  0  5  2  2  4  2  5  3  2  8  2  1  11  1  2  1  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5 | 0  1  5  26  4 | 0  5  10  26  3 | 2  10  15  26  2
                                                                           `                 ^
                                                                           ,
30   RTN     0  0  0     26    15    20   5  0  5  2  2  4  2  5  3  2  8  2  1  11  1  2  1  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5 | 0  1  5  26  4 | 0  5  10  26  3
                                                         `                ^
                                                                           ,
26   REG_R   2  0  0     27    15    20   5  0  5  2  2  4  2  5  3  2  8  2  1  11  1  2  1  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5 | 0  1  5  26  4 | 0  5  10  26  3
                                                         `                ^
                                                                  ,
27   LOD     1  0  5     28    15    20   5  0  5  2  2  4  2  5  3  2  8  2  1  11  1  2  1  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5 | 0  1  5  26  4 | 0  5  10  26  3
                                                         `                ^
                                                                  ,
28   MUL     0  0  1     29    15    20   5  0  5  2  2  4  2  5  6  2  8  2  1  11  1  2  1  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5 | 0  1  5  26  4 | 0  5  10  26  3
                                                         `                ^
                                                                  ,
29   STO     0  0  0     30    15    20   5  0  5  2  2  4  2  5  6  2  8  2  1  11  1  2  1  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5 | 0  1  5  26  4 | 6  5  10  26  3
                                                         `                ^
                                                                  ,
30   RTN     0  0  0     26    10    15   5  0  5  2  2  4  2  5  6  2  8  2  1  11  1  2  1  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5 | 0  1  5  26  4
                                        `               ^
                                                                  ,
26   REG_R   2  0  0     27    10    15   5  0  5  2  2  4  2  5  6  2  8  2  1  11  1  2  1  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5 | 0  1  5  26  4
                                        `               ^
                                                         ,
27   LOD     1  0  5     28    10    15   5  0  5  2  2  4  6  5  6  2  8  2  1  11  1  2  1  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5 | 0  1  5  26  4
                                        `               ^
                                                         ,
28   MUL     0  0  1     29    10    15   5  0  5  2  2  24  6  5  6  2  8  2  1  11  1  2  1  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5 | 0  1  5  26  4
                                        `               ^
                                                         ,
29   STO     0  0  0     30    10    15   5  0  5  2  2  24  6  5  6  2  8  2  1  11  1  2  1  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5 | 24  1  5  26  4
                                        `                ^
                                                         ,
30   RTN     0  0  0     26    5     10   5  0  5  2  2  24  6  5  6  2  8  2  1  11  1  2  1  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5
                       `               ^
                                                         ,
26   REG_R   2  0  0     27    5     10   5  0  5  2  2  24  6  5  6  2  8  2  1  11  1  2  1  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5
                       `               ^
                                                ,
27   LOD     1  0  5     28    5     10   5  0  5  24  2  24  6  5  6  2  8  2  1  11  1  2  1  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5
                       `               ^
                                                ,
28   MUL     0  0  1     29    5     10   5  0  120  24  2  24  6  5  6  2  8  2  1  11  1  2  1  0  0  0
Stack: 0  0  0  0  5 | 0  1  1  40  5
                       `               ^
                                                ,
29   STO     0  0  0     30    5     10   5  0  120  24  2  24  6  5  6  2  8  2  1  11  1  2  1  0  0  0
Stack: 0  0  0  0  5 | 120  1  1  40  5
                       `                 ^
                                                ,
30   RTN     0  0  0     40    1     5    5  0  120  24  2  24  6  5  6  2  8  2  1  11  1  2  1  0  0  0
Stack: 0  0  0  0  5
          `           ^
                                                ,
40   REG_R   1  0  0     41    1     5    5  0  120  24  2  24  6  5  6  2  8  2  1  11  1  2  1  0  0  0
Stack: 0  0  0  0  5
          `           ^
                                          ,
41   LOD     0  0  5     42    1     5    120  0  120  24  2  24  6  5  6  2  8  2  1  11  1  2  1  0  0  0
Stack: 0  0  0  0  5
          `           ^
                                          ,
42   SIO     0  0  0     43    1     5    120  0  120  24  2  24  6  5  6  2  8  2  1  11  1  2  1  0  0  0
Stack: 0  0  0  0  5
          `           ^
PL/0 VM: 120
                                          ,
43   SIO     0  0  3     44    1     5    120  0  120  24  2  24  6  5  6  2  8  2  1  11  1  2  1  0  0  0
Stack: 0  0  0  0  5
          `           ^
```

# Known Issues  
~There are a couple of issues with global variables and variable scopes that have to be defined and addressed.~  
~As of now, global variables are **NOT** supported by the compiler. While they are acceptable, they produce undefined behavior.~  

~Variables you wish to use in procedures must be passed as arguments.~  
~The arguments themselves are evaluations of expressions, variable are **passed by value** and not by reference.~  

~Given the following `program.plc`:~  
```javascript
var n;
procedure fact(n);
	begin
		if n < 0 then return -1;
		if n < 2 then return 1;

		return fact(n - 1) * n;
	end;

procedure abs(n);
	begin
		if n < 0 then return -n;
		return n;
	end;

begin
	read n;
	write fact(abs(n)) + 5 + fact(n);
end.
```

~This error would appear:~  
```java
$ ./compiler.exe demo/prog01.plc
PL/0 COMPILER: 
INTERRUPTED - redeclaration of n
LINE 10, 15: procedure abs(n);
                           ^
```  
~Changing the argument name _(and preceding references)_ to 'k' _(or any other identifier)_ would allow the file to compile.~
### ^This issue has been resolved.
