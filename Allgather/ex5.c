/*
This algorithm is an implementation of Allgather.
In the first step, processes that are a distance 1 apart exchange their data.
In the second step, processes that are a distance 2 apart exchange their own data
as well as the data they received in the previous step. In the third step, processes
that are a distance 4 apart exchange their own data as well the data they received
in the previous two steps. In this way, for a power-of-two number of processes,
all processes get all the data in lg p steps.
For long messages (> 512 KB), the recursive doubling runs much slower than ring.
*/

#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include <math.h>

int main(int argc, char *argv[])
{

    int numtasks, rank, dest, tag, n, source, rc, count, iterations, siz;
    long n_bytes, sleep_time;
    int pow_2;

    int *array;
    MPI_Status Stat;

    siz = atoi(argv[1]);        // Size of the data to broadcast
    iterations = atoi(argv[2]); // Number of iterations to repeat the procedure
    sleep_time = atoi(argv[3]); // Wait time between iterations (microseconds)

    n_bytes = pow(2, siz);
    n = n_bytes / sizeof(int);

    int msg[n];

    srand((unsigned)time(NULL) + rank);

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int nstep = (int)log2(numtasks);
    array = (int *)malloc(sizeof(int *) * (numtasks * n));
    srand((unsigned)time(NULL) + rank);

    MPI_Barrier(MPI_COMM_WORLD);
    double start = MPI_Wtime();

    for (int iteration = 0; iteration < iterations; iteration++)
    {
        for (size_t i = 0; i < n; i++)
        {
            msg[i] = rand() % 1000;
            array[i] = msg[i];
        }

        for (int i = 0; i < nstep; i++)
        {
            int p = pow(2, i);
            int a = numtasks / pow(2, i + 1); // Number of subtrees
            int k = numtasks / a;             // Number of items in the subtree
            int st = (rank / k) + 1;          // Subtree considered
            int j = rank + p;                 // Leap forward
            int tmp = n * p;

            if (j >= st * k)
            {
                j = rank - p; // Jump back
            }
            rc = MPI_Send(array, tmp, MPI_INT, j, 10, MPI_COMM_WORLD);
            rc = MPI_Recv(&array[tmp], tmp, MPI_INT, j, 10, MPI_COMM_WORLD, &Stat);
        }
        /*
        printf("Task %d received: [ ", rank);
        for (int i = 0; i < numtasks * n; i++)
        {
            printf("%d ", array[i]);
        }
        printf("]\n");
        */
        usleep(sleep_time);
    }

    MPI_Barrier(MPI_COMM_WORLD);
    double end = MPI_Wtime();

    if (rank == 0)
    {
        printf("Time:%f\n", end - start);
    }

    MPI_Finalize();

    free(array);
}
