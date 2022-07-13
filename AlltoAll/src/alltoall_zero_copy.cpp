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

void alltoall_zero_copy(char *sendbuf, int sendcount, MPI_Datatype sendtype, char *recvbuf, int recvcount, MPI_Datatype recvtype,  MPI_Comm comm) {
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

    // 2. initial data to recv_buffer and intermediate buffer
    double initial_start = MPI_Wtime();
    char* temp_buffer = (char*)malloc(local_size);
	memcpy(temp_buffer, recvbuf, local_size);
	memcpy(sendbuf, recvbuf, local_size);

	int bits[nprocs];
	bits[0] = 0; // the number of bits k' > k
	for (int j = 1; j < nprocs; j++) bits[j] = bits[j>>1]+(j&0x1);

	double initial_end = MPI_Wtime();
	double initial_time = initial_end - initial_start;

	// 3. exchange data with log(P) steps
	double exchange_start =  MPI_Wtime();
	int commblocks[nprocs];
	MPI_Datatype commtypes[nprocs];
	MPI_Aint recvindex[nprocs];
	MPI_Aint sendindex[nprocs];
	double total_create_dt_time = 0, total_comm_time = 0;
	unsigned int mask = 0xFFFFFFFF;
	int b, j;
	for (int k = 1; k < nprocs; k <<= 1) {
		// 1) create struct data-type that send and receive data from two buffers
		double create_datatype_start = MPI_Wtime();
		b = 0; j = k;
		while (j < nprocs) {
			int index = (rank + j) % nprocs;
			commblocks[b] = unit_size;
			commtypes[b] = MPI_CHAR;
			
			// send to recvbuf when the number of bits k' > k is odd 
			if ((bits[j&mask]&0x1)==0x1) {
				recvindex[b] = (MPI_Aint)((char*)recvbuf+index*unit_size);

				if ((j & mask) == j) // recv from sendbuf when the number of bits k' > k is even
					sendindex[b] = (MPI_Aint)((char*)sendbuf+index*unit_size);
				else // from intermediate buffer
					sendindex[b] = (MPI_Aint)((char*)temp_buffer+index*unit_size);
			}
			// send to intermediate buffer
			else {
				recvindex[b] = (MPI_Aint)((char*)temp_buffer+index*unit_size);

				if ((j & mask) == j) // recv from sendbuf when the number of bits k' > k is even
					sendindex[b] = (MPI_Aint)((char*)sendbuf+index*unit_size);
				else
					sendindex[b] = (MPI_Aint)((char*)recvbuf+index*unit_size);
			}
			b++;
			j++; if ((j & k) != k) j += k; // data blocks whose kth bit is 1
		}

		MPI_Datatype sendblocktype;
		MPI_Type_create_struct(b, commblocks, sendindex, commtypes, &sendblocktype);
		MPI_Type_commit(&sendblocktype);
		MPI_Datatype recvblocktype;
		MPI_Type_create_struct(b,commblocks, recvindex, commtypes, &recvblocktype);
		MPI_Type_commit(&recvblocktype);
		double create_datatype_end = MPI_Wtime();
		total_create_dt_time += (create_datatype_end - create_datatype_start);

		// 2) exchange data
		double comm_start = MPI_Wtime();
		int sendrank = (rank - k + nprocs) % nprocs;
		int recvrank = (rank + k) % nprocs;
		MPI_Sendrecv(MPI_BOTTOM, 1, sendblocktype, sendrank, 0, MPI_BOTTOM, 1, recvblocktype, recvrank, 0, comm, MPI_STATUS_IGNORE);

		MPI_Type_free(&recvblocktype);
		MPI_Type_free(&sendblocktype);
		double comm_end = MPI_Wtime();
		total_comm_time += (comm_end - comm_start);

		mask <<= 1;
	}
	free(temp_buffer);
	double exchange_end =  MPI_Wtime();
	double exchange_time = exchange_end - exchange_start;

	double u_end = MPI_Wtime();
	double max_u_time = 0;
	double total_u_time = u_end - u_start;
	MPI_Allreduce(&total_u_time, &max_u_time, 1, MPI_DOUBLE, MPI_MAX, comm);
	if (total_u_time == max_u_time) {
		 std::cout << "[ZerocopyBruck] [" << nprocs << " " << sendcount << "] " << total_u_time << " " << rotation_time << " " << exchange_time << " ["
				 << total_create_dt_time << " " << total_comm_time << "] " << std::endl;
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
        alltoall_zero_copy((char*)send_buffer, 1, MPI_INT, (char*)recv_buffer, 1, MPI_INT, MPI_COMM_WORLD);
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