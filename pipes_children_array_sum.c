/*Write a program that creates an array of size N.

 Initialize the array with some numbers. 

 Create n child processes divide the array between them. 

 Each child will add the portion and return their sum to parent process. 

 Parent will add the results and display a final sum*/

//run with -w to suppress warnings


#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

static void showReturnStatus(pid_t childpid, int status)
{
    if (WIFEXITED(status) && !WEXITSTATUS(status))
        printf("Child %ld terminated normally\n", (long)childpid);
    else if (WIFEXITED(status))
        printf("Child %ld terminated with return status %d\n", (long)childpid, WEXITSTATUS(status));
    else if (WIFSIGNALED(status))
        printf("Child %ld terminated due to uncaught signal %d\n", (long)childpid, WTERMSIG(status));
    else if (WIFSTOPPED(status))
        printf("Child %ld stopped due to signal %d\n", (long)childpid, WSTOPSIG(status));
}

int main()
{
    int arraySize = 9;
    int numProcess = 3;
    int array[arraySize];
    int fd[2];

    //number of elements each process will handle
    int numPerProcess = arraySize / numProcess;

    if (pipe(fd) == -1)
    {
        fprintf(stderr, "%s: failed to create a pipe\n");
        exit(EXIT_FAILURE);
    }


    //initialise array with numbers from 0 to 8 , and calculate correct value of sum
    int sum = 0;
    for (int i = 0; i < arraySize; i++)
    {
        array[i] = i;
        sum += array[i];
    }

	//display sum to know the CORRECT answer (our target)
    printf("Sum calculated by parent: %d\n", sum); 
    printf("Parent PID[%d]\n", getpid());


    //loop to create multiple children processes depending on the number we want
    for (int childP = 0; childP < numProcess; childP++)
    {
        int child = fork();

        if (child < 0)
        {
            fprintf(stderr, "%s: failed to fork child %d\n", childP);
            exit(EXIT_FAILURE);
        }

        //IN CHILD 
        if (child == 0)
        {
            close(fd[0]); //close READ end of pipe
            
            int childSum = 0;
            
            //calculate starting and ending points for this child
            int start = childP * numPerProcess;
            int stop = start + numPerProcess;
            printf("Child PID %d: processing elements %d..%d\n", getpid(), start, stop - 1);
            
            //loop to calculate the child sum
            for (int i = start; i < stop; i++)
                childSum += array[i];

            printf("Child Process No [%d] PID [%d] Return Sum : %d\n", childP, getpid(), childSum);
            
            //write the child sum to the WRITE end of the pipe
            write(fd[1], &childSum, sizeof(childSum));
            
            //close the WRITE end of the pipe
            close(fd[1]);
            
            //Exit from the child because it is done with its job now
            exit(0);
        }
    }

    //OUTSIDE children loop - handle PARENT code


    //close WRITE end
    close(fd[1]);

    sum = 0;
    pid_t pid;
    int status = 0;

    //loop to wait for all children
    //as long as parent still has children to wait for (wait not returning -1)
    //then we will do the following statements
    while ((pid = wait(&status)) != -1)
    {
        int number;

        //call function to display return status of child with this pid
        showReturnStatus(pid, status);
        
        //read from READ end 
        read(fd[0], &number, sizeof(number));
        printf("sum from pipe: %d\n", number);
        
        //add the partial sum (number) for the child to the sum
        sum += number;
    }

    printf("Parent Process with PID [%d] accumulated sum : %d\n", getpid(), sum);
    return 0;
}