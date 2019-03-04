/*
 * C code to implement the UNIX "more" command.
 * The output of the command will be
 * piped out to the "more" command.
 *
 * Adapted to work with more than 1 pipes.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>


int main(int argc, char *argv[]){
	
	//static char *argv_args[] = { "ls", "-l", 0};

	if (strcmp(argv[1], "more") == 0){
		printf("Error: bad usage\n");
		exit(0);
	}

	/* Number of argments in the command. */
	int size = argc - 1;

	/* Array to store the arguments. */
	char *data[size+1];
	
	for(int i = 0; i < size; i++){
		data[i] = argv[i+1];
	}

	data[size] = 0;


	static char *argv_more[] = { "more", 0};
	static char **cmds[2];

	//static char **cmds[] = { argv_args, argv_more };
	
	/* Assign appropriate arguments to cmds array. */
	cmds[0] = data;
	cmds[1] = argv_more;



	int n_commands = size;
	int pipefd[2];

	/* If pipe fails, show error. */
	if (pipe(&pipefd[0]) < 0)
		perror("Error: ");

	fflush(stdout);

	for (int i = 0; i < n_commands; i++){
		/* fork the current process. */
		int pid = fork();

		if (pid == 0){
			int command_no = i;
			int prev_pipe = ((command_no - 1) % 2) * 2;
			int current_pipe = (command_no % 2) * 2;
			
			/* If current command is the first command,
			 * close the read end, else read from the last pipe.
			 */

			if (command_no == 0)
				close(pipefd[0]);
			else{
				dup2(pipefd[prev_pipe], 0);
				close(pipefd[current_pipe]);
			}

			/* If current command is the last one, close 
			 * the write, else write to the pipe.
			 */

			if (command_no == n_commands - 1)
				close(pipefd[current_pipe + 1]);
			else
				dup2(pipefd[current_pipe + 1], 1);

			execvp(cmds[i][0], cmds[i]);
			fprintf(stderr, "Failed to exec: %s (%d: %s)\n", cmds[i][0], errno, strerror(errno));
			_exit(1);
		}
	}
	return 0;
}	

