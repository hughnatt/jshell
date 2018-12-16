/**
 * JADE VANDAL - TANGUY SAUTON
 * TP SHELL - SE - INFO4
 * Polytech Grenoble 2018-2019
 **/
#define TRUE 1
#define FALSE 0

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "readcmd.h"

/**
 * Entry Point
 */
int main(int argc, char const *argv[])
{
	int status, dead, child;
	int input, output;
	pid_t pid;
	struct cmdline *l;
	int piper[2];
	int i;

	printf("shade> ");
	l = readcmd();

	while (TRUE)
	{

		/* If input stream closed, normal termination */
		if (!l)
		{
			printf("exit\n");
			exit(0);
		}

		if (l->err)
		{
			/* Syntax error, read another command */
			printf("error: %s\n", l->err);
			continue;
		}

		input = -1;
		if (l->in)
		{
			if ((input = open(l->in, O_RDONLY)) == -1)
			{
				perror("Input file doesn't exist");
				printf("Standard input selected\n");
			}
		}

		output = -1;
		if (l->out)
		{
			if ((output = open(l->out, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR)) == -1)
			{
				perror("Can't create output file");
				printf("Standard ouput selected");
			}
		}

		int nbCmd = 0;
		for (i = 0; l->seq[i] != 0; i++)
		{
			nbCmd++;
		}

		for (i = 0; l->seq[i] != 0; i++)
		{

			char **cmd = l->seq[i];

			int lastCommand = (i == nbCmd - 1);

			//cmd is exit --> normal termination
			if (strcmp(cmd[0], "exit") == 0)
			{
				printf("exit\n");
				exit(0);
			}

			//cmd is cd, we change directory and get out of the for loop
			if (strcmp(cmd[0], "cd") == 0)
			{
				//No directory specified : do nothing
				if (!cmd[1])
				{
					if (chdir(getenv("HOME")) != 0)
					{
						perror("Can't change directory");
					}
					break;
				}
				else
				{
					//Try to change dir
					if (chdir(cmd[1]) != 0)
					{
						perror("Can't change directory");
					}
					break;
				}
			}

			pipe(piper);

			//cmd is not exit, we'll execute it in another process
			pid = fork();

			//
			if (pid < 0)
			{
				perror("Shell error : fork failed\n");
				break;
			}
			/* Son Code
			 *
             */
			else if (pid == 0)
			{
				//STDIN/STDOUT Redirection

				//If input is specified (meaning second command, or input file specified)
				if (input != -1)
				{
					dup2(input, STDIN_FILENO);
					close(input);
				}
				else
				{
					//input is not defined, meaning it's the first command an
					//no input file has been specified
					//We are already reading from STDIN
				}

				//"Middle" Command, output is piped
				if (!lastCommand)
				{
					dup2(piper[1], STDOUT_FILENO);
					close(piper[1]);
				//Last Command
				}
				else
				{
					//Output is defined, redirect STDOUT to output file
					if (output != -1)
					{
						dup2(output, STDOUT_FILENO);
						close(output);
					}
					else
					{
						//Output is undefined, and this is the last command,
						// we stay with STDOUT
					}
				}

				int error_value = execvp(cmd[0], cmd);

				if (error_value == -1)
				{
					//Error while executing the command
					perror("Shell error on execv ");
					exit(-1);
				}
				// Command has been executed succesfully
				printf("\n");
				exit(0);
			}
			else
			{
				/* Father Code
				 * We wait until the son dies
				 */
				child = TRUE;
				while (child)
				{
					dead = wait(&status);
					if (dead == pid)
					{
						child = FALSE;
					}
				}
				//Only useful in case of piped command

				//piper[1] refers to the write end of the pipe
				//We can close it now that the pipe is done
				close(piper[1]);
				//piper[0] refers to the read end of the pipe
				//It will be the next input
				input = piper[0];
			}
		}

		//Print a new line and read a command
		printf("shade> ");
		l = readcmd();
	}
	return 0;
}
