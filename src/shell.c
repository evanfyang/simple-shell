/* File: shell.c
 * Course: CS 270-002
 * Project: Program 4 - A Simple Shell
 * Purpose: Implementation of a simple shell that supports executing external
 * commands, performing simple redirections, and evaluating a few built-in commands.
 */

/* References:
 * https://www.geeksforgeeks.org/command-line-arguments-in-c-cpp/
 * https://www.cs.utah.edu/~germain/PPS/Topics/C_Language/file_IO.html
 * https://stackoverflow.com/questions/8058704/how-to-detect-empty-string-from-fgets
 * https://www.geeksforgeeks.org/eof-and-feof-in-c/
 * https://www.geeksforgeeks.org/strcmp-in-c-cpp/
 * https://www.gnu.org/software/libc/manual/html_node/Basic-Signal-Handling.html
 */

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "shell.h"

// Define constants and globals.
#define MAX_INPUT 512
char *prompt = "shell> ";

// Declare function prototypes.
void printPrompt(FILE *infile);
void changeDirectory(struct command *cmd);
void setEnvironment(struct command *cmd);
void callExternal(struct command *cmd);

int main(int argc, char **argv)
{
    // Define command buffer.
    char cmdline[MAX_INPUT];
    // Pointer to input file.
    FILE *infile;

    // If no file name argument, set file input to stdin.
    if (argc < 2)
    {
        infile = stdin;
    }
    // Otherwise, open the file given by the command argument for reading.
    else
    {
        infile = fopen(argv[1], "r");
        // If an error occured on fopen(), print error message and exit.
        if (infile == NULL)
        {
            perror("Error executing fopen()");
            exit(1);
        }
    }

    // Prints prompt if read from stdin and loop through commands from stdin
    // or from specified infile.
    // Loop reads a line from infile and stores it into cmdline. If the returned
    // cmdline is not NULL, process it. 
    printPrompt(infile);
    while (fgets(cmdline, MAX_INPUT, infile) != NULL)
    {
        // If error occured on call to fgets(), print error message and exit.
        if (ferror(infile))
        {
            perror("Error executing fgets()");
            exit(2);
        }
        // If end of file, exit shell.
        if (feof(infile))
        {
            exit(0);
        }
        // Parse command and assign to command struct pointer.
        struct command *currentCmd = parse_command(cmdline);

        // If empty command, do nothing.
        if (currentCmd -> args[0] == NULL)
        {
            /* NOP */;
        }
        // If command is cd, call changeDirectory().
        else if (strcmp(currentCmd -> args[0], "cd") == 0)
        {
            changeDirectory(currentCmd);
        }
        // If command is setenv, call setEnvironment().
        else if (strcmp(currentCmd -> args[0], "setenv") == 0)
        {
            setEnvironment(currentCmd);
        }
        // If command is exit, exit the program with status 0.
        else if (strcmp(currentCmd -> args[0], "exit") == 0)
        {
            exit(0);
        }
        // If not a built-in command, call external function.
        else
        {
            callExternal(currentCmd);
        }
        // Frees all data belonging to the pointer returned by parse_command.
        free_command(currentCmd);
        // Prints prompt if reading from stdin.
        printPrompt(infile);
    }

    return 0;
}

// Function that prints shell prompt.
void printPrompt(FILE *infile)
{
    // If reading from stdin, print prompt.
    if (infile == stdin)
    {
        fprintf(stderr, "%s", prompt);
        fflush(stderr);
    }
}

// Function that implements built-in command cd.
void changeDirectory(struct command *cmd)
{
    // Initialize variables.
    int chdirRetVal;
    char *home;
    // If cd called with no arguments, change to home directory.
    if (cmd -> args[1] == NULL)
    {
        // Get HOME environment.
        home = getenv("HOME");
        // If HOME environment was not removed, call chdir().
        if (home != NULL)
        {
            // Change directory to home.
            chdirRetVal = chdir(home);
            // If an error occured on chdir(), print an error message.
            if (chdirRetVal == -1)
            {
                perror("Error executing chdir()");
            }
        }
        // Otherwise, print error message.
        else
        {
            perror("Error executing getenv()");
        }
    }
    // cd called with arguments.
    else
    {
        // Change directory to specified directory from command structure args.
        chdirRetVal = chdir(cmd -> args[1]);
        // If an error occured on chdir(), print an error message.
        if (chdirRetVal == -1)
        {
            perror("Error executing chdir()");
        }
    }
}

// Function that implements built-in command setnev.
void setEnvironment(struct command *cmd)
{
    // If user does not supply arguments, print error message and do not
    // call setenv() or unsetenv().
    if (cmd -> args[1] == NULL)
    {
        perror("Error calling setenv command");
    }
    // Otherwise, call setenv() or unsetenv().
    else
    {
        // If user only supplies only one argument, unset the named
        // environemnt variable.
        if (cmd -> args[2] == NULL)
        {
            int unsetenvRetVal = unsetenv(cmd -> args[1]);
            // If an error occured on unsetenv(), print error message.
            if (unsetenvRetVal == -1)
            {
                perror("Error executing unsetenv()");
            }
        }
        // User supplied more than 1 argument, set the named environemnt
        // variable with the specified value.
        else
        {
            int setenvRetVal = setenv(cmd -> args[1], cmd -> args[2], 1);
            // If an error occured on setenv(), print error message.
            if (setenvRetVal == -1)
            {
                perror("Error executing setenv()");
            }
        }
    }
}

// Function that handles external commands and redirections.
void callExternal(struct command *cmd)
{
    // Call fork() and store result in variable.
    pid_t forkRetVal = fork();

    // If fork() returns a negative value, an error occured.
    if (forkRetVal < 0)
    {
        // Print an error message and exit.
        perror("Error executing fork()");
        exit(3);
    }

    // Child process.
    if (forkRetVal == 0)
    {
        // If input redirection file specified, call open() on specified file.
        if (cmd -> in_redir != NULL)
        {
            int infileDescriptor = open(cmd -> in_redir, O_RDONLY, 0666);
            // If open() returns a negative value, an error occured.
            if (infileDescriptor < 0)
            {
                // Print an error message and exit the program.
                perror("Error executing open()");
                exit(4);
            }

            // Move file descriptor from call to open to new file descriptor.
            int newDescriptor = dup2(infileDescriptor, 0); // newfd == 0: stdin.
            // If dup2() returns negative value, an error occured.
            if (newDescriptor < 0)
            {
                // Print an error message and exit the program.
                perror("Error executing dup2()");
                exit(5);
            }
        }
        // If output redirection file specified, call open() on specified file.
        if (cmd -> out_redir != NULL)
        {
            int outfileDescriptor = open(cmd -> out_redir,  O_WRONLY | O_CREAT | O_TRUNC, 0666);
            // If open() returns a negative value, an error occured.
            if (outfileDescriptor < 0)
            {
                // Print an error message and exit the program.
                perror("Error executing open()");
                exit(4);
            }

            // Move file descriptor from call to open to new file descriptor.
            int newDescriptor = dup2(outfileDescriptor, 1); // newfd == 1: stdout.
            // If dup2() returns negative value, an error occured.
            if (newDescriptor < 0)
            {
                // Print an error message and exit the program.
                perror("Error executing dup2()");
                exit(5);
            }
        }
        // Calls execvp() on program and arguments from command structure and
        // stores result in a variable.
        int execRetVal = 0;
        execRetVal = execvp(cmd -> args[0], cmd -> args);
        // If execve() returns a negative value, an error occured.
        if (execRetVal == -1)
        {
            // Print an error message and exit the program.
            perror("Error executing exec()");
            exit(6);
        }
    }

    // Parent process.
    if (forkRetVal > 0)
    {
        // Initialize int variable to hold exit status.
        int exitStatus = 0;

        // Ignores keyboard interrupt signal.
        signal(SIGINT, SIG_IGN);

        // Calls waitpid() to suspend execution until child process terminates.
        // Stores result of call to waitpid() in a pid_t variable.
        // exitStatus passed by reference to store information about the
        // process's exit status.
        pid_t processID = waitpid(forkRetVal, &exitStatus, 0);

        // Resets the signal handler for SIGINT to default.
        signal(SIGINT, SIG_DFL);

        // If waitpid() returns a negative value, an error occured.
        if (processID < 0)
        {
            // Print an error message and exit the program.
            perror("Error executing waitpid()");
            exit(7);
        }
        // Otherwise, check child exit status.
        else
        {
            // If the child exited normally with status 0, do not print a message.
            if (WIFEXITED(exitStatus) && WEXITSTATUS(exitStatus) == 0)
            {
                /* NOP */;
            }
            // If the child exited normally with nonzero status, print a message to stderr.
            if (WIFEXITED(exitStatus) && WEXITSTATUS(exitStatus) != 0)
            {
                fprintf(stderr, "Command returned %d\n", WEXITSTATUS(exitStatus));
            }
            // If the child was killed by a signal, print a message to stderr.
            if (WIFSIGNALED(exitStatus))
            {
                fprintf(stderr, "Command killed: %s\n", strsignal(WTERMSIG(exitStatus)));
            }
        }
    }
}
