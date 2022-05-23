/*
This simple program, generates required size of data and does the
allgather in a iteration required by user, using Gather and Broadcast.
*/

#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <math.h>

int main(int argc, char *argv[])
{

    int numtasks, rank, dest, tag, n, source, rc, count, iterations, siz;
    long sleep_time;
    int pow_2;

    int *array;
    MPI_Status Stat;

    siz = atoi(argv[1]);        // Size of the data to broadcast
    iterations = atoi(argv[2]); // Number of iterations to repeat the procedure
    sleep_time = atoi(argv[3]); // Wait time between iterations (microseconds)

    n = pow(2, siz);

    int msg[n];

    srand((unsigned)time(NULL) + rank);

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    array = (int *)malloc(sizeof(int *) * (numtasks * n));

    MPI_Barrier(MPI_COMM_WORLD);
    double start = MPI_Wtime();
    for (int iteration = 0; iteration < iterations; iteration++)
    {

        for (size_t i = 0; i < n; i++)
        {
            msg[i] = rand() % 1000;
        }

        MPI_Gather(&msg, n, MPI_INT, array, n, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(array, n * numtasks, MPI_INT, 0, MPI_COMM_WORLD);

        /*
        printf("Process [%d] received: [", rank);

        for (int i = 0; i < n * numtasks; i++)
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
        printf("time:%f\n", end - start);
    }

    MPI_Finalize();
}
