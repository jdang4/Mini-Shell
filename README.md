# Shell-Simulator

This program basically simulates how the shell works. It is able to run most of the standard system calls as well as putting the processes into the background when requested by user (when the command is followed by an "&"). Now although this simulates how a regualr Linux shell works, it is not an exact replica of it. It does not have as many error checks as the actual one and there are some commands (grep, |, <<) that are not recognized.

How to Run:

1.) You can run the program with adding arguments into doit which will only execute the command given.
    "./doit cd dir"
    
2.) You can run the shell simulation by including no arguments into the doit program which will start up the simulation. You can exit the simulation by typing in "exit", which will exit out of the shell after it waits for all the background processes have finished.
    "./doit"
