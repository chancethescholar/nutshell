#ifndef GLOBAL_H
#define GLOBAL_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/signal.h>
#include <fcntl.h>
#include <regex.h>
#include <pwd.h>
#include <glob.h>
#include <string.h>
#include <signal.h>  
#define ERROR 	1
#define OK		0
#define EVER   ;;
#define MAXARGS 500
#define MAXCMD 500

extern int yylineno;
extern char** environ;

///////Alias Table/////////////
typedef struct Node{
	char* aliasName;
	char* aliasWord;
	
	struct Node *next;
} Node;


Node *head = NULL;
int size = 0;
////////End Alias Table///////////

char* infile_name=NULL;  //in file description
char* out_file_name=NULL;	//out file description
char* err_file_name=NULL; //error file  description
int open_permission =0; //open option
int background = 0; //check & background or not

typedef struct com {
	char* comname;				// command name
	int nargs;				// arguments count

	char *args[MAXARGS]; //inital argment string arrary

} COMMAND;

COMMAND comtab[MAXCMD]; //initial command table

int currcmd= 0; // current cmd index

void yyerror(const char * s);
int yylex();
void alias_print();
void alias(char*, char*);
void unalias(char*);
char* searchAlias(char*);
void printenv();
void printcmdt(); //print command table
void run(); //execute command via execvp
char* environmentVariable(char*);
char* tildeExpansion(char*);	
void prompt();
void setSignal();
int contain_char(char*, char);
char* combine_string(char*, char*);
void escape(char*);

#endif
