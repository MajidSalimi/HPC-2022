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

void alltoall_datatype(char *sendbuf, int sendcount, MPI_Datatype sendtype, char *recvbuf, int recvcount, MPI_Datatype recvtype,  MPI_Comm comm) {
	double u_start = MPI_Wtime();

    int rank, nprocs;
    MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(comm, &nprocs);

    int typesize;
    MPI_Type_size(sendtype, &typesize);

    int unit_size = sendcount * typesize;
    int local_size = sendcount * nprocs * typesize;

    // 1. local rotation
    double rotation_start = MPI_Wtime();
    for (int i = 0; i < nprocs; i++) {
    	int index = (2*rank-i+nprocs)%nprocs;
    	memcpy(&recvbuf[index*unit_size], &sendbuf[i*unit_size], unit_size);
    }
    double rotation_end = MPI_Wtime();
    double rotation_time = rotation_end - rotation_start;

    // 2. exchange data with log(P) steps
    double exchange_start =  MPI_Wtime();
 	double total_create_dt_time=0, total_comm_time=0, total_copy_time=0;
 	for (int k = 1; k < nprocs; k <<= 1) {
 		// 1) create data type
		double create_datatype_start = MPI_Wtime();
		int displs[(nprocs+1)/2];
		int sendb_num = 0;
		for (int i = 1; i < nprocs; i++) {
			if (i & k)
				displs[sendb_num++] = ((rank+i)%nprocs)*unit_size;
		}
		MPI_Datatype send_type;
		MPI_Type_create_indexed_block(sendb_num, unit_size, displs, MPI_CHAR, &send_type);
		MPI_Type_commit(&send_type);
		double create_datatype_end = MPI_Wtime();
		total_create_dt_time += create_datatype_end - create_datatype_start;

		// 2) exchange data
		double comm_start = MPI_Wtime();
		int recv_proc = (rank + k) % nprocs; // receive data from rank + 2^k process
		int send_proc = (rank - k + nprocs) % nprocs; // send data from rank - 2^k process
		MPI_Sendrecv(recvbuf, 1, send_type, send_proc, 0, sendbuf, 1, send_type, recv_proc, 0, comm, MPI_STATUS_IGNORE);
		MPI_Type_free(&send_type);
		double comm_end = MPI_Wtime();
		total_comm_time += (comm_end - comm_start);

		// 3) copy time
		double copy_start = MPI_Wtime();
		for (int i = 0; i < sendb_num; i++)
			memcpy(recvbuf+displs[i], sendbuf+displs[i], unit_size);
		double copy_end = MPI_Wtime();
		total_copy_time += (copy_end - copy_start);
 	}
 	double exchange_end = MPI_Wtime();
 	double exchange_time = exchange_end - exchange_start;

    double u_end = MPI_Wtime();
 	double max_u_time = 0;
 	double total_u_time = u_end - u_start;
 	MPI_Allreduce(&total_u_time, &max_u_time, 1, MPI_DOUBLE, MPI_MAX, comm);
	if (total_u_time == max_u_time) {
		 std::cout << "[ModDtBruck] [" << nprocs << " " << sendcount << "] " << total_u_time << " " << rotation_time << " " << exchange_time << " ["
				 << total_create_dt_time << " " << total_comm_time << " " << total_copy_time << "] " << std::endl;
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

  	int n_bytes = n * sizeof(int);
    double alltoall_time[max_iter];
    int *arr = (int*)malloc(n*sizeof(int));

    
    while (iter < max_iter) {

        int* send_buffer = new int[n];
        int* recv_buffer = new int[n];

        for(int i = 0; i < n; i++) {
            send_buffer[i] = rank * 100 * n + i * 100;
        }

        printf("Process %d send\n", rank);
        for (int i=0; i<n; i++) {
            printf("%d ", send_buffer[i]);
        }
        printf("\n");

        MPI_Barrier(MPI_COMM_WORLD);
        t1_b = MPI_Wtime();
        alltoall_datatype((char*)send_buffer, 1, MPI_INT, (char*)recv_buffer, 1, MPI_INT, MPI_COMM_WORLD);
        MPI_Barrier(MPI_COMM_WORLD);
        t2_b = MPI_Wtime();

        MPI_Barrier(MPI_COMM_WORLD);
        for(int i = 0; i < nprocs; i++) {
        	if(i == rank) {
                printf("Rank %d: ", rank);
                // print entire array
                for(int j = 0; j < n; j++) {
                    printf("%d ", recv_buffer[j]);
                }
                printf("\n");
                fflush(stdout);
            }

            MPI_Barrier(MPI_COMM_WORLD);
		}

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