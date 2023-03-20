/* The code creates N delegator processes and an array of pipes for each delegator process. 
Each delegator process then creates N child processes and an array of pipes for each child process. 
Each child process generates a random value using its process ID, writes the value to the pipe, and then exits. 
The delegator process reads the values from the pipes for each child process, 
prints the values received from the child processes, writes the values to its pipe, and then exits. 
The parent process reads the values from the pipes for each delegator process, 
stores the values in an array, and prints the final array combined in the parent. */


// include necessary header files
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

// define a constant value N
#define N 2

// main function
int main()
{
    // print a message about creating a full N-ary tree of height 3
    printf("Main process: creating a full %d-ary tree of height 3.\n", N);

    // print a message about being in the parent process
    printf("in parent process\n");

    // create an array of pipes for each delegator process
    int dpipefd[N][2];

    // create N delegator processes
    for (int i = 0; i < N; i++)
    {
        // create a pipe for each delegator process
        if (pipe(dpipefd[i]) == -1)
        {
            perror("pipe");
            exit(EXIT_FAILURE);
        }

        // create a new process (delegator)
        pid_t pid = fork();

        // check for errors creating the process
        if (pid == -1)
        {
            perror("fork failure delegator process");
            exit(EXIT_FAILURE);
        }
        // in the delegator process
        else if (pid == 0)
        {
            // close the read end of the pipe
            close(dpipefd[i][0]);

            // print a message indicating which delegator process is being executed
            printf("in delegator process %d\n", i);

            // create N child processes for each delegator process
            pid_t childpid[N];
            int pipefd[N][2];

            // create an array of pipes for each child process
            for (int o = 0; o < N; o++)
            {
                // create a pipe for each child process
                if (pipe(pipefd[o]) == -1)
                {
                    perror("pipe");
                    exit(EXIT_FAILURE);
                }

                // create a new process (child)
                childpid[o] = fork();

                // check for errors creating the process
                if (childpid[o] == -1)
                {
                    perror("fork");
                    exit(EXIT_FAILURE);
                }
                // in a child process
                else if (childpid[o] == 0)
                {
                    // close the read end of the pipe
                    close(pipefd[o][0]);

                    // generate a random value using the child process ID
                    srand(getpid());
                    int value = rand();

                    // write the value to the pipe
                    if (write(pipefd[o][1], &value, sizeof(value)) == -1)
                    {
                        perror("write");
                        exit(EXIT_FAILURE);
                    }

                    // close the write end of the pipe
                    close(pipefd[o][1]);

                    // print a message indicating which child process is being executed and what its random value is
                    printf("in child process %d of delegator process %d: random value = %d\n", o, i, value);

                    // exit the child process successfully
                    exit(EXIT_SUCCESS);
                }

                // close the write end of the pipe
                close(pipefd[o][1]);
            }

            // read the values from the pipes for each child process
            int values[N];
            for (int o = 0; o < N; o++)
            {
                if (read(pipefd[o][0], &values[o], sizeof(values[o])) == -1)
                {
                    perror("read");
                    exit(EXIT_FAILURE);
                }
                // close the read end of the pipe
                close(pipefd[o][0]);
            }

            // Print the values received from the child processes
            printf("In delegator process %d: random values =", i);
            for (int o = 0; o < N; o++)
            {
                printf(" %d", values[o]);
            }
            printf("\n");

            // Wait for all children processes to finish
            for (int j = 0; j < N; j++)
            {
                wait(NULL);
            }

            // Write the value to the pipe
            if (write(dpipefd[i][1], &values, sizeof(values)) == -1)
            {
                perror("write");
                exit(EXIT_FAILURE);
            }

            exit(EXIT_SUCCESS);
        }
        close(dpipefd[i][1]); // Close the write end of the pipe
    }

    // Initialize variables
    int fvalues[N * N];
    int findex = 0;

    // Read values from the pipe and store them in the fvalues array
    for (int o = 0; o < N; o++)
    {
        if (read(dpipefd[o][0], &fvalues[findex + o * N], sizeof(int) * N) == -1)
        {
            perror("read");
            exit(EXIT_FAILURE);
        }
        close(dpipefd[o][0]); // Close the read end of the pipe
    }

    // Wait for all delegator processes to finish
    for (int i = 0; i < N; i++)
    {
        wait(NULL);
    }

    // Print the final array combined in the parent
    printf("Final array combined in parent:");
    for (int o = 0; o < N * N; o++)
    {
        printf(" %d", fvalues[o]);
    }
    printf("\n");

    printf("Main process: all tasks completed.\n");

    return 0;
}