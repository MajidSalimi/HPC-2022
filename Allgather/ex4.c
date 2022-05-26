/*
Ring

This algorithm is an implementation of Allgather.
A loop method is used in which the data of each process is sent around a virtual ring of processes.
In the first step, each process i sends its contribution to process i + 1 and receives
the contribution from process i-1 (with wrap-around). From the second step onwards each
process i forwards to process i+1 the data it received from process i-1 in the previous step.
If p is the number of processes, the entire algorithm takes p-1 steps.
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
    srand((unsigned)time(NULL) + rank);

    MPI_Barrier(MPI_COMM_WORLD);
    double start = MPI_Wtime();

    for (int iteration = 0; iteration < iterations; iteration++)
    {
        if (numtasks < 2)
        {
            for (size_t i = 0; i < n; i++)
            {
                msg[i] = rand() % 1000;
                array[i] = msg[i];
            }
        }
        else
        {
            for (int ring = 0; ring < numtasks; ring++)
            {
                if (rank == 0)
                {
                    if (ring == 0)
                    {
                        for (size_t i = 0; i < n; i++)
                        {
                            msg[i] = rand() % 1000;
                            array[i] = msg[i];
                        }
                        rc = MPI_Send(&msg, n, MPI_INT, rank + 1, 10, MPI_COMM_WORLD);
                    }
                    else
                    {
                        int tmp = ring * n;
                        rc = MPI_Recv(&array[tmp], n, MPI_INT, numtasks - 1, 10, MPI_COMM_WORLD, &Stat);
                        rc = MPI_Send(&array[tmp], n, MPI_INT, rank + 1, 10, MPI_COMM_WORLD);
                    }
                }
                else
                {
                    int tmp_recv = (ring + 1) * n;
                    int tmp_send = (ring)*n;

                    if (ring == 0)
                    {
                        for (size_t i = 0; i < n; i++)
                        {
                            msg[i] = rand() % 1000;
                            array[i] = msg[i];
                        }
                        if (rank == numtasks - 1)
                        {
                            rc = MPI_Send(&msg, n, MPI_INT, 0, 10, MPI_COMM_WORLD);
                        }
                        else
                        {
                            rc = MPI_Send(&msg, n, MPI_INT, rank + 1, 10, MPI_COMM_WORLD);
                        }

                        rc = MPI_Recv(&array[tmp_recv], n, MPI_INT, rank - 1, 10, MPI_COMM_WORLD, &Stat);
                    }
                    else
                    {
                        rc = MPI_Recv(&array[tmp_recv], n, MPI_INT, rank - 1, 10, MPI_COMM_WORLD, &Stat);
                        if (rank == numtasks - 1)
                        {
                            rc = MPI_Send(&array[tmp_send], n, MPI_INT, 0, 10, MPI_COMM_WORLD);
                        }
                        else
                        {
                            rc = MPI_Send(&array[tmp_send], n, MPI_INT, rank + 1, 10, MPI_COMM_WORLD);
                        }
                    }
                }
            }
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
