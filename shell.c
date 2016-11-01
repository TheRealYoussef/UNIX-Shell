#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_LINE 80

int tokenizeInput(char [], char* [], int*);

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
		if (fgets(line, MAX_LINE, stdin) == NULL) {
			continue;
		}
		tokenizeStatus = tokenizeInput(line, args, &concurrent);
		if (tokenizeStatus != 1) {
			if (tokenizeStatus == -1)
				shouldRun = 0;
			continue;
		}
		pid = fork();
		executeStatus = executeCommand(pid, args, concurrent);
		if (executeStatus == 0) {
			return 1;
		}
	}
	return 0;
}

int tokenizeInput(char line[], char* args[], int* concurrent) {
	int i, currentToken = 0, tokenIndex = 0, shouldRun = 1;
	for (i = 0; shouldRun == 1; ++i) {
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
	if (currentToken == 0) {
		return 0;
	}
	if (currentToken == 1 && strcmp(args[0], "exit") == 0) {
		return -1;
	} 
	if (strcmp(args[currentToken - 1], "&") == 0) {
		*concurrent = 1;
		--currentToken;
	}
	else {
		*concurrent = 0;
	}
	args[currentToken] = NULL;
	return 1;
}

int executeCommand(pid_t pid, char* args[], int concurrent) {
	if (pid < 0) {
		printf("Fork failed\n");
		return -1;
	}
	else if (pid == 0) {
		execvp(args[0], args);
		printf("Command does not exist\n");
		return 0;
	}
	else {
		if (concurrent == 0) {
			waitpid(pid, NULL, 0);
		}
		return 1;
	}
}
