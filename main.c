#include <stdio.h>
#include <string.h>
#include "global.h"

int yyparse();
char *getcwd(char *buf, size_t size);

int main()
{
		char* inFileName = NULL;  //in file description
		char* outFileName = NULL;	//out file description
		char* errFileName = NULL; //error file  description
		int openPermission = 0; //open option
		int background = 0; //check & for command in background
		int currentCommand = 0; // current cmd index

    aliasIndex = 0;
    varIndex = 0;

    getcwd(cwd, sizeof(cwd));

    strcpy(varTable.var[varIndex], "PWD");
    strcpy(varTable.word[varIndex], cwd);
    varIndex++;
    strcpy(varTable.var[varIndex], "HOME");
    strcpy(varTable.word[varIndex], cwd);
    varIndex++;
    strcpy(varTable.var[varIndex], "PROMPT");
    strcpy(varTable.word[varIndex], "shell");
    varIndex++;
    strcpy(varTable.var[varIndex], "PATH");
    strcpy(varTable.word[varIndex], ".:/bin");
    varIndex++;

    system("clear");
    while(1)
    {
        printf("[%s]>> ", varTable.word[2]);
        yyparse();
        argc = 0;
    }

    return 0;
}

void execute()
{
	int j=0;
	while(commandTable[j].comName != NULL)
	{
		//printf("cmd is %s , args number is %d    \n", commandTable[j].comName, commandTable[j].numArgs);


		j++;
	}
	int numBuiltinCommands=j;
	int input_fd;// initial input fd
	int i;
	int out_fd;


	int origin_in = dup(0);
	int origin_out = dup(1);
	int origin_error = dup(2);
	//save original in out error

	if (inFileName) { 	// open file
		input_fd = open(inFileName, O_RDONLY); // read only
	} else { 			// if there is not file use default input
		input_fd = dup(origin_in);
	}

	pid_t child;
	for ( i = 0; i < numBuiltinCommands; i++ ) { // loop from first command to last

		dup2(input_fd, 0);//redirect input to stdin
		close(input_fd);//close input_fd

		if (i == numBuiltinCommands - 1) { //this is last commmand

			if(outFileName) { //open file

				mode_t open_mode = S_IRUSR | S_IROTH| S_IWUSR | S_IRGRP ; //io
				out_fd = open(outFileName, openPermission, open_mode);
			}

			else {
				out_fd = dup(origin_out);// use original output
			}

		}

		// this is not last command, so inital pipeline
		else {
			int pipe_fd[2];
			pipe(pipe_fd);
			out_fd = pipe_fd[1];
			input_fd = pipe_fd[0];
		}

		dup2(out_fd, 1);  // redirect output

		if(errFileName) {

			// 0777 permissions rwxrwxrwx
			if((out_fd = open(errFileName, O_CREAT|O_TRUNC|O_WRONLY, 0777))!= -1)//this is  2>
			{
				dup2(out_fd, 2);
			}
			else
			dup2( STDOUT_FILENO, STDERR_FILENO); //this is 2>&1
		}

		close(out_fd); //close out fd

		//check for builtin commands

		///////////////search alias table////////////////////
		if(strcmp(commandTable[i].comName,subAliases(commandTable[i].comName))!=0){
			char* token;
			char* temp;
			temp = (char *)malloc((strlen(subAliases(commandTable[i].comName))+1)*sizeof(char));
			strcpy(temp,subAliases(commandTable[i].comName));
			temp= tildeExpansion(temp);

			token = strtok(temp," ");

			//walk through other tokens
			commandTable[i].numArgs = 0;
			while(token!=NULL){
				commandTable[i].args[commandTable[i].numArgs]=token;
				commandTable[i].numArgs++;
				token = strtok(NULL," ");
			}
			commandTable[i].comName = commandTable[i].args[0];
		}

		if ( strcmp(commandTable[i].comName, "bye")==0){ // exit
			exit(0);
		} else if ( strcmp(commandTable[i].comName, "cd")==0 ){

			int ret;
			//printf("numArgs is %d    \n", commandTable[i].numArgs);

			if (commandTable[i].numArgs >= 1 ){
				ret = runCD(commandTable[i].args[1] );
			}else{
				ret = runCDnoargs();
			}

			if (ret != 0) {
				fprintf(stderr, "Error at line %d: No such file or directory!\n",yylineno);
			}
			continue;

		}else if (strcmp(commandTable[i].comName, "setenv") == 0){
			if (commandTable[i].args[1] !=NULL && commandTable[i].args[2]!=NULL && commandTable[i].args[3]==NULL){
				runSetEnv(commandTable[i].args[1],commandTable[i].args[2]);
			}else{
				fprintf(stderr, "Error at line %d: Invalid command!\n",yylineno);
			}
			continue;
		}else if (strcmp(commandTable[i].comName, "printenv")==0 ){
			if (commandTable[i].args[1] ==NULL){
				runPrintEnv();
			}else{
				fprintf(stderr, "Error at line %d: Invalid command!\n",yylineno);
			}
			continue;

		}else if (strcmp(commandTable[i].comName, "unsetenv") == 0){
			if (commandTable[i].args[1] !=NULL && commandTable[i].args[2]==NULL){
				runUnsetEnv(commandTable[i].args[1]);
			}else{
				fprintf(stderr, "Error at line %d: Invalid command!\n",yylineno);
			}
			continue;
		}else if (strcmp(commandTable[i].comName, "alias") == 0){
			if (commandTable[i].args[1] !=NULL && commandTable[i].args[2]!=NULL && commandTable[i].args[3]==NULL){
				runSetAlias(commandTable[i].args[1], commandTable[i].args[2]);
			}else if (commandTable[i].args[1] ==NULL){
				runListAlias();
			}else{
				fprintf(stderr, "Error at line %d: Invalid command!\n",yylineno);
			}
			continue;
		}else if (strcmp(commandTable[i].comName, "unalias") == 0){
			if (commandTable[i].args[1]!=NULL && commandTable[i].args[2]==NULL){
				runRemoveAlias(commandTable[i].args[1]);
			}else{
				fprintf(stderr, "Error at line %d: Invalid command!\n",yylineno);
				//printf("Invalid command!");
			}
			continue;
		}else{ // else we sort the arguments and fork!
			child = fork(); //after check build in now we can fork
		}

		if (child == 0) { //this is child
			execvp(commandTable[i].comName,commandTable[i].args);// execute non build in

			perror("execvp");
			_exit(1);
		}
		else if (child < 0) {
			//fprintf(stderr, "child fork fail \n");
			fprintf(stderr, "Error at line %d: child fork fail!\n",yylineno);
			_exit(1);
		}

		if (!background) {
			waitpid(child, NULL, 0);
		}
	}
	//loop all of those simple command

	dup2(origin_in, 0); //original in out error go to default
	dup2(origin_out, 1);
	dup2(origin_error, 2);
	close(origin_in);
	close(origin_out);
	close(origin_error);

	//clear command table to empty
	int z;
	int x;
	for (z =0;z<numBuiltinCommands;z++)
	{

		for(x =0;x<500;x++)
		{
			commandTable[z].args[x]=NULL;
		}
		commandTable[z].comName=NULL;
		commandTable[z].numArgs=0;

	}

	reset();
}

int containChar(char* string, char character)
{
	int i = 0;
	while (i != strlen(string)){
		if (string[i] == character){
			return 1;
		}
		i++;
	}
	return 0;
}

void escape(char* string){
	char* origin = string;
	char* final = string;
	while (*origin) {
		*final = *origin++;
		final += (*final != '\\' || *(final + 1) == '\\');
	}
	*final = '\0';
}

void reset(){
	currentCommand = 0;
	inFileName = NULL;
	outFileName = NULL;
	errFileName = NULL;
	openPermission = 0;
	background = 0;
	//fileno() function returns the integer file descriptor associated with the stream pointed to by stream
	//isatty() function checks if fd is an open file descriptor in command line,
	//1 if fd is an open file descriptor or 0 if not
	if(isatty(fileno(stdin)))
	{
		printf("[%s]>> ", varTable.word[2]);
		fflush(stdout);
		argc = 0;
	}
}

char* subAliases(char* name)
{
  if(aliasSize == 0)
    return name;

  Node* current = head;

  for(int i = 0; i < aliasSize; i++)
  {
    if(strcmp(current -> name, name) == 0)
    {
      return current -> word;
    }
    current = current -> next;
  }

  return name;
}

bool ifAlias(char* name)
{
  Node* current = head;
  for(int i = 0; i < aliasSize; i++)
  {
    if(strcmp(current -> name, name) == 0)
    {
        return true;
    }
    current = current -> next;
  }
  return false;
}

char* getPath(char* command)
{
	if(strcmp(command, "wc") == 0)
		return "/usr/bin/wc";
	else if(strcmp(command, "grep") == 0)
		return "/usr/bin/grep";
	else if(strcmp(command, "ls") == 0)
		return "/bin/ls";
	else if(strcmp(command, "rm") == 0)
		return "/bin/rm";
	else if(strcmp(command, "cp") == 0)
		return "/bin/cp";
	else if(strcmp(command, "cat") == 0)
		return "/bin/cat";
	else if(strcmp(command, "mkdir") == 0)
		return "/bin/mkdir";
	else if(strcmp(command, "rmdir") == 0)
		return "/bin/rmdir";
	else if(strcmp(command, "mv") == 0)
		return "/usr/bin/mv";
	else if(strcmp(command, "head") == 0)
		return "/usr/bin/head";
	else if(strcmp(command, "awk") == 0)
		return "/usr/bin/awk";
	else if(strcmp(command, "sort") == 0)
		return "/usr/bin/sort";
	else if(strcmp(command, "ssh") == 0)
		return "/usr/bin/ssh";
	else if(strcmp(command, "date") == 0)
		return "/bin/date";
	else if(strcmp(command, "ping") == 0)
		return "/sbin/ping";
	else if(strcmp(command, "tty") == 0)
		return "/usr/bin/tty";
	else if(strcmp(command, "rev") == 0)
		return "/usr/bin/rev";
	else if(strcmp(command, "echo") == 0)
		return "/bin/echo";
	else if(strcmp(command, "touch") == 0)
		return "/bin/touch";
	else if(strcmp(command, "pwd") == 0)
		return "/bin/pwd";
	else if(strcmp(command, "man") == 0)
		return "usr/bin/man";
}

char* tildeExpansion(char* string) {
	int length = strlen(string)+1;
	char* newString = (char *)malloc((length)*sizeof(char));
	char* value = (char *)malloc((length)*sizeof(char));
	int stringPtr =0;
	int i = 0;

	for (i =0; i< length; ++i){
		//	while(i!=length-1){
		if(string[i] == '~'){
			if (string[i+1] && string[i+1] != '/'){
				char* variable = (char *)malloc((length)*sizeof(char));
				int variablePtr = 0;
				i = i +1;

				while (i <= length && string[i]!='/'&& string[i]!='\0'){
					variable[variablePtr] = string[i];
					variablePtr++;
					i++;
				}

				variable[variablePtr] = '\0';

				//search for variable value
				struct passwd *pw;
				if((pw = getpwnam(variable))!='\0'){
					value = pw->pw_dir;


				}else{
					strcat(value, "~");
					strcat(value, variable);
				}

				variablePtr = 0;

			}else{
				value = getenv("HOME");
			}

			strcat(value, "/");

			int j;

			for (j = 0; j < strlen(value); ++j) {
				newString[stringPtr] = value[j];
				stringPtr++;
			}

		}else{
			newString[stringPtr] = string[i];
			stringPtr++;
		}
	}

	newString[stringPtr] = '\0';

	return newString;
}
