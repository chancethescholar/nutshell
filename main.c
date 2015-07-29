#include <stdio.h>
#include <string.h>
#include "global.h"

main(){
	// Ignore signal (SIG_IGN): The signal is ignored and the code execution will continue even if not meaningful.
	// SIGINT Interrupt from keyboard
	signal(SIGINT, SIG_IGN); 
	signal(SIGINT, setSignal);
	while(1){
		prompt();
		
		if(yyparse()){
			return (ERROR);

		}else{
			return (OK);
		}
	}
	return 0;
}


//yacc use left recusion,

/********************Alias**********************/

//alias name word - adds a new alias to the shell, a shorthand form of a long command.

void alias(char* name, char* word){
	if(size==0){
		Node* newNode = (Node*) malloc (sizeof (Node));
		newNode -> aliasName = name;
		newNode -> aliasWord = word;
		newNode -> next = NULL;
		head = newNode;
	}else{
		///find nested alias
		char* newWord = NULL;
		newWord = (char *)malloc((strlen(word)+1)*sizeof(char));
		strcpy(newWord, word);
		
		Node* nest = head;
		while (nest != NULL){
			
			if (strcmp(nest -> aliasName, newWord)==0){
				newWord = nest -> aliasWord;
				break;
			}
			nest = nest -> next;
		}
		
		Node* current = head;
		
		if (strcmp(current -> aliasName, name) == 0){
			current -> aliasWord = newWord;
			return;
		}
		
		while(current->next!= NULL){
			if (strcmp(current -> aliasName, name) == 0){
				current -> aliasWord = newWord;
				return;
			}
			current = current -> next;
		}
		
		Node* newNode = (Node*) malloc (sizeof (Node));
		newNode -> aliasName = name;
		newNode -> aliasWord = newWord;
		newNode -> next = NULL;
		current -> next = newNode;
		
	}
	size++;
}

//alias - the alias command with no arguments lists all of the current aliases
void alias_print(){
	if(size==0){
		//printf("No alias in the list!");
		fprintf(stderr, "Error at line %d: No alias in the list!\n",yylineno);
	}
	
	Node *current = head;
	
	while (current!=NULL){
		printf("%s=%s\n",current -> aliasName, current -> aliasWord);
		current = current -> next;
	}
}

//unalias name - remove the alias for name from the alias list.
void unalias(char* name){
	if(size==0){
		//printf("No alias in the list!");
		fprintf(stderr, "Error at line %d: No alias in the list!\n",yylineno);
	}
	
	Node *current = head;
	
	if (strcmp(current -> aliasName, name) == 0){
		if (current -> next !=NULL){
			head = current -> next;
		}else{
			head =	NULL;
		}
		free (current);
		size--;
	}else if (current -> next == NULL){
		//printf ("Alias %s is not in the list!",name);
		fprintf(stderr, "Error at line %d: Alias %s is not in the list!\n",yylineno,name);
	}else{
		while (current->next!=NULL){
			if (strcmp(current -> next-> aliasName, name) == 0){
				Node *trash = current -> next;
				current ->next = trash -> next;
				free(trash);
				size--;
				return;
			}
			current = current -> next;
		}
		//printf ("Alias %s is not in the list!",name);
		fprintf(stderr, "Error at line %d: Alias %s is not in the list!\n",yylineno,name);
	}
}

//search alias
char* searchAlias(char* name){
	if(size==0) return name;
	
	Node *current = head;
	
	while (current!=NULL){
		if (strcmp(current -> aliasName, name) == 0){
			return current -> aliasWord;
		}
		current = current -> next;
	}
	return name;
}

/********************End Alias**********************/

void printenv(){
	char **p = environ;
	
	while (*p != NULL){
		printf("%s (%p)\n", *p, *p);
		*p++;
	}
}

void printcmdt()//print command table
{
	int j=0;
	while(comtab[j].comname != NULL)
	{
		j++;
	}
}

void run()
{
	int j=0;
	while(comtab[j].comname != NULL)
	{
		//printf("cmd is %s , args number is %d    \n", comtab[j].comname, comtab[j].nargs);

		
		j++;
	}
	int num_simple_commands=j;
	int input_fd;// initial input fd
	int i;
	int out_fd;

	
	int origin_in = dup(0);
	int origin_out = dup(1);
	int origin_error = dup(2);
	//save original in out error
	
	if (infile_name) { 	// open file 
		input_fd = open(infile_name, O_RDONLY); // read only
	} else { 			// if there is not file use default input
		input_fd = dup(origin_in);
	}
		
	pid_t child;
	for ( i = 0; i < num_simple_commands; i++ ) { // loop from first command to last
		
		dup2(input_fd, 0);//redirect input to stdin
		close(input_fd);//close input_fd
			
		if (i == num_simple_commands - 1) { //this is last commmand

			if (out_file_name) { //open file

				//     #define S_IRWXU 0000700     /* -rwx------ */
				//      #define S_IREAD 0000400     /* read permission, owner */
				//      #define S_IRUSR S_IREAD
				//      #define S_IWRITE 0000200    /* write permission, owner */
				//      #define S_IWUSR S_IWRITE
				//      #define S_IEXEC 0000100     /* execute/search permission, owner */
				//      #define S_IXUSR S_IEXEC
				//      #define S_IRWXG 0000070     /* ----rwx--- */
				//      #define S_IRGRP 0000040     /* read permission, group */
				//      #define S_IWGRP 0000020     /* write    "         "   */
				//      #define S_IXGRP 0000010     /* execute/search "   "   */
				//      #define S_IRWXO 0000007     /* -------rwx */
				//      #define S_IROTH 0000004     /* read permission, other */
				//      #define S_IWOTH 0000002     /* write    "         "   */
				//      #define S_IXOTH 0000001     /* execute/search "   "   */

				mode_t open_mode = S_IRUSR | S_IROTH| S_IWUSR | S_IRGRP ; //io
				out_fd = open(out_file_name, open_permission, open_mode);
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
		
		if (err_file_name) {  

			// 0777 permissions rwxrwxrwx
			if((out_fd = open(err_file_name, O_CREAT|O_TRUNC|O_WRONLY, 0777))!= -1)//this is  2>
			{
				dup2( out_fd, 2); 
			}
			else
			dup2( STDOUT_FILENO, STDERR_FILENO); //this is 2>&1		
		}
				
		close(out_fd); //close out fd
				
		//check for builtin commands
		
		///////////////search alias table////////////////////
		if(strcmp(comtab[i].comname,searchAlias(comtab[i].comname))!=0){
			char* token;
			char* temp;
			temp = (char *)malloc((strlen(searchAlias(comtab[i].comname))+1)*sizeof(char));
			strcpy(temp,searchAlias(comtab[i].comname));
			temp= tildeExpansion(temp);	

			token = strtok(temp," ");

			//walk through other tokens	
			comtab[i].nargs = 0;
			while(token!=NULL){
				comtab[i].args[comtab[i].nargs]=token;
				comtab[i].nargs++;
				token = strtok(NULL," ");
			}
			comtab[i].comname = comtab[i].args[0];
		}
		
		////////////////////////////////////	
		
		if ( strcmp(comtab[i].comname, "bye")==0){ // exit
			exit(0);
		} else if ( strcmp(comtab[i].comname, "cd")==0 ){
			
			int ret;
			//printf("nargs is %d    \n", comtab[i].nargs);
			
			if (comtab[i].nargs >= 1 ){
				ret = chdir(comtab[i].args[1] );
			}else{
				ret = chdir( getenv("HOME") );
			}
			
			if (ret != 0) {
				fprintf(stderr, "Error at line %d: No such file or directory!\n",yylineno);
			}		
			continue;
			
		}else if (strcmp(comtab[i].comname, "setenv") == 0){
			if (comtab[i].args[1] !=NULL && comtab[i].args[2]!=NULL && comtab[i].args[3]==NULL){ 
				setenv(comtab[i].args[1],comtab[i].args[2],1);
			}else{
				fprintf(stderr, "Error at line %d: Invalid command!\n",yylineno);
			}
			continue;
		}else if (strcmp(comtab[i].comname, "printenv")==0 ){ 
			if (comtab[i].args[1] ==NULL){
				printenv();
			}else{
				fprintf(stderr, "Error at line %d: Invalid command!\n",yylineno);
			}
			continue;
			
		}else if (strcmp(comtab[i].comname, "unsetenv") == 0){ 
			if (comtab[i].args[1] !=NULL && comtab[i].args[2]==NULL){
				unsetenv(comtab[i].args[1]);
			}else{
				fprintf(stderr, "Error at line %d: Invalid command!\n",yylineno);
			}
			continue;
		}else if (strcmp(comtab[i].comname, "alias") == 0){
			if (comtab[i].args[1] !=NULL && comtab[i].args[2]!=NULL && comtab[i].args[3]==NULL){
				alias(comtab[i].args[1], comtab[i].args[2]);
			}else if (comtab[i].args[1] ==NULL){
				alias_print();
			}else{
				fprintf(stderr, "Error at line %d: Invalid command!\n",yylineno);
			}
			continue;
		}else if (strcmp(comtab[i].comname, "unalias") == 0){
			if (comtab[i].args[1]!=NULL && comtab[i].args[2]==NULL){
				unalias(comtab[i].args[1]);
			}else{
				fprintf(stderr, "Error at line %d: Invalid command!\n",yylineno);
				//printf("Invalid command!");
			}
			continue;
		}else{ // else we sort the arguments and fork!			
			child = fork(); //after check build in now we can fork
		}
		
		if (child == 0) { //this is child 
			execvp(comtab[i].comname,comtab[i].args);// execute non build in
			
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
	for (z =0;z<num_simple_commands;z++)
	{
		
		for(x =0;x<MAXARGS;x++)
		{
			comtab[z].args[x]=NULL;
		}
		comtab[z].comname=NULL;
		comtab[z].nargs=0;
		
	}
	currcmd= 0;
	infile_name=NULL;
	out_file_name=NULL;
	err_file_name=NULL;
	open_permission =0;
	background = 0;
	
	prompt();
}

int contain_char(char* string, char character)
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

char* combine_string(char* string1, char* string2)
{
	char* newString = (char *)malloc((strlen(string1)+strlen(string2)+1)*sizeof(char));
	strcpy(newString, string1);
	strcat(newString, string2);
	return newString;
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

char* environmentVariable(char* string) {	
	int length = strlen(string)+1;
	char* newString = (char *)malloc((length)*sizeof(char));

	char* value = (char *)malloc((length)*sizeof(char));

	int stringPtr =0;
	int i = 0;

	while(i!=length-1){
		if(string[i] == '$' && string[i+1] == '{'){
			char* variable = (char *)malloc((length)*sizeof(char));
			int variablePtr = 0;
			i = i +2;

			while (i <= length && string[i]!= '}'){
				variable[variablePtr] = string[i];
				variablePtr++;
				i++;
			}

			variable[variablePtr] = '\0';

			//search for variable value
			if(getenv(variable)!=NULL){
				value = getenv(variable);
			}else{
				strcat(value, "${");
				strcat(value, variable);
				strcat(value, "}");
			}

			int j;

			for (j = 0; j < strlen(value); ++j) {
				newString[stringPtr] = value[j];
				stringPtr++;
			}

			variablePtr = 0;

		}else{
			newString[stringPtr] = string[i];
			stringPtr++;
		}
		i++;
	}

	newString[stringPtr] = '\0';

	return newString;
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


void prompt(){
	// The fileno() function returns the integer file descriptor associated with the stream pointed to by stream.
	// The isatty() function tests whether fd is an open file descriptor referring to a terminal.
	// isatty() returns 1 if fd is an open file descriptor referring to a terminal; otherwise 0 is returned
	if (isatty(fileno(stdin))) {
		printf("shell>");
		fflush(stdout);  //flush a stream
	}		
}

void setSignal(){
	printf("\n");
	prompt();
	fflush(stdout);
}

