# LC-3 Virtual Machine

After taking a computer organization course in college, I was inspired to create a program that emulates the  [LC-3 Computer](https://en.wikipedia.org/wiki/Little_Computer_3).

The [C program](./lc3.c) was written for a UNIX environment.

## Compile

```
gcc -o lc3 lc3.c -Wall
```

## Execute

The program expects the path to an LC3 object file.

```
./lc3 [/path/to/file.obj]
```

The program will run the executable as an LC3 program. Sample LC3 programs can be found [here](./lc3-sample-code), and their respective executables can be found [here](./lc3-sample-obj). 
Any additional programs must be assembled separately.
