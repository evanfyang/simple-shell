AUTHOR: Evan Yang
EMAIL: evan.yang@uky.edu
DATE: 4/15/2019
COURSE: CS 270-002
Project: Program 4 - A Simple Shell

CONTENTS:

README.txt: this file.

Makefile: makefile for building this program.

shell.c: implementation of a simple shell that supports executing external
commands, performing simple redirections, and evaluating a few built-in commands.

shell.h: header file that provides the definition of the command structure, and
prototypes for the provided functions parse_command and free_command.

parser.c: implementation of parse_command and free_command.

/tests: a subdirectory containing test inputs, expected outputs, and helper commands.

run-tests: a shell script to run tests and compare the shell's output with the
expected output.

RUNNING:

In the terminal, change the working directory to the one containing all of the
files listed under CONTENTS. Next, type the following command to build the
program:

    make

To run the shell program, type the following into the terminal:

    ./shell

To run tests on the shell program, type the following into the terminal:

    ./run-tests

IMPLEMENTATION NOTES:

The following functions from shell.c were used for executing commands:

    void changeDirectory(struct command *cmd)
    void setEnvironment(struct command *cmd)
    void callExternal(struct command *cmd)

Implementing the built-in commands in functions and writing a function to
handle external commands grants more modularity for the source code as well
as better readability for the main routine. Since the exit command only requires
one line to implement, there was no need to write a function for it. Note that
all functions pass a pointer to a command structure from the main routine after
the command line was parsed by parse_command.

fgets() was used to read the command line from either stdin or from a specified
stream (infile). fgets() was chosen to read the command line since it will stop
reading when a newline character is read or when MAX_INPUT - 1 characters are
read, whichever comes first. This protects the shell program from buffer
overflows. After call to fgets(), it is checked for errors that may have
occurred or if end-of-file was reached. If either of those occurs, the program
exits normally with a nonzero status or with status 0, respectively.

In the main routine, the built-in commands were distinguished from external
commands with an if - else if - else conditional statement that checks if the
first element of the command structure pointer array is NULL, matches one of
the built-in commands, or does not match any of the built in commands (external).
The comparison to check for built-in/external commands uses strcmp() since a
comparison of the first command argument and a bare literal string will not work.
NULL cannot be passed to strcmp(), so to check for empty argument, a regular
comparison of the first command argument with NULL is implemented.

A helper function was defined to print the shell prompt if the input file is
from stdin:

    void printPrompt(FILE *infile)

The function checks the input file to see if it is stdin. If it is indeed stdin,
then print the shell prompt. Otherwise, does not print the shell prompt. The
variable storing the shell prompt is defined as a global constant to make it
immutable and easy to change if desired.

LIMITATIONS:

There is a maximum length that is imposed on input lines. This maximum length is
defined with a length of 512 (MAX_INPUT). This limitation is imposed on
input lines to avoid buffer overflow attacks.

The provided parse_command function in parser.c  supports a maximum of 30
arguments. Commands longer than 30 arguments cannot be used.

There were no failing test cases, memory leaks, or other memory errors that were
detected during the testing process. Further testing outside of provided test
cases did not detect any errors.

REFERENCES:

The following websites were used as references for various parts of the project:

    https://www.geeksforgeeks.org/command-line-arguments-in-c-cpp/
    https://www.cs.utah.edu/~germain/PPS/Topics/C_Language/file_IO.html
    https://stackoverflow.com/questions/8058704/how-to-detect-empty-string-from-fgets
    https://www.geeksforgeeks.org/eof-and-feof-in-c/
    https://www.geeksforgeeks.org/strcmp-in-c-cpp/
    https://www.gnu.org/software/libc/manual/html_node/Basic-Signal-Handling.html

The implementation of the main routine was partially based on code
written by Dr. Neil Moore, who showed an example of how to handle command line
arguments when initially calling the shell program (lines 43- 61 in shell.c).
Credit also goes to Dr. Moore for explaining how and when to print the shell
prompt and how to read and loop over the commands given in stdin or from an
input file (lines 67, 68, and 112). In addition, the files shell.h, parser.c and
Makefile were all provided by Dr. Moore for this project.

Implementation of the callExternal() (lines 207 to 326 in shell.c) function was
based on the call_redirected() function that I wrote from Lab 2 of this course
(from file 'CS270Lab2.cpp'). The call_redirected() was repurposed to include
support for both input and output redirection, disallow keyboard interrupt
signals to kill the shell when killing a command, and to use execvp() and
waitpid() as opposed to using execve() and wait(), among many other changes.
