/*
This simple program, generates required size of data and does the broadcast in an iteration.
To run: mpirun -n nprocs a.out data_size iterations delay_between_iterations(us)
Note: data_size will be in powers of 2. you just need to put power. for example if data_size=5, it will be 2^5 bytes. 
*/
#include <stdio.h>
#include <malloc.h>
#include <time.h>
#include <mpi.h>
#include <unistd.h>
#include <math.h>
#include <stdlib.h>
#include <ctype.h>


double  Mean(double[], int);
double  Median(double[], int);
void quickSort(double[], int lowIndex, int highIndex);
int partition (double arr[], int lowIndex, int highIndex);
void swapElements(double* x, double* y);
void printArray(int arr[], int size);
void ring(int arr[], int n, int myid, int procs);
void binary_tree(int arr[], int n, int myid, int procs);
void binomial_tree(int arr[], int n, int myid, int procs);
void linear_pipeline_broadcast(int arr[], int n, int myid, int procs);
void binomial_pipeline_broadcast(int arr[], int n, int myid, int procs);
void check_broadcast(int arr[], int n, int myid, int procs);
int largest_divisor(int n);

//void    Print_times(double[], int);

int main(int argc, char **argv)
{
  int i, procs, myid, *arr, n, err, max_iter ,iter = 0, choiche=0;
  long n_bytes, sleep_time;
  int pow_2;
  double t1_b, t2_b;
  // initialize MPI_Init
  err = MPI_Init(&argc, &argv);
  if (err != MPI_SUCCESS)
  {
    printf("\nError initializing MPI.\n");
    MPI_Abort(MPI_COMM_WORLD, err);
  }

  MPI_Comm_size(MPI_COMM_WORLD, &procs);
  MPI_Comm_rank(MPI_COMM_WORLD, &myid);

 
  if (argc < 5)
  {
    if (myid == 0)
    {
      printf("\n Invalid Number of Arguements.\n");
    }
    MPI_Finalize();
    return 0;
  }
  

  pow_2 = atoi(argv[1]);      //size of the data to broadcast: n_bytes= 2^pow_2
  max_iter = atoi(argv[2]);   //number of iterations to repeat the procedure
  sleep_time = atoi(argv[3]); //Wait time between iterations (microseconds)
  choiche = atoi(argv[4]);

  n_bytes = pow(2, pow_2);
  n = n_bytes / sizeof(int);

  double bcast_time[max_iter];
  arr = malloc(sizeof(int)*n);
  // Generate Data
  if (myid == 0)
  {
    for (i = 0; i < n; i++)
    { // generate random int data between 0 and 99
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
    //check_broadcast(arr, n, myid,procs);
    switch(choiche)
    {
      case 0:
        MPI_Bcast(arr, n, MPI_INT, 0, MPI_COMM_WORLD); break;
      case 1:
        ring(arr, n, myid,procs); break;
      case 2:
        binomial_tree(arr,n,myid,procs); break;
      case 3:
        binary_tree(arr,n,myid,procs); break;
      case 4:
        linear_pipeline_broadcast(arr,n,myid,procs); break;
      case 5:
        binomial_pipeline_broadcast(arr,n,myid,procs);  break;
      default: 
        if(myid == 0)
          printf("Wrong choice, you must pick a number from 0 to 5\n");
          free(arr);
          MPI_Finalize();
          return 0;
        break;
    }
      

    
    
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
    printf("\nThe size of the data to broadcast: %ld ",n_bytes);
    printf("\nMean of communication times: %f ", Mean(bcast_time, max_iter));
    printf("\nMedian of communication times: %f ", Median(bcast_time, max_iter));
    printf("\n");
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
  quickSort(a,0, n-1);
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
void swapElements(double* x, double* y)
{ 
  double temp = *x; 
  *x = *y; 
  *y = temp; 
}   
// Partition function
int partition (double arr[], int lowIndex, int highIndex) 
{ 
    int pivotElement = arr[highIndex];
    int i = (lowIndex - 1); 
    for (int j = lowIndex; j <= highIndex- 1; j++) 
    { 
    if (arr[j] <= pivotElement) 
    { 
    i++; 
    swapElements(&arr[i], &arr[j]); 
    } 
} 
  swapElements(&arr[i + 1], &arr[highIndex]); 
  return (i + 1); 
}   
// QuickSort Function
void quickSort(double arr[], int lowIndex, int highIndex) 
{ 
  if (lowIndex < highIndex) 
  { 
    int pivot = partition(arr, lowIndex, highIndex); 
    quickSort(arr, lowIndex, pivot - 1); 
    quickSort(arr, pivot + 1, highIndex); 
  } 
}   
// Function to print array
void printArray(int arr[], int size) 
{ 
  int i; 
  for (i=0; i < size; i++) 
  printf("%d ", arr[i]); 
}

void ring(int arr[], int n, int myid, int procs)
{
    MPI_Status status;
    if(myid == 0)
    {
      MPI_Send(arr, n, MPI_INT, myid+1, 0, MPI_COMM_WORLD);
    }
     
    else if(myid == procs-1)
      MPI_Recv(arr, n, MPI_INT, myid-1, 0, MPI_COMM_WORLD, &status);
    else
    {
      MPI_Recv(arr, n, MPI_INT, myid-1, 0, MPI_COMM_WORLD, &status);
      MPI_Send(arr, n, MPI_INT, myid+1, 0, MPI_COMM_WORLD);
    }
}

void binary_tree(int arr[], int n, int myid, int procs)
{
    MPI_Status status;
    if(myid == 0)
    {
      if(procs < 3)
      {
        MPI_Send(arr, n, MPI_INT, myid+1, 0, MPI_COMM_WORLD);
      }
      else
      {
        MPI_Send(arr, n, MPI_INT, myid+1, 0, MPI_COMM_WORLD);
        MPI_Send(arr, n, MPI_INT, myid+2, 0, MPI_COMM_WORLD);
      }
    }
    else if(myid > procs - 3 && myid < procs)
    {
      
      MPI_Recv(arr, n, MPI_INT, (myid-1)/2, 0, MPI_COMM_WORLD, &status);

    }
    else
    {
      MPI_Recv(arr, n, MPI_INT, (myid-1)/2, 0, MPI_COMM_WORLD, &status);
      if(myid*2+1 < procs)
        MPI_Send(arr, n, MPI_INT, myid*2+1, 0, MPI_COMM_WORLD);
      if(myid*2+2 <procs)
        MPI_Send(arr, n, MPI_INT, myid*2+2, 0, MPI_COMM_WORLD);
      
    }
}
void binomial_tree(int arr[], int n, int myid, int procs)
{
  int recv=-1;
  int done=-1;
  int send=-1;
  MPI_Status status;
  for(int i=(int)log2(procs);i>=0;i--)
      {
        if(myid != 0)
        {
          if(myid % (int)pow(2,i)== 0)
          {
            if(recv == -1)
            {
              recv=myid-pow(2,i);
              
            }
              
          }
        }
      }
    
      for(int i=(int)log2(procs)-1;i>=0;i--)
      {
        
        if(myid == 0)
          {
            send=(int)pow(2,i)+myid;
            
            MPI_Send(arr, n, MPI_INT,send, 0, MPI_COMM_WORLD);
            if(!(log2(procs) == (int)log2(procs)) && i==(int)log2(procs)-1)
              {
                int initial= (int)pow(2,(int)log2(procs));
                
                  
                  MPI_Send(arr, n, MPI_INT,initial, 0, MPI_COMM_WORLD);
               
              }
          
          }
          else
          {
            
            if(done == -1)
            {
            
              done=1;
              MPI_Recv(arr, n, MPI_INT, recv , 0, MPI_COMM_WORLD, &status);
           
            }
            
            if(myid % (int)pow(2,i+1)== 0)
            {
              send=(int)pow(2,i)+myid;
              
              if(myid != procs-1 && send < procs)
              {
                MPI_Send(arr, n, MPI_INT, send, 0, MPI_COMM_WORLD);
              }
             
            }
        
          }
      }
}

void linear_pipeline_broadcast(int arr[], int n, int myid, int procs)
{
    int logbase=(int)largest_divisor(n);
     MPI_Status status;
     
    if(myid == 0)
    {
     
      for(int i=0;i<n;i=i+logbase)
      {
        MPI_Send(arr+i, logbase, MPI_INT, myid+1, 0, MPI_COMM_WORLD);
      }
      
    }
     
    else if(myid == procs-1)
    {
      
      for(int i=0;i<n;i=i+logbase)
      {
        MPI_Recv(arr+i, logbase, MPI_INT, myid-1, 0, MPI_COMM_WORLD, &status);
        
      }
   
    }
      
    else
    {
      
      for(int i=0;i<n;i=i+logbase)
      {
        
        MPI_Recv(arr+i, logbase, MPI_INT, myid-1, 0, MPI_COMM_WORLD, &status);
        
        MPI_Send(arr+i, logbase, MPI_INT, myid+1, 0, MPI_COMM_WORLD);
      }
       
    }
}

void binomial_pipeline_broadcast(int arr[], int n, int myid, int procs)
{
 int recv=-1;
  int done=-1;
  int send=-1;
   int logbase=(int)largest_divisor(n);
  MPI_Status status;
  for(int i=(int)log2(procs);i>=0;i--)
      {
        if(myid != 0)
        {
          if(myid % (int)pow(2,i)== 0)
          {
            if(recv == -1)
            {
              recv=myid-pow(2,i);
              
            }
              
          }
        }
      }
    
      for(int i=(int)log2(procs)-1;i>=0;i--)
      {
        
        if(myid == 0)
          {
            send=(int)pow(2,i)+myid;
            for(int i=0;i<n;i=i+logbase)
            { 
              MPI_Send(arr+i, logbase, MPI_INT,send, 0, MPI_COMM_WORLD);
            }
            if(!(log2(procs) == (int)log2(procs)) && i==(int)log2(procs)-1)
              {
                int initial= (int)pow(2,(int)log2(procs));
                
                  for(int i=0;i<n;i=i+logbase)
                  { 
                    MPI_Send(arr+i, logbase, MPI_INT,initial, 0, MPI_COMM_WORLD);
                  }
               
              }
          
          }
          else
          {
            
            if(done == -1)
            {
         
              done=1;
              for(int i=0;i<n;i=i+logbase)
                { 
                  MPI_Recv(arr+i, logbase, MPI_INT, recv , 0, MPI_COMM_WORLD, &status);
                }
            
            }
            
            if(myid % (int)pow(2,i+1)== 0)
            {
              send=(int)pow(2,i)+myid;
              
              if(myid != procs-1  && send < procs)
              {
                
                for(int i=0;i<n;i=i+logbase)
                { 
                  MPI_Send(arr+i, logbase, MPI_INT, send, 0, MPI_COMM_WORLD);
                }
              }
             
            }
            
          }
      }
}

int largest_divisor(int n) 
{

	int i;
  int which=1;
	for (i = n - 1; i >= 1; --i) {
		if (n % i == 0)
      if(which > 0)
        which=which-1;
      else
			  return i;
	}
	return 1;

}

void check_broadcast(int arr[], int n, int myid, int procs)
{
  int *arr2;
  int a = 0;
  MPI_Bcast(arr, n, MPI_INT, 0, MPI_COMM_WORLD);
  arr2 = malloc(sizeof(int)*n);
  if(myid==0)
  {
    for(int i=0;i<n;i++)
    {
      arr2[i]=arr[i];
        
    }
  }
  //ring(arr2,n,myid,procs);
  //binomial_tree(arr2,n,myid,procs);
  binary_tree(arr2,n,myid,procs);
  //linear_pipeline_broadcast(arr2,n,myid,procs);
  //binomial_pipeline_broadcast(arr2,n,myid,procs);
  for(int i=0;i<n;i++)
  {
    if(arr[i]!=arr2[i])
    {
        a=1;
    }
      
  }
  if(a==1)
    printf("Matrices are different!\n");
  else
    printf("Matrices are the same\n");

}

