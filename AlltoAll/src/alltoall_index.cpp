#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>
#include <iostream>
#include <unistd.h>
#include <bits/stdc++.h>

double  Mean(double[], int);
double  Median(double[], int);
static int rank, nprocs;

void alltoall_index(char *sendbuf, int sendcount, MPI_Datatype sendtype, char *recvbuf, int recvcount, MPI_Datatype recvtype,  MPI_Comm comm) {
    
    int rank, nprocs;
    MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(comm, &nprocs);

    int typesize;
    MPI_Type_size(sendtype, &typesize);

    int unit_size = sendcount * typesize;
    int local_size = sendcount * nprocs * typesize;

	// 1. local rotation
	memcpy(recvbuf, sendbuf, local_size);
	for (int i = 0; i < nprocs; i++) {
		int index = (i - rank + nprocs) % nprocs;
		memcpy(&sendbuf[index*unit_size], &recvbuf[i*unit_size], unit_size);
	}

    // 2. exchange data with log(P) steps
    char* temp_buffer = (char*)malloc(local_size);
    for (int k = 1; k < nprocs; k <<= 1) {
    	// 1) find which data blocks to send
    	int send_indexes[(nprocs+1)/2];
    	int sendb_num = 0;
		for (int i = 1; i < nprocs; i++) {
			if (i & k)
				send_indexes[sendb_num++] = i;
		}

		// 2) copy blocks which need to be sent at this step
		for (int i = 0; i < sendb_num; i++) {
			int offset = send_indexes[i] * unit_size;
			memcpy(temp_buffer+(i*unit_size), sendbuf+offset, unit_size);
		}

		// 3) send and receive
		int recv_proc = (rank - k + nprocs) % nprocs; // receive data from rank - 2^step process
		int send_proc = (rank + k) % nprocs; // send data from rank + 2^k process
		int comm_size = sendb_num * unit_size;
		MPI_Sendrecv(temp_buffer, comm_size, MPI_CHAR, send_proc, 0, recvbuf, comm_size, MPI_CHAR, recv_proc, 0, comm, MPI_STATUS_IGNORE);

		// 4) replace with received data
		for (int i = 0; i < sendb_num; i++) {
			int offset = send_indexes[i] * unit_size;
			memcpy(sendbuf+offset, recvbuf+(i*unit_size), unit_size);
		}
    }

    free(temp_buffer);

    // 3. second rotation
	for (int i = 0; i < nprocs; i++) {
		int index = (rank - i + nprocs) % nprocs;
		memcpy(&recvbuf[index*unit_size], &sendbuf[i*unit_size], unit_size);
	}    
}

int main(int argc, char **argv) {

    double t1_b, t2_b;
    int iter = 0;
    // MPI Initial
    if (MPI_Init(&argc, &argv) != MPI_SUCCESS)
        std::cout << "ERROR: MPI_Init error\n" << std::endl;
    if (MPI_Comm_size(MPI_COMM_WORLD, &nprocs) != MPI_SUCCESS)
    	std::cout << "ERROR: MPI_Comm_size error\n" << std::endl;
    if (MPI_Comm_rank(MPI_COMM_WORLD, &rank) != MPI_SUCCESS)
    	std::cout << "ERROR: MPI_Comm_rank error\n" << std::endl;
	
	int n = atoi(argv[1]);      //number of elements for each process
  	int max_iter = atoi(argv[2]);   //number of iterations to repeat the procedure
  	int sleep_time = atoi(argv[3]); //Wait time between iterations (microseconds)

    if (rank == 0) {
        if (argc < 4) {
            std::cout << "\n Invalid Number of Arguements.\n,\n";
            MPI_Finalize();
            return 0;
        }
    }

  	int n_bytes = n * sizeof(int);
    double alltoall_time[max_iter];

    while (iter < max_iter) {

        int* send_buffer = new int[n];
        int* recv_buffer = new int[n];

        for(int i = 0; i < n; i++) {
            send_buffer[i] = rank * 100 * n + i * 100;
        }

        /* code section to print array
		
		MPI_Barrier(MPI_COMM_WORLD);
        for(int i = 0; i < nprocs; i++) {
        	if(i == rank) {
                printf("Rank %d send: ", rank);
                // print entire array
                for(int j = 0; j < n; j++) {
                    printf("%d ", send_buffer[j]);
                }
                printf("\n");
                fflush(stdout);
            }

            MPI_Barrier(MPI_COMM_WORLD);
		}
		printf("\n");
		*/

        MPI_Barrier(MPI_COMM_WORLD);
        t1_b = MPI_Wtime();
        alltoall_index((char*)send_buffer, 1, MPI_INT, (char*)recv_buffer, 1, MPI_INT, MPI_COMM_WORLD);
        MPI_Barrier(MPI_COMM_WORLD);
        t2_b = MPI_Wtime();

        /* code section to print array

        MPI_Barrier(MPI_COMM_WORLD);
        for(int i = 0; i < nprocs; i++) {
        	if(i == rank) {
                printf("Rank %d received: ", rank);
                // print entire array
                for(int j = 0; j < n; j++) {
                    printf("%d ", recv_buffer[j]);
                }
                printf("\n");
                fflush(stdout);
            }

            MPI_Barrier(MPI_COMM_WORLD);
		}
		*/

        delete[] send_buffer;
	    delete[] recv_buffer;

        if (rank == 0) {
            alltoall_time[iter] = ((t2_b - t1_b) * 1000);
        }

        usleep(sleep_time);
        iter++;
    
    }

    MPI_Barrier(MPI_COMM_WORLD);
    if (rank == 0) {
        std::cout << "\nThe size of the data sent: " << n_bytes << " Bytes";
        std::cout << "\nMean of communication times: " << Mean(alltoall_time, max_iter) << "ms";
        std::cout << "\nMedian of communication times: " << Median(alltoall_time, max_iter) << "ms\n";
    } 
    
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

  std::sort(a, a + n);
  if (n % 2 != 0)
    return a[n / 2];

  return (a[(n - 1) / 2] + a[n / 2]) / 2.0;
}

	