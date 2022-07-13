#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>
#include <iostream>
#include <unistd.h>
#include <bits/stdc++.h>

using namespace std;
double  Mean(double[], int);
double  Median(double[], int);

typedef enum {
    SEND, RECEIVE
} MsgType;

/**
 * Displays a given array
 * @param arr       The array to be displayed
 * @param arr_size  The size of the array
 */
void print_array(int* arr, int arr_size) {

    MPI_Barrier(MPI_COMM_WORLD);

    printf("[ ");

    for(int i = 0; i < arr_size; i++) {
        printf("%d, ", arr[i]);
    }

    printf("]");

    MPI_Barrier(MPI_COMM_WORLD);
}

/**
 * Displays the message that has been exchanged
 * @param self_rank     The rank of the current process
 * @param other_rank    The rank of the process which was hte other end of the communication
 * @param msg_size      The size (in number of elements) of the message exchanged
 * @param msg           The actual message that was exchanged
 * @param msgType       Whether the message was a SEND or a RECEIVE
 */
void print_msg(int self_rank, int other_rank, int msg_size, int* msg, MsgType msgType) {

    switch (msgType) {
        case SEND:
            printf("Process %d sent ", self_rank);
            print_array(msg, msg_size);
            printf(" to process %d\n", other_rank);
            break;

        case RECEIVE:
            printf("Process %d received ", self_rank);
            print_array(msg, msg_size);
            printf(" from process %d\n", other_rank);
            break;

        default:
            printf("Illegal state!\n");
            break;
    }

    fflush(stdout);
}

/**
 * Perfoms compaction on the array by removing the specified element.
 * This therefore prepares the next message to be sent
 * @param idx   Index of element to be removed
 * @param arr   The array of messages
 * @param size  The total number of messages in the array
 */
void compact_array(int idx, int* arr, int size) {

    for(int i = idx + 1; i < size; i++) {
        arr[i-1] = arr[i];
    }
}

/**
 * Uses MPI_Send and MPI_Receive to simulate an all-to-all personalized communication
 * among processes as if thery were arranged in a ring topology
 * @param msg_arr   The initial array of messages
 * @param res_arr   The final array of messages
 * @param rank      The rank of the process (corresponds to the node of the ring)
 * @param num_proc  The number of processes (corresponds to the number of nodes in the ring)
 * 
 * @return The time taken for the communication to terminate
 */
void all_to_all_personalized(int* msg_arr, int* res_arr, int rank, int num_proc) {

    int buf[num_proc - 1];

    /* creating first message to send */
    for(int i = 0; i < num_proc; i++) {
        if(i < rank) {
            buf[i] = msg_arr[i];
        }
        else if(i == rank) {
            continue;
        } else {
            buf[i-1] = msg_arr[i];
        }
    }

    res_arr[rank] = msg_arr[rank]; /* store the initial value */

    for(int i = 1; i < num_proc; i++) {

        int send_to = (rank + 1) % num_proc;
        int msg_size = num_proc - i;

        MPI_Send(buf, msg_size, MPI_INT, send_to, rank, MPI_COMM_WORLD);

        int recv_from = (rank + num_proc - 1) % num_proc;
        int origin = (num_proc + rank - i) % num_proc;
        MPI_Status status;

        MPI_Recv(buf, msg_size, MPI_INT, recv_from, recv_from, MPI_COMM_WORLD, &status);

        /* store the result */
        int data_index = (origin <= rank) ? origin : 0;
        res_arr[origin] = buf[data_index];

        compact_array(data_index, buf, msg_size);
    }

    return;
}

int main(int argc, char** argv) {

    int num_proc, rank, n, err, max_iter ,iter = 0;
    long n_bytes, sleep_time;
    double t1_b, t2_b;

    // initialize MPI_Init
    err = MPI_Init(&argc, &argv);
    if (err != MPI_SUCCESS) {
        cout << "\nError initializing MPI.\n";
        MPI_Abort(MPI_COMM_WORLD, err);
    }

    /* Get numbmer of processes */
    MPI_Comm_size(MPI_COMM_WORLD, &num_proc);

    /* Get current process id */
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    n = atoi(argv[1]);      //number of elements for each process
    max_iter = atoi(argv[2]);   //number of iterations to repeat the procedure
    sleep_time = atoi(argv[3]); //Wait time between iterations (microseconds)

    if (rank == 0) {
        if (argc < 4) {
            std::cout << "\n Invalid Number of Arguements.\n,\n";
            MPI_Finalize();
            return 0;
        }
    }

    n_bytes = n * sizeof(int);

    int *arr = (int*)malloc(n*sizeof(int));

    for(int i = 0; i < n; i++) {
        arr[i] = rank * 100 * n + i * 100;
    }

    /* code section to print array
    
    MPI_Barrier(MPI_COMM_WORLD);
    for(int i = 0; i < num_proc; i++) {
        if(i == rank) {
            printf("Rank %d send array: ", rank);
            // print entire array
            for(int j = 0; j < n; j++) {
                printf("%d ", arr[j]);
            }
            printf("\n");
            fflush(stdout);
        }

        MPI_Barrier(MPI_COMM_WORLD);
    }
    printf("\n");
    */

    iter = 0;
    double alltoall_time[max_iter];
    int *res_arr = (int*)malloc(n*sizeof(int));

    while (iter < max_iter) {

        MPI_Barrier(MPI_COMM_WORLD);
        t1_b = MPI_Wtime();
        all_to_all_personalized(arr, res_arr, rank, num_proc);
        MPI_Barrier(MPI_COMM_WORLD);
        t2_b = MPI_Wtime();

        if (rank == 0) {
            alltoall_time[iter] = ((t2_b - t1_b) * 1000);
        }

        /* code section to print array

        MPI_Barrier(MPI_COMM_WORLD);
        for(int i = 0; i < num_proc; i++) {
        	if(i == rank) {
                printf("Rank %d received: ", rank);
                // print entire array
                for(int j = 0; j < n; j++) {
                    printf("%d ", res_arr[j]);
                }
                printf("\n");
                fflush(stdout);
            }

            MPI_Barrier(MPI_COMM_WORLD);
		}
        */
        
        usleep(sleep_time);
        iter++;
    }
    

    MPI_Barrier(MPI_COMM_WORLD);

    if (rank == 0) {
        cout << "\nThe size of the data sent: " << n_bytes << " Bytes";
        cout << "\nMean of communication times: " << Mean(alltoall_time, max_iter) << "ms";
        cout << "\nMedian of communication times: " << Median(alltoall_time, max_iter) << "ms\n";
    } 

    free(arr);
    free(res_arr);
    MPI_Finalize();
    return 0;
}

double Mean(double a[], int n) {

  double sum = 0.0;
  for (int i = 0; i < n; i++)
    sum += a[i];

  return (sum / (double)n);
}

double Median(double a[], int n) {

  sort(a, a + n);
  if (n % 2 != 0)
    return a[n / 2];

  return (a[(n - 1) / 2] + a[n / 2]) / 2.0;
}
