# MavShell (msh)
A Bash-like shell implemented in C using `fork` and `exec`.

Submitted as Assignment-1 for CSE 3320 (Operating Systems - Fall 2022) at the University of Texas at Arlington.

To compile: ```gcc msh.c -o msh```

To run: ```./msh```

Supported commands:
| Command | Description |
| --- | ----------- |
| ```cd``` | Changes the current working directory |
| ```history``` | Displays the upto the last 15 commands entered |
| ```!n``` where `n` is an integer | Reruns the `n`-th command in history |
| ```listpids``` | Lists out the Process IDs (PIDs) of upto the last 20 processes forked by the shell |
| ```exit``` | Kills the MavShell process |

Additionally, MavShell can run all the usual programs residing in ```/usr/bin/``` and other directories that `PATH` points to (such as `ls` and `ps` among many others).
