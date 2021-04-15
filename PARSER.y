%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"

Node* head = NULL;
int aliasSize = 0;

char* inFileName = NULL;  //in file description
char* outFileName = NULL;	//out file description
char* errFileName = NULL; //error file  description
int openPermission = 0; //open option
int background = 0; //check & for command in background

%}


%token	<string> STRING

%token 	NOTOKEN END GT LT PIPE ERRORF ERROR1 AND GTGT GTGTAND GTAND  TERMINATOR

%union	{char *string;}

%%


complete_command:
commands
;

commands:
command
| commands command
;

command:
pipeline io_redirection   background END {
	//printf(" execute \n");
	//printcmdt();
	execute(); // execute  complete command in command table


}
| END {

}
| error END { yyerrok; }
;

pipeline:
pipeline PIPE {
	currentCommand++; //another new simple command create by increase command table array index

} command_arguments
| command_arguments
;

command_arguments:
command_word arguments {

}
;

arguments:
arguments argument
| //can emputy
;
argument:
STRING { // printf(" argument %s ", $1);

	if (containChar($1, '?') || containChar($1, '*') )// whildcard reconize from tokens
	{
		glob_t pattern;
		if (glob($1, 0, NULL, &pattern) == 0)// find match pattern
		{
			int num;
			num = pattern.gl_pathc; //number of matching pattern

			int i;


			for (i = 0; i < pattern.gl_pathc; i++)// for loop to add each matching pattern to arguments in same simple command
			{
				commandTable[currentCommand].numArgs++;
				commandTable[currentCommand].args[commandTable[currentCommand].numArgs]=strdup(pattern.gl_pathv[i]);
			}
		}
	}
	else{
		//cannot find any match pattern just add *?pattern as argument
		commandTable[currentCommand].numArgs++;
		commandTable[currentCommand].args[commandTable[currentCommand].numArgs]=$1;
	}
}
;

command_word:
STRING {//printf("command is \n", $1);
	//every command star with cmd XXXXX here we inital command element
	commandTable[currentCommand].comName=$1;
	commandTable[currentCommand].args[0]=$1;
	commandTable[currentCommand].numArgs = 0;

}
;

io_redirection:
io_redirection iodirect
| // can emputy
;

iodirect:
GTGT STRING {
	openPermission = O_WRONLY | O_CREAT | O_APPEND ;// APPEND mode

	outFileName=$2;
}
| GT STRING {
	openPermission = O_WRONLY  | O_TRUNC| O_CREAT; //insert is creat mode
	outFileName=$2;
}
| GTGTAND STRING {
	openPermission = O_WRONLY  | O_CREAT| O_APPEND;
	outFileName = $2;
	errFileName = $2;
}
| GTAND STRING {/*printf("    > %s \n", $2);*/
	openPermission = O_WRONLY  | O_TRUNC| O_CREAT;
	//outFileName = $2;
	errFileName = $2;
}
|
ERRORF STRING {/*printf("    2> %s \n", $2);*/
	openPermission = O_WRONLY | O_TRUNC| O_CREAT ;
	errFileName=$2;
}
|
ERROR1 {/*printf("    2>&1 %s \n", $2);*/
	errFileName="error";
}
| LT STRING {/*printf("    < %s \n", $2);*/
	inFileName=$2;
}
;

background:
AND {//printf(" enter & \n");
	background = 1; //indicate background
}
| // can empty
;
%%

void
yyerror(const char * s)
{

	fprintf(stderr, "Error at line %d: %s!\n",yylineno,s);
}

int runSetEnv(char* variable, char* word)
{
	if(strcmp(variable, "PWD") == 0)
	{
		strcpy(varTable.word[0], word);
		return 1;
	}

	else if(strcmp(variable, "HOME") == 0)
	{
		strcpy(varTable.word[1], word);
		return 1;
	}

	else if(strcmp(variable, "PROMPT") == 0)
	{
		strcpy(varTable.word[2], word);
		return 1;
	}

	else if(strcmp(variable, "PATH") == 0)
	{
		strcpy(varTable.word[3], word);
		return 1;
	}

	setenv(variable, word, 1);
	var_count++;
	return 1;

}

int runPrintEnv()
{
	for(int i = 0; i < varIndex; i++)
	{
		printf("%s=", varTable.var[i]);
		printf("%s\n", varTable.word[i]);
	}

	int count = 0;
	int i = 0;
	while(environ[i])
	{
		count++;
		i++;
	}

	i = count - var_count;
	while(environ[i])
	{
	  printf("%s\n", environ[i++]);
	}

    return 1;
}

int runUnsetEnv(char *variable)
{
	if(strcmp(variable, "PWD") == 0 || strcmp(variable, "HOME") == 0 || strcmp(variable, "PROMPT") == 0 || strcmp(variable, "PATH") == 0)
	{
		fprintf(stderr, "Error: Cannot unset %s\n", variable);
		return 0;
	}

	unsetenv(variable);
	var_count--;
	return 1;
}

int runSetAlias(char *name, char *word) {
	if(strcmp(name, word) == 0)
	{
		printf("Error, expansion of \"%s\" would create a loop.\n", name);
		return 1;
	}

	Node* current = head;
	for(int i = 0; i < aliasSize; i++)
	{
		if((strcmp(current -> name, word) == 0 && strcmp(current -> word, name) == 0) || strcmp(current -> name, name) == 0)
		{
			printf("Error, expansion of \"%s\" would create a loop.\n", name);
			return 1;
		}
		current = current -> next;
	}

	if(aliasSize == 0) //if there are no aliases in the list
	{
		//create list with root pointing at beginning of list
		struct Node* root = (struct Node*)malloc(sizeof(struct Node));
		root -> name = name;
		root -> word = word;
		root -> next = NULL;
		head = root;
		//map.insert<name, word>;
	}

	else //else if there exists an alias already
	{
		//copy contents of word string to newWord string
		char* newWord = (char*)malloc((strlen(word)+1)*sizeof(char));
		strcpy(newWord, word);

		//create new node in list
		Node* node = head;
		while(node != NULL) //while not at end of list
		{
			if(node -> name == newWord) //check if word exists as an alias of a different word
			{
				newWord = node -> word; //prevents infinite-loop alias expansion
				break;
			}
			node = node -> next;
		}

		Node* current = head;

		if(current -> name == name)
		{
			current -> word = newWord;
			return 1;
		}

		while(current -> next != NULL)
		{
			if(current -> name == name)
			{
				current -> word = newWord;
				return 1;
			}
			current = current -> next;
		}

		struct Node* newNode = (struct Node*)malloc(sizeof(struct Node));
		newNode -> name = name;
		newNode -> word = newWord;
		newNode -> next = NULL;
		current -> next = newNode;
	}
	aliasSize += 1; //adjust size of alias list
	return 1;
}

int runListAlias(void) {
	if(aliasSize == 0)
	{
		fprintf(stderr, "Error: No existing aliases\n");
	}

	Node* current = head;

	for(int i = 0; i < aliasSize; i++)
	{
		//for(auto const& it: aliases)
		//{
		printf("%s=%s\n", current -> name, current -> word);
		//}
		current = current -> next;
	}
	return 1;
}

int runRemoveAlias(char *name)
{
	if(aliasSize == 0) //if no aliases exist
	{
		printf("Error: No alias %s found\n", name);
	}

	Node* current = head;

	if(strcmp(current -> name, name) == 0) //if alias with name found
	{
		if(current -> next != NULL)
		{
			head = current -> next;
		}

		else
		{
			head =	NULL;
		}

		free(current); //delete node
		aliasSize -= 1; //adjust size of alias list
	}

	else if(current -> next == NULL)
	{
		fprintf(stderr, "Error: Alias %s not found\n", name);
	}

	else
	{
		while(current -> next != NULL)
		{
			if(strcmp(current -> next -> name, name) == 0)
			{
				Node* del = current -> next;
				current -> next = del -> next;
				free(del);
				aliasSize -= 1;
				return 1;
			}
			current = current -> next;
		}
		fprintf(stderr, "Error: Alias %s not found\n", name);
	}
	return 1;

}

int runCDnoargs(void)
{
	int result = chdir(getenv("HOME"));
	if(result != 0)
	{
		printf("No such directory");
	}
	return 1;
}

int runCD(char* arg)
{
	if (arg[0] != '/')
	{ // arg is relative path
		strcat(varTable.word[0], "/");
		strcat(varTable.word[0], arg);

		if(chdir(varTable.word[0]) == 0) {
			return 1;
		}
		else {
			getcwd(cwd, sizeof(cwd));
			strcpy(varTable.word[0], cwd);
			printf("Directory not found\n");
			return 1;
		}
	}
	else { // arg is absolute path
		if(chdir(arg) == 0){
			strcpy(varTable.word[0], arg);
				return 1;
		}
		else {
			printf("Directory not found\n");
        return 1;
		}
	}
	return 1;
}
