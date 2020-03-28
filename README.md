
# PM/0 Virtual Machine and Compiler

### Gonen Matias - COP 3402 Spring 2020

***Implemented in C***, the stack machine has **two** memory stores:

1. The **stack**, which contains **data** to be used by the PM/0 CPU.

2. The **Code** (or "text"), which contains **instructions** for the VM.

  

It has **four** registers to handle the stack and the code/text segments.

  

1.  **BP** - Base Pointer

2.  **SP** - Stack Pointer

3.  **PC** - Program Counter

4.  **IR** - Instruction Register (*implemented as pointer to the instruction in memeory*)

  

This machine also has a Register File (**RF**) with eight (8) registers.

  

## Running the VM

To run the **PL/0 VM** you must first compile the **PVM**'s source code to produce an executable.
`gcc pvm.c -o pvm.exe`

Once the code is compiled you are able to run the VM provided a machine code **[INPUT_FILE]** as follows:
`./pvm.exe [INPUT_FILE]`

Furthermore, the **Virtual Machine** supports the following **Directives**:
**-a**  `./pvm.exe [INPUT_FILE] -a`  **prints** the **Decoded Instructions Set**
**-v**  `./pvm.exe [INPUT_FILE] -v`  **prints** the **VM State** on *each cycle*

### Saving the output

Both of these Directives write to `stdout`, so it's possible to direct the output to using the output redirection operator (**>**). For example: `./pvm.exe [INPUT_FILE] -v -a >vmlog.txt`  **writes** both the **Decoded Instructions Set** and the **VM State** to *vmlog.txt*. Note that you can change *vmlog.txt* to **any filepath** you wish.

  

#### Checking for memory leaks

Compile the source code with the following:
`gcc -g pvm.c -o pvm.exe`
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

(3) `lod`  **LOD R, L, M** - Load value into a selected register from the stack location at offset **M** from **L** 
lexicographical levels down

(4) `sto`  **STO** - - Store value from a selected register in the stack location at offset **M** from **L** lexicographical 
levels down

(5) `cal`  **CAL** - **0, L, M** - Call procedure at code index **M**

(6) `inc`  **INC** - **0, 0, M** - Allocate **M** locals (increment sp by M). First four are **Functional Value,**  **Static 
Link (SL)**, **Dynamic Link (DL)**, and **Return Address (RA)**

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

  

# PL/0-Compiler
The PL/0 Compiler consists of a **Scanner** and a **Parser** that both run as part of the compilation process.
The compiler reads PL/0 **source code** and converts it to **machine code** (an instruction set).

To run the **PL/0 Compiler** you must first compile the **Compiler**'s source code to produce an executable.
`gcc compiler.c -o compiler.exe`

Once the code is compiled you are able to run the Compiler provided a PL/0 source code **[INPUT_FILE]** as follows: `./compiler.exe [INPUT_FILE]`

The compiler will output and explain any error that occurs during the compilation process.
An example of that is:

    $ ./compile.exe codes/while05.txt -o compiled_factorial.plmc
    PL/0 COMPILER:
    INTERRUPTED - Assignment operator ":=" expected (ERR 13).
    LINE 15, 14:    factorial = i * factorial;
                              ^
By default the compiler saves the compiled **machine code** to `a.plmc` *(PLMC stands for PL/0 Machine Code)*.

You can configure the compiler using the following directives:

**-l**  `./compiler.exe [INPUT_FILE] -l`  **prints** the **lexeme** *table and list* produced from the source code.

**-a**  `./compiler.exe [INPUT_FILE] -a` **prints** the **produced Machine Code** (not decoded)

**-o** `./compiler.exe [INPUT_FILE] -o [OUTPUT_FILE]` - Directs the compiler to write the produced machine code to a specific file, which path to is **[OUTPUT_FILE]**.

For example the Following `./compiler.exe -a factorial.plc -o factorial.plmc` will compile the source code file, **factorial.plc**, and save the produced machine code to **factorial.plmc** *as well as* write (**print**) that **machine code** to **STDIN**.


# Compiling and Running PL/0 Code
To compile and run a PL/0 source code you have written you must first compile both the PL/0 Virtual Machine and the PL/0 Compiler (instructed previously). Once you have both ready, you can proceed.

Given a written **PL/0 source code** file: `factorial.plc`.

First, compile the source code by running the PL/0 compiler:
`./compiler.exe factorial.plc -o factorial.plmc`

Afterwards, run the produced machine code on the Virtual machine:
`./pvm.exe factorial.plmc`