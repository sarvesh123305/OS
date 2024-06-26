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
#define DELIMITER " \t\n"  // check space,tabs,newline

typedef struct Node 		//Maintaining History
{
	char data[MAX_LEN];
	struct Node *prev;

} Node;
struct backJob 				//Handling Background Jobs
{
    char jobName[300];
    pid_t PID;  
};

typedef struct backJob backJob;
backJob jobs[50];
typedef Node *LinkedList;

void handleBackgroundJobs(char **args, int argCount);
void displayJobs();

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
	FILE *file = fopen("history.txt", "r"); //fopen  buffering IO ,faster
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
void reverseDisplayHistory(Node* iterator){
	if(!iterator)
		return ;
	reverseDisplayHistory(iterator -> prev);
	printf("%s",iterator->data);
}
void displayHistory(LinkedList tail){
	Node* iterator  = tail;
	printf("\nHISTORY DATA : \n");
	reverseDisplayHistory(iterator);
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
void removeExtraSpaceAtEnd(char *buf){
	for(int i = MAX_LEN-1; i >= 0 ; i--){
		if(isalpha(buf[i]) || buf[i] == '&')
			break;
		else 
			buf[i] = '\0'; // null 
	}
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
	// printf("What's the issue ,I did that  ? %d",(access(filename, X_OK) == 0));
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
	*hasInput = 0, *hasOutput = 0;
	char *outputTokens[2], *inputTokens[2];

	char *inputPointer = strstr(command, "<"); // string from <

	if(inputPointer)
		*hasInput = 1;
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
		if (!isExecutablePresent(*inputFile))
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
char **getArgsForPipe(char *line)
{
  int bufsize = MAX_LEN, position = 0;
  char **tokens = malloc(bufsize * sizeof(char*));

  if (!tokens) {
    perror("Malloc failed");
    exit(EXIT_FAILURE);
  }

  char *token = strtok(line, DELIMITER);
  while (token != NULL) {
   tokens[position++] = token;
   token = strtok(NULL, DELIMITER);
  }
  tokens[position] = NULL;
  return tokens;
}
int testPipe(char *line)
{
	int commandCount = 0 , numberOfPipes = 0 , status;
	char **args;
	for (int i = 1; line[i] != '\0'; i++) {
			if( line[i] == '|' && line[i+1] != '|' && line[i-1] != '|') //not || 
				numberOfPipes++;
	}
	int* pipefds = (int*) malloc( ( 2 * numberOfPipes ) * sizeof(int) ); // 2 pipes, in & out
	char* token = (char*) malloc( MAX_LEN * sizeof(char) );
	token = strtok_r (line,"|",&line); //break down into parts on | 
	for(int i = 0; i < numberOfPipes; i++ ){	
    		if( pipe(pipefds + i * 2) < 0 ){ 
				perror("Pipe Creation Failed");
				return 3;
			}
	}
	do{
	    pid_t pid = fork();
	    if( pid == CHILD ){
	        if( commandCount != 0 ){
	           	if( dup2( pipefds[(commandCount-1)*2], 0) < 0) //replace ip with this
				{
					perror("Input Failed");
					exit(1);
				}
        	}
        	if( commandCount != numberOfPipes){
            		if( dup2(pipefds[commandCount*2+1], 1) < 0 )
					{
						perror("Output failed");
						exit(1);
					}
       	 	}

            for( int i = 0; i < 2 * numberOfPipes ; i++ )
				close(pipefds[i]);

	    	args = getArgsForPipe(token);
            execvp(args[0],args);   //Testing , to be removed
            perror("exec faill");
 	    } 
	    else if( pid < 0 ){
			perror("Fork Failed");
			return 3;
			}
 	    
		commandCount++;//parent process
	}

	while( commandCount <= numberOfPipes && ( token = strtok_r(NULL,"|",&line) ));

	for( int i = 0; i < 2 * numberOfPipes; i++ ){
		close(pipefds[i]);
	}
	// free(pipefds);
	return 1;
}
//  void
// pipeline(char *cmd[MAX_LEN][MAX_LEN],int countArgs)
// {
	//SINGLE PIPE CODE
	// int pipefd[2],status,done = 0 ; 
	// pid_t pid;
	// pipe(pipefd); //Create pipe
	// pid = fork();
	// if(pid == CHILD){
	// 	dup2(pipefd[1],STDOUT_FILENO);
	// 	execlp(argv[0],argv[0],(char*)NULL);
	// }
	// pid = fork();
	// if(pid == CHILD)
	// {
	// 	close(pipefd[1]);
	// 	dup2(pipefd[0],STDIN_FILENO);
	// 	execlp(argv[2],argv[2],(char*)NULL);
	// }

	// close(pipefd[0]);
	// close(pipefd[1]);

	// waitpid(-1,&status,0);
	// waitpid(-1,&status,0);
// }
int mainFunction(char *buf,int bufferLength,int *isP1set,char directoryPaths[MAX_LEN][MAX_LEN],char *currentDirtoryPath
,LinkedList *tail,char *path){
		int colonPresent = 0,hasInput, hasOutput, stdOut = dup(STDOUT_FILENO), stdIn = dup(STDIN_FILENO),argCount = 0;
		colonPresent = countColonPresentInPath(path);
		if (executeEnviromentCommand(buf, bufferLength, isP1set, &colonPresent, directoryPaths))
		{
			updatePath(currentDirtoryPath, *isP1set);
			return 1; //Enviroment Command
		}
		// printf("PATH %s ",path);
		int isPipelinePresent = getPipelinePresentStatus(buf,bufferLength);
		if(isPipelinePresent)
		{	
			testPipe(buf);
			return 10;
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
				printf("File not found\n");
				return -2 ; //File not found
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
			fprintf(stderr,"BFU : %s",buf);

			buf[leng] = '\0';
			if(leng > 0 && buf[leng-1] == '\n' ) //When clear is used , \n is added at last second
						buf[leng-1] = '\0';
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
		fprintf(stderr,"arguments[argCounts] %s",arguments[argCount]);
		if(argCount > 0  && !strcmp(arguments[argCount-1], "&")) // background jobs
		{
			//YET TO DO This, till then ignore background jobs so as to bypass errors
			arguments[argCount-1] = '\0';
			handleBackgroundJobs(arguments,argCount);
			return 10;
		}
		else if(argCount == 1 && !strcmp(arguments[0] , "jobs")) //Displaying all jobs
		{
			displayJobs();
			return 10;
		}

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
					// if (newInput == NOTPOSSIBLE)
					// {
					// 	perror("Error performing Input Redirection");
					// 	return 10;
					// }
					dup2(newInput, 0); // oldfd newfd    replaces fd for above newinput file to 0 i.e stdin
					close(newInput);   // Original assigned fp is closed because it is now pointing to stdin
				}

				if (hasOutput)
				{

					int newOutput;
					if (hasOutput) // Now Output would be to file
						newOutput = open(outputFile, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
					// Refer Documentation
					// printf("move towards head");

					// if (newOutput == NOTPOSSIBLE)
					// {
					// 	perror("Error performing Output Redirection");
					// 	return  10 ;//continue
					// }
					dup2(newOutput, 1); // replaces ff for above newoutput file to 1 i.e stdout
					close(newOutput);	// Original assigned fp is closed because it is now pointing to stdout
				}

				colonPresent = countColonPresentInPath(path);
				if(isRedirectionPresent)
					customExecvpWithPath(colonPresent, directoryPaths, args);
				else{
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
			waitpid(pid, NULL, 0);
			// wait(NULL); // parent waiting for a child
		}
	}
void handlerCtrlC(int num){
	// write(STDOUT_FILENO,"I won't die\n",13);
}
	pid_t CHILD_ID = -1;
	char CURR_JOB[2000];
	int backgroundJob = 0;
void shift(int i)
{
     for(int j=i; j <= backgroundJob; j++)
     {
          strcpy(jobs[j].jobName, jobs[j+1].jobName);
          jobs[j].PID = jobs[j+1].PID;
     }
     
     backgroundJob--;
}
void handler(int sig)
{
     int x;
     pid_t pid = waitpid(-1, &x, WNOHANG);
     if(pid > 0)
     {
          char str[200]; int flag =0;
          for(int i=0; i < backgroundJob; i++)
          {
               if(jobs[i].PID == pid)
               {

                    strcpy(str, jobs[i].jobName);
                    shift(i);
                    flag = 1;      
                    break;
               }
          }

          if(WEXITSTATUS(x) == 0 && WIFEXITED(x) && flag) 	//Process execution done successfullyy
          fprintf(stderr,"\033[1;31m\n%s with PID %d exited normally\n\033[0m", str, pid);

          else if(flag) fprintf(stderr,"\033[1;31m\n%s with PID %d failed to exit normally\n\033[0m",str, pid);
          if(flag)
		  	displayPrompt();
     }
     
     return;
}


void handleBackgroundJobs(char **args, int argCount)
{
    int status;
    pid_t pid = fork(), wpid;

    //  args[argCount] = NULL;
     if (pid == FORK_FAILURE) 
     {    
          perror("Error");
          exit(EXIT_FAILURE);
     }

     else if (pid == CHILD) 
     {   
          setpgid(0, 0);//making process parent as 0  so as to disassociate it from curr proc
		//   perror("CHILD IT IS ");
          if (execvp(args[0], args) < 0) //Testings
          {     
               perror("Command not found");
               exit(EXIT_FAILURE);
          }
     }
     else 
     {
				// perror("Should I enter"); //Dont wait for the job to complete just push to bg jobs
			   strcpy(jobs[backgroundJob].jobName, args[0]);
               
               for(int i = 1; i < argCount-1; i++)
               {
                    strcat(jobs[backgroundJob].jobName, " ");
                    strcat(jobs[backgroundJob].jobName, args[i]);
               }

               jobs[backgroundJob].PID = pid;
               backgroundJob++;
               printf("[%d] %d\n", backgroundJob, pid);
				// perror("LALAL");
			 
     }
	//  printf("SUCCEESSS");
     return;
}

char * readFile(char *filename, int n)
{   
    FILE* file = fopen(filename, "r");
    char line[MAX_LEN];
    char *data = malloc(sizeof(char) * MAX_LEN);
    int i = 0;
    while (fgets(line, sizeof(line), file)) 
    {
        i++;
        if(i == n)
            strcpy(data, line);
    }
    fclose(file);
    return data;
} 

void displayJobs(){
	char processes[MAX_LEN];
	for(int i = 0 ; i < backgroundJob; i++){
		sprintf(processes,"/proc/%d/status",jobs[i].PID);
		FILE *fp = fopen(processes,"r");
		if(!fp)
			return ;
		
		char *statusInfo = readFile(processes, 3); //Get process state by reading file 3 bcoz start from 3 rd line
		char *temp = statusInfo;
		statusInfo = strtok(statusInfo, " :\n\t");	//Pulling out state from state:RUNNING
		statusInfo = strtok(NULL, " :\n\t");

		strcpy(statusInfo, "Running");

		printf("[%d] %s %s [%d]\n",i+1, statusInfo, jobs[i].jobName, jobs[i].PID);
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
	
	signal(SIGINT,handlerCtrlC);

	// Maintaining history here
	LinkedList tail;
	Node* iterator;
	initList(&tail);
	initList(&iterator);
	initialLoadLinkedList(&tail);
	signal(SIGCHLD, handler);

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
		removeExtraSpaceAtEnd(buf);
		int bufferLength = strlen(buf);
		appendToFileAndLinkedList(&tail,buf);
		mainFunction(buf,bufferLength,&isP1set,directoryPaths,currentDirtoryPath,&tail,path);
	}
	exit(EXIT_SUCCESS);
}
