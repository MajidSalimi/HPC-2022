#include <stdio.h>
#include <iostream>
#include <malloc.h>
#include <time.h>
#include <mpi.h>
#include <unistd.h>
#include <bits/stdc++.h>

using namespace std;

double  Mean(double[], int);
double  Median(double[], int);

int main(int argc, char **argv) {
  int i, procs, myid, *arr, n, err, max_iter ,iter = 0;
  long n_bytes, sleep_time;
  double t1_b, t2_b;
  // initialize MPI_Init
  err = MPI_Init(&argc, &argv);
  if (err != MPI_SUCCESS) {
    cout << "\nError initializing MPI.\n";
    MPI_Abort(MPI_COMM_WORLD, err);
  }

  MPI_Comm_size(MPI_COMM_WORLD, &procs);
  MPI_Comm_rank(MPI_COMM_WORLD, &myid);

  if (myid == 0) {
    if (argc < 4) {
      cout << "\n Invalid Number of Arguements.\n,\n";
      MPI_Finalize();
      return 0;
    }
  }

  n = atoi(argv[1]);      //number of elements for each process
  max_iter = atoi(argv[2]);   //number of iterations to repeat the procedure
  sleep_time = atoi(argv[3]); //Wait time between iterations (microseconds)

  n_bytes = n * sizeof(int);

  double alltoall_time[max_iter];
  arr = (int*)malloc(n*sizeof(int));

  // Generate Data
  for (i = 0; i < n; i++) { 
    arr[i] = myid * 100 * n + i * 100;
  }

  /* code section to print array
  
  MPI_Barrier(MPI_COMM_WORLD);
  for(int i = 0; i < procs; i++) {
    if(i == myid) {
          printf("Rank %d received: ", myid);
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
	
  //iteration
  iter = 0;
  int *buffer_recv;
  buffer_recv = (int*)malloc(n*sizeof(int));

  while (iter < max_iter) {
    MPI_Barrier(MPI_COMM_WORLD);
    t1_b = MPI_Wtime();
    MPI_Alltoall(arr, 1, MPI_INT, buffer_recv, 1, MPI_INT, MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);
    t2_b = MPI_Wtime();

    /* code section to print array

    MPI_Barrier(MPI_COMM_WORLD);
    for(int i = 0; i < procs; i++) {
      if(i == myid) {
            printf("Rank %d received: ", myid);
            // print entire array
            for(int j = 0; j < n; j++) {
                printf("%d ", buffer_recv[j]);
            }
            printf("\n");
            fflush(stdout);
        }
      MPI_Barrier(MPI_COMM_WORLD);
		}
    */
		
    if (myid == 0) {
      alltoall_time[iter] = ((t2_b - t1_b) * 1000);
    }

    usleep(sleep_time);
    iter++;
  }

  if (myid == 0) {
    cout << "\nThe size of the data sent: " << n_bytes << " Bytes";
    cout << "\nMean of communication times: " << Mean(alltoall_time, max_iter) << "ms";
    cout << "\nMedian of communication times: " << Median(alltoall_time, max_iter) << "ms\n";
  } 

  free(arr);
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