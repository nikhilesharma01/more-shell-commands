/* 
 * Code to implement the "ls -l"
 * UNIX command in C.
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

int main(int argc, char* argv[]){
	DIR *current_dir; 				/* The directory we're browsing, identified by argv. */
	struct dirent *current_file;	/* A file in the directory. */
	struct stat file_stat; 			/* Struct to retrieve the file stats. */
	struct passwd *tf; 				/* To determine file owner. */
	struct group *gf;				/* To determine file group. */
	char buf[512];

	/* Assign directory from the console to this argument. */
	current_dir = opendir(argv[1]);

	/* Keep going until we've listed all files. */
	while((current_file = readdir(current_dir)) != NULL){
		
		/* print "current_directory/current_file". */
		sprintf(buf, "%s/%s", argv[1], current_file->d_name);
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

			/* Owner. */
			tf = getpwuid(file_stat.st_uid);
			printf("\t%s ", tf->pw_name);

			/* Group. */
			gf = getgrgid(file_stat.st_gid);
			printf("\t%s ", gf->gr_name);

			/* Print [size] [filename] [time of last modification] */
			printf("%zu", file_stat.st_size);
			printf(" %s", current_file->d_name);
			printf(" %s", ctime(&file_stat.st_mtime));

		}
	}
	closedir(current_dir);

	return 0;
}
