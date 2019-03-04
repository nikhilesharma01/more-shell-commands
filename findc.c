/* 
 * C program to implement the 
 * UNIX "find" function.
 *
 */

#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

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

/* Compare for exact match. */
int name_match(const char *path, const char *pattern){
	return strcmp(strrchr(path, '/')+1, pattern) == 0;
}


/* Check if the current file is the required file. */
void check_file(const char *path, const expression *exp, int follow_link){
	struct stat file;
	follow_link ? stat(path, &file) : lstat(path, &file);

	if(name_match(path, exp->operand.pattern))
		printf("%s\n", path);
}

/* Browse a directory to check if the file exists there. 
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


int main(int argc, char **argv){
	/* Follow any symbolic links in subdirectories.*/
	int follow_link = 1;
	
	if(argc < 3)
		error();
	
	//printf("%d\n", argc);

	char *path;
	expression exp;

	int i = 2;

	/* Get the file path. */
	path = argv[i];
	//printf("%s\n", path);
	i--;
	
	while(i < argc-1){
		/* Search type = name, because we 
			want an exact filename match. */
		exp.type = NAME;

		/* Get filename. */
		exp.operand.pattern = argv[i];
		printf("%s\n", exp.operand.pattern);
		i++;
	}

	/* Browse directory for name match. */
	browse_directory(path, &exp, follow_link);

	return 0;
}
