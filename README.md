
  

# PM/0 Virtual Machine and Compiler

  

### Gonen Matias - COP 3402 Spring 2020

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

procedure abs(k);
	begin
		if k < 0 then return -k;
		return k;
	end;

begin
	read n;
	write fact(abs(n)) + 5 + fact(fact(n * (2 / 2)));
end.
```

Given an input of **3**, the output of this program (`.\pvm.exe a.plmc`) is:
```sh
$ ./pvm.exe a.plmc
PL/0 VM: 
Input integer = 3
			# 3! + 5 + ((3 * (2/2))!)!
			# 6 + 5 + ((3 * 1)!)!
			# 11 + (6)!
			# 11 + 720
PL/0 VM: 731		# = 731
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
`gcc -g pvm.c -o pvm.exe`  
Once the code is compiled you are able to run the VM provided a machine code **[INPUT_FILE]** as follows:  
`./pvm.exe [INPUT_FILE]`  
Furthermore, the **Virtual Machine** supports the following **Directives**:  
**-a**  `./pvm.exe [INPUT_FILE] -a`  **prints** the **Decoded Instructions Set**  
**-v**  `./pvm.exe [INPUT_FILE] -v`  **prints** the **VM State** on *each cycle*  
### Saving the output  
Both of these Directives write to `stdout`, so it's possible to direct the output to using the output redirection operator ( **>** ). For example:  
`./pvm.exe [INPUT_FILE] -v -a >vmlog.txt`  **writes** both the **Decoded Instructions Set** and the **VM State** to *vmlog.txt*.  
Note that you can change *vmlog.txt* to **any filepath** you wish.  
  
#### Checking for memory leaks  
Compile the source code with the following:`gcc -g pvm.c -o pvm.exe`
Afterwards, run the program with the following line:`valgrind --leak-check=full ./pvm.exe [INPUT_FILE]`
    
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
(25) `reg_l`  **REG_L** - **0, 0, M** - Makes Register Pointer _(**RP**)_ **M** levels "deeper"   
(26) `reg_r`  **REG_R** - **0, 0, 0** - Moves Register Pointer _(**RP**)_ "one level higher"
(27) `reg_b`  **REG_B** - **R, 0, 0** - Reg[**R**] = _**RP**_ ---_(E.g. loads current **RP** (**value**) into Reg[**R**])_
  
# The PL/0 Compiler
The PL/0 Compiler consists of a **Scanner** and a **Parser** that both run as part of the compilation process.The compiler reads PL/0 **source code** and converts it to **machine code** (an instruction set).
  
To run the **PL/0 Compiler** you must first compile the **Compiler**'s source code to produce an executable.  
`gcc -lm -g compiler.c -o compiler.exe`  
_**NOTE** that we must link C's **math library** by using the **-lm directive**_.  

Once the code is compiled you are able to run the Compiler provided a PL/0 source code **[INPUT_FILE]** as follows: `./compiler.exe [INPUT_FILE]`  

The compiler will output and explain any error that occurs during the compilation process.
An example of that is:

```bash  
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
**NOTE:** You must first compile both the PL/0 Virtual Machine and the PL/0 Compiler (instructed previously).
Given a **PL/0 source code** file `factorial.plc`, we can compile and run it on the VM by following these steps:

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
0      JMP    0  0  31 
1      INC    0  0  5  
2      LOD    0  0  4  
3      LIT    1  0  0  
4      LSS    0  0  1  
5      JPC    0  0  10 
6      LIT    0  0  1  
7      NEG    0  0  0  
8      STO    0  0  0  
9      RTN    0  0  0  
10     LOD    0  0  4  
11     LIT    1  0  2  
12     LSS    0  0  1  
13     JPC    0  0  17 
14     LIT    0  0  1  
15     STO    0  0  0  
16     RTN    0  0  0  
17     LOD    0  0  4  
18     REG_B  2  0  0  
19     REG_L  0  0  3  
20     LOD    1  0  4  
21     LIT    2  0  1  
22     SUB    1  1  2  
23     STO    1  0  9  
24     CAL    0  1  1  
25     REG_R  2  0  0  
26     LOD    1  0  5  
27     MUL    0  0  1  
28     STO    0  0  0  
29     RTN    0  0  0  
30     RTN    0  0  0  
31     INC    0  0  6  
32     SIO    0  0  0  
33     STO    0  0  4  
34     REG_B  1  0  0  
35     REG_L  0  0  2  
36     LOD    0  0  4  
37     STO    0  0  10 
38     CAL    0  0  1  
39     REG_R  1  0  0  
40     LOD    0  0  6  
41     SIO    0  0  0  
42     SIO    0  0  3  
```
---
The Virtual Machine's State while running the program, with input 5 from the user:
```javascript
VM State Log:
-------------
                         PC   BP    SP    REGISTERS[20]
Initial values           0     1     0    0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0
Stack: 

0    JMP     0  0  31    31    1     0    0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0
Stack: 

31   INC     0  0  6     32    1     6    0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  0  0


PL/0 VM: Input integer = 5

32   SIO     0  0  0     33    1     6    5  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  0  0


33   STO     0  0  4     34    1     6    5  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5  0


34   REG_B   1  0  0     35    1     6    5  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5  0


35   REG_L   0  0  2     36    1     6    5  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5  0


36   LOD     0  0  4     37    1     6    5  0  5  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5  0


37   STO     0  0  10    38    1     6    5  0  5  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5  0


38   CAL     0  0  1     1     6     6    5  0  5  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5  0


1    INC     0  0  5     2     6     11   5  0  5  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5  0 | 0  1  1  39  5


2    LOD     0  0  4     3     6     11   5  0  5  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5  0 | 0  1  1  39  5


3    LIT     1  0  0     4     6     11   5  0  5  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5  0 | 0  1  1  39  5


4    LSS     0  0  1     5     6     11   5  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5  0 | 0  1  1  39  5


5    JPC     0  0  10    10    6     11   5  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5  0 | 0  1  1  39  5


10   LOD     0  0  4     11    6     11   5  0  5  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5  0 | 0  1  1  39  5


11   LIT     1  0  2     12    6     11   5  0  5  2  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5  0 | 0  1  1  39  5


12   LSS     0  0  1     13    6     11   5  0  0  2  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5  0 | 0  1  1  39  5


13   JPC     0  0  17    17    6     11   5  0  0  2  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5  0 | 0  1  1  39  5


17   LOD     0  0  4     18    6     11   5  0  5  2  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5  0 | 0  1  1  39  5


18   REG_B   2  0  0     19    6     11   5  0  5  2  2  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5  0 | 0  1  1  39  5


19   REG_L   0  0  3     20    6     11   5  0  5  2  2  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5  0 | 0  1  1  39  5


20   LOD     1  0  4     21    6     11   5  0  5  2  2  0  5  0  0  0  0  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5  0 | 0  1  1  39  5


21   LIT     2  0  1     22    6     11   5  0  5  2  2  0  5  1  0  0  0  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5  0 | 0  1  1  39  5


22   SUB     1  1  2     23    6     11   5  0  5  2  2  0  4  1  0  0  0  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5  0 | 0  1  1  39  5


23   STO     1  0  9     24    6     11   5  0  5  2  2  0  4  1  0  0  0  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5  0 | 0  1  1  39  5


24   CAL     0  1  1     1     11    11   5  0  5  2  2  0  4  1  0  0  0  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5  0 | 0  1  1  39  5


1    INC     0  0  5     2     11    16   5  0  5  2  2  0  4  1  0  0  0  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5  0 | 0  1  1  39  5 | 0  1  6  25  4


2    LOD     0  0  4     3     11    16   5  0  5  2  2  4  4  1  0  0  0  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5  0 | 0  1  1  39  5 | 0  1  6  25  4


3    LIT     1  0  0     4     11    16   5  0  5  2  2  4  0  1  0  0  0  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5  0 | 0  1  1  39  5 | 0  1  6  25  4


4    LSS     0  0  1     5     11    16   5  0  5  2  2  0  0  1  0  0  0  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5  0 | 0  1  1  39  5 | 0  1  6  25  4


5    JPC     0  0  10    10    11    16   5  0  5  2  2  0  0  1  0  0  0  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5  0 | 0  1  1  39  5 | 0  1  6  25  4


10   LOD     0  0  4     11    11    16   5  0  5  2  2  4  0  1  0  0  0  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5  0 | 0  1  1  39  5 | 0  1  6  25  4


11   LIT     1  0  2     12    11    16   5  0  5  2  2  4  2  1  0  0  0  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5  0 | 0  1  1  39  5 | 0  1  6  25  4


12   LSS     0  0  1     13    11    16   5  0  5  2  2  0  2  1  0  0  0  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5  0 | 0  1  1  39  5 | 0  1  6  25  4


13   JPC     0  0  17    17    11    16   5  0  5  2  2  0  2  1  0  0  0  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5  0 | 0  1  1  39  5 | 0  1  6  25  4


17   LOD     0  0  4     18    11    16   5  0  5  2  2  4  2  1  0  0  0  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5  0 | 0  1  1  39  5 | 0  1  6  25  4


18   REG_B   2  0  0     19    11    16   5  0  5  2  2  4  2  5  0  0  0  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5  0 | 0  1  1  39  5 | 0  1  6  25  4


19   REG_L   0  0  3     20    11    16   5  0  5  2  2  4  2  5  0  0  0  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5  0 | 0  1  1  39  5 | 0  1  6  25  4


20   LOD     1  0  4     21    11    16   5  0  5  2  2  4  2  5  0  4  0  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5  0 | 0  1  1  39  5 | 0  1  6  25  4


21   LIT     2  0  1     22    11    16   5  0  5  2  2  4  2  5  0  4  1  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5  0 | 0  1  1  39  5 | 0  1  6  25  4


22   SUB     1  1  2     23    11    16   5  0  5  2  2  4  2  5  0  3  1  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5  0 | 0  1  1  39  5 | 0  1  6  25  4


23   STO     1  0  9     24    11    16   5  0  5  2  2  4  2  5  0  3  1  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5  0 | 0  1  1  39  5 | 0  1  6  25  4


24   CAL     0  1  1     1     16    16   5  0  5  2  2  4  2  5  0  3  1  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5  0 | 0  1  1  39  5 | 0  1  6  25  4


1    INC     0  0  5     2     16    21   5  0  5  2  2  4  2  5  0  3  1  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5  0 | 0  1  1  39  5 | 0  1  6  25  4 | 0  6  11  25  3


2    LOD     0  0  4     3     16    21   5  0  5  2  2  4  2  5  3  3  1  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5  0 | 0  1  1  39  5 | 0  1  6  25  4 | 0  6  11  25  3


3    LIT     1  0  0     4     16    21   5  0  5  2  2  4  2  5  3  0  1  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5  0 | 0  1  1  39  5 | 0  1  6  25  4 | 0  6  11  25  3


4    LSS     0  0  1     5     16    21   5  0  5  2  2  4  2  5  0  0  1  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5  0 | 0  1  1  39  5 | 0  1  6  25  4 | 0  6  11  25  3


5    JPC     0  0  10    10    16    21   5  0  5  2  2  4  2  5  0  0  1  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5  0 | 0  1  1  39  5 | 0  1  6  25  4 | 0  6  11  25  3


10   LOD     0  0  4     11    16    21   5  0  5  2  2  4  2  5  3  0  1  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5  0 | 0  1  1  39  5 | 0  1  6  25  4 | 0  6  11  25  3


11   LIT     1  0  2     12    16    21   5  0  5  2  2  4  2  5  3  2  1  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5  0 | 0  1  1  39  5 | 0  1  6  25  4 | 0  6  11  25  3


12   LSS     0  0  1     13    16    21   5  0  5  2  2  4  2  5  0  2  1  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5  0 | 0  1  1  39  5 | 0  1  6  25  4 | 0  6  11  25  3


13   JPC     0  0  17    17    16    21   5  0  5  2  2  4  2  5  0  2  1  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5  0 | 0  1  1  39  5 | 0  1  6  25  4 | 0  6  11  25  3


17   LOD     0  0  4     18    16    21   5  0  5  2  2  4  2  5  3  2  1  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5  0 | 0  1  1  39  5 | 0  1  6  25  4 | 0  6  11  25  3


18   REG_B   2  0  0     19    16    21   5  0  5  2  2  4  2  5  3  2  8  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5  0 | 0  1  1  39  5 | 0  1  6  25  4 | 0  6  11  25  3


19   REG_L   0  0  3     20    16    21   5  0  5  2  2  4  2  5  3  2  8  0  0  0  0  0  0  0  0  0
Stack: 0  0  0  0  5  0 | 0  1  1  39  5 | 0  1  6  25  4 | 0  6  11  25  3


20   LOD     1  0  4     21    16    21   5  0  5  2  2  4  2  5  3  2  8  0  3  0  0  0  0  0  0  0
Stack: 0  0  0  0  5  0 | 0  1  1  39  5 | 0  1  6  25  4 | 0  6  11  25  3


21   LIT     2  0  1     22    16    21   5  0  5  2  2  4  2  5  3  2  8  0  3  1  0  0  0  0  0  0
Stack: 0  0  0  0  5  0 | 0  1  1  39  5 | 0  1  6  25  4 | 0  6  11  25  3


22   SUB     1  1  2     23    16    21   5  0  5  2  2  4  2  5  3  2  8  0  2  1  0  0  0  0  0  0
Stack: 0  0  0  0  5  0 | 0  1  1  39  5 | 0  1  6  25  4 | 0  6  11  25  3


23   STO     1  0  9     24    16    21   5  0  5  2  2  4  2  5  3  2  8  0  2  1  0  0  0  0  0  0
Stack: 0  0  0  0  5  0 | 0  1  1  39  5 | 0  1  6  25  4 | 0  6  11  25  3


24   CAL     0  1  1     1     21    21   5  0  5  2  2  4  2  5  3  2  8  0  2  1  0  0  0  0  0  0
Stack: 0  0  0  0  5  0 | 0  1  1  39  5 | 0  1  6  25  4 | 0  6  11  25  3


1    INC     0  0  5     2     21    26   5  0  5  2  2  4  2  5  3  2  8  0  2  1  0  0  0  0  0  0
Stack: 0  0  0  0  5  0 | 0  1  1  39  5 | 0  1  6  25  4 | 0  6  11  25  3 | 0  11  16  25  2


2    LOD     0  0  4     3     21    26   5  0  5  2  2  4  2  5  3  2  8  2  2  1  0  0  0  0  0  0
Stack: 0  0  0  0  5  0 | 0  1  1  39  5 | 0  1  6  25  4 | 0  6  11  25  3 | 0  11  16  25  2


3    LIT     1  0  0     4     21    26   5  0  5  2  2  4  2  5  3  2  8  2  0  1  0  0  0  0  0  0
Stack: 0  0  0  0  5  0 | 0  1  1  39  5 | 0  1  6  25  4 | 0  6  11  25  3 | 0  11  16  25  2


4    LSS     0  0  1     5     21    26   5  0  5  2  2  4  2  5  3  2  8  0  0  1  0  0  0  0  0  0
Stack: 0  0  0  0  5  0 | 0  1  1  39  5 | 0  1  6  25  4 | 0  6  11  25  3 | 0  11  16  25  2


5    JPC     0  0  10    10    21    26   5  0  5  2  2  4  2  5  3  2  8  0  0  1  0  0  0  0  0  0
Stack: 0  0  0  0  5  0 | 0  1  1  39  5 | 0  1  6  25  4 | 0  6  11  25  3 | 0  11  16  25  2


10   LOD     0  0  4     11    21    26   5  0  5  2  2  4  2  5  3  2  8  2  0  1  0  0  0  0  0  0
Stack: 0  0  0  0  5  0 | 0  1  1  39  5 | 0  1  6  25  4 | 0  6  11  25  3 | 0  11  16  25  2


11   LIT     1  0  2     12    21    26   5  0  5  2  2  4  2  5  3  2  8  2  2  1  0  0  0  0  0  0
Stack: 0  0  0  0  5  0 | 0  1  1  39  5 | 0  1  6  25  4 | 0  6  11  25  3 | 0  11  16  25  2


12   LSS     0  0  1     13    21    26   5  0  5  2  2  4  2  5  3  2  8  0  2  1  0  0  0  0  0  0
Stack: 0  0  0  0  5  0 | 0  1  1  39  5 | 0  1  6  25  4 | 0  6  11  25  3 | 0  11  16  25  2


13   JPC     0  0  17    17    21    26   5  0  5  2  2  4  2  5  3  2  8  0  2  1  0  0  0  0  0  0
Stack: 0  0  0  0  5  0 | 0  1  1  39  5 | 0  1  6  25  4 | 0  6  11  25  3 | 0  11  16  25  2


17   LOD     0  0  4     18    21    26   5  0  5  2  2  4  2  5  3  2  8  2  2  1  0  0  0  0  0  0
Stack: 0  0  0  0  5  0 | 0  1  1  39  5 | 0  1  6  25  4 | 0  6  11  25  3 | 0  11  16  25  2


18   REG_B   2  0  0     19    21    26   5  0  5  2  2  4  2  5  3  2  8  2  2  11  0  0  0  0  0  0
Stack: 0  0  0  0  5  0 | 0  1  1  39  5 | 0  1  6  25  4 | 0  6  11  25  3 | 0  11  16  25  2


19   REG_L   0  0  3     20    21    26   5  0  5  2  2  4  2  5  3  2  8  2  2  11  0  0  0  0  0  0
Stack: 0  0  0  0  5  0 | 0  1  1  39  5 | 0  1  6  25  4 | 0  6  11  25  3 | 0  11  16  25  2


20   LOD     1  0  4     21    21    26   5  0  5  2  2  4  2  5  3  2  8  2  2  11  0  2  0  0  0  0
Stack: 0  0  0  0  5  0 | 0  1  1  39  5 | 0  1  6  25  4 | 0  6  11  25  3 | 0  11  16  25  2


21   LIT     2  0  1     22    21    26   5  0  5  2  2  4  2  5  3  2  8  2  2  11  0  2  1  0  0  0
Stack: 0  0  0  0  5  0 | 0  1  1  39  5 | 0  1  6  25  4 | 0  6  11  25  3 | 0  11  16  25  2


22   SUB     1  1  2     23    21    26   5  0  5  2  2  4  2  5  3  2  8  2  2  11  0  1  1  0  0  0
Stack: 0  0  0  0  5  0 | 0  1  1  39  5 | 0  1  6  25  4 | 0  6  11  25  3 | 0  11  16  25  2


23   STO     1  0  9     24    21    26   5  0  5  2  2  4  2  5  3  2  8  2  2  11  0  1  1  0  0  0
Stack: 0  0  0  0  5  0 | 0  1  1  39  5 | 0  1  6  25  4 | 0  6  11  25  3 | 0  11  16  25  2


24   CAL     0  1  1     1     26    26   5  0  5  2  2  4  2  5  3  2  8  2  2  11  0  1  1  0  0  0
Stack: 0  0  0  0  5  0 | 0  1  1  39  5 | 0  1  6  25  4 | 0  6  11  25  3 | 0  11  16  25  2


1    INC     0  0  5     2     26    31   5  0  5  2  2  4  2  5  3  2  8  2  2  11  0  1  1  0  0  0
Stack: 0  0  0  0  5  0 | 0  1  1  39  5 | 0  1  6  25  4 | 0  6  11  25  3 | 0  11  16  25  2 | 0  16  21  25  1


2    LOD     0  0  4     3     26    31   5  0  5  2  2  4  2  5  3  2  8  2  2  11  1  1  1  0  0  0
Stack: 0  0  0  0  5  0 | 0  1  1  39  5 | 0  1  6  25  4 | 0  6  11  25  3 | 0  11  16  25  2 | 0  16  21  25  1


3    LIT     1  0  0     4     26    31   5  0  5  2  2  4  2  5  3  2  8  2  2  11  1  0  1  0  0  0
Stack: 0  0  0  0  5  0 | 0  1  1  39  5 | 0  1  6  25  4 | 0  6  11  25  3 | 0  11  16  25  2 | 0  16  21  25  1


4    LSS     0  0  1     5     26    31   5  0  5  2  2  4  2  5  3  2  8  2  2  11  0  0  1  0  0  0
Stack: 0  0  0  0  5  0 | 0  1  1  39  5 | 0  1  6  25  4 | 0  6  11  25  3 | 0  11  16  25  2 | 0  16  21  25  1


5    JPC     0  0  10    10    26    31   5  0  5  2  2  4  2  5  3  2  8  2  2  11  0  0  1  0  0  0
Stack: 0  0  0  0  5  0 | 0  1  1  39  5 | 0  1  6  25  4 | 0  6  11  25  3 | 0  11  16  25  2 | 0  16  21  25  1


10   LOD     0  0  4     11    26    31   5  0  5  2  2  4  2  5  3  2  8  2  2  11  1  0  1  0  0  0
Stack: 0  0  0  0  5  0 | 0  1  1  39  5 | 0  1  6  25  4 | 0  6  11  25  3 | 0  11  16  25  2 | 0  16  21  25  1


11   LIT     1  0  2     12    26    31   5  0  5  2  2  4  2  5  3  2  8  2  2  11  1  2  1  0  0  0
Stack: 0  0  0  0  5  0 | 0  1  1  39  5 | 0  1  6  25  4 | 0  6  11  25  3 | 0  11  16  25  2 | 0  16  21  25  1


12   LSS     0  0  1     13    26    31   5  0  5  2  2  4  2  5  3  2  8  2  2  11  1  2  1  0  0  0
Stack: 0  0  0  0  5  0 | 0  1  1  39  5 | 0  1  6  25  4 | 0  6  11  25  3 | 0  11  16  25  2 | 0  16  21  25  1


13   JPC     0  0  17    14    26    31   5  0  5  2  2  4  2  5  3  2  8  2  2  11  1  2  1  0  0  0
Stack: 0  0  0  0  5  0 | 0  1  1  39  5 | 0  1  6  25  4 | 0  6  11  25  3 | 0  11  16  25  2 | 0  16  21  25  1


14   LIT     0  0  1     15    26    31   5  0  5  2  2  4  2  5  3  2  8  2  2  11  1  2  1  0  0  0
Stack: 0  0  0  0  5  0 | 0  1  1  39  5 | 0  1  6  25  4 | 0  6  11  25  3 | 0  11  16  25  2 | 0  16  21  25  1


15   STO     0  0  0     16    26    31   5  0  5  2  2  4  2  5  3  2  8  2  2  11  1  2  1  0  0  0
Stack: 0  0  0  0  5  0 | 0  1  1  39  5 | 0  1  6  25  4 | 0  6  11  25  3 | 0  11  16  25  2 | 1  16  21  25  1


16   RTN     0  0  0     25    21    26   5  0  5  2  2  4  2  5  3  2  8  2  2  11  1  2  1  0  0  0
Stack: 0  0  0  0  5  0 | 0  1  1  39  5 | 0  1  6  25  4 | 0  6  11  25  3 | 0  11  16  25  2


25   REG_R   2  0  0     26    21    26   5  0  5  2  2  4  2  5  3  2  8  2  2  11  1  2  1  0  0  0
Stack: 0  0  0  0  5  0 | 0  1  1  39  5 | 0  1  6  25  4 | 0  6  11  25  3 | 0  11  16  25  2


26   LOD     1  0  5     27    21    26   5  0  5  2  2  4  2  5  3  2  8  2  1  11  1  2  1  0  0  0
Stack: 0  0  0  0  5  0 | 0  1  1  39  5 | 0  1  6  25  4 | 0  6  11  25  3 | 0  11  16  25  2


27   MUL     0  0  1     28    21    26   5  0  5  2  2  4  2  5  3  2  8  2  1  11  1  2  1  0  0  0
Stack: 0  0  0  0  5  0 | 0  1  1  39  5 | 0  1  6  25  4 | 0  6  11  25  3 | 0  11  16  25  2


28   STO     0  0  0     29    21    26   5  0  5  2  2  4  2  5  3  2  8  2  1  11  1  2  1  0  0  0
Stack: 0  0  0  0  5  0 | 0  1  1  39  5 | 0  1  6  25  4 | 0  6  11  25  3 | 2  11  16  25  2


29   RTN     0  0  0     25    16    21   5  0  5  2  2  4  2  5  3  2  8  2  1  11  1  2  1  0  0  0
Stack: 0  0  0  0  5  0 | 0  1  1  39  5 | 0  1  6  25  4 | 0  6  11  25  3


25   REG_R   2  0  0     26    16    21   5  0  5  2  2  4  2  5  3  2  8  2  1  11  1  2  1  0  0  0
Stack: 0  0  0  0  5  0 | 0  1  1  39  5 | 0  1  6  25  4 | 0  6  11  25  3


26   LOD     1  0  5     27    16    21   5  0  5  2  2  4  2  5  3  2  8  2  1  11  1  2  1  0  0  0
Stack: 0  0  0  0  5  0 | 0  1  1  39  5 | 0  1  6  25  4 | 0  6  11  25  3


27   MUL     0  0  1     28    16    21   5  0  5  2  2  4  2  5  6  2  8  2  1  11  1  2  1  0  0  0
Stack: 0  0  0  0  5  0 | 0  1  1  39  5 | 0  1  6  25  4 | 0  6  11  25  3


28   STO     0  0  0     29    16    21   5  0  5  2  2  4  2  5  6  2  8  2  1  11  1  2  1  0  0  0
Stack: 0  0  0  0  5  0 | 0  1  1  39  5 | 0  1  6  25  4 | 6  6  11  25  3


29   RTN     0  0  0     25    11    16   5  0  5  2  2  4  2  5  6  2  8  2  1  11  1  2  1  0  0  0
Stack: 0  0  0  0  5  0 | 0  1  1  39  5 | 0  1  6  25  4


25   REG_R   2  0  0     26    11    16   5  0  5  2  2  4  2  5  6  2  8  2  1  11  1  2  1  0  0  0
Stack: 0  0  0  0  5  0 | 0  1  1  39  5 | 0  1  6  25  4


26   LOD     1  0  5     27    11    16   5  0  5  2  2  4  6  5  6  2  8  2  1  11  1  2  1  0  0  0
Stack: 0  0  0  0  5  0 | 0  1  1  39  5 | 0  1  6  25  4


27   MUL     0  0  1     28    11    16   5  0  5  2  2  24  6  5  6  2  8  2  1  11  1  2  1  0  0  0
Stack: 0  0  0  0  5  0 | 0  1  1  39  5 | 0  1  6  25  4


28   STO     0  0  0     29    11    16   5  0  5  2  2  24  6  5  6  2  8  2  1  11  1  2  1  0  0  0
Stack: 0  0  0  0  5  0 | 0  1  1  39  5 | 24  1  6  25  4


29   RTN     0  0  0     25    6     11   5  0  5  2  2  24  6  5  6  2  8  2  1  11  1  2  1  0  0  0
Stack: 0  0  0  0  5  0 | 0  1  1  39  5


25   REG_R   2  0  0     26    6     11   5  0  5  2  2  24  6  5  6  2  8  2  1  11  1  2  1  0  0  0
Stack: 0  0  0  0  5  0 | 0  1  1  39  5


26   LOD     1  0  5     27    6     11   5  0  5  24  2  24  6  5  6  2  8  2  1  11  1  2  1  0  0  0
Stack: 0  0  0  0  5  0 | 0  1  1  39  5


27   MUL     0  0  1     28    6     11   5  0  120  24  2  24  6  5  6  2  8  2  1  11  1  2  1  0  0  0
Stack: 0  0  0  0  5  0 | 0  1  1  39  5


28   STO     0  0  0     29    6     11   5  0  120  24  2  24  6  5  6  2  8  2  1  11  1  2  1  0  0  0
Stack: 0  0  0  0  5  0 | 120  1  1  39  5


29   RTN     0  0  0     39    1     6    5  0  120  24  2  24  6  5  6  2  8  2  1  11  1  2  1  0  0  0
Stack: 0  0  0  0  5  0


39   REG_R   1  0  0     40    1     6    5  0  120  24  2  24  6  5  6  2  8  2  1  11  1  2  1  0  0  0
Stack: 0  0  0  0  5  0


40   LOD     0  0  6     41    1     6    120  0  120  24  2  24  6  5  6  2  8  2  1  11  1  2  1  0  0  0
Stack: 0  0  0  0  5  0


41   SIO     0  0  0     42    1     6    120  0  120  24  2  24  6  5  6  2  8  2  1  11  1  2  1  0  0  0
Stack: 0  0  0  0  5  0


PL/0 VM: 120

42   SIO     0  0  3     43    1     6    120  0  120  24  2  24  6  5  6  2  8  2  1  11  1  2  1  0  0  0
Stack: 0  0  0  0  5  0
```
