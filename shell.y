%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "main.c"

%}


%token	<stringvalue> WORD 

%token 	NOTOKEN NEWLINE GT LT PIPE ERRORF ERROR1 AND GTGT GTGTAND GTAND  TERMINATOR

%union	{
	char   *stringvalue;
}

%%



complete_command:	
commands
;

commands: 
command
| commands command 
;

command: 
pipeline io_redirection   background NEWLINE {
	//printf(" execute \n");
	//printcmdt();
	run(); // execute  complete command in command table
	
	
}
| NEWLINE { 
	
}
| error NEWLINE { yyerrok; }
;

pipeline:
pipeline PIPE {
	currcmd++; //another new simple command create by increase command table array index
	
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
WORD { // printf(" argument %s ", $1);

	if (contain_char($1, '?') || contain_char($1, '*') )// whildcard reconize from tokens
	{
		glob_t pattern;
		if (glob($1, 0, NULL, &pattern) == 0)// find match pattern
		{
			int num;
			num = pattern.gl_pathc; //number of matching pattern

			int i;

			
			for (i = 0; i < pattern.gl_pathc; i++)// for loop to add each matching pattern to arguments in same simple command
			{	
				comtab[currcmd].nargs++; 
				comtab[currcmd].args[comtab[currcmd].nargs]=strdup(pattern.gl_pathv[i]);
			}
		}
	}
	else{
		//cannot find any match pattern just add *?pattern as argument
		comtab[currcmd].nargs++;
		comtab[currcmd].args[comtab[currcmd].nargs]=$1;
	}
}
;

command_word:
WORD {//printf("command is \n", $1);
	//every command star with cmd XXXXX here we inital command element
	comtab[currcmd].comname=$1;
	comtab[currcmd].args[0]=$1;
	comtab[currcmd].nargs = 0;

}
;

io_redirection:
io_redirection iodirect
| // can emputy
;

iodirect:
GTGT WORD {
	open_permission = O_WRONLY | O_CREAT | O_APPEND ;// APPEND mode
	
	out_file_name=$2;
}
| GT WORD {	
	open_permission = O_WRONLY  | O_TRUNC| O_CREAT; //insert is creat mode
	out_file_name=$2;
}
| GTGTAND WORD {
	open_permission = O_WRONLY  | O_CREAT| O_APPEND;
	out_file_name = $2;
	err_file_name = $2;	
}
| GTAND WORD {/*printf("    > %s \n", $2);*/
	open_permission = O_WRONLY  | O_TRUNC| O_CREAT;
	//out_file_name = $2;
	err_file_name = $2;
}
|
ERRORF WORD {/*printf("    2> %s \n", $2);*/
	open_permission = O_WRONLY | O_TRUNC| O_CREAT ;
	err_file_name=$2;
}
|
ERROR1 {/*printf("    2>&1 %s \n", $2);*/
	err_file_name="error";
}
| LT WORD {/*printf("    < %s \n", $2);*/
	infile_name=$2;	
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

int yywrap()
{	
	return 1;
}


