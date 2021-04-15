#ifndef GLOBAL_H
#define GLOBAL_H

#include "stdbool.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/signal.h>
#include <fcntl.h>
#include <regex.h>
#include <limits.h>
#include <pwd.h>
#include <glob.h>
#include <string.h>
#include <signal.h>
#include <fnmatch.h>
#include <dirent.h>

extern int yylineno;
extern char** environ;

///////Alias Table/////////////
typedef struct Node{
	char* name;
	char* word;

	struct Node* next;
} Node;


Node* head;
int aliasSize;;

struct evTable {
   char var[128][100];
   char word[128][100];
};

char cwd[PATH_MAX];
struct evTable varTable;
int aliasIndex, varIndex;

char* inFileName;  //in file description
char* outFileName;	//out file description
char* errFileName; //error file  description
int openPermission; //open option
int background; //check & background or not

typedef struct com
{
	char* comName;				// command name
	int numArgs;				// arguments count

	char* args[500]; //inital argment string arrary

} COMMAND;

COMMAND commandTable[500]; //initial command table

int currentCommand; // current cmd index
int argc;

void yyerror(const char * s);
int yylex();
void execute(); //execute non built in commands using execv
void reset();
int containChar(char* string, char character);
void escape(char* string);

int runSetEnv(char* variable, char* word);
int runPrintEnv();
int runUnsetEnv(char *variable);
int var_count;
char* envExpansion(char* arg);
int runSetAlias(char *name, char *word);
int runListAlias(void);
int runRemoveAlias(char *name);
char* subAliases(char* name);
bool ifAlias(char* name);
int runCDnoargs(void);
int runCD(char* arg);
char* tildeExpansion(char* string);
char* getPath(char* command);


#endif
