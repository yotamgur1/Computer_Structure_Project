//=====================================================
//                  Outside libraries 
//=====================================================
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

//=====================================================
//         Definitions according to instructions
//=====================================================
#define ROWS_DEPTH 4096
#define REGIESTERS_NUMBER 16
#define MAX_ROW_LENGTH 500
#define MAX_LABEL_LENGTH 50


//=====================================================
//          Structures used in the program
//=====================================================
typedef struct label
{
	char name[MAX_LABEL_LENGTH + 1];
	int address;
} Label;

//=====================================================
//					  Declerations
//=====================================================
Label * create_labels_array(FILE *program);
int is_it_line_with_things(char * line);
int which_opcode_is_it(char * line);
int which_register_is_it(char * reg);
void remove_spaces(char * line);
int isDigit(char * word);
int hexa_string_to_decimal(char * hexa_string);
void remove_comments(char * line);
int which_immediate_is_it(char * imm_string, Label * label_list);
void print_lines_to_file(FILE *program, FILE *memin, Label * lables_list);

//=====================================================
//                 Functions
//=====================================================
Label * create_labels_array(FILE *program)
{
	Label lables_list[ROWS_DEPTH + 1];					 //this array will contain 4096 rows, in place where the adress!=-1 there is a Label
	char line[MAX_ROW_LENGTH + 1];
	char *found_label;
	int i = 0, p;
	while (fgets(line, sizeof(line), program)) {		//if there is a label we will put its adress and name, if not the adress will be -1
		p = is_it_line_with_things(line);
		if (p == 0)
			continue;
		found_label = strchr(line, ':');
		if (found_label != NULL)
		{
			*found_label = '\0';
			strcpy(lables_list[i].name, line);
			lables_list[i].address = i;
		}
		else
		{
			if (i == 0)
			{
				*lables_list[i].name = '\0';
				lables_list[i].address = -1;
			}
			if (strstr(line, "$imm") != NULL) {
				i++;
				*lables_list[i].name = '\0';
				lables_list[i].address = -1;
				*lables_list[i+1].name = '\0';
				lables_list[i+1].address = -1;
				i++;
			}
			else
			{
				if (strstr(line, ".word") == NULL)
				{
					i++;
					*lables_list[i].name = '\0';
					lables_list[i].address = -1;
				}
			}
		}
	}
	rewind(program);
	return & lables_list;
}

int is_it_line_with_things(char * line)															//this function will return 0 if its only a white spaces or comments and 1 if there is a value to the line
{
	char local_string[MAX_ROW_LENGTH];
	strcpy(local_string, line);
	remove_comments(local_string);
	remove_spaces(local_string);
	if ((strchr(local_string, ':') == NULL) && (strchr(local_string, '.') == NULL) && (strchr(local_string, '$') == NULL))
		return 0;
	return 1;
}

int which_opcode_is_it(char * line)            //return the opcode of a line, return 22 if its the .word command, return -1 if its a Label and -2 if there is no appropriate opcode
{

	char * is_label = strchr(line, ':');
	char * which_operation;
	if (is_label != NULL)
		return -1;
	else
	{
		if (which_operation = strstr(line, "add") != NULL)
			return 0;
		if (which_operation = strstr(line, "sub") != NULL)
			return 1;
		if (which_operation = strstr(line, "mul") != NULL)
			return 2;
		if (which_operation = strstr(line, "and") != NULL)
			return 3;
		if (which_operation = strstr(line, "or") != NULL)
			return 4;
		if (which_operation = strstr(line, "xor") != NULL)
			return 5;
		if (which_operation = strstr(line, "sll") != NULL)
			return 6;
		if (which_operation = strstr(line, "sra") != NULL)
			return 7;
		if (which_operation = strstr(line, "srl") != NULL)
			return 8;
		if (which_operation = strstr(line, "beq") != NULL)
			return 9;
		if (which_operation = strstr(line, "bne") != NULL)
			return 10;
		if (which_operation = strstr(line, "blt") != NULL)
			return 11;
		if (which_operation = strstr(line, "bgt") != NULL)
			return 12;
		if (which_operation = strstr(line, "ble") != NULL)
			return 13;
		if (which_operation = strstr(line, "bge") != NULL)
			return 14;
		if (which_operation = strstr(line, "jal") != NULL)
			return 15;
		if (which_operation = strstr(line, "lw") != NULL)
			return 16;
		if (which_operation = strstr(line, "sw") != NULL)
			return 17;
		if (which_operation = strstr(line, "reti") != NULL)
			return 18;
		if (which_operation = strstr(line, "in") != NULL)
			return 19;
		if (which_operation = strstr(line, "out") != NULL)
			return 20;
		if (which_operation = strstr(line, "halt") != NULL)
			return 21;
		if (which_operation = strstr(line, ".word") != NULL)
			return 22;
	}
	return -2;
}

int which_register_is_it(char * reg)            //return the opcode of a line, return 22 if its the .word command, return -1 if its a Label and -2 if there is no appropriate opcode
{
	char * which_register;
	if (which_register = strstr(reg, "$zero") != NULL)
		return 0;
	if (which_register = strstr(reg, "$imm") != NULL)
		return 1;
	if (which_register = strstr(reg, "$v0") != NULL)
		return 2;
	if (which_register = strstr(reg, "$a0") != NULL)
		return 3;
	if (which_register = strstr(reg, "$a1") != NULL)
		return 4;
	if (which_register = strstr(reg, "$a2") != NULL)
		return 5;
	if (which_register = strstr(reg, "$a3") != NULL)
		return 6;
	if (which_register = strstr(reg, "$t0") != NULL)
		return 7;
	if (which_register = strstr(reg, "$t1") != NULL)
		return 8;
	if (which_register = strstr(reg, "$t2") != NULL)
		return 9;
	if (which_register = strstr(reg, "$s0") != NULL)
		return 10;
	if (which_register = strstr(reg, "$s1") != NULL)
		return 11;
	if (which_register = strstr(reg, "$s2") != NULL)
		return 12;
	if (which_register = strstr(reg, "$gp") != NULL)
		return 13;
	if (which_register = strstr(reg, "$sp") != NULL)
		return 14;
	if (which_register = strstr(reg, "$ra") != NULL)
		return 15;
	return -1;

}

void remove_spaces(char * line)
{
	int count = 0;
	for (int i = 0; line[i]; i++)
		if (line[i] != ' ' && line[i]!='	' )
			line[count++] = line[i]; // here count is
								   // incremented
	line[count] = '\0';
}

void remove_comments(char * line)
{
	char *ptr;
	ptr = strchr(line, '#');
	if (ptr != NULL) {
		*ptr = '\0';
	}
}

int isDigit(char * word)													  //takes word and check if the first letter is a number or not = if its a decimal number return 1, 0 if its a label and -1 if its hexa
{
	if (strstr(word, "0x") == NULL)
	{
		if ((word[0] > 57 || word[0] < 48) && word[0] != '-')				//its a label
			return 0;
		return 1;											
	}
	return -1;
}

int hexa_string_to_decimal(char * hexa_string)										//the hexa_string includes 0x
{
	int i = 0, value, length;
	char * token;
	token = &hexa_string[2];
	long long decimal = 0, base = 1;
	length = strlen(token);
	for (i = length--; i >= 0; i--)
	{
		if (token[i] >= '0' && token[i] <= '9')
		{
			decimal += (token[i] - 48) * base;
			base *= 16;
		}
		else if (token[i] >= 'A' && token[i] <= 'F')
		{
			decimal += (token[i] - 55) * base;
			base *= 16;
		}
		else if (token[i] >= 'a' && token[i] <= 'f')
		{
			decimal += (token[i] - 87) * base;
			base *= 16;
		}
	}
	return decimal;
}

int which_immediate_is_it(char * imm_string, Label * label_list)
{
	int i;
	int is_digit = isDigit(imm_string);
	if (is_digit == 1) {
		return atoi(imm_string);
	}
	if (strstr(imm_string, "0x") == NULL)												//handling with Label
	{
		for (i = 0; i < MAX_ROW_LENGTH; i++)
			if (label_list[i].address != -1)
				if (strstr(imm_string, label_list[i].name) != NULL)
					return label_list[i].address;
	}
	else                                                                           //handling with hexa number
		return hexa_string_to_decimal(imm_string);
	return 0;
}

void print_lines_to_file(FILE *program, FILE *memin, Label * lables_list)
{
	int lines[ROWS_DEPTH + 1];
	int p;
	for (int p = 0; p < ROWS_DEPTH + 1; p++)
		lines[p] = 0;
	int j, memory_line = 0, data = 0, i = 0, lines_to_print = 0;
	char temp_line[MAX_ROW_LENGTH + 1];
	int current_location_at_file = 0;
	int two_word_arguments[2];
	char * token;
	char * temp_pointer;
	int which_immediate;
	char line[MAX_ROW_LENGTH + 1];
	while (fgets(line, sizeof(line), program))
	{
		p = is_it_line_with_things(line);
		if (p == 0)
			continue;
		if (strchr(line, '#') != NULL)
			remove_comments(line);
		if (strstr(line, "$imm") != NULL)									   //imm function that will be in 2 rows, line doesn't change at this point
		{
			token = strtok(line, " ");									      //first the token will hold only the opcode
			lines[current_location_at_file] = which_opcode_is_it(token);
			j = 1;
			while (token != NULL)
			{
				token = strtok(NULL, ",");
				if (token != NULL)											  //goes over all the line until it ends
				{
					if (j < 4)												//all the registers before the immediate 
					{
							lines[current_location_at_file] = lines[current_location_at_file] * 16 + which_register_is_it(token);
							j++;
					}
					else                                                   //handling the immediate
					{
						lines[current_location_at_file] = lines[current_location_at_file] & 0xfffff;
						temp_pointer = token;
						remove_spaces(temp_pointer);
						current_location_at_file++;
						lines[current_location_at_file] = which_immediate_is_it(temp_pointer, lables_list) & 0xfffff;
					}
				}
			}
			current_location_at_file++;
		}
		else                                                                  //we are handling Rtype or . commands
		{
			if ((strstr(line, ".word") == NULL && (strstr(line, ":") == NULL)))	   	// we are handling Rtype command
			{
				token = strtok(line, " ");
				lines[current_location_at_file] = which_opcode_is_it(token);
				j = 1;
				while (token != NULL)
				{
					token = strtok(NULL, ",");
					if (token != NULL)											  //goes over all the line until it ends
					{
						if (j < 4)
						{
							lines[current_location_at_file] = lines[current_location_at_file] * 16 + which_register_is_it(token);
							j++;
						}
					}
				}
				lines[current_location_at_file] = lines[current_location_at_file] & 0xfffff;
				current_location_at_file++;
			}
			if (strstr(line, ".word") != NULL)									//handling .word command
			{
				j = 0;
				token = strtok(line, "d");
				token = strtok(NULL, " ");
				while (token != NULL)
				{
					j++;
					if (token != NULL)
					{
						remove_spaces(token);
						if (j == 1) {												//token this is the adress in the memory
							if (strstr(token, "0x") == NULL)
								memory_line = atoi(token);							//decimal nubmer and not hexa
							else
								memory_line = hexa_string_to_decimal(token);
						}
						if (j == 2)
						{
							if (strstr(token, "0x") == NULL)
								data = atoi(token);
							else
								data = hexa_string_to_decimal(token);
						}
					}
					token = strtok(NULL, " ");
				}
				if (memory_line > current_location_at_file)						//the memory needed to be written is at a location of memin that is still empty
				{
					i = current_location_at_file;
					while (i <= memory_line)
					{
						if (i == memory_line)
							lines[i] = data;
						i++;
					}
					if (lines_to_print < i)
						lines_to_print = i;
				}
				else                                                        //the memory needed to be written is at a location that already got something at
					lines[memory_line-1] = data;
			}
		}
	}
	if (current_location_at_file > lines_to_print)
		lines_to_print = current_location_at_file;
for(j=0;j<lines_to_print;j++)
	fprintf(memin,"%05X\n", lines[j] & 0xfffff);
}

//=====================================================
//                  Main Assembler Program
//=====================================================
/*
	argv[0] will be the assembler - asm.exe
	argv[1] = program.asm - the program in Assembly we want to run
	argv[2] = memim.txt - the memory picture that will be the output file for the system

*/
int main(int argc, char *argv[])
{
	if (argc != 3)
	{
		printf("The files needed in order to run the Assmbler are not in the desired location");
		return 1;
	}
	FILE *program = fopen(argv[1], "r");
	if (!program) {
		printf("The file in argv[1] is not the assembly program needed");
		return 1;
	}
	FILE *memin = fopen(argv[2], "w+");
	if (!memin) {
		printf("The file in argv[2] is not the right main memory picture file");
		return 1;
	}

	Label * lables_list = create_labels_array(program);    //array that will contain all the Lables in the correct adress and the Label name
	print_lines_to_file(program, memin, lables_list);
	
	fclose(program);
	fclose(memin);


	return 0;
}