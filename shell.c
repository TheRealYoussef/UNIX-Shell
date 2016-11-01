#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

//Maximum command length is 80 characters
#define MAX_LINE 80

/*
*Splits the command into tokens. Returns 0 if the command is empty, -1 if 
*the command is exit, and 1 otherwise.
*/
int tokenizeInput(char [], char* [], int*);

/*
*The child process executes the command, and the parent process waits for 
*the child process to end if the command didn't end with '&', and doesn't
*wait otherwise.
*/
int executeCommand(pid_t, char* args[], int);

int main() {
	int shouldRun = 1;
	char line[MAX_LINE];
	char* args[MAX_LINE / 2 + 1];
	int concurrent;
	pid_t pid;
	int tokenizeStatus;
	int executeStatus;
	while (shouldRun == 1) {
		printf("osh> ");
		fflush(stdout);
		if (fgets(line, MAX_LINE, stdin) == NULL) { //Read command
			continue;
		}
		tokenizeStatus = tokenizeInput(line, args, &concurrent); //Tokenize command
		if (tokenizeStatus != 1) { //If the command is "exit" or empty
			if (tokenizeStatus == -1) //If the command is "exit"
				shouldRun = 0; //Stop running
			continue;
		}
		pid = fork(); //Fork
		executeStatus = executeCommand(pid, args, concurrent); //Execute command
		if (executeStatus == 0) { //If child failed to execute the command
			return 1;
		}
	}
	return 0;
}

int tokenizeInput(char line[], char* args[], int* concurrent) {
	int i, currentToken = 0, tokenIndex = 0, shouldRun = 1;
	for (i = 0; shouldRun == 1; ++i) {
		//Split command at whitespaces, '\n' and '\0'
		if (line[i] == ' ' || line[i] == '\t' || line[i] == '\n' || line[i] == '\0') {
			if (line[i] == '\0') {
				shouldRun = 0;
			}
			line[i] = '\0';
			if (i - tokenIndex > 0) {
				args[currentToken] = &line[tokenIndex];
				++currentToken;
			}
			tokenIndex = i + 1;
		}
	}
	if (currentToken == 0) { //If command is empty
		return 0;
	}
	if (currentToken == 1 && strcmp(args[0], "exit") == 0) { //If command is "exit"
		return -1;
	} 
	if (strcmp(args[currentToken - 1], "&") == 0) { //If command ended with '&'
		*concurrent = 1; //Run it concurrently
		--currentToken;
	}
	else { //Command didn't end with '&'
		*concurrent = 0; //Don't run it concurrently
	}
	args[currentToken] = NULL;
	return 1;
}

int executeCommand(pid_t pid, char* args[], int concurrent) {
	if (pid < 0) { //Fork failed
		printf("Fork failed\n");
		return -1;
	}
	else if (pid == 0) { //Child process
		execvp(args[0], args); //Execute the command
		printf("Command does not exist\n"); //Command was incorrect
		return 0;
	}
	else { //Parent process
		if (concurrent == 0) { //Command didn't end with '&'
			waitpid(pid, NULL, 0); //Wait for child process to finish
		}
		return 1;
	}
}
