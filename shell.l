
%{
#include <string.h>
#include "y.tab.h"
%}

%option yylineno
%%


\n		{return NEWLINE;}
"<"		{return LT;}
">"		{return GT;}
">>"	{return GTGT;}
">&"	{return GTAND;}
">>&"	{return GTGTAND;}
"|" 	{return PIPE;}
"2>"	{return ERRORF;}
"2>&1"	{return ERROR1;}
"&" 	{ return AND;}
[ \t]	{}	//igbnore tab, white space	


\"[^\n\"]*\" {//ignore ""  
yytext = (char*)(yytext+1); // delete quotes
yytext[ strlen(yytext) - 1 ] = '\0'; // delete last quotes

yylval.stringvalue = strdup(yytext);
yylval.stringvalue= environmentVariable(yylval.stringvalue);
escape(yylval.stringvalue);

return WORD;
}

[^ \t\n][^ \t\n]*  {// reconnize everything else token
		yylval.stringvalue = strdup(yytext);
		yylval.stringvalue= environmentVariable(yylval.stringvalue);
		yylval.stringvalue = tildeExpansion(yylval.stringvalue);
		escape(yylval.stringvalue);
		return WORD;
	}

%%


