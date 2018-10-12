# Assignment 2: Mini Shell 
Runs a mini shell 'minish'.

## Build and Run Mini Shell:
To build the mini shell, run:
```
make
```

To start the mini shell, run:
```
./minish
```
minish must be started before running any commands.

## Mini Shell Commands
Run an executable:
```
./minish <command> <arg0> <arg1> ... <argn>
```

Run a command in background mode
```
./minish <command> <arg0> <arg1> ... <argn> &
```

Redirect standard input:
```
./minish <command> < <input file>
```

Redirect standard output:
```
./minish <command> > <output file>
```

Run multiple commands with a pipe:
```
./minish <command1> <args1> | <command2> <args2> | ... | <commandn> <argsn>
```

Quit foreground process with `Control + C`

Kill a background process whose pid is p:
```
./minish kill p
```

Exit minish
```
./minish exit
```
