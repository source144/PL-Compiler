# PM/0 Virtual Machine
### Gonen Matias - COP 3402 Spring 2020
***Implemented in C***, the stack machine has **two** memory stores:
 1. The **stack**, which contains **data** to be used by the PM/0 CPU.
 2. The **Code** (or "text"), which contains **instructions** for the VM.

It has **four** registers to handle the stack and the code/text segments.

 1. **BP** - Base Pointer
 2. **SP** - Stack Pointer
 3. **PC** - Program Counter
 4. **IR**  - Instruction Register (*implemented as pointer to the instruction in memeory*)

This machine also has a Register File (**RF**) with eight (8) registers.

## Running the VM
To run the **PM/0 VM** you must first compile the source code to produce an executable.

    gcc pvm.c
    
 Once the code is compiled you are able to run the VM trough the command line as follows:
 
    ./a.exe [INPUT_FILE]

where **INPUT_FILE** is the file containing the instruction set to be executed by the VM.

**NOTE:** The program does not execute without an *INPUT_FILE*.

### Saving the output
To save the output of the a certain execution, run the VM with the following arguments:
 
    ./a.exe [INPUT_FILE] >[OUTPUT_FILE]

#### Checking for memory leaks
Comile the source code with the following:

    gcc -g pvm.c
    
Afterwards, run the program with the following line:

    valgrind --leak-check=full ./a.exe [INPUT_FILE] 
 as simple as that.


## Instruction Set Architecture
Each instruction contains **four components**: 

 1. **OP** - The operation code
 2. **R** -  Refers to a register
 3. **L** - The lexicographical level or a register in arithmetic and relational instructions
 4. **M** - Depending on the operation, it indicates a **constant**, **program address**, **data address**, or **a register**

### Available instructions
01. - **LIT** -  **R, 0, M** - Loads a constant value (literal) **M** into Register **R**

02. - **RTN**  0, **0, 0** - Returns from a subroutine and restore the caller environment

03. - **LOD  R, L, M**  - Load value into a selected register from the stack location at offset **M** from **L** lexicographical levels down

04. - **STO** -  - Store value from a selected register in the stack location at  offset **M** from **L** lexicographical levels down

05. - **CAL** - **0, L, M** - Call procedure at code index **M**

06. - **INC** - **0, 0, M** - Allocate **M** locals (increment sp by M). First four are **Functional Value,** **Static Link (SL)**, **Dynamic Link (DL)**,  and **Return Address (RA)**

07. - **JMP** - **0, 0, M** - Jump to instruction **M**

08. - **JPC** - **R, 0, M** - Jump to instruction **M** if **R** = 0

09. - **SIO** - **R, 0, 1** - Write a register to the screen

10. - **SIO** - **R, 0, 2** - Read in input from the user and store it in a register

11. - **SIO** - **0, 0, 3** - End of program (program stops running)

#### Arithmetic instructions
12. - **NEG** - Reg[**R**] = -Reg[**R**]
13. - **ADD** - Reg[**R**] = Reg[**L**] + Reg[**M**]
14. - **SUB** - Reg[**R**] = Reg[**L**] - Reg[**M**]
15. - **MUL** - Reg[**R**] = Reg[**L**] * Reg[**M**]
16. - **DIV** - Reg[**R**] = Reg[**L**] / Reg[**M**]
17. - **ODD** - Reg[**R**] = Reg[**L**] % 2
18. - **MOD** - Reg[**R**] = Reg[**L**] % Reg[**M**]
19. - **EQL** - Reg[**R**] = Reg[**L**] == Reg[**M**]
20. - **NEQ** - Reg[**R**] = Reg[**L**] != Reg[**M**]
21. - **LSS** - Reg[**R**] = Reg[**L**] < Reg[**M**]
22. - **LEQ** - Reg[**R**] = Reg[**L**] <= Reg[**M**]
23. - **GTR** - Reg[**R**] = Reg[**L**] > Reg[**M**]
24. - **GEQ** - Reg[**R**] = Reg[**L**] <= Reg[**M**]
# PL-Compiler
