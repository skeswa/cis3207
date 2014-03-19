#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "util.h"

/*************************** FUNCTION DECLARATIONS ***************************/

// TODO HANDLE CD HERE

// Trims whitespace off of the back of the string.
// Takes a string to trim the whitespace off the back of.
// Returns nothing.
void curtail(char *str) {
  char *end = str + strlen(str) - 1;
  while(end >= str && isspace(*end)) end--;
  *(end + 1) = 0; // Advance the null terminator
}

void doExecution() {
    pid_t pid;
    Process *proc;
    char *input = (char *) malloc(sizeof(char) * (INPUT_BUFFER_SIZE + 1)); 
    char *currdir = (char *) malloc(sizeof(char) * (CURRDIR_BUFFER_SIZE + 1));
    char *hostname = (char *) malloc(sizeof(char) * (CURRDIR_BUFFER_SIZE + 1));
    char *fgetsResult;
    //int pipefd[2];
    // Read the input
    memset(input, 0, (INPUT_BUFFER_SIZE + 1));
    memset(currdir, 0, (CURRDIR_BUFFER_SIZE + 1));
    memset(hostname, 0, (CURRDIR_BUFFER_SIZE + 1));
    // Read the machine name
    gethostname(hostname, CURRDIR_BUFFER_SIZE);
    // The loop that goes for days
    while (1) {
        // Read current directory
        getcwd(currdir, CURRDIR_BUFFER_SIZE);
        printf("%s> [%s]# ", hostname, currdir);
        // Set the buffer to "empty"
        input[0] = 0;
        // Read from command line
        fgetsResult = fgets(input, INPUT_BUFFER_SIZE, stdin);
        // Make sure the input string is curtailed first
        curtail(input);
        // Continue with loop
        if (strlen(input) >= 1) {
            // Read proc
            proc = readCommand(input, strlen(input));
            // Verify that the executable exists
            // Put the prog in argv
            if (proc->argv) proc->argv[0] = proc->prog;
            else {
                // We have to make up an argv
                proc->argv = (char **) malloc(sizeof(char *) * 2);
                proc->argv[0] = proc->prog;
                proc->argv[1] = NULL;
            }
            // EXEC BREH
            //pipe(pipefd);
            pid = fork();
            if (pid > 0) {
                // This is the parent
                proc->pid = pid;
                if (!(proc->backgrounded)) {
                    // Wait for the kid
                    do {
                        pid = wait(&(proc->status));
                        // if (pid != proc->pid) process_terminated(pid);
                    } while (pid != proc->pid);
                    // Child has now exited
                    // printf("Child exited with status %d.\n", proc->status);
                } else {
                    printf("Child started in background.\n");
                }
            } else if (pid < 0) {
                printf("THE FORK FAILED....PANICCCCCCCCCCCCCCC!\n");
                exit(1);
            } else {
                // This is the child
                if (proc->backgrounded) {
                    // IF backgrounding - then we have to do something
                    setpgid(0, 0);
                }
                // Input redirection
                if (proc->in != -1) {
                    // Replace stdin with infile
                    close(0);
                    dup(proc->in);
                    close(proc->in);
                }
                // Output redirection
                if (proc->out != -1) {
                    // Replace stdout with outfile
                    close(1);
                    dup(proc->out);
                    close(proc->out);
                }
                execvp(proc->prog, proc->argv);
                // If we got this far the call failed
                printf("Could not execute the command as entered - please try again.");
                exit(1);
            }
        } else if (fgetsResult == NULL) {
            printf("Received Ctrl + D (the EOF) - now exiting...\n");
            exit(0);
        } else {
            // Empty input - so don't do anything
        }
    }
}

/**************************** PROGRAM ENTRY POINT ****************************/

/*  The main function - starts the shell */
int main() {
    /*char *s = "ls -al . < in.txt > out.txt | grep . | cd work";
    Process *p = readCommand(s, strlen(s));
    printf("%s\n", p->prog);*/
    doExecution();
    return 0;
}