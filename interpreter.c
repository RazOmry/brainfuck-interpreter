/**
* @file interpreter.c
* @brief this is an interpreter for BrainFuck language written in C.
*
* created by Raz Omry, 18.04.2018
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MEMORY_SIZE 256

#define INVALID_BLOCK -1
#define CLOSED_BLOCK 0
#define OPENED_BLOCK 1

#define TRUE 1

char memory[MEMORY_SIZE] = { 0 }; // memory for program
char *current_cell = memory;	// pointer for current cell in memory

// functions declaration
char* get_user_input();
char* read_block(char* command);
void parse_command(char* command);
void parse_block(char* command);
int find_closing(char* command);
int is_open_block(char* command);

int main(void)
{
	char* command = NULL; // user's command string

	printf("BrainF**k 1.2, by Raz Omry, 2018\n");

	// main loop
	while (TRUE)
	{
		printf(">>> ");

		do
			command = get_user_input(command);
		while (command[0] == '\0');

		if (strchr(command, '['))
		{
			int state = is_open_block(command);
			if (state == OPENED_BLOCK)
				command = read_block(command); // waiting for closing block
			else if (state == INVALID_BLOCK)
			{
				printf("Error! unbalanced blocks\n");
				continue;
			}

			parse_block(command); // parsing the block
		}
		else
			parse_command(command); // parsing basic command

		free(command);
		command = NULL;
	}

	return 0;
}

/**
* gets input from user and returns a dynamically allocated string
*
* @return string - the input of the user
*/
char* get_user_input()
{
	char* string = malloc(sizeof(char));
	char ch = 0;
	const int MIN_STR_LEN = 2;

	string[0] = 0;

	// reading untill end of input
	for (int i = 0; (ch = getchar()) != '\n' && ch != EOF; i++)
	{
		string = (char *)realloc(string, (i + MIN_STR_LEN) * sizeof(char));
		string[i] = ch;
		string[i + 1] = 0;
	}

	return string;
}

/**
* parses basic command (without blocks)
*
* @param command - the command to parse and execute
*
* @return None (void)
*/
void parse_command(char* command)
{
	size_t command_len = strlen(command);

	for (size_t i = 0; i < command_len; i++)
	{
		switch (command[i])
		{
		case '+':
			(*current_cell)++;
			break;
		case '-':
			(*current_cell)--;
			break;
		case '>':
			if ((memory + MEMORY_SIZE) - current_cell > 0)
				current_cell++;
			else
			{
				printf("Error! current cell is at the end of memory\n");
				return;
			}
			break;
		case '<':
			if (current_cell - memory > 0)
				current_cell--;
			else
			{
				printf("Error! current cell is in the begining of memory\n");
				return;
			}
			break;
		case '.':
			printf("%c", *current_cell); // output
			break;
		case ',':
			*current_cell = getchar();
			if (*current_cell == '\n')
				*current_cell = 0;
			break;
		default:
			break;
		}
	}
}

/**
* parses block command (complex command)
*
* @param command - the command to parse and execute
*
* @return None (void)
*/
void parse_block(char *command)
{
	size_t start_block_index = INVALID_BLOCK, end_block_index = 0;
	char* block_string = NULL;

	if (command[0] == 0)
		return;

	// getting indexes of brackets
	block_string = strchr(command, '[');
	start_block_index = block_string - command;
	end_block_index = find_closing(command);

	// getting command before block
	block_string = calloc(start_block_index + 1, sizeof(char));
	strncpy(block_string, command, start_block_index);

	if (start_block_index != 0)
		parse_command(block_string);
	free(block_string);

	// getting command inside the block
	block_string = calloc(end_block_index - start_block_index, sizeof(char));
	strncpy(block_string, command + start_block_index + 1, end_block_index - start_block_index - 1);

	// executing the block command
	while (*current_cell != 0)
	{
		if (strchr(block_string, '['))
			parse_block(block_string);
		else
			parse_command(block_string);
	}

	free(block_string);

	// making the command after the block
	size_t len = strlen(command) - end_block_index;
	if (len > 1)
	{
		block_string = calloc(len, sizeof(char));
		strncpy(block_string, command + end_block_index + 1, len - 1);

		if (strchr(block_string, '['))
			parse_block(block_string);
		else
			parse_command(block_string);
		free(block_string);
	}
}

/**
* waiting for block input and closing bracket (and after this parse the command)
*
* @param command - the command add into the user's input and parse it
*
* @return string - the new (full) command from user
*/
char* read_block(char* command)
{
	char* new_command = NULL; // the command from user
	int block_state = INVALID_BLOCK;

	while ((block_state = is_open_block(command)) == OPENED_BLOCK)
	{
		printf("... ");
		new_command = get_user_input(new_command);
		command = realloc(command, strlen(command) + strlen(new_command) + 1);
		strncat(command, new_command, strlen(new_command)); // adding to last command
		free(new_command);
	}

	if (block_state == INVALID_BLOCK)
	{
		printf("Error! unbalanced brackets\n");
		return NULL;
	}
	return command;
}

/**
* finding the closing bracket to some block (we know that it is block command)
*
* @param command - the command with block to find its closing bracket
*
* @return int - the index of the closing bracket in the block
*/
int find_closing(char* command)
{
	int balance = 0;
	size_t command_len = strlen(command);

	for (size_t i = 0; i < command_len; i++)
	{
		if (command[i] == '[')
			balance++;
		else if (command[i] == ']')
		{
			balance--;
			if (balance == 0)
				return i; // found the closing bracket!
		}
	}
	return INVALID_BLOCK;
}

/**
* check if the block is open/closed and valid
*
* @param command - to check if the block is valid & balanced
*
* @return int - the state of the blocks in the command (OPENED_BLOCK / CLOSED_BLOCK / INVALID_BLOCK)
*/
int is_open_block(char* command)
{
	int balance = 0;
	size_t command_len = strlen(command);

	for (size_t i = 0; i < command_len; i++)
	{
		if (command[i] == '[')
			balance++;
		else if (command[i] == ']')
		{
			balance--;
			if (balance < 0)
				return INVALID_BLOCK; // error: there is closing bracket before opening bracket
		}
	}

	return balance > 0 ? OPENED_BLOCK : CLOSED_BLOCK;
}
