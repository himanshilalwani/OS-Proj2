#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define N 3 // constant value of n

int main()
{
    printf("Main process: creating a full %d-ary tree of height 3.\n", N);

    printf("in parent process\n");
    // create n delegator processes
    for (int i = 0; i < N; i++)
    {
        pid_t pid = fork();
        if (pid == -1)
        {
            perror("fork failure delegator process");
            exit(EXIT_FAILURE);
        }
        // in delegator process
        else if (pid == 0)
        {
            printf("in delegator process %d\n", i);
            // create children
            pid_t childpid[N];
            for (int o = 0; o < N; o++)
            {
                childpid[o] = fork();
                if (childpid[o] == -1)
                {
                    perror("fork");
                    exit(EXIT_FAILURE);
                }
                // in a child process
                else if (childpid[o] == 0)
                {
                    printf("in child process %d of delegator process %d\n", o, i);
                    exit(EXIT_SUCCESS);
                }
            }

            // wait for all children processes to finish
            for (int j = 0; j < N; j++)
            {
                wait(NULL);
            }

            exit(EXIT_SUCCESS);
        }
    }

    // wait for all delegator processes to finish
    for (int i = 0; i < N; i++)
    {
        wait(NULL);
    }

    printf("Main process: all tasks completed.\n");

    return 0;
}
