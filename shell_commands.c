/*
 * C program to implement 4 shell commands
 * in C. The different commands are:
 * 		1. ls -l
 *		2. find
 * 		3. grep
 * 		4. more
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>
#include <fcntl.h>
#include <errno.h>

/* Used in the "find" command. */
enum{
	NAME
};

typedef struct{
	int type;
	union{
		const char *pattern;
		const char *path;
	} operand;
} expression;

void implement_ls(char *commands[]);
void implement_find(int num_commands, char *commands[]);
void implement_grep(int num_commands, char *commands[]);
void implement_more(int num_commands, char *commands[]);

/******************** Implement "ls -l" command. *****************/
void implement_ls(char *commands[]){
	DIR *current_dir; 				/* The directory we're browsing, identified by argv. */
	struct dirent *current_file;	/* A file in the directory. */
	struct stat file_stat; 			/* Struct to retrieve the file stats. */
	struct passwd *tf; 				/* To determine file owner. */
	struct group *gf;				/* To determine file group. */
	char buf[512];

	/* Assign directory from the console to this argument. */
	current_dir = opendir(commands[2]);

	/* Keep going until we've listed all files. */
	while((current_file = readdir(current_dir)) != NULL){

		/* print "current_directory/current_file". */
		sprintf(buf, "%s/%s", commands[2], current_file->d_name);
		/* retrieve file info. */
		stat(buf, &file_stat);

		if (current_file->d_name[0] != '.'){
			/* Check file type. */
			switch (file_stat.st_mode & S_IFMT){
				case S_IFBLK: printf("b "); break;
				case S_IFCHR: printf("c "); break;
				case S_IFDIR: printf("d "); break;
				case S_IFIFO: printf("p "); break;
				case S_IFLNK: printf("l "); break;
				case S_IFSOCK: printf("s "); break;

				default: 	printf("- "); break;
			}


			/* Update file permissions. */
			printf( (file_stat.st_mode & S_IRUSR) ? "r" : "-");
			printf( (file_stat.st_mode & S_IWUSR) ? "w" : "-");
			printf( (file_stat.st_mode & S_IXUSR) ? "x" : "-");
			printf( (file_stat.st_mode & S_IRGRP) ? "r" : "-");
			printf( (file_stat.st_mode & S_IWGRP) ? "w" : "-");
			printf( (file_stat.st_mode & S_IXGRP) ? "x" : "-");
			printf( (file_stat.st_mode & S_IROTH) ? "r" : "-");
			printf( (file_stat.st_mode & S_IWOTH) ? "w" : "-");
			printf( (file_stat.st_mode & S_IXOTH) ? "x" : "-");


			printf("\t%ld ", file_stat.st_nlink);

			/* Get owner id and name. */
			tf = getpwuid(file_stat.st_uid);
			printf("\t%s ", tf->pw_name);

			/* Get group id and name. */
			gf = getgrgid(file_stat.st_gid);
			printf("\t%s ", gf->gr_name);

			/* Print [size] [filename] [time of last modification] */
			printf("%zu", file_stat.st_size);
			printf(" %s", current_file->d_name);
			printf(" %s", ctime(&file_stat.st_mtime));

		}
	}
	closedir(current_dir);
}

/***********************************************************************/

/********************** Implement "find" command. *********************/
/* Uses helper functions:
 *		1. browse_directory: browse the current directory for files.
 *		2. check_file: check the current file status.
 * 		3. name_match: checks for the exact filename match.
 */


/*************** Helper functions for "find" *****************/

/* Compares for exact filename match. */
int name_match(const char *path, const char *pattern){
	return strcmp(strrchr(path, '/')+1, pattern) == 0;
}


/* Checks if the current file is the required file. */
void check_file(const char *path, const expression *exp, int follow_link){
	struct stat file;
	follow_link ? stat(path, &file) : lstat(path, &file);

	if(name_match(path, exp->operand.pattern))
		printf("%s\n", path);
}

/* Browses a directory to check if the file exists there.
 *
 * const char *path: Directory path
 * const expression *exp: Search criteria
 * int follow_link: Whether to follow a symbolic link to a subdirectory.
 *
 */
void browse_directory(const char *path, const expression *exp, int follow_link){
	DIR *dir = opendir(path);
	struct dirent *entry;
	struct stat dir_stat;

	if(!dir){
		perror(path);
		return;
	}
	//printf("Dir open\n");
	while((entry = readdir(dir)) != NULL){
		char buff[strlen(path) + strlen(entry->d_name) + 2];
	//	printf("%s\n", path);
		sprintf(buff, "%s/%s", path, entry->d_name);

		follow_link ? stat(buff, &dir_stat) : lstat(buff, &dir_stat);

		/* If not the parent. */
		if(strcmp(entry->d_name, "..") != 0){

			/* If the present directory has a sub-directory which is not root
			 * browse the subdirectoy.
			 */
			if(((dir_stat.st_mode & S_IFMT) == S_IFDIR) && strcmp(entry->d_name, ".") != 0)
				browse_directory(buff, exp, follow_link);
			else
				/* Else check if the file matches filename. */
				check_file(buff, exp, follow_link);
		}
	}
	closedir(dir);
}

void error(){
	printf("Usage error!\n");
	exit(-1);
}

/******************* End of helper functions ************************/

/**************************** "Find" *********************************/
void implement_find(int num_commands, char *commands[]){
	int follow_link = 1;

	if (num_commands != 4)
		error();

	char *path;
	expression exp;

	int i = num_commands - 1;
	path = commands[i]; 		/* Get file path.*/

	i--;

	while(i < num_commands - 1){
		/* Search type = by NAME.*/
		exp.type = NAME;

		/* Get filename. */
		exp.operand.pattern = commands[i];
		printf("%s\n", exp.operand.pattern);
		i++;
	}

	/* Browse directory for name match. */
	browse_directory(path, &exp, follow_link);
}

/*********************** End of "Find" *************************/

/************************* "Grep" *****************************/
/* Uses helper function:
 *		1. grepc: searches for <string> in <filename>.
 */

/* Search for <string> in <filename>. */
void grepc(char *commands[]){
	int file_descriptor, r, j = 0, count = 0;
	char temp, line[100];

	/* Check if the file opens correctly. */
	if ((file_descriptor = open(commands[3], O_RDONLY)) != -1){

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
				if (strstr(line, commands[2]) != NULL){

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


void implement_grep(int num_commands, char *commands[]){
	struct stat file_stat;

	if (num_commands == 4){
		if(stat(commands[3], &file_stat) == 0)
			grepc(commands);
		else{
			perror("stat()");
			exit(-2);
		}
	}

	else if (num_commands < 4){
		printf("Error: Too few arguments! Exiting! \n");
	}
	else if (num_commands > 4){
		printf("Error: Too many arguments! Exiting! \n");
	}

}

/******************* End of "Grep" ***********************/



/*********************** "More" *************************/
void implement_more(int num_commands, char *commands[]){
	/* Number of argments in the command. */
	int size = num_commands - 1;

	/* Array to store the arguments. */
	char *data[size+1];

	for(int i = 1; i < size; i++){
		data[i-1] = commands[i+1];
	}

	data[size] = 0;


	static char *argv_more[] = { "more", 0};
	static char **cmds[2];

	//static char *argv_args[] = { "ls", "-l", 0};
	//static char **cmds[] = { argv_args, argv_more };

	/* Assign appropriate arguments to cmds array. */
	cmds[0] = data;
	cmds[1] = argv_more;



	int n_commands = size;
	int pipefd[2];

	printf("%d\n", n_commands);
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
}

/********************* End of "more" ***********************/


/************************ Main ************************/
int main(int argc, char *argv[]){
	//printf("%s\n", argv[1]);

	if (strcmp(argv[1], "ls") == 0)
		implement_ls(argv);
	else if (strcmp(argv[1], "find") == 0)
		implement_find(argc, argv);
	else if (strcmp(argv[1], "grep") == 0)
		implement_grep(argc, argv);
	else if (strcmp(argv[1], "more") == 0)
		implement_more(argc, argv);
	else{
		perror("Invalid command. Exiting!\n");
		return -2;
	}

	return 0;
}
