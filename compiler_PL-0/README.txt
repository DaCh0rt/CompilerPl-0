/*-------How to Run----------*/
-compile with: gcc hw4compiler.c
-run with: ./a.out [-l] [-a] [-v] [-d] [-s] <filename.txt>
	-[] denotes optional
	-<> denotes placeholders
	-l is the flag to turn on printing the lexeme list to screen.
	-a is the flag to turn on printing the generated assembly code.
	-v is the flag to turn on printing the VM execution trace.
	-d is my added flag to turn on extra debug information.
	-s is my added flag to turn on printing the source file.
-note: expects filename to always be the last arguement.
	order of flags does not matter but I did not know how to
	express that somewhat formally.