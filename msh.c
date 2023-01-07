// The MIT License (MIT)
// 
// Copyright (c) 2016 Trevor Bakker 
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

//#define DEBUG

#define WHITESPACE " \t\n"      // We want to split our command line up into tokens
									// so we need to define what delimits our tokens.
									// In this case  white space
									// will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255    // The maximum command-line size

#define MAX_NUM_ARGUMENTS 11     // Mav shell only supports ten arguments


void exit_cleanup(char *history[], int num_hist)
{
	/// @brief Performs exit cleanup before the program terminates
	/// @param history the history array that needs to be freed
	/// @param num_hist the size of the history array
	for(int i = 0; i < num_hist; i++)
	{
		free(history[i]);
		history[i] = NULL;
	}
}

int main()
{

	pid_t pids[20]; // Stores the PIDs of last 20 processes made using fork()
	int num_pids = 0; // Number of PID entries to display for listpid - max 20
	char *history[15]; // Stores the last 15 commands entered by the user
	int num_hist = 0; // Number of entries to display for history - max 15

	char * command_string = malloc( MAX_COMMAND_SIZE );

	while( 1 )
	{
		// Print out the msh prompt
		printf ("msh> ");

		// Read the command from the commandline.  The
		// maximum command that will be read is MAX_COMMAND_SIZE
		// This while command will wait here until the user
		// inputs something since fgets returns NULL when there
		// is no input
		while( !fgets (command_string, MAX_COMMAND_SIZE, stdin) );

		// We check if a command of the form !n is invoked where n is an integer
		if(strlen(command_string) > 0 && command_string[0] == '!')
		{
			// Store the index of the command to execute from history
			int idx = atoi(command_string + 1);

			// If the index exists in history (upto 15)
			if(idx < num_hist)
			{
				// Set command_string to the appropriate command from history
				
				// Make room for the string
				command_string = realloc(command_string, sizeof(char) * strlen(history[idx]) + 1);
				// Copy the string
				strcpy(command_string, history[idx]);
			}
			else
			{
				printf("Command not in history.\n");
				continue;
			}
		}

		/* Parse input */
		char *token[MAX_NUM_ARGUMENTS];

		int   token_count = 0;                                 
																													 
		// Pointer to point to the token
		// parsed by strsep
		char *argument_ptr;                                         
																													 
		char *working_string  = strdup( command_string );                

		// we are going to move the working_string pointer so
		// keep track of its original value so we can deallocate
		// the correct amount at the end
		char *head_ptr = working_string;

		// Tokenize the input strings with whitespace used as the delimiter
		while ( ( (argument_ptr = strsep(&working_string, WHITESPACE ) ) != NULL) && 
							(token_count<MAX_NUM_ARGUMENTS))
		{
			token[token_count] = strndup( argument_ptr, MAX_COMMAND_SIZE );
			if( strlen( token[token_count] ) == 0 )
			{
				token[token_count] = NULL;
			}
				token_count++;
		}
		
		// num_hist keeps track of the number of commands stored in history and will take a maximum value of 15
		if (num_hist < 15)
		{
			// If we have not reached our limit of 15, we will simply append the next command
			// to our history array.
			// make room for the command in memory
			history[num_hist] = malloc(MAX_COMMAND_SIZE);
			// copy the command into the space in memory we allocated
			strncpy(history[num_hist], command_string, MAX_COMMAND_SIZE);
			// update our size of the history array - because we added a new element
			num_hist++;
		}
		else
		{
			// If we ran out of capacity, we will discard element 0
			// shift the array one place to the left and add the element at the end

			// free the string at element 0 to avoid memory leaks
			free(history[0]);
			// shift every element one place to the left
			for (int i = 0; i < 14; i++)
			{
				history[i] = history[i + 1];
			}
			// allocate memory to store the last command entered
			history[14] = malloc(sizeof(char) * strlen(command_string) + 1);
			// copy it in
			strcpy(history[14], command_string);
		}

		if(token[0] != NULL)
		{
			if (!strcmp(token[0], "exit") || !strcmp(token[0], "quit"))
			{
				// this is the case where the exit/quit command was called
				// cleanup after ourselves -- free all the memory malloc'd
				exit_cleanup(history, num_hist);
				// exit out of the program
				exit(0);
			}
			else if (!strcmp(token[0], "cd"))
			{
				// change directory -- chdir function
				chdir(token[1]);
			}
			else if (!strcmp(token[0], "listpids"))
			{
				// simply print out each of the pids
				for(int i = 0; i < num_pids; i++)
				{
					printf("%d\n", pids[i]);
				}
			}
			else if(!strcmp(token[0], "history"))
			{
				for(int i = 0; i < num_hist; i++)
				{
					printf("%d: %s", i, history[i]);
				}
			}
			// Code starts here
			else
			{
				// Use fork to create a copy of the process
				pid_t pid = fork();

				// Error
				if (pid == -1)
				{
					printf("Error forking a new process.\n");
				}
				// Child
				else if (pid == 0)
				{
					// Capture the return value of execvp to check if we had an error
					int ret = execvp(token[0], &token[0]);
					// if execvp failed, that probably means it was an invalid command
					if (ret == -1)
					{
						printf("%s: Command not found.\n", token[0]);
						return EXIT_FAILURE;
					}
				}
				// Parent
				else
				{
					int status;
					wait(&status); // Wait for the child to die
					// Same approach as history
					// num_pids maintains the size of the pids array
					if(num_pids < 20)
					{
						// if we have not reached our limit of 20
						// just insert it at the end
						pids[num_pids++] = pid;
					}
					else
					{
						// Otherwise, shift everything one place to the left
						for (int i = 0; i < 19; i++)
						{
							pids[i] = pids[i + 1]; // copy element left
						}
						pids[19] = pid; // insert at the end
					}
					
				}
			}
		}
		

		free( head_ptr );

	}
	return 0;
	// e2520ca2-76f3-90d6-0242ac120003
}
