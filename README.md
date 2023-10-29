# Computer_Structure_Project

## Assembly Language Programs

This readme provides an overview of three assembly language programs: **Sort (S)**, **Binom**, **Disktest**, and **Triangle (A, B, C)**. Each program has a specific purpose and structure. 

### Assembler

The assembler is responsible for processing an assembly file, typically named `memin.txt`, containing assembly code, and generating an output file that represents the memory image.

#### MAIN Function

The assembler is organized around a `MAIN` function, which accepts command-line arguments specifying input and output files.

#### First Pass

In the first pass, the assembler scans the input file to identify and collect labels. Each label is stored in an array of `Label` structures, which include the label's name and its position within the assembly file.

#### Second Pass

The second pass involves generating the memory image in the `memin.txt` file. The assembler uses both the array of labels from the first pass and an array of integers to write the hexadecimal decimal numbers for each line, effectively creating the memory image for the program.

### Simulator

The simulator is designed to work with three input files: `memin.txt`, `irq2in.txt`, and `diskin.txt`. `memin.txt` specifies when the simulator receives an external interrupt from `irq2`, while `diskin.txt` describes the initial state of the hard disk. The simulator emulates processor operation.

The `MAIN` function opens input and output files and calls the `run_simulation` function, which acts as a loop as long as the processor is operational. The simulation process is divided into three main parts:

1. **Collect Instruction:** This function processes the relevant instruction line, breaking it down into its components.
2. **Instruction Decoding:** For I-type instructions, this step further dissects the command into relevant components.
3. **Execution:** The `executer` function executes the instruction and returns the PC (program counter) value to indicate the program's current location.

### Programs in Assembly Language

#### Sort

This program initializes values for sorting, including the first number's address, an initial index (starting as zero), and the size of the array to sort. The program then enters a loop, incrementing an index and checking if the element at the current index is smaller than the next one. If not, it calls a "swap" function to exchange the values. The program continues until the index traverses all numbers without finding a smaller one.

#### Binom

This program begins by initializing the stack pointer ($sp) to avoid stack overflow and loads values of `n` and `k` from memory. It performs recursive calls for `binom(n-1, k-1)` and `binom(n-1, k)` to calculate binomial coefficients. When the recursion terminates, it sums the results and completes the program.

#### Disktest

The program reads a buffer sector and stores its content in memory. It then iterates through sectors, summing their content, and writes the result back to the hard disk.

#### Triangle (A, B, C)

The program calculates the lengths of sides AB, BC, and CA, and then draws a triangle based on these values. It handles cases where AB is greater than BC and vice versa, using nested loops to color pixels according to specific criteria.


