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
#include <string.h>
#include <signal.h>

// define a constant value N
#define N 2

void sigusr_handler(int signum)
{
    if (signum == SIGUSR1)
    {
        printf("Received SIGUSR1 signal from child process.\n");
    }
}
// main function
int main(int argc, char *argv[])
{
    if (argc != 8)
    {
        printf("Error: Missing arguments\n");
        printf("Usage: %s -l LowerBound -u UpperBound -[e|r] -n NumOfNodes\n", argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "-l") != 0 || strcmp(argv[3], "-u") != 0 || (strcmp(argv[5], "-e") != 0 && strcmp(argv[5], "-r") != 0) || (strcmp(argv[6], "-n") != 0))
    {
        printf("Error: Invalid arguments\n");
        printf("Usage: %s -l LowerBound -u UpperBound -[e|r] -n NumOfNodes\n", argv[0]);
        return 1;
    }

    int lowerbound = atoi(argv[2]);
    int upperbound = atoi(argv[4]);
    char *work = argv[5];
    int numNodes = atoi(argv[7]);

    // print a message about creating a full N-ary tree of height 3
    printf("Main process: creating a full %d-ary tree of height 3.\n", numNodes);

    // print a message about being in the parent process
    printf("in parent process\n");

    // signals
    signal(SIGUSR1, sigusr_handler);

    // create an array of pipes for each delegator process
    int dpipefd[numNodes][2];

    // create N delegator processes
    for (int i = 0; i < numNodes; i++)
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

            // Dividing the range in N (numNodes) sub-intervals
            int total_length = upperbound - lowerbound + 1;
            int subrange_length = total_length / numNodes;

            int start = subrange_length * i + lowerbound;
            int stop = start + subrange_length - 1;

            // create N child processes for each delegator process
            pid_t childpid[numNodes];
            int pipefd[numNodes][2];

            // create an array of pipes for each child process
            for (int o = 0; o < numNodes; o++)
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
                    dup2(pipefd[o][1], 1);
                    if (strcmp(work, "-e") == 0)
                    {
                        int r = (stop - start + 1) / numNodes; // range
                        int s = r * o + start;                 // start
                        int e = s + r - 1;                     // end
                        if (e + r > stop)
                        {
                            e = stop;
                        }
                        char *arg[4];
                        arg[0] = "./prime1";
                        char string_s[100];
                        char string_e[100];
                        sprintf(string_s, "%d", s);
                        sprintf(string_e, "%d", e);
                        arg[1] = string_s;
                        arg[2] = string_e;
                        arg[3] = NULL;
                        execvp(arg[0], arg);
                        perror("execvp");
                        exit(EXIT_FAILURE);
                    }

                    // // generate a random value using the child process ID
                    // srand(getpid());
                    // int value = rand();

                    // write the value to the pipe
                    // if (write(pipefd[o][1], &value, sizeof(value)) == -1)
                    // {
                    //     perror("write");
                    //     exit(EXIT_FAILURE);
                    // }

                    // // close the write end of the pipe
                    // close(pipefd[o][1]);

                    // // print a message indicating which child process is being executed and what its random value is
                    // printf("in child process %d of delegator process %d: random value = %d\n", o, i, value);

                    // exit the child process successfully
                    exit(EXIT_SUCCESS);
                }

                // close the write end of the pipe
                close(pipefd[o][1]);
            }

            // read the values from the pipes for each child process
            int values[numNodes];
            for (int o = 0; o < numNodes; o++)
            {
                if (read(pipefd[o][0], &values[o], sizeof(values[o])) == -1)
                {
                    perror("read");
                    exit(EXIT_FAILURE);
                }
                char foo[4096];

                int nbytes = read(pipefd[o][0], foo, sizeof(foo));
                printf("Output: (%.*s)\n", nbytes, foo);
                // close the read end of the pipe
                close(pipefd[o][0]);
            }

            // Print the values received from the child processes
            printf("In delegator process %d: random values =", i);
            for (int o = 0; o < numNodes; o++)
            {
                printf(" %d", values[o]);
            }
            printf("\n");

            // Wait for all children processes to finish
            for (int j = 0; j < numNodes; j++)
            {
                wait(NULL);
                if (kill(getppid(), SIGUSR1) == -1)
                {
                    perror("kill");
                    exit(EXIT_FAILURE);
                }
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
    int fvalues[numNodes * numNodes];
    int findex = 0;

    // Read values from the pipe and store them in the fvalues array
    for (int o = 0; o < numNodes; o++)
    {
        if (read(dpipefd[o][0], &fvalues[findex + o * numNodes], sizeof(int) * numNodes) == -1)
        {
            perror("read");
            exit(EXIT_FAILURE);
        }
        close(dpipefd[o][0]); // Close the read end of the pipe
    }

    // Wait for all delegator processes to finish
    for (int i = 0; i < numNodes; i++)
    {
        wait(NULL);
    }

    // Print the final array combined in the parent
    printf("Final array combined in parent:");
    for (int o = 0; o < numNodes * numNodes; o++)
    {
        printf(" %d", fvalues[o]);
    }
    printf("\n");

    printf("Main process: all tasks completed.\n");

    return 0;
}