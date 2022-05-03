/*
This simple program, generates required size of data and does the broadcast in an iteration.
To run: mpirun -n nprocs a.out data_size iterations delay_between_iterations(us)
Note: data_size will be in powers of 2. you just need to put power. for example if data_size=5, it will be 2^5 bytes. 
*/
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
//void    Print_times(double[], int);

int main(int argc, char **argv)
{
  int i, procs, myid, *arr, n, err, max_iter ,iter = 0;
  long n_bytes, sleep_time;
  int pow_2;
  double t1_b, t2_b;
  // initialize MPI_Init
  err = MPI_Init(&argc, &argv);
  if (err != MPI_SUCCESS)
  {
    cout << "\nError initializing MPI.\n";
    MPI_Abort(MPI_COMM_WORLD, err);
  }

  MPI_Comm_size(MPI_COMM_WORLD, &procs);
  MPI_Comm_rank(MPI_COMM_WORLD, &myid);

  if (myid == 0)
  {
    if (argc < 4)
    {
      cout << "\n Invalid Number of Arguements.\n,\n";
      MPI_Finalize();
      return 0;
    }
  }

  pow_2 = atoi(argv[1]);      //size of the data to broadcast: n_bytes= 2^pow_2
  max_iter = atoi(argv[2]);   //number of iterations to repeat the procedure
  sleep_time = atoi(argv[3]); //Wait time between iterations (microseconds)

  n_bytes = pow(2, pow_2);
  n = n_bytes / sizeof(int);

  double bcast_time[max_iter];
  arr = new int[n];

  // Generate Data
  if (myid == 0)
  {
    for (i = 0; i < n; i++)
    { // generate random int data between 0 and 100
      arr[i] = rand() % 100;
    }
  }
  //iteration
  iter = 0;
  while (iter < max_iter)
  {
    //-------------------------------------------------------------------------Broadcast
    MPI_Barrier(MPI_COMM_WORLD);
    t1_b = MPI_Wtime();
    MPI_Bcast(arr, n, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);
    t2_b = MPI_Wtime();
    if (myid == 0)
    {
      bcast_time[iter] = ((t2_b - t1_b) * 1000);
    }
    
    usleep(sleep_time);
    iter++;
  } // end iteration

  if (myid == 0)
  {
    cout << "\nThe size of the data to broadcast: " << n_bytes << " Bytes";
    cout << "\nMean of communication times: " << Mean(bcast_time, max_iter) << "ms";
    cout << "\nMedian of communication times: " << Median(bcast_time, max_iter) << "ms";
    //Print_times(bcast_time, max_iter);
  } 

  free(arr);
  MPI_Finalize();
  return 0;
} // end main

double Mean(double a[], int n)
{
  double sum = 0.0;
  for (int i = 0; i < n; i++)
    sum += a[i];

  return (sum / (double)n);
}

double Median(double a[], int n)
{
  sort(a, a + n);
  if (n % 2 != 0)
    return a[n / 2];

  return (a[(n - 1) / 2] + a[n / 2]) / 2.0;
}

// void Print_times(double a[], int n)
// {
//   cout << "\n------------------------------------";
//   for (int t = 0; t < n; t++)
//     cout << "\n " << a[t];
//    cout << "\n ";
// }