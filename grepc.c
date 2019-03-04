/*
 * C code to implement "grep <string> <filename>"
 * command in C.
 *
 * Output: Line number and line containing <string>.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>

void implement_grep(char *argv[]){
	int file_descriptor, r, j = 0, count = 0;
	char temp, line[100];
	
	/* Check if the file opens correctly. */
	if ((file_descriptor = open(argv[2], O_RDONLY)) != -1){
		
		/* Read a character from the file. */
		while ((r = read(file_descriptor, &temp, sizeof(char))) != 0){

			/* Fill the line with 0 */
			if (temp != '\n'){
				line[j] = temp;
	//			memset(line, 0, sizeof(line));
				j++;
			}
			else {
				count++;
				
				/* If match found */
				if (strstr(line, argv[1]) != NULL){

					/* Null terminate and print line number and line. */
					line[j] = '\0';
					printf("%d: %s\n", count, line);
				}

				memset(line, 0, sizeof(line));
				j = 0;
			}
			
		}
	}
	else
		printf("Error opening file!\n");
}

int main(int argc, char *argv[]){
	struct stat file_stat;
	
	if (argc == 3){
		if(stat(argv[2], &file_stat) == 0)
			implement_grep(argv);
		else{
			perror("stat()");
			exit(-2);
		}
	}
	
	else if (argc < 3){
		printf("Error: Too few arguments! Exiting! \n");
	//	exit(1);
	}
	else if (argc > 3){
		printf("Error: Too many arguments! Exiting! \n");
	//	exit(1);
	}

	return 0;
}
