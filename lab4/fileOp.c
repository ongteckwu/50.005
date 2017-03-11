#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>

#define MAX_INPUT 256
#define SIZE 1024
#define STAT_BUFFER_SIZE 100
#define NAME_SIZE 100
#define MAX_FILE_COUNT 100000

#define LIST_NORMAL 0
#define LIST_PROPERTY 1
#define SORTED_LIST_NORMAL 2
#define SORTED_LIST_PROPERTY 3

#define SORT_BY_TIME 1
#define SORT_BY_SIZE 2
#define SORT_BY_NAME 3

#define MAX_TREE_DEPTH 10000

#define TREE_NORMAL 0
#define TREE_DEPTH 1
#define TREE_SORT 2


struct statwname {
	struct stat st;
	char name[NAME_SIZE];
	char relname[NAME_SIZE];
};

int timecmpfunc(const void * a, const void * b) {
	return (int) difftime(((struct statwname *) a)->st.st_mtime, ((struct statwname *) b)->st.st_mtime);
}

int sizecmpfunc(const void * a, const void * b) {
	return (int) ((struct statwname *) a)->st.st_size - ((struct statwname *) b)->st.st_size;
}

int namecmpfunc(const void * a, const void * b) {
	return strcmp(((struct statwname *) a)->name, ((struct statwname *) b)->name);
}

// from the linux kernel
// strips both sides of a string off whitespaces
char *strstrip(char *s) {
	size_t size;
	char *end;

	size = strlen(s);

	if (!size)
		return s;

	end = s + size - 1;
	// back strip
	while (end >= s && isspace(*end))
		end--;
	*(end + 1) = '\0';

	// front strip
	while (*s && isspace(*s))
		s++;

	return s;
}

// Whether a string is all digits
int isDigitString(char * str) {
	int i;
		for (i = 0; i < strlen(str); i++) {
		if (!isdigit(str[i])) {
			return 0;
		}
	}
	return 1;
}

// recursive find
int findRecurseDirPrint(const char * dir, const char * regex) {
	DIR * directory;
	struct stat buf;

	// checks if directory exists
	if ((directory = opendir(dir)) == NULL) {
		fprintf(stderr, "Directory %s does not exist\n", dir);
		return -1;
	}

	struct dirent * pent;
	// reads every file in the directory
	while (pent = readdir(directory)) {
		// ignore . and ..
		if (strncmp("..", pent->d_name, 2) == 0 || strncmp(".", pent->d_name, 1) == 0) {
			continue;
		}

		// append directory name at the back to create relative path
		char * name;
		name = malloc(strlen(pent->d_name) + strlen(dir) + 20);
		if (strncmp(".", dir, 1) != 0) {
			strcpy(name, dir);
			strcat(name, "/");
			strcat(name, pent->d_name);
		}	
		else 
			strcpy(name, pent->d_name);

		if (stat(name, &buf) < 0) {
			fprintf(stderr, "Something wrong with reading file %s %s\n", dir, name);
			return -1;
		}

		// check if file conforms to find expression
		if (strstr(pent->d_name, regex) != NULL) {
			// if directory
			if (S_ISDIR(buf.st_mode)) {
				// print directory
				printf("%s/\n", name);
				// print everything within directory
				recurseDirPrint(name);
			} else {
				// if file
				// print file
				printf("%s\n", name);
			}
		}
		// else if it's a directory
		else if (S_ISDIR(buf.st_mode)) {
			findRecurseDirPrint(name, regex);
		}

		free(name);
	}

	closedir(directory);
	return 0;
}

// prints a directory recursively
int recurseDirPrint(const char * dir) {
	DIR * directory;
	struct stat buf;

	// checks if directory exists
	if ((directory = opendir(dir)) == NULL) {
		fprintf(stderr, "Directory %s does not exist\n", dir);
		return -1;
	}
	struct dirent * pent;
	// reads every file in directory
	while (pent = readdir(directory)) {
		// ignore . and ..
		if (strncmp("..", pent->d_name, 2) == 0 || strncmp(".", pent->d_name, 1) == 0) {
			continue;
		}

		// append directory name at the back to create relative path
		char * name;
		name = malloc(strlen(pent->d_name) + strlen(dir) + 20);
		if (strncmp(".", dir, 1) != 0) {
			strcpy(name, dir);
			strcat(name, "/");
			strcat(name, pent->d_name);
		}	
		else 
			strcpy(name, pent->d_name);

		if (stat(name, &buf) < 0) {
			fprintf(stderr, "Something wrong with reading file %s %s\n", dir, name);
			return -1;
		}
			
		// if directory
		if (S_ISDIR(buf.st_mode)) {
			// print directory
			printf("%s/\n", name);
			// print everything within directory
			recurseDirPrint(name);
		} else {
			// if file
			// print file
			printf("%s\n", name);
		}

		free(name);
	}


	closedir(directory);
	return 0;
}

// tree print with no sort
int treePrint(const char * dir, int pad, int maxlevel) {
	DIR * directory;
	struct stat buf;

	// checks if directory exists
	if ((directory = opendir(dir)) == NULL) {
		fprintf(stderr, "Directory %s does not exist\n", dir);
		return -1;
	}

	// introduces padding
	char * padding;
	if (pad > 0) {
		padding = malloc(pad + 3);
		memset(padding, ' ', pad);
		padding[pad] = 0;
		strcat(padding, "|-");
	}

	struct dirent * pent;

	while (pent = readdir(directory)) {
		// ignore . and ..
		if (strncmp("..", pent->d_name, 2) == 0 || strncmp(".", pent->d_name, 1) == 0) {
			continue;
		}

		// append directory name at the back to create relative path
		char * name;
		name = malloc(strlen(pent->d_name) + strlen(dir) + 20);
		if (strncmp(".", dir, 1) != 0) {
			strcpy(name, dir);
			strcat(name, "/");
			strcat(name, pent->d_name);
		}	
		else 
			strcpy(name, pent->d_name);

		if (stat(name, &buf) < 0) {
			fprintf(stderr, "Something wrong with reading file %s %s\n", dir, name);
			return -1;
		}
		
		// if requires padding, print with padding
		if (pad > 0) {
			printf("%s%s\n", padding, pent->d_name);
		} 
		else { 
			printf("%s\n", pent->d_name);
		}

		// if directory and max depth has not been reached
		if (S_ISDIR(buf.st_mode) && pad < maxlevel) {
			// print directory
			treePrint(name, pad+1, maxlevel);
		}

		free(name);
	}

	if (pad > 0) {
		free(padding);
	}

	closedir(directory);
	return 0;
}


// tree print with sort
int treePrintSort(const char * dir, int pad, int maxlevel, int sortedState) {
	DIR * directory;
	struct stat buf;

	// checks if directory exists
	if ((directory = opendir(dir)) == NULL) {
		fprintf(stderr, "Directory %s does not exist\n", dir);
		return -1;
	}

	// introduces padding
	char * padding;
	if (pad > 0) {
		padding = malloc(pad + 3);
		memset(padding, ' ', pad);
		padding[pad] = 0;
		strcat(padding, "|-");
	}

	struct dirent * pent;

	// create statwname buffers
	struct statwname * buffers;
	buffers = malloc(sizeof(struct statwname) * STAT_BUFFER_SIZE);
	int current_buffer_size = STAT_BUFFER_SIZE;
	struct statwname * current_buffer = buffers;
	int count = 0;

	// fill up buffers
	while (pent = readdir(directory)) {
		// ignore . and ..
		if (strncmp("..", pent->d_name, 2) == 0 || strncmp(".", pent->d_name, 1) == 0) {
			continue;
		}

		if (count >= STAT_BUFFER_SIZE) {
			// resize buffer by 100 whenever buffer is full
			buffers = realloc(buffers, sizeof(buffers) + sizeof(struct statwname) * STAT_BUFFER_SIZE);
		}

		// append directory name at the back to create relative path
		char * name;
		name = malloc(strlen(pent->d_name) + strlen(dir) + 20);
		if (strncmp(".", dir, 1) != 0) {
			strcpy(name, dir);
			strcat(name, "/");
			strcat(name, pent->d_name);
		}	
		else 
			strcpy(name, pent->d_name);

		// get property
		if (stat(name, &current_buffer->st) < 0) {
			fprintf(stderr, "Something wrong with reading file %s\n", pent->d_name);
			return EXIT_FAILURE;
		}

		// copy relative name so that if it is a directory, it can be recursed into
		strcpy(current_buffer->relname, name);
		free(name);
		// copy absolute name so that name can be printed later
		strcpy(current_buffer->name, pent->d_name);
		//
		count++;
		current_buffer++;
	}

	// sort by either time, name, or size
	switch (sortedState) {
		case SORT_BY_NAME: {
			// name
			qsort(buffers, count, sizeof(struct statwname), namecmpfunc);
			break;
		}
		case SORT_BY_TIME: {
			// time
			qsort(buffers, count, sizeof(struct statwname), timecmpfunc);
			break;
		}
		case SORT_BY_SIZE: {
			qsort(buffers, count, sizeof(struct statwname), sizecmpfunc);
			break;
		}
	}

	// print the files
	int i;
	for (i = 0; i < count; i++) {
		// if requires padding, print with padding
		if (pad > 0) {
			printf("%s%s\n", padding, buffers[i].name);
		} 
		else { 
			printf("%s\n", buffers[i].name);
		}

		// if directory and max depth has not been reached
		if (S_ISDIR(buffers[i].st.st_mode) && pad < maxlevel) {
			// print directory
			treePrintSort(buffers[i].relname, pad+1, maxlevel, sortedState);
		}

	}

	if (pad > 0) {
		free(padding);
	}

	free(buffers);

	closedir(directory);
	return 0;
}


int main(){

//variables
char command[100];//to store users command

//for create option
char *create;
create="create";//be careful about \n

//for delete
char *delete;
delete="delete";//be careful about \n

//for display
char *display;
display="display";//be careful about \n

//for list
char *list;
list="list";//be careful about \n

char *property;
property="property";//be careful about \n

char *propertytime;
propertytime="property time";//be careful about \n

char *propertysize;
propertysize="property size";//be careful about \n

char *propertyname;
propertysize="property name";//be careful about \n

char *find;
find="find";//be careful about \n

char *tree;
tree="tree";//be careful about \n

char *treetime;
treetime=" time";//be careful about " " 

char *treesize;
treesize=" size";//be careful about " "

char *treename;
treename=" name";//be careful about " "

char *fileName;//to store fileName entered by user
int ret;//check return value


//pointers
char *ptr;
char *ptrTime;//sorting by time
char *ptrName;//sorting by name
char *ptrSize;//sorting by size

while(1){
	//Q1
	printf("csh>");
	fgets(command,MAX_INPUT,stdin);//take input from user and save it in command
	printf("command %s\n",command);

//Create, Case #1
	ptr=strstr(command,create);
	if(ptr!=NULL)
	{
		FILE * fd;
		char * filename = strstrip(ptr + strlen(create) + 1);
		if ((fd = fopen(filename, "w+")) != NULL) {
			printf("%s is created\n", filename);
			fclose(fd);
		}
		else {
			printf("File is not created. Error has occurred. Try again\n");
		}
	//Implement your code to handle create here
	}

//--------------------------------------------------------------------
//Delete, Case #2
 ptr=strstr(command,delete);
	if(ptr!=NULL)
	{
		char * filename = strstrip(ptr + strlen(delete) + 1);
		if ((remove(filename) == 0)) {
			printf("%s is successfully deleted\n", filename);
		}
		else {
			printf("File does not exist\n");
		}
	//Implement your code to handle detele here
	}

//--------------------------------------------------------------------
//Display, Case #3
 ptr=strstr(command,display);
	if(ptr!=NULL)
	{
	//Implement your code to handle display here
		const char buffer[SIZE];
		FILE * fd;
		char * filename = strstrip(ptr + strlen(display) + 1);
		if ((fd = fopen(filename, "r")) != NULL) {
			while(fread((void *) buffer, sizeof(char), SIZE, fd) != 0) {
				printf("%s", buffer);
			}
		}
		else {
			printf("File cannot be read. Error has occurred. Try again\n");
		}

		fclose(fd);
	}


//--------------------------------------------------------------------
//list, Case #4
 ptr=strstr(command,list);
	if(ptr!=NULL)
	{
	//Implement your code to handle list here

	/*There are many cases to handle in this part*/

		// strtok
		/* state machine: LIST_NORMAL - list normally
		 * 				  LIST_PROPERTY - list with property
		 * 				  SORTED_LIST_NORMAL - sorted name list without property
		 * 				  SORTED_LIST_PROPERTY - sorted list with property
		*/
		// if count 1: normal list
		// if count 2: either int or property
		// if count 3: if int, property

		/* sorted: SORT_BY_TIME - time
		           SORT_BY_SIZE - size
		           SORT_BY_NAME - name
	   */
		int maxFileCount = MAX_FILE_COUNT;
		int state = LIST_NORMAL;
		int sortedState = SORT_BY_TIME;
		int invalidCommand = 0;
		char * pch;
		pch = strtok (ptr + strlen(list) + 1," ");
		while (pch != NULL)
		{
			switch(state) {
				case LIST_NORMAL: {
					if (isDigitString(strstrip(pch))) {
						maxFileCount = atoi(pch);
					}
					else if (strncmp(pch, "property", 8) == 0) {
						state = LIST_PROPERTY;
					}
					else if (strncmp(pch, "name", 4) == 0) {
						state = SORTED_LIST_NORMAL;
					}
					else {
						printf("Invalid command: %s; pch %s\n", ptr, pch);
						invalidCommand = 1;
					}
					break;
				}
				case LIST_PROPERTY: {
					if (strncmp(pch, "time", 4) == 0) {
						state = SORTED_LIST_PROPERTY;
						sortedState = SORT_BY_TIME;
					}
					else if (strncmp(pch, "size", 4) == 0) {
						state = SORTED_LIST_PROPERTY;
						sortedState = SORT_BY_SIZE;
					}
					else if (strncmp(pch, "name", 4) == 0) {
						state = SORTED_LIST_PROPERTY;
						sortedState = SORT_BY_NAME;
					}
					else {
						printf("Invalid command: %s\n", ptr);
						invalidCommand = 1;
					}
					break;
				}
			}
			if (invalidCommand) break;
			pch = strtok (NULL, " ");
		}

		if (invalidCommand) {
			continue;
		}

		DIR * directory = opendir(".");
		struct dirent * pent;
		int count = 0;

		switch (state) {
			case LIST_NORMAL: {
				while ((count < maxFileCount) && (pent = readdir(directory))) {
					printf("%s\n", pent->d_name);
					count++;
				}
				break;
			}
			case LIST_PROPERTY: {
				struct stat buf;
				while ((count <= maxFileCount) && (pent = readdir(directory))) {
					// get property
					if (stat(pent->d_name, &buf) < 0) {
						fprintf(stderr, "Something wrong with reading file %s\n", pent->d_name);
						return EXIT_FAILURE;
					}
					char buff[40];
					struct tm * timeinfo;
					timeinfo = localtime (&buf.st_mtime);
					strftime(buff, sizeof(buff), "%a %b %d %H:%M:%S SGT %Y", timeinfo);
					printf("%-15s Size:%-15jd Last Modified: %s\n", pent->d_name, buf.st_size, buff);
					count++;
				}
				break;
			}
			case SORTED_LIST_NORMAL: {
				struct dirent **namelist;
				int n;
				n = scandir(".", &namelist, 0, alphasort);
				if (n < 0) {
					perror("scandir\n");
				} else {
					int i;
					n = (maxFileCount < n) ? maxFileCount : n;
					for (i = 0; i < n; i++) {
						printf("%s\n", namelist[i]->d_name);
						free(namelist[i]);
					}
				}
				free(namelist);
				break;
			}
			case SORTED_LIST_PROPERTY: {
				if (sortedState == SORT_BY_NAME) {
					// name
					struct stat buf;
					struct dirent **namelist;
					int n;
					n = scandir(".", &namelist, 0, alphasort);
					if (n < 0) {
						perror("scandir\n");
					} else {
						int i;
						for (i = 0; i < n; i++) {
							if (stat(namelist[i]->d_name, &buf) < 0) {
								fprintf(stderr, "Something wrong with reading file %s\n", namelist[i]->d_name);
								return EXIT_FAILURE;
							}
							char buff[40];
							struct tm * timeinfo;
							timeinfo = localtime (&buf.st_mtime);
							strftime(buff, sizeof(buff), "%a %b %d %H:%M:%S SGT %Y", timeinfo);
							printf("%-15s Size:%-15jd Last Modified: %s\n", namelist[i]->d_name, buf.st_size, buff);
							free(namelist[i]);
						}
					}
					free(namelist);
					break;
				}
				// else
				struct statwname * buffers;
				buffers = malloc(sizeof(struct statwname) * STAT_BUFFER_SIZE);
				int current_buffer_size = STAT_BUFFER_SIZE;
				struct statwname * current_buffer = buffers;

				while ((count <= maxFileCount) && (pent = readdir(directory))) {
					if (count >= STAT_BUFFER_SIZE) {
						// resize buffer by 100 whenever buffer is full
						buffers = realloc(buffers, sizeof(buffers) + sizeof(struct statwname) * STAT_BUFFER_SIZE);
					}
					
					// get property
					if (stat(pent->d_name, &current_buffer->st) < 0) {
						fprintf(stderr, "Something wrong with reading file %s\n", pent->d_name);
						return EXIT_FAILURE;
					}
					// copy name so that name can be printed later
					strcpy(current_buffer->name, pent->d_name);
					count++;
					current_buffer++;
				}

				// sort by either time or size
				switch (sortedState) {
					case SORT_BY_TIME: {
						// time
						qsort(buffers, count, sizeof(struct statwname), timecmpfunc);
						break;
					}
					case SORT_BY_SIZE: {
						qsort(buffers, count, sizeof(struct statwname), sizecmpfunc);
						break;
					}
				}

				// print the files
				int i;
				for (i = 0; i < count; i++) {
					char buff[40];
					struct tm * timeinfo;
					timeinfo = localtime (&buffers[i].st.st_mtime);
					strftime(buff, sizeof(buff), "%a %b %d %H:%M:%S SGT %Y", timeinfo);
					printf("%-15s Size:%-15jd Last Modified: %s\n", buffers[i].name, buffers[i].st.st_size, buff);
				}

				free(buffers);
				break;
			}
		}

		closedir(directory);
	/*
	//list
	//list 1
	//list 1 property
	//list property time
	//list property size
	//etc..
	*/
	}

//--------------------------------------------------------------------
//Find, Case #5
 ptr=strstr(command,find);
	if(ptr!=NULL)
	{
		char * reg = strstrip(ptr + strlen(find) + 1);
		findRecurseDirPrint(".", reg);

	//Implement your code to handle find here

	}
//--------------------------------------------------------------------
//tree Case #6
 ptr=strstr(command,tree);
	if(ptr!=NULL)
	{
	//Implement your code to handle tree here
	/*There are many cases to handle in this part*/

		/*
		states: TREE_NORMAL - prints tree normally
				TREE_DEPTH  - prints tree with depth
				TREE_SORT  - prints sorted tree
		*/
		int maxTreeDepth = MAX_TREE_DEPTH;
		int state = TREE_NORMAL;
		int sortedState = SORT_BY_TIME;
		int invalidCommand = 0;
		char * pch;
		pch = strtok (ptr + strlen(tree) + 1," ");
		while (pch != NULL)
		{
			switch(state) {
				case TREE_NORMAL: {
					if (isDigitString(strstrip(pch))) {
						maxTreeDepth = atoi(pch);
						state = TREE_DEPTH;
					}
					else if (strncmp(pch, "size", 4) == 0) {
						state = TREE_SORT;
						sortedState = SORT_BY_SIZE;
					}
					else if (strncmp(pch, "name", 4) == 0) {
						state = TREE_SORT;
						sortedState = SORT_BY_NAME;
					}
					else if (strncmp(pch, "time", 4) == 0) {
						state = TREE_SORT;
						sortedState = SORT_BY_TIME;
					}
					else {
						printf("Invalid command: %s\n", ptr);
						invalidCommand = 1;
					}
					break;
				}
				case TREE_DEPTH: {
					if (strncmp(pch, "size", 4) == 0) {
						state = TREE_SORT;
						sortedState = SORT_BY_SIZE;
					}
					else if (strncmp(pch, "name", 4) == 0) {
						state = TREE_SORT;
						sortedState = SORT_BY_NAME;
					}
					else if (strncmp(pch, "time", 4) == 0) {
						state = TREE_SORT;
						sortedState = SORT_BY_TIME;
					}
					else {
						printf("Invalid command: %s\n", ptr);
						invalidCommand = 1;
					}
					break;
				}
			}
			if (invalidCommand) break;
			pch = strtok (NULL, " ");
		}

		if (invalidCommand) {
			continue;
		}

		switch (state) {
			case TREE_DEPTH:
			case TREE_NORMAL: {
				treePrint(".", 0, maxTreeDepth);
				break;
			}
			case TREE_SORT: {
				treePrintSort(".", 0, maxTreeDepth, sortedState);
				break;
			}
		}

	/*
	//tree
	//tree 1 
	//tree 1 time
	//tree size
	//tree name
	//etc..
	*/
	}
//--------------------------------------------------------------------
//Wrong command, Case #7
		/*	
		printf("You entered wrong command\n");
	
		*/


}//end while
}//end main
