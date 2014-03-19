#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <limits.h>
#include <libgen.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "util.h"

/************************** FUNCTION IMPLEMENTATIONS *************************/

unsigned int advance(char *str, unsigned int cursor, unsigned char fancy) {
    size_t len = strlen(str);
    unsigned int i = 0;
    for (i = cursor; i < len; i++) {
        if (!fancy) {
            if (!(isspace(str[i]))) {
                return i;
            }
        } else {
            if (str[i] == CHAR_PIPE || str[i] == CHAR_CARROT_RIGHT || str[i] == CHAR_CARROT_LEFT || str[i] == CHAR_AMP) {
                return i;
            }
        }
    }
    return len;
}

char **processArgList(char *str, size_t len) {
    unsigned int i = 0, j = 0, inWhitespace = 1, count = 0, limit = 0;
    char *argstr = (char *) malloc(len + 1), **argv;
    memcpy(argstr, str, len);
    argstr[len] = 0; // Assert the null char
    // Calculate the limit
    limit = advance(str, 0, 1);
    for (i = 0; i < limit; i++) {
        if (isspace(argstr[i])) {
            argstr[i] = 0;
            inWhitespace = 1;
        } else {
            if (inWhitespace) {
                count++;
                inWhitespace = 0;
            }
        }
    }
    // Allocate argv
    argv = (char **) malloc(sizeof(char *) * (count + 2));
    j = 1;
    inWhitespace = 1;
    // Loop through record the locations of all the sub strings
    for (i = 0; i < limit; i++) {
        if (!argstr[i]) {
            inWhitespace = 1;
        } else {
            if (inWhitespace) {
                argv[j] = (char *) (argstr + i);
                j++;
                inWhitespace = 0;
            }
        }
    }
    // Must be null terminated
    argv[count + 1] = NULL;
    return argv;
}

FILE *processFile(char *str) {
    return fopen(str, "rw");
}

Process *readCommand(char *str, size_t len) {
    int cursor = -1;
    unsigned int i, end = 0, status = READ_STATUS_PROG;
    char c, *s;
    Process *p = (Process *) malloc(sizeof(Process));
    // Fix defaults
    p->backgrounded = 0;
    // Read through the command character by character
    for (i = 0; i < (len + 1); i++) {
        c = (i < len) ? str[i] : 0;
        // This switch manages what we're currently doing
        switch (status) {
            /**** Reading Program Location ****/ 
            case READ_STATUS_PROG:
                if (cursor == -1) cursor = (i = advance(str, i, 0)); // Set the cursor if it hasn't been set yet
                end = i; // Update the end
                // Check if we're done yet
                if (isspace(c) || !c) {
                    // We are done with reading in the program
                    p->prog = (char *) malloc(end - cursor + 1);
                    // Copy the prog into s
                    memcpy(p->prog, (cursor + str), (end - cursor + 1));
                    (p->prog)[end - cursor] = 0; // Assert the null char
                    // Last but not least, increment the status & reset the cursor
                    status = READ_STATUS_ARGS;
                    cursor = -1;
                }
                break;
            /**** Reading Program Arguments ****/ 
            case READ_STATUS_ARGS:
                if (cursor == -1) cursor = (i = advance(str, i, 0)); // Set the cursor if it hasn't been set yet
                end = i; // Update the end
                if (c == CHAR_PIPE || c == CHAR_CARROT_RIGHT || c == CHAR_CARROT_LEFT || c == CHAR_AMP || !c) {
                    // We are done with reading in the args list string - move on
                    p->argv = processArgList(str + cursor, (end - cursor + 1));
                    // Last but not least, increment the status & reset the cursor
                    cursor = -1;
                    if (c == CHAR_PIPE) status = READ_STATUS_PIPE;
                    else if (c == CHAR_CARROT_RIGHT) status = READ_STATUS_OUT;
                    else if (c == CHAR_CARROT_LEFT) status = READ_STATUS_IN;
                    else if (c == CHAR_AMP) status = READ_STATUS_AMP;
                    else {
                        // If we're here, we're done reading str - we're outta here
                        return p;
                    }
                }
                break;
            /**** Reading Stdin/Stdout Replacement ****/ 
            case READ_STATUS_IN:
            case READ_STATUS_OUT:
                // Before we do anything - account for the right pointer
                if (cursor == -1) { 
                    cursor = (i = advance(str, i, 0)); // Set the cursor if it hasn't been set yet
                    c = str[i];
                }
                end = i; // Update the end
                if (isspace(c) || !c) {
                    // We are done with reading in the infile
                    s = (char *) malloc(end - cursor + 1);
                    memcpy(s, (cursor + str), end - cursor + 1);
                    s[end - cursor] = 0; // Assert the null char
                    if (status == READ_STATUS_IN) p->in = processFile(s);
                    else p->out = processFile(s);
                    // We need to advance the cursor again to see whats next
                    if (!c) return p;
                    else {
                        i = advance(str, i, 1);
                        // Check what we have now
                        if (i >= len) {
                            return p;
                        } else {
                            c = str[i];
                            cursor = -1;
                            if (c == CHAR_PIPE) status = READ_STATUS_PIPE;
                            else if (c == CHAR_CARROT_RIGHT) status = READ_STATUS_OUT;
                            else if (c == CHAR_CARROT_LEFT) status = READ_STATUS_IN;
                            else if (c == CHAR_AMP) status = READ_STATUS_AMP;
                        }
                    }
                }
                break;
            /**** Reading Stdout Replacement ****/ 
            case READ_STATUS_PIPE:
                if (cursor == -1) cursor = (i = advance(str, i, 0)); // Set the cursor if it hasn't been set yet
                end = advance(str, i, 1); // Update the end
                // We need to call read recursively here
                p->next = readCommand(str + cursor, end - cursor + 1);
                // Continue from after the pipe
                i = end;
                if (i >= len) {
                    return p;
                } else {
                    c = str[i];
                    cursor = -1;
                    if (c == CHAR_PIPE) status = READ_STATUS_PIPE;
                    else if (c == CHAR_CARROT_RIGHT) status = READ_STATUS_OUT;
                    else if (c == CHAR_CARROT_LEFT) status = READ_STATUS_IN;
                    else if (c == CHAR_AMP) status = READ_STATUS_AMP;
                }
                break;
            /**** Reading Backgrounding ****/ 
            case READ_STATUS_AMP:
                if (cursor == -1) cursor = (i = advance(str, i, 1)); // Set the cursor if it hasn't been set yet
                // Flag it in the process then exit
                p->backgrounded = 1;
                // Continue
                if (i >= len) {
                    return p;
                } else {
                    c = str[i];
                    cursor = -1;
                    if (c == CHAR_PIPE) status = READ_STATUS_PIPE;
                    else if (c == CHAR_CARROT_RIGHT) status = READ_STATUS_OUT;
                    else if (c == CHAR_CARROT_LEFT) status = READ_STATUS_IN;
                    else if (c == CHAR_AMP) status = READ_STATUS_AMP;
                }
                break;
        }
    }
    // Return p by default
    return p;
}

int pathOfExe(char *pth, char *exe) {
    char *searchpath;
    char *beg, *end;
    int stop, found;
    int len;

    if (strchr(exe, '/') != NULL) {
        if (realpath(exe, pth) == NULL) return 0;
        return isExecutable(pth);
    }

    searchpath = getenv("PATH");
    if (searchpath == NULL) return 0;
    if (strlen(searchpath) <= 0) return 0;

    beg = searchpath;
    stop = 0; found = 0;
    do {
        end = strchr(beg, ':');
        if (end == NULL) {
           stop = 1;
           strncpy(pth, beg, FULL_PATH_MAX);
           len = strlen(pth);
        } else {
           strncpy(pth, beg, end - beg);
           pth[end - beg] = '\0';
           len = end - beg;
        }
        if (pth[len - 1] != '/') strncat(pth, "/", 1);
        strncat(pth, exe, FULL_PATH_MAX - len);
        found = isExecutable(pth);
        if (!stop) beg = end + 1;
    } while (!stop && !found);

    return 0;
}

int isExecutable(char *filename) {
    int result;
    struct stat statinfo;

    result = stat(filename, &statinfo);
    if (result < 0) return 0;
    if (!S_ISREG(statinfo.st_mode)) return 0;

    if (statinfo.st_uid == geteuid()) return statinfo.st_mode & S_IXUSR;
    if (statinfo.st_gid == getegid()) return statinfo.st_mode & S_IXGRP;
    return statinfo.st_mode & S_IXOTH;
}