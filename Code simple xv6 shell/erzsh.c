/*
erzsh.c - shell with very basic functionality.
Ivan Vakal & CPTR-365
2-4-2023
*/

#include "kernel/types.h"
#include "user/user.h"
#include "kernel/fcntl.h"
#include <stddef.h>

#define MAX_INPUT_LENGTH 128

int main(int argc, char *argv[])
{
    char buf[MAX_INPUT_LENGTH]; // Buffer for user input
    int pid = fork();           // pid = fork;

    // Check if the number of command-line arguments is incorrect
    if (argc != 1)
    {
        printf("Usage: [commmand] %s\n", argv[0]); // Print usage message
        exit(1);                                   // Exit with error
    }

    // Loop for the shell
    while (1)
    {
        // Print prompt
        printf("EZ$ ");

        memset(buf, 0, sizeof(buf)); // Clear buffer
        gets(buf, sizeof(buf));      // Read user input

        // Fork a child process
        if (pid == 0) // Child process
        {
            // Parse the command
            char *args[MAX_INPUT_LENGTH / 2];    // Declare argument array
            int i = 0;                           // Initialize argument index
            int start = -1;                      // Initialize start index
            for (int j = 0; buf[j] != '\0'; j++) // Loop through input buffer
            {
                if (buf[j] != ' ' && buf[j] != '\n') // Check for non-space characters
                {
                    if (start == -1) // If start index is not set
                    {
                        start = j; // Set start index to current position
                    }
                }
                else // If space or newline character is encountered
                {
                    if (start != -1) // If start index is set
                    {
                        buf[j] = '\0';           // Null-terminate the argument
                        args[i++] = &buf[start]; // Store argument in array
                        start = -1;              // Reset start index
                    }
                }
            }
            if (start != -1) // If start index is set
            {
                args[i++] = &buf[start]; // Store last argument in array
            }
            args[i] = NULL; // Null-terminate argument array

            // Check if the command is 'cd'
            if (strcmp(args[0], "cd") == 0) // Check if directory argument is provided
            {
                // Change directory
                if (args[1] != NULL)
                {
                    if (chdir(args[1]) < 0)
                    {
                        printf("cd: no such directory\n"); // Print error message
                    }
                }
                else
                {
                    printf("cd: missing argument\n"); // Print error message
                }
            }
            else // Command other than 'cd'
            {
                // Check if the command should be run in the background
                if (i > 0 && strcmp(args[i - 1], "&") == 0)
                {
                    args[i - 1] = NULL; // Remove the ampersand token
                    // Execute the command in the background
                    exec(args[0], args);
                    printf("exec failed\n"); // This line should not be reached if exec is successful
                    exit(1);
                }
                else
                {
                    // Execute the command in the foreground
                    exec(args[0], args);
                    printf("exec failed\n"); // This line should not be reached if exec is successful
                    exit(1);
                }
            }
        }
        else if (pid < 0) // Error in forking
        {
            printf("fork error\n");
            exit(1);
        }
        else // Parent process
        {
            // Parent process waits for child to terminate
            wait(0);
        }
    }

    exit(0); // Exit program
}
