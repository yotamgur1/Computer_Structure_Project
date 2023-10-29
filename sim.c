//=====================================================
//                  Outside libraries 
//=====================================================	
	#define _CRT_SECURE_NO_WARNINGS
	#include <stdio.h>
	#include <stdlib.h>
	#include <ctype.h>
	#include <math.h>
	#include <string.h>
	#include <inttypes.h>


//=====================================================
//         Definitions according to instructions
//=====================================================

	#define INPUT_MAX_LEN         500
	#define MEMIN_LINE_LENGTH     5
	#define DISKIN_LINE_LENGTH    5
	#define NUM_OF_PROC_REGISTERS 16
	#define NUM_OF_IO_REGISTERS   23
	#define MEM_DEPTH			  4096
	#define NUM_OF_ROWS			  256
	#define NUM_OF_COLS			  256
	#define NUM_OF_SECTORS        128
	#define NUM_OF_INTS_IN_SECTOR 128
	#define SECTOR_SIZE			  512


	typedef enum { //operations according pages 3-4
		add  = 0,
		sub  = 1,
		mul  = 2,
		and  = 3,
		or   = 4,
		xor  = 5,
		sll  = 6,
		sra  = 7,
		srl  = 8,
		beq  = 9,
		bne  = 10,
		blt  = 11,
		bgt  = 12,
		ble  = 13,
		bge  = 14,
		jal  = 15,
		lw   = 16,
		sw   = 17,
		reti = 18,
		in   = 19,
		out  = 20,
		halt = 21
	} Operations;

	typedef enum { //register according table in page 2
		zero = 0,
		imm  = 1,
		v0   = 2,
		a0   = 3,
		a1   = 4,
		a2   = 5,
		a3   = 6,
		t0   = 7,
		t1   = 8,
		t2   = 9,
		s0   = 10,
		s1   = 11,
		s2   = 12,
		gp   = 13,
		sp   = 14,
		ra   = 15
	} ProcRegisters;

	typedef enum { // according the table in pages 5-6
		irq0enable   = 0,	
		irq1enable   = 1,
		irq2enable   = 2,	
		irq0status   = 3,
		irq1status   = 4,	
		irq2status   = 5,	
		irqhandler   = 6,
		irqreturn    = 7,
		clks		 = 8, 	
		leds		 = 9, 	
		display7seg  = 10,	
		timerenable  = 11, 	
		timercurrent = 12, 
		timermax	 = 13, 	
		diskcmd		 = 14, 	
		disksector	 = 15, 
		diskbuffer	 = 16, 	
		diskstatus	 = 17,	
		reserved1	 = 18,
		reserved2	 = 19,	
		monitoraddr  = 20,	
		monitordata  = 21,
		monitorcmd	 = 22	
	} IORegs;

	typedef enum { //page 5
		no_cmd = 0,
		read   = 1,
		write  = 2
	} RD_WR;

	typedef enum { //disk status according page 6 
		available = 0,
		busy	  = 1
	}disk_status;


//=====================================================
//          Structures used in the program
//=====================================================
	typedef struct instruction {		//each instruction would be I-type apart from "halt" and "reti"
					   int	operation;	//8 bits
		unsigned short int	rd;			//4 bits
		unsigned short int	rs;			//4 bits
		unsigned short int	rt;			//4 bits
		unsigned	   int	immi;		//20 bits
	} Instruction;


//=====================================================
//					  Declerations
//=====================================================

			 int R[NUM_OF_PROC_REGISTERS]						   = { 0 };
			 int IORegister[NUM_OF_IO_REGISTERS]			       = { 0 };
			 int MEM[MEM_DEPTH]									   = { 0 };
	unsigned int hard_disk[NUM_OF_SECTORS * NUM_OF_INTS_IN_SECTOR] = { 0 };
			 int now_interrupt									   =   0  ;
	unsigned int monitor_data[NUM_OF_ROWS][NUM_OF_COLS]			   = { 0 };
	unsigned int irq2_num[MEM_DEPTH]							   = { 0 };
	unsigned int disk_num										   =   0  ;
			 int I_order										   =   0  ;
			 int mem_order										   =   0  ;
	unsigned int cycle											   =   0  ;
			 int count_irq2										   =   0  ;
			 int return_from_reti								   =   0  ;
			 int test											   =   1  ;
 //=====================================================
 //                 Functions
 //=====================================================

void update_irq2in(FILE* file_irq2_in)
{
	char irq_line[INPUT_MAX_LEN + 1];
	int i = 0;
	while (fgets(irq_line, INPUT_MAX_LEN + 1, file_irq2_in)) {
		irq2_num[i] = strtol(irq_line, NULL, 10);
		i++;
	}
	irq2_num[i] = -1;												// so we won't restart when the cycle starts again
}


void timer() { 
	if (IORegister[timerenable] == 1) {
		if (IORegister[timercurrent] == IORegister[timermax]) {
			IORegister[timercurrent] = 0;
			IORegister[irq0status] = 1;
		}
		else
			IORegister[timercurrent]++;
	}
}

void monitor_update() {
	unsigned int row = (IORegister[monitoraddr] / 256); //row number
	unsigned int col = (IORegister[monitoraddr] % 256); //col number
	monitor_data[row][col] = IORegister[monitordata];
	IORegister[monitorcmd] = 0;
}



int load_mem(FILE* file_memin) {   //load memin.txt into MEM[]
	int i = 0;
	char data[MEMIN_LINE_LENGTH + 1];
	while (fgets(data, MEMIN_LINE_LENGTH + 1, file_memin)) {
		if (strcmp(data, "\n") == 0)		//if its the end of line it sound'lt be counted as a line
			continue;
		MEM[i] = strtoll(data, NULL, 16);
		i++;
	}
}

int print_to_memout(FILE* file_memout) {  //called at the end of the run and print the MEM values into memout.txt
	for (int i = 0; i < MEM_DEPTH; i++)
		fprintf(file_memout,"%.5X\n", (MEM[i] & 0xFFFFF));
	return 0;
}	

void init_disk(FILE* file_diskin) {   //load diskin.txt into hard_disk[]
	char line_in[DISKIN_LINE_LENGTH + 2];
	int i = 0;
	while (fgets(line_in, DISKIN_LINE_LENGTH + 1, file_diskin)) {
		if (strcmp(line_in, "\n") == 0) ////if its the end of line it sound'lt be counted as a line
			continue;
		hard_disk[i] = strtol(line_in, NULL, 16);
		i++;
	}
}

void update_disk() {  //update the disk according to the value at IORegister[diskcmd], IORegister[disksector] & IORegister[diskbuffer]
	IORegister[diskstatus] = busy;
	int i = 0;
	if (IORegister[diskcmd] == read) {
		for (i = 0; i < NUM_OF_INTS_IN_SECTOR; i++)
			MEM[IORegister[diskbuffer] + i] = hard_disk[IORegister[disksector] * NUM_OF_INTS_IN_SECTOR + i];
	}

	if (IORegister[diskcmd] == write) {
		for (i = 0; i < NUM_OF_INTS_IN_SECTOR; i++)
			hard_disk[IORegister[disksector] * NUM_OF_INTS_IN_SECTOR + i] = MEM[IORegister[diskbuffer] + i];
	}
	disk_num = 1; 
}

void display_the_number(FILE* file_display7seg) {  //prints the display7seg into the file
	fprintf(file_display7seg, "%d ", cycle+1);
	fprintf(file_display7seg, "%08X\n", IORegister[display7seg]);
}

void lights_on(FILE* file_leds, int reg) {  //prints leds into leds.txt
	fprintf(file_leds, "%d ", cycle + 1);
	fprintf(file_leds, "%08X \n", reg);
}

char* collect_inst(FILE* file_memin, int PC) { //collect the current instruction from memin.txt
	//printf("%d \n", PC);
	char instruction[MEMIN_LINE_LENGTH + 1];
	char no_instr[] = "00000";
	int i = 0;
	rewind(file_memin);
	int read = 1;
	while (read) {
		read = fgets(instruction, MEMIN_LINE_LENGTH + 1, file_memin);
		if (strcmp(instruction, "\n") == 0)			//if its the end of the line shouldnt be counted as a line 
			continue;
		if (i == PC)								//so its the right place- get this line
			break;
		i++;
	}
	if (read == 0) {								
		return NULL;
	}
	return instruction;
}

long int collect_i_type(FILE* file_memin, int PC) { //if the I-type collect the I line with same method as collect_inst function
	char i_value_string[MEMIN_LINE_LENGTH + 1];
	int i = 0;
	rewind(file_memin);
	int read = 1;
	while (read) {
		read = fgets(i_value_string, MEMIN_LINE_LENGTH + 1, file_memin);
		if (strcmp(i_value_string, "\n") == 0) //if its the end of the line
			continue;
		if (i == PC)
			break;
		i++;
	}
	long int i_value = strtoll(i_value_string, NULL, 16);
	return (i_value & 0xFFFFF);
}



void instruction_decoder (char* instruction_string, Instruction* instruction,int PC, FILE* file_memin) { //decode the instrucrion
	//printf("%d \n", PC);
	long long int instruction_int = strtoll(instruction_string, NULL, 16);
	instruction->operation = ((instruction_int & 0xFF000) >> 12);   //mask the operation
	instruction->rd = ((instruction_int & 0x00F00) >> 8);			//mask rd
	instruction->rs = ((instruction_int & 0x000F0) >> 4);			//mask rs
	instruction->rt = ((instruction_int & 0x0000F));				// mask rt
	if (((instruction->operation != halt) & (instruction->operation != reti)) & \
		((instruction->rs == imm) | (instruction->rt == imm) | (instruction->rd == imm))) {		//if so - its an I-type so imm field needs to be added
		PC++;												//because the function doesnt allowed to change PC (void) PC is changed just locally
		instruction->immi = collect_i_type(file_memin, PC); //similar to collect_inst function but collect the I number
	}
	else {
		instruction->immi = 0;								// if its not an I-type, register omm set to zero
	}
}

void update_file_hwregtrace(FILE* file_hwregtrace, char* instruction_string, int reg) { //print to hwregtrace.txt according to the instructions
	fprintf(file_hwregtrace, "%d ", cycle+1);
	long long int operation = strtoll(instruction_string, NULL, 16);
	int opeartion_int = ((operation & 0xFF000) >> 12);
	(opeartion_int == in) ? (fprintf(file_hwregtrace, "%s ", "READ")) : (fprintf(file_hwregtrace, "%s ", "WRITE"));
	switch (reg) {
		case irq0enable:
			fprintf(file_hwregtrace, "%s", "irq0enable ");
			break;
		case irq1enable:
			fprintf(file_hwregtrace, "%s", "irq1enable ");
			break;
		case irq2enable:
			fprintf(file_hwregtrace, "%s", "irq2enable ");
			break;
		case irq0status:
			fprintf(file_hwregtrace, "%s", "irq0status ");
			break;
		case irq1status:
			fprintf(file_hwregtrace, "%s", "irq1status ");
			break;
		case irq2status:
			fprintf(file_hwregtrace, "%s", "irq2status ");
			break;
		case irqhandler:
			fprintf(file_hwregtrace, "%s", "irqhandler ");
			break;
		case irqreturn:
			fprintf(file_hwregtrace, "%s", "irqreturn ");
			break;
		case clks:
			fprintf(file_hwregtrace, "%s", "clks ");
			break;
		case leds:
			fprintf(file_hwregtrace, "%s", "leds ");
			break;
		case display7seg:
			fprintf(file_hwregtrace, "%s", "display7seg ");
			break;
		case timerenable:
			fprintf(file_hwregtrace, "%s", "timerenable ");
			break;
		case timercurrent:
			fprintf(file_hwregtrace, "%s", "timercurrent ");
			break;
		case timermax:
			fprintf(file_hwregtrace, "%s", "timermax ");
			break;
		case diskcmd:
			fprintf(file_hwregtrace, "%s", "discmd ");
			break;
		case disksector:
			fprintf(file_hwregtrace, "%s", "disksector ");
			break;
		case diskbuffer:
			fprintf(file_hwregtrace, "%s", "diskbuffer ");
			break;
		case diskstatus:
			fprintf(file_hwregtrace, "%s", "diskstatus ");
			break;
		case reserved1:
			fprintf(file_hwregtrace, "%s", "reserved1 ");
			break;
		case reserved2:
			fprintf(file_hwregtrace, "%s", "reserved2 ");
			break;
		case monitoraddr:
			fprintf(file_hwregtrace, "%s", "monitoraddr ");
			break;
		case monitordata:
			fprintf(file_hwregtrace, "%s", "monitordata ");
			break;
		case monitorcmd:
			fprintf(file_hwregtrace, "%s", "monitorcmd ");
			break;
	}
	fprintf(file_hwregtrace, "%08X\n", IORegister[reg]);
}



int executer(Instruction instruction, char* instruction_string, int PC, FILE* file_trace, \
	FILE* file_hwregtrace, FILE* file_leds, FILE* file_display7seg) { //in charge of executing the instructions

	unsigned	   int immi = instruction.immi;
	unsigned short int rd   = instruction.rd  ;
	unsigned short int rs   = instruction.rs  ;
	unsigned short int rt   = instruction.rt  ;
	
	(((instruction.operation != halt) & (instruction.operation != reti)) & ((rs == imm) | (rt == imm) | (rd == imm))) ? \
		(((immi & (1 << 19))>>19) ? (R[imm] = immi | 0xFFF00000) : (R[imm] = immi)) : (R[imm] = 0); //if its a I type change R[imm] (with regard to sign extension) if not, R[imm] = 0 
	
	fprintf(file_trace, "%.3X %s %.8X %.8X %.8X %.8X %.8X %.8X %.8X %.8X %.8X %.8X %.8X %.8X %.8X %.8X %.8X %.8X\n", PC, instruction_string, \
		R[zero], R[imm], R[v0], R[a0], R[a1], R[a2], R[a3], R[t0], R[t1], R[t2], R[s0], R[s1], R[s2], R[gp], R[sp], R[ra]); //print to trace.txt

	switch (instruction.operation) { 
		/*between the different operations.
		every instruction would change PC with regard to the type of the operation (mainly if an access to memory is neede or not),
		and if its an I - type*/
	case add:
 			((rd != zero) & (rd != imm)) && (R[rd] = (R[rt] + R[rs]));
			test ? (((rt == imm) | (rs == imm) | (rd == imm)) ? (cycle = cycle + 2) : (cycle++)) : (((rt == imm) | (rs == imm) | (rd == imm)) && (I_order = 1)); 
			((rt == imm) | (rs == imm) | (rd == imm)) ? (PC = PC + 2) : (PC++);
			return PC;		
	case sub:
			((rd != zero) & (rd != imm)) && (R[rd] = (R[rs] - R[rt]));
			test ? (((rt == imm) | (rs == imm) | (rd == imm)) ? (cycle = cycle + 2) : (cycle++)) : (((rt == imm) | (rs == imm) | (rd == imm)) && (I_order = 1));
			((rt == imm) | (rs == imm) | (rd == imm)) ? (PC = PC + 2) : (PC++);
			return PC;
	case mul:
			((rd != zero) & (rd != imm)) && (R[rd] = (R[rs] * R[rt]));
			test ? (((rt == imm) | (rs == imm) | (rd == imm)) ? (cycle = cycle + 2) : (cycle++)) : (((rt == imm) | (rs == imm) | (rd == imm)) && (I_order = 1));
			((rt == imm) | (rs == imm) | (rd == imm)) ? (PC = PC + 2) : (PC++);
			return PC;
	case and:
			((rd != zero) & (rd != imm)) && (R[rd] = (R[rs] & R[rt]));
			test ? (((rt == imm) | (rs == imm) | (rd == imm)) ? (cycle = cycle + 2) : (cycle++)) : (((rt == imm) | (rs == imm) | (rd == imm)) && (I_order = 1));
			((rt == imm) | (rs == imm) | (rd == imm)) ? (PC = PC + 2) : (PC++);
			return PC;
	case or:
			((rd != zero) & (rd != imm)) && (R[rd] = (R[rs] | R[rt]));
			test ? (((rt == imm) | (rs == imm) | (rd == imm)) ? (cycle = cycle + 2) : (cycle++)) : (((rt == imm) | (rs == imm) | (rd == imm)) && (I_order = 1));
			((rt == imm) | (rs == imm) | (rd == imm)) ? (PC = PC + 2) : (PC++);
			return PC;
	case xor:
			((rd != zero) & (rd != imm)) && (R[rd] = (R[rs] ^ R[rt]));
			test ? (((rt == imm) | (rs == imm) | (rd == imm)) ? (cycle = cycle + 2) : (cycle++)) : (((rt == imm) | (rs == imm) | (rd == imm)) && (I_order = 1));
			((rt == imm) | (rs == imm) | (rd == imm)) ? (PC = PC + 2) : (PC++);
			return PC;
	case sll:
			((rd != zero) & (rd != imm)) && (R[rd] = (R[rs] << R[rt]));
			test ? (((rt == imm) | (rs == imm) | (rd == imm)) ? (cycle = cycle + 2) : (cycle++)) : (((rt == imm) | (rs == imm) | (rd == imm)) && (I_order = 1));
			((rt == imm) | (rs == imm) | (rd == imm)) ? (PC = PC + 2) : (PC++);
			return PC;
	case sra: 
			((rd != zero) & (rd != imm)) && (R[rd] = (R[rs] >> R[rt]));
			test ? (((rt == imm) | (rs == imm) | (rd == imm)) ? (cycle = cycle + 2) : (cycle++)) : (((rt == imm) | (rs == imm) | (rd == imm)) && (I_order = 1));
			((rt == imm) | (rs == imm) | (rd == imm)) ? (PC = PC + 2) : (PC++);
			return PC;
	case srl:
			((rd != zero) & (rd != imm)) && (R[rd] = (((unsigned int)R[rs]) >> R[rt]));  // unsigned int would make the shift be logical (not coppying the MSB)
			test ? (((rt == imm) | (rs == imm) | (rd == imm)) ? (cycle = cycle + 2) : (cycle++)) : (((rt == imm) | (rs == imm) | (rd == imm)) && (I_order = 1));
			((rt == imm) | (rs == imm) | (rd == imm)) ? (PC = PC + 2) : (PC++);
			return PC;
	case beq:
		if (R[rs] == R[rt]) {
			test ? (((rt == imm) | (rs == imm) | (rd == imm)) ? (cycle = cycle + 2) : (cycle++)) : (((rt == imm) | (rs == imm) | (rd == imm)) && (I_order = 1));
			return(R[rd] & 0xFFFFF);
		}
		else {
			test ? (((rt == imm) | (rs == imm) | (rd == imm)) ? (cycle = cycle + 2) : (cycle++)) : (((rt == imm) | (rs == imm) | (rd == imm)) && (I_order = 1));
			((rt == imm) | (rs == imm) | (rd == imm)) ? (PC = PC + 2) : (PC++);
			return PC;
		}
	case bne:
		if (R[rs] != R[rt]) {
			test ? (((rt == imm) | (rs == imm) | (rd == imm)) ? (cycle = cycle + 2) : (cycle++)) : (((rt == imm) | (rs == imm) | (rd == imm)) && (I_order = 1));
			return (R[rd] & 0xFFFFF);
		}
		else {
			test ? (((rt == imm) | (rs == imm) | (rd == imm)) ? (cycle = cycle + 2) : (cycle++)) : (((rt == imm) | (rs == imm) | (rd == imm)) && (I_order = 1));
			((rt == imm) | (rs == imm) | (rd == imm)) ? (PC = PC + 2) : (PC++);
			return PC;
		}
	case blt:
		if (R[rs] < R[rt]) {
			test ? (((rt == imm) | (rs == imm) | (rd == imm)) ? (cycle = cycle + 2) : (cycle++)) : (((rt == imm) | (rs == imm) | (rd == imm)) && (I_order = 1));
			return (R[rd] & 0xFFFFF);
		}
		else {
			test ? (((rt == imm) | (rs == imm) | (rd == imm)) ? (cycle = cycle + 2) : (cycle++)) : (((rt == imm) | (rs == imm) | (rd == imm)) && (I_order = 1));
			((rt == imm) | (rs == imm) | (rd == imm)) ? (PC = PC + 2) : (PC++);
			return PC;
		}
	case bgt:
		if (R[rs] > R[rt]) {
			test ? (((rt == imm) | (rs == imm) | (rd == imm)) ? (cycle = cycle + 2) : (cycle++)) : (((rt == imm) | (rs == imm) | (rd == imm)) && (I_order = 1));
			return (R[rd] & 0xFFFFF);
		}
		else {
			test ? (((rt == imm) | (rs == imm) | (rd == imm)) ? (cycle = cycle + 2) : (cycle++)) : (((rt == imm) | (rs == imm) | (rd == imm)) && (I_order = 1));
			((rt == imm) | (rs == imm) | (rd == imm)) ? (PC = PC + 2) : (PC++);
			return PC;
		}
	case ble:
		if (R[rs] <= R[rt]) {
			test ? (((rt == imm) | (rs == imm) | (rd == imm)) ? (cycle = cycle + 2) : (cycle++)) : (((rt == imm) | (rs == imm) | (rd == imm)) && (I_order = 1));
			return (R[rd] & 0xFFFFF);
		}
		else {
			test ? (((rt == imm) | (rs == imm) | (rd == imm)) ? (cycle = cycle + 2) : (cycle++)) : (((rt == imm) | (rs == imm) | (rd == imm)) && (I_order = 1));
			((rt == imm) | (rs == imm) | (rd == imm)) ? (PC = PC + 2) : (PC++);
			return PC;
		}
	case bge:
		if (R[rs] >= R[rt]) {
			test ? (((rt == imm) | (rs == imm) | (rd == imm)) ? (cycle = cycle + 2) : (cycle++)) : (((rt == imm) | (rs == imm) | (rd == imm)) && (I_order = 1));
			return (R[rd] & 0xFFFFF);
		}
		else {
			test ? (((rt == imm) | (rs == imm) | (rd == imm)) ? (cycle = cycle + 2) : (cycle++)) : (((rt == imm) | (rs == imm) | (rd == imm)) && (I_order = 1));
			((rt == imm) | (rs == imm) | (rd == imm)) ? (PC = PC + 2) : (PC++);
			return PC;
		}
	case jal:
		if ((rd != zero) & (rd != imm)) {
			((rt == imm) | (rs == imm)) ? (R[rd] = PC + 2) : (R[rd] = PC + 1);
			test ? (((rt == imm) | (rs == imm) | (rd == imm)) ? (cycle = cycle + 2) : (cycle++)) : (((rt == imm) | (rs == imm) | (rd == imm)) && (I_order = 1));
			return (R[rs] & 0xFFFFF);
		}
		else {
			printf("rd is zero or imm so not possible to write to it the current PC, skiping instruction..");
			test ? (((rt == imm) | (rs == imm) | (rd == imm)) ? (cycle = cycle + 2) : (cycle++)) : (((rt == imm) | (rs == imm) | (rd == imm)) && (I_order = 1));
			((rt == imm) | (rs == imm) | (rd == imm)) ? (PC = PC + 2) : (PC++);
			return PC;
		}
	case lw:
		test ? (((rt == imm) | (rs == imm) | (rd == imm)) ? (cycle = cycle + 3) : (cycle = cycle + 2)) : (((rt == imm) | (rs == imm) | (rd == imm)) && (I_order = 1));
		((rd == imm) | (rt == imm) | (rs == imm)) ? (PC = PC + 2) : (PC = PC + 1);
		((rd != zero) & (rd != imm)) && (R[rd] = ((MEM[R[rs] + R[rt]]))); //check its not wirting into the imm or zero registers
		((R[rd] & (1 << 19)) >> 19) && (R[rd] = R[rd] | 0xFFF00000);
		return PC;
	case sw:
 		test ? (((rt == imm) | (rs == imm) | (rd == imm)) ? (cycle = cycle + 3) : (cycle = cycle + 2)) : (((rt == imm) | (rs == imm) | (rd == imm)) && (I_order = 1));
		((rt == imm) | (rs == imm) | (rs == imm)) ? (PC = PC + 2) : (PC = PC + 1);
		(MEM[R[rs] + R[rt]]) = (R[rd]);
		return PC;
	case reti:
		test ? (((rt == imm) | (rs == imm) | (rd == imm)) ? (cycle = cycle + 2) : (cycle++)) : (((rt == imm) | (rs == imm) | (rd == imm)) && (I_order = 1));
		now_interrupt = 0;
		return IORegister[irqreturn];
	case in:
		if ((rd != zero) & (rd != imm)) { //check its not writing into the zero or imm registers
			R[rd] = (IORegister[R[rs] + R[rt]]);
			update_file_hwregtrace(file_hwregtrace, instruction_string, (R[rs]+R[rt])); // its a read instruction
		}
		test ? (((rt == imm) | (rs == imm) | (rd == imm)) ? (cycle = cycle + 2) : (cycle++)) : (((rt == imm) | (rs == imm) | (rd == imm)) && (I_order = 1));
		((rd == imm) | (rt == imm) | (rs == imm)) ? (PC = PC + 2) : (PC++);
		return PC;
	case out:
		IORegister[R[rs] + R[rt]] = R[rd];
		update_file_hwregtrace(file_hwregtrace, instruction_string, (R[rs] + R[rt])); //its a write instruction
		switch (R[rs] + R[rt]) {
		case leds:
			lights_on(file_leds, R[rd]);
			break;
		case display7seg:
			display_the_number(file_display7seg);
			break;
		case monitorcmd:
			if (IORegister[monitorcmd] == 1)
				monitor_update();
			break;
		case diskcmd:
			if ((IORegister[diskcmd] != 0) & (IORegister[diskstatus] == 0)) 
				update_disk(); //
			break;
		}
		test ? (((rt == imm) | (rs == imm) | (rd == imm)) ? (cycle = cycle + 2) : (cycle++)) : (((rt == imm) | (rs == imm) | (rd == imm)) && (I_order = 1));
		((rd == imm) | (rt == imm) | (rs == imm)) ? (PC = PC + 2) : (PC++);
		return PC;
	case halt:
		test ? (((rt == imm) | (rs == imm) | (rd == imm)) ? (cycle = cycle + 2) : (cycle++)) : (((rt == imm) | (rs == imm) | (rd == imm)) && (I_order = 1));
		return MEM_DEPTH; //if PC == MEM_DEPTH its will cause the program to finish- like halt instruction should do.
	default:
		printf("Not a valid operation, skipping it...");
		test ? (((rt == imm) | (rs == imm) | (rd == imm)) ? (cycle = cycle + 2) : (cycle++)) : (((rt == imm) | (rs == imm) | (rd == imm)) && (I_order = 1));
		((rd == imm) | (rt == imm) | (rs == imm)) ? (PC = PC + 2) : (PC++);
		return PC;
	}

}


int check_if_interrupt_by_irq2() {  //check if there is a interrupt from irq2, using the irq2_num made at the beginning of the run
	if (irq2_num[count_irq2] < cycle) {
		IORegister[irq2status] = 1;
			count_irq2++;
			return 1;
	}
	else{
		IORegister[irq2status] = 0;
		return 0;
		}
};



void print_to_monitor(FILE* file_monitor_txt, FILE* file_monitor_yuv, unsigned int monitor_data[NUM_OF_ROWS][NUM_OF_COLS]) { 
	//at the end of the run, prints to monitor.txt and monitor.yuv according to the monitor_data array used
	for (int i = 0; i < NUM_OF_ROWS; i++) {					
		for (int j = 0; j < NUM_OF_COLS; j++) {											
			fprintf(file_monitor_txt, "%02X\n", monitor_data[i][j]);
			fprintf(file_monitor_yuv, "%C", monitor_data[i][j]);
		}
	}
}

void print_to_diskout(FILE* file_diskout) { //at the end of the run, prints the values of hard_disk into diskout.txt
	for (int i = 0; i < NUM_OF_SECTORS * NUM_OF_INTS_IN_SECTOR; i++) {
		fprintf(file_diskout, "%05X\n", (hard_disk[i] & 0xFFFFF));
	}
}

void finisher_printer(FILE* file_cycles, FILE* file_monitor_txt, FILE* file_monitor_yuv, unsigned int monitor_data[256][256], FILE* file_diskout, FILE* file_memout, FILE* file_regout) {
	//in charge of the printing need to be made at the end of the run
	fprintf(file_cycles, "%d", cycle);
	print_to_monitor(file_monitor_txt, file_monitor_yuv, monitor_data);
	print_to_diskout(file_diskout);
	print_to_memout(file_memout);
	fprintf(file_regout, "%.8X\n %.8X\n %.8X\n %.8X\n %.8X\n %.8X\n %.8X\n %.8X\n %.8X\n %.8X\n %.8X\n %.8X\n %.8X\n %.8X\n",\
		R[v0], R[a0], R[a1], R[a2], R[a3], R[t0], R[t1], R[t2], R[s0], R[s1], R[s2], R[gp], R[sp], R[ra]);
}
 
void run_simulation(FILE* file_memin, FILE* file_diskin, FILE* file_irq2in, FILE* file_memout, \
	FILE* file_regout, FILE* file_hwregtrace, FILE* file_cycles, FILE* file_leds,\
	FILE* file_display7seg, FILE* file_diskout, FILE* file_monitor_txt, FILE* file_monitor_yuv, FILE* file_trace) {
	/*this function is in chrage of spliting the job between the different functions, and collecting it
	into to the core we planned*/
	unsigned int PC			 = 0;
	unsigned int irq		 = 0;
	unsigned int halt_hit	 = 0;
	char instruction_string[MEMIN_LINE_LENGTH + 1];
	Instruction	current_instruction;
	IORegister[clks] = cycle;
	load_mem(file_memin);
	IORegister[irq1status] = 1;
	init_disk(file_diskin);
	update_irq2in(file_irq2in);

	do {
		check_if_interrupt_by_irq2(); 

		unsigned int irq = ((IORegister[irq0enable] & IORegister[irq0status]) | (IORegister[irq1enable] & IORegister[irq1status]) | (IORegister[irq2enable] & IORegister[irq2status]));

		timer();
		if ((irq == 1) & (now_interrupt == 0)) {
			now_interrupt = 1;

			IORegister[irqreturn] = PC;			//goes back to current location after interrpt finish
			PC = IORegister[irqhandler];		//jump to interrupt location
		};
		if (now_interrupt == 1) {
			update_disk();
		}

		switch (disk_num)
		{
		case 0:
			break;
		case 1024:								//time to handle instruction
			IORegister[diskstatus] = available;
			IORegister[diskcmd] = no_cmd;
			IORegister[irq1status] = 1;
			disk_num = 0;
			break;
		default:
			disk_num++;
		}


		strcpy(instruction_string, collect_inst(file_memin, PC)); 

		instruction_decoder(instruction_string, &current_instruction, PC, file_memin);
		
		PC = executer(current_instruction, instruction_string, PC, file_trace, file_hwregtrace, file_leds, file_display7seg);


		halt_hit = (PC == MEM_DEPTH); //if halt incountred, it will set PC to the MEM_DEPTH

		if (test == 0){
			cycle++;
			(mem_order == 1) && (cycle++);
			(I_order == 1) && (cycle++);
			mem_order = 0;
			I_order = 0;
		}
		//printf("MEM[256] = %d, MEM[257] = %d, MEM[258] = %d \n", MEM[256], MEM[257], MEM[258]);
	} while (!halt_hit);

	finisher_printer(file_cycles, file_monitor_txt, file_monitor_yuv, monitor_data, file_diskout, file_memout, file_regout);
	//printf("MEM[256] = %d, MEM[257] = %d, MEM[258] = %d, MEM[259] = %d, MEM[260] = %d, MEM[261] = %d, MEM[262] = %d, MEM[263] = %d, MEM[264] = %d, MEM[265] = %d, MEM[266] = %d, MEM[267] = %d,\
MEM[268] = %d, MEM[269] = %d, MEM[270] = %d, MEM[271] = %d", MEM[256], MEM[257], MEM[258], MEM[259], MEM[260], MEM[261], MEM[262], MEM[263], MEM[264], MEM[265], MEM[266], MEM[267], \
		MEM[268], MEM[269], MEM[270], MEM[271]);
	//printf("MEM[256] = %d, MEM[257] = %d, MEM[258] = %d", MEM[256], MEM[257], MEM[258]);
		 
}

//=====================================================
//                  Main Simulator Program
//=====================================================
int main(int argc, char** argv) {
	if (argc != 14) {
		if (argc < 14)
			printf("Not enough arguments for valid execution of the Assembler!");
		else
			printf("Too many arguments for valid execution of the Assembler!"  );
		return 1;
	}

	// Inputs 
	FILE* file_memin	   = fopen(argv[1] , "r");
	FILE* file_diskin	   = fopen(argv[2] , "r");
	FILE* file_irq2in	   = fopen(argv[3] , "r");
	FILE* file_memout	   = fopen(argv[4] , "w");
	FILE* file_regout      = fopen(argv[5] , "w");
	FILE* file_trace       = fopen(argv[6] , "w");
	FILE* file_hwregtrace  = fopen(argv[7] , "w");
	FILE* file_cycles	   = fopen(argv[8] , "w");
	FILE* file_leds		   = fopen(argv[9] , "w");
	FILE* file_display7reg = fopen(argv[10], "w");
	FILE* file_diskout	   = fopen(argv[11], "w");
	FILE* file_monitor_txt = fopen(argv[12], "w");
	FILE* file_monitor_yuv = fopen(argv[13], "wb");

	if (!(file_memin && file_diskin &&      file_irq2in &&  file_memout &&		file_regout && file_trace         &&   file_hwregtrace && \
		 file_cycles &&   file_leds && file_display7reg && file_diskout && file_monitor_txt && file_monitor_yuv)) {
		printf("*Cant open one or more of the files");
		exit(1);
	}

	run_simulation(file_memin, file_diskin, file_irq2in, file_memout, file_regout, file_hwregtrace, file_cycles, file_leds, file_display7reg, file_diskout, file_monitor_txt, file_monitor_yuv, file_trace);

	fclose(file_memin);
	fclose(file_diskin);
	fclose(file_irq2in);
	fclose(file_memout);
	fclose(file_regout);
	fclose(file_trace);
	fclose(file_hwregtrace);
	fclose(file_cycles);
	fclose(file_leds);
	fclose(file_display7reg);
	fclose(file_diskout);
	fclose(file_monitor_txt);
	fclose(file_monitor_yuv);


	return 0;
} 
