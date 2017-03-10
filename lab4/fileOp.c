#include <stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<unistd.h>
#include<string.h>
#include<ctype.h>
#include<dirent.h>
//#include<sys/stat.h >
#define MAX_INPUT 256
#define SIZE 1024

// from the linux kernel
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

int isDigitString(char * str) {
	for (int i = 0; str[i] != 0; i++) {
		if (!isdigit(str[i])) {
			return 0;
		}
	}
	return 1;
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
		/* state machine: 0 - list normally
		 * 				  1 - list with property
		 * 				  2 - sorted name list without property
		 * 				  3 - sorted list with property
		*/
		// if count 1: normal list
		// if count 2: either int or property
		// if count 3: if int, property

		/* sorted: 1 - time
		           2 - size
		           3 - name
	   */
		int maxFileCount = 100000;
		int state = 0;
		int sortedState = 1;
		int invalidCommand = 0;
		char * pch;
		pch = strtok (ptr + strlen(list) + 1," ");
		while (pch != NULL)
		{
			switch(state) {
				case 0: {
					if (isDigitString(pch)) {
						maxFileCount = atoi(pch);
					}
					else if (strncmp(pch, "property", 8) == 0) {
						state = 1;
					}
					else if (strncmp(pch, "name", 4) == 0) {
						printf("State 2!\n");
						state = 2;
						sortedState = 3;
					}
					else {
						printf("Invalid command: %s\n", ptr);
						invalidCommand = 1;
					}
					break;
				}
				case 1: {
					if (strncmp(pch, "time", 4) == 0) {
						state = 3;
						sortedState = 1;
					}
					else if (strncmp(pch, "size", 4) == 0) {
						state = 3;
						sortedState = 2;
					}
					else if (strncmp(pch, "name", 4) == 0) {
						state = 3;
						sortedState = 3;
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
			case 0: {
				while ((count <= maxFileCount) && (pent = readdir(directory))) {
					printf("%s\n", pent->d_name);
					count++;
				}
				break;
			}
			case 1: {
				while ((count <= maxFileCount) && (pent = readdir(directory))) {
					// get property
					printf("%s\n", pent->d_name);
					count++;
				}
				break;
			}
			case 2: {
				struct dirent **namelist;
				int n;
				n = scandir(".", &namelist, 0, alphasort);
				if (n < 0) {
					perror("scandir\n");
				} else {
					for (int i = 0; i < n; i++) {
						printf("%s\n", namelist[i]->d_name);
						free(namelist[i]);
					}
				}
				free(namelist);
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
		struct stat buf;
		char * reg = strstrip(ptr + strlen(find) + 1);
		DIR * directory = opendir(".");
		struct dirent * pent;
		while (pent = readdir(directory)) {
			if (strstr(pent->d_name, reg) != NULL) {
				if (stat(pent->d_name, &buf) < 0) {
					perror("Something wrong with reading file %s", pent->d_name);
					return EXIT_FAILURE;
				}

				// if directory
				if (S_ISDIR(buf.st_mode)) {

				}

			}
		}

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
