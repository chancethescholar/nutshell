# shell
Shell Project version 1.0 04/12/2015 README

COMPILE:
“make”, and “./shell”

MISSING FEATURES
- File Name Completion

FEATURES
- Shell Types(word, white space, metacharacters (<>|”\&))
- Built-in Commands (setenv, printenv, unsetenv, cd, alias, unalias, bye)
- Environment Variable Expansion ${variable} 
- Wildcard Matching (*,?)
- Tilde Expansion
- Pipe
- I/O Redirection (‘<‘, ‘>’, ‘>>’)
- Execute Command in The Background (if & exists at the end of the line)
- Standard error redirection

Files:
- global.h
- main.c
- shell.y
- shell.l
- makefile
