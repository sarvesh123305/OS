#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <ncurses.h>
#include <sys/termios.h>


#define MAX_LEN 150
#define FORK_FAILURE -1
#define CHILD 0
#define NOREDIRECTION 'N'
#define INPUTREDIRECTION 'I'
#define OUTPUTREDIRECTION 'O'
#define NOTPOSSIBLE -1

typedef struct Node
{
	char data[MAX_LEN];
	struct Node *prev;

} Node;

typedef Node *LinkedList;

void initList(LinkedList *node)
{
	*node = NULL;
	return;
}
void append(LinkedList *node, char *str)
{
	Node *newNode = (Node *)malloc(sizeof(Node));
	if (!newNode) // malloc failed
		return;

	newNode->prev = NULL;

	strcpy(newNode->data, str);
	if(!(*node)){
		*node = newNode;
	}
	else{
		newNode -> prev = (*node);
		*node = newNode;
	}
}

void initialLoadLinkedList(LinkedList *tail){
	FILE *file = fopen("history.txt", "r"); 
    if (!file) {
		return ;
    }

    char buffer[MAX_LEN];

    while (fgets(buffer, MAX_LEN, file)) {
		append(tail,buffer);
    }

    fclose(file);
}
void appendToFileAndLinkedList(LinkedList* tail,char *str){
	FILE *file = fopen("history.txt", "a+"); 
    if (!file) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }
	    fprintf(file, "%s", str);
		append(tail,str);
	    fclose(file);
}
void upsideDown(Node* iterator){
	if(!iterator)
		return ;
	upsideDown(iterator -> prev);
	printf("%s",iterator->data);
}
void displayHistory(LinkedList tail){
	Node* iterator  = tail;
	printf("\nHISTORY DATA : \n");
	upsideDown(iterator);
}
void freeList(LinkedList *tail){
	Node* current = *tail;
	while(current){
		Node* next = current -> prev;
		free(current);
		current = next;
	}
	*tail = NULL;
	return ;
}
void clearHistory(LinkedList *tail){
	FILE *file = fopen("history.txt", "w"); 
	  if (!file) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }
	freeList(tail);
	printf("History Cleared");
	return ;
}
void updatePath(char *currentDirtoryPath, int isPS1Changed)
{ // Update current Directory path
	char tmpDirectoryPath[MAX_LEN];

	if (!getcwd(tmpDirectoryPath, sizeof(tmpDirectoryPath)))
		exit(EXIT_FAILURE);

	if (!isPS1Changed)
	{
		setenv("PS1", tmpDirectoryPath, 1);
		strcpy(currentDirtoryPath, tmpDirectoryPath);
	}
	else
		setenv("PS1", getenv("PS1"), 1);

	return; // Successfully return to main with new Path
}
void removeTrailingSpacesIfAny(char *buf)
{ // Remove unnecessary spaces in front if present
	if (!buf)
		return;

	int len = strlen(buf), i = 0;
	while (i < len)
	{
		if (isspace(buf[i]))
			i++;
		else
			break;
	}
	memmove(buf, buf + i, len - i + 1); // +1 to copy the null terminator
	return;
}
void checkSpecialCharactersPresentInFront(char *buf)
{
	if (!buf)
		return;
	if (!isalpha(buf[0]))
	{ // It is not
		printf("Invalid!!Special Symbols at the start is not allowed");
		exit(EXIT_FAILURE);
	}
}
int returnArguementCount(char *argv)
{
	int wordCount = 0;
	int flag = 0;
	for (int i = 0; i < strlen(argv); i++)
	{
		if (isspace(argv[i]))
		{
			if (flag)
			{
				flag = 0;
				wordCount++;
			}
		}
		else
		{
			flag = 1;
		}
	}
	return wordCount;
}
int isExecutablePresent(char *filename)
{
	return (access(filename, X_OK) == 0);
}
int countColonPresentInPath(char *path)
{
	int count = 0;
	int flag = 0;
	for (int i = 0; i < strlen(path); i++)
	{
		if (path[i] == ':')
		{
			if (flag)
				count++;
		}
		else
		{
			flag = 1;
		}
	}
	return count + 1;
}
void returnPaths(char origPath[MAX_LEN][MAX_LEN], char *path, int noOfPaths)
{
	char pathArr[noOfPaths][MAX_LEN];

	char buffer[MAX_LEN];
	strncpy(buffer, path, sizeof(buffer));
	buffer[sizeof(buffer) - 1] = '\0'; // Ensure null-terminated string

	char *token = strtok(buffer, ": \t"); // Skip both colon and space/tab
	int count = 0;

	while (token != NULL && count < MAX_LEN)
	{
		strcpy(pathArr[count], token);
		token = strtok(NULL, ": \t");
		count++;
	}

	for (int i = 0; i < noOfPaths; i++)
		strcpy(origPath[i], pathArr[i]);
}
int getRedirectionStatus(char *buf, int bufferLength)
{
	for (int i = 0; i < bufferLength; i++)
	{
		if (buf[i] == '<' || buf[i] == '>')
			return 1;
	}
	return 0;
}
void displayPrompt()
{
	char computerName[MAX_LEN];
	if (!gethostname(computerName, sizeof(computerName)) == 0)
		perror("Error fetching computer name");

	printf("\x1b[2;37m%s\x1b[0m\n", getenv("PS1"));
	// printf("%s\n", getenv("PS1"));
	// printf("\x1b[48;5;237m %s \x1b[0m ", computerName);
	// printf("\033[0;32m%s$ ", computerName);
	// printf("\033[0;30m");
}
int executeEnviromentCommand(char *buf, int bufferLength, int *isP1set, int *colonPresent, char directoryPaths[MAX_LEN][MAX_LEN])
{
	if (bufferLength >= 3 && buf[0] == 'P' && buf[1] == 'S')
	{
		if (buf[5] == '\\' && buf[6] == 'w' && buf[7] == '$')
		{
			*isP1set = 0;
		}
		else
		{
			char tempString[20];
			strcpy(tempString, buf + 5);
			tempString[strlen(tempString) - 2] = '\0';
			setenv("PS1", tempString, 1);
			*isP1set = 1;
		}
		return 1;
	}

	if (bufferLength > 5 && (strncmp(buf, "PATH=", 5) == 0))
	{
		char newPath[MAX_LEN];
		strcpy(newPath, buf + 6);
		newPath[strlen(newPath) - 2] = '\0'; // remove last colon
		removeTrailingSpacesIfAny(newPath);
		setenv("CUST_PATH", newPath, 1);
		*colonPresent = countColonPresentInPath(newPath);
		returnPaths(directoryPaths, newPath, *colonPresent);
		// PATH="/bin:/sbin"
		return 1;
	}
	return 0;
}
// additional functionality change username , remove " " if present in arguements
void customExecvpWithPath(int colonPresent, char directoryPaths[MAX_LEN][MAX_LEN], char *arguments[MAX_LEN])
{
	// fprintf(stderr,"%d yerror" ,colonPresent);

	for (int i = 0; i < colonPresent; i++)
	{
		char tempString[MAX_LEN];
		strcpy(tempString, directoryPaths[i]);
		if (tempString[strlen(directoryPaths[i]) - 1] != '/')
			strcat(tempString, "/");
		strcat(tempString, arguments[0]);
		int isExecPresent = isExecutablePresent(tempString);
		if (isExecPresent)
			execv(tempString, arguments);
	}
	perror("Incorrect Syntax or Command not found...");
	exit(EXIT_FAILURE);
}
int checkForCustomCommands(int argCount, char *arguments[MAX_LEN], char *currentDirtoryPath, int isP1set,LinkedList *tail)
{

	if (argCount == 1 && strcmp(arguments[0], "exit") == 0)
	{
		system(arguments[0]);
		return 1;
	}
	if (argCount >= 1 && strcmp(arguments[0], "cd") == 0)
	{
		char path[MAX_LEN] = "";
		if (argCount == 1)
		{
			strcpy(path, "/");
		}
		else if (argCount == 2 && !strcmp("-", arguments[1]))
		{
			strcpy(path, "..");
		}
		else if (argCount == 2 && !strcmp("-P", arguments[1]))
		{
			strcpy(path, "/");
		}
		else
		{
			for (int i = 1; i < argCount; i++)
			{
				strcat(path, arguments[i]);
				if (i != argCount - 1)
					strcat(path, " ");
			}
		}
		if (chdir(path))
			perror("Error changing directory");
		updatePath(currentDirtoryPath, isP1set);
		return 2;
	}
	if(strcmp(arguments[0], "history") == 0){
		if(argCount == 2 && strcmp("-c", arguments[1]) == 0){
		// 	// clearall
			clearHistory(tail);
		}
		else{
			displayHistory(*tail);
		}
		return 3;
	}

	return 0;
}
int setRedirection(char *command, int *hasInput, int *hasOutput, char **outputFile, char **inputFile, char *args[MAX_LEN])
{
	char *outputTokens[2], *inputTokens[2];

	char *inputPointer = strstr(command, "<"); // string from <
	*hasInput = !(inputPointer == NULL), *hasOutput = 0;

	char *outputPointer = strstr(command, ">");
	if (outputPointer)
		*hasOutput = 1;

	outputTokens[0] = (char *)malloc(strlen(command) * sizeof(char));
	strcpy(outputTokens[0], command);

	if (*hasOutput)
	{
		outputTokens[0] = strtok(command, ">");
		outputTokens[1] = strtok(NULL, ">");
		*outputFile = strtok(outputTokens[1], " \n\r\t"); // All above in fun with newline tab space ,carriage return move cursor to top of string
	}
	// PATH="/bin"
	inputTokens[0] = outputTokens[0];
	if (*hasInput)
	{

		inputTokens[0] = strtok(inputTokens[0], "<");
		inputTokens[1] = strtok(NULL, "<");
	}
	int numberOfArguments = 0;
	if (*hasInput)
	{
		if (!inputTokens[1])
			return 1;

		*inputFile = strtok(inputTokens[1], " \n\r\t");
		// printf("daasd : %s",*inputFile);
		if (isExecutablePresent(*inputFile))
			return 2;
	}

	inputTokens[0] = strtok(inputTokens[0], " \n\r\t");

	while (inputTokens[0])
	{
		args[numberOfArguments] = (char *)malloc(sizeof(char) * strlen(inputTokens[0]));
		strcpy(args[numberOfArguments], inputTokens[0]);
		inputTokens[0] = strtok(NULL, " \n\t\r");
		numberOfArguments++;
	}

	args[numberOfArguments] = NULL;

	if (*hasOutput)
	{
		if (!outputFile)
		{
			return 3;
		}
	}
	return 0;
}
void setArguements(char *buf, char *arguments[MAX_LEN], int *argCount)
{	
	char *token = strtok(buf, " ");
	while (token)
	{
		arguments[*argCount] = strdup(token); // Allocate memory for each argument
		(*argCount) = (*argCount) + 1;
		while (*token && *token == ' ')
			token++;

		token = strtok(NULL, " ");
	}
	arguments[*argCount] = NULL;
	return;
}
void removeQuotesIfPresent(char *str)
{
	int len = strlen(str);
	if (str[0] == '"')
		memmove(str, str + 1, len - 2);
	if (str[len - 1] == '"')
		str[len - 2] = '\0';
}
int getPipelinePresentStatus(char *buf,int bufferLength){
	for (int i = 0; i < bufferLength; i++)
		if (buf[i] == '|')
			return 1;

	return 0;
}
static void
pipeline(char ***cmd)
{
	int fd[2];
	pid_t pid;
	int fdd = 0;				/* Backup */
	while (*cmd != NULL) {

		pipe(fd);				/* Sharing bidiflow */
		if ((pid = fork()) == -1) {
			printf("Fork failed");
			perror("fork");

			exit(1);
		}
		else if (pid == 0) {
			dup2(fdd, 0);
			if (*(cmd + 1) != NULL) {
				dup2(fd[1], 1);
			}
			close(fd[0]);
			execvp((*cmd)[0], *cmd);
			// char directoryPaths[MAX_LEN][MAX_LEN];
			// char *path = getenv("PATH"); // sets default path to that of shell path
			// setenv("CUST_PATH", path, 1);
			// int colonPresent = 0;
			// returnPaths(directoryPaths, path, countColonPresentInPath(path)); // default path is this
			// customExecvpWithPath(countColonPresentInPath(path),directoryPaths,*cmd);
			exit(1);
		}
		else {
			wait(NULL); 		/* Collect childs */
			close(fd[1]);
			fdd = fd[0];
			cmd++;
		}
	}
}
int mainFunction(char *buf,int bufferLength,int *isP1set,char directoryPaths[MAX_LEN][MAX_LEN],char *currentDirtoryPath
,LinkedList *tail,char *path){
		int colonPresent = 0,hasInput, hasOutput, stdOut = dup(STDOUT_FILENO), stdIn = dup(STDIN_FILENO),argCount = 0;
		colonPresent = countColonPresentInPath(path);
		if (executeEnviromentCommand(buf, bufferLength, isP1set, &colonPresent, directoryPaths))
		{
			updatePath(currentDirtoryPath, *isP1set);
			return 1; //Enviroment Command
		}

		int isPipelinePresent = getPipelinePresentStatus(buf,bufferLength);
		if(1)
		{
			// printf("\nYET TO IMPLEMENT >>> DONT DO THIS\n");
			// char tempUse[MAX_LEN] = "ls -al | rev";
			// char*** sad = split_commands(tempUse);
			// char tempCopy[MAX_LEN];
			// strcpy(tempCopy, tempUse); // Create a copy of tempUse
			// char *tokensize = strtok(tempCopy,"|");
			// char **cmd[MAX_LEN];
			// char *tempArguements[MAX_LEN];
			// int k = 0 ;
			// while(tokensize){
			// 	int argCount = 0;
			// 	tempArguements[k++] = tokensize;
			// 	// printf("\nDhirubhai %s",tokensize);
			// 	tokensize = strtok(NULL, "|");
			// }
			// tempArguements[k] = NULL;
			// for (int i = 0; i < k; i++) {
			// 	int uselesscounter = 0;
			// 	cmd[i] = malloc(sizeof(char*) * MAX_LEN); 
			// 	setArguements(tempArguements[i],cmd[i],&uselesscounter);
			// }
			// cmd[k] = malloc(sizeof(char*) * MAX_LEN); 
			// *cmd[k] = NULL;
			//  printf("Commands:\n");
			// for (int i = 0; i < k; i++) {
			// 	// printf("Command %d:", i + 1);
			// 	int j = 0;
			// 	int count = 0 ;
			// 	while (cmd[i][j]) {
			// 		// printf("%s ", cmd[i][j]);
			// 		j++;
			// 	}
			// 	cmd[i][j] = NULL;
			// 	cmd[i][j+1] = NULL;
			// 	// printf("%s",cmd[i][j]);
			// 	// printf("%s",cmd[i][j+1]);

			// 	// printf("\n");
			// }
			// cmd[k+1] = malloc(sizeof(char*) * MAX_LEN); 
			// *cmd[k+1] = NULL;
			// for (int i = 0; i < k; i++) {
			// 	for(int j = 0 ; j < 4; j++)
			// 			printf("%s ",cmd[i][j]);
			// 			printf("\n");
			// }
			// printf("%s ",cmd[k-1][1]);

			// char *ls[] = {"ls","-al",NULL};
			// char *rev[] = {"rev", NULL};
			// char *nl[] = {"nl", NULL};
			// char *cat[] = {"cat", "-e", NULL};
			// char **cmd[] = { ls,rev, nl, cat, NULL};
			// // pipeline(cmd);
			// pipeline(sad);
			// 	exit(0);
		}

		char *outputFile = NULL, *inputFile = NULL, *args[MAX_LEN], *arguments[MAX_LEN];

		int isRedirectionPresent = getRedirectionStatus(buf, bufferLength);
		if (isRedirectionPresent)
		{
			// preprocess and set ip/op file names and flags
			// Pointer not updating from the above file
			switch (setRedirection(buf, &hasInput, &hasOutput, &outputFile, &inputFile, args))
			{
			case 1:
				printf("Input file missing , Please Enter Input file\n");
				return -1 ;  //Input file missing
				break;
			case 2:
				printf("Command not found\n");
				return -2 ; //Command not found
				break;
			case 3:
				printf("Output file missing , Enter an Output file\n");
				return -3;//Output file missing
				break;
			// default : printf("Redirection not present");
			}
		}
		else
		{
			int wordCount = returnArguementCount(buf), leng = strlen(buf);
			buf[leng - 1] = '\0';

			setArguements(buf, arguments, &argCount);
			switch (checkForCustomCommands(argCount, arguments, currentDirtoryPath, *isP1set,tail))
			{
				case 1:
					exit(EXIT_SUCCESS);
					break;
				case 2:
				case 3:
					// continue;
					return 10;//10 for continue
					break;
			}
		
			
		}
		// if(argCount > 0  && arguements[argCount-1])
		// printf("%d argCount",argCount);
		int pid = fork();
		if (pid == FORK_FAILURE)
		{

			perror("Forking the process failed!!,Exiting the program");
			exit(EXIT_FAILURE);
		}

		if (pid == CHILD)
		{
			// if (isRedirectionPresent)
			// {

				if (hasInput)
				{
					int newInput = open(inputFile, O_RDONLY); // Input would be from file
					if (newInput == NOTPOSSIBLE)
					{
						perror("Error performing Input Redirection");
						return 10;
					}
					dup2(newInput, 0); // oldfd newfd    replaces fd for above newinput file to 0 i.e stdin
					close(newInput);   // Original assigned fp is closed because it is now pointing to stdin
				}

				if (hasOutput)
				{

					int newOutput;
					if (hasOutput) // Now Output would be to file
						newOutput = open(outputFile, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
					// Refer Documentation
					printf("move towards head");

					// if (newOutput == NOTPOSSIBLE)
					// {
					// 	perror("Error performing Output Redirection");
					// 	return  10 ;//continue
					// }
					dup2(newOutput, 1); // replaces ff for above newoutput file to 1 i.e stdout
					close(newOutput);	// Original assigned fp is closed because it is now pointing to stdout
				}

				if(isRedirectionPresent)
					customExecvpWithPath(colonPresent, directoryPaths, args);
				else{
					colonPresent = countColonPresentInPath(path);
					customExecvpWithPath(colonPresent, directoryPaths, arguments);
				}

 				if (hasInput)
				{
					dup2(stdIn, 0); // Resetting the changes done to input
					close(stdIn);	// Orig fd of stdin close
				}
				if (hasOutput)
				{
					dup2(stdOut, 1); // Resetting the changes done to input
					close(stdOut);	 // Orig fd of stdout close
				}
			// }
			// else
			// {
			// 	colonPresent = countColonPresentInPath(path);
			// 	customExecvpWithPath(colonPresent, directoryPaths, arguments);
			// }
		}
		else
		{
			wait(NULL); // parent waiting for a child
		}
	}

int main()
{ 
	pid_t pid; 
	char buf[MAX_LEN], directoryPaths[MAX_LEN][MAX_LEN], currentDirtoryPath[MAX_LEN];
	char *path = getenv("PATH"); // sets default path to that of shell path
	setenv("CUST_PATH", path, 1);
	// system("clear");
	int isP1set = 0;
	updatePath(currentDirtoryPath, isP1set);
	returnPaths(directoryPaths, path, countColonPresentInPath(path)); // default path is this
	

	// Maintaining history here
	LinkedList tail;
	Node* iterator;
	initList(&tail);
	initList(&iterator);
	initialLoadLinkedList(&tail);
	
	while (1)
	{
		strcpy(path, getenv("CUST_PATH"));
		displayPrompt();
		 if (!fgets(buf, MAX_LEN, stdin))
		 {
		  	printf("Exitt!! || Ctrl+D");
		 	break;
		 }
		removeTrailingSpacesIfAny(buf);
		int bufferLength = strlen(buf);
		appendToFileAndLinkedList(&tail,buf);
		mainFunction(buf,bufferLength,&isP1set,directoryPaths,currentDirtoryPath,&tail,path);
	}
	exit(EXIT_SUCCESS);
}
