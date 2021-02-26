# Mini-shell
Mini simulation of a linux shell.

User guides:

1. Compile source program minishell.cpp to produce an executable file minishell. 

> g++ minishell.cpp -lreadline -o minishell

2. Run the executable file.
> ./minishell

COMMANDS: bg, fg, list, Ctrl+Z, Ctrl+C, exit.

sh >bg [name of executable file] [a list of arguments] Action: shell runs the executable file with a list of arguments at the background and continues to accept input from the user.  
 
Example: shell runs the executable file test with a list of [arguments]: running 2 5 at the background and continues to accept input from the user.   
sh >bg ./test running 2 5 
sh > 
 
-----------
sh >fg [name of executable file] [a list of arguments] Action: shell runs the executable file with a list of arguments at the foreground and does not accept input from the user until foreground process finishes.  
 
Example: shell runs the executable file test with a list of [arguments]: running 2 2 at the foreground and does not accept input from the user until foreground process finishes.   
sh >fg ./test running 2 5 
running
running
Process (pid) completed
sh >

-----------
sh >list  
Action: Display the  PID, state, path, and arguments of ALL non-terminated processes.  
 
Example: sh >list 
sh >bg ./test test1  2 5
sh >bg ./test test2  5 10
sh >list
4096:  running  ./test test1  2 5
4097:  running  ./test test2  5 10
sh >fg. /test test3  2 5
Process  4098  terminated
sh >fg. /test test4  5 10
Process  4099  stopped
sh > list
4096:  running  ./test test1  2 5
4097:  running  ./test test2  5 10
4099:  stopped  ./test test4  5 10
sh > 
 
-----------
sh >Ctrl+Z 
Action: Stop the running foreground process and display a message. 
Example: sh >Ctrl+Z 
sh >Process  4096  stopped

------------
sh >Ctrl+C 
Action: Terminate the running foreground process and display a message.
Example: sh >Ctrl+C 
sh >Process  4096  terminated
 
-------------
sh >exit 
Action: shell terminates all processes(with corresponding messages), if any, and exits.   
Example: sh >exit 
Process  4096  terminated
Process  4097  terminated
$

 
-------------- 
 Shell should display a message after a process has completed.
 Example: 
 Process 16529 completed 


