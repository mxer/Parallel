#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define filename "sorted_list.txt"

int cmpfunc (const void * a, const void * b)
{
  double da = *(double*) a, db = *(double*) b;
  if (da > db)
    return 1;
  else if (da < db)
    return -1;
  else
    return 0;
}

double* merge(double* A, double* B, int sizeA, int sizeB)
{
  double* list;
  list = (double *) malloc((sizeA + sizeB) * sizeof (double));

  int a = 0, b = 0, idx = 0;

  while (a < sizeA && b < sizeB)
  {
    if (A[a] < B[b])
    {
      list[idx++] = A[a++];
    }
    else
    {
      list[idx++] = B[b++];
    }
  }

  while (a < sizeA)
  {
    list[idx++] = A[a++];
  }

  while (b < sizeB)
  {
    list[idx++] = B[b++];
  }

  return list;
}


int main(int argc, char **argv)
{
  // Local variables
  int P, p, N, I, i, step, evenphase, *Ip, msg = 1;
  double *A, *B, start_time, end_time;
  FILE *fp;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &P);
  MPI_Comm_rank(MPI_COMM_WORLD, &p);

  /* Find problem size N from command line */
  if (argc < 2)
  {
    fprintf(stdout, "No size N given");
    exit(1);
  }
  N = atoi(argv[1]);

  start_time = MPI_Wtime();

  /* local size. Modify if P does not divide N */
  Ip = (int *) malloc(P * sizeof (int)); // Number of elements of every processor
  for (i = 0; i < P; i++)
  {
    Ip[i] = (N + P - i - 1) / P;
  }
  I = Ip[p]; // Number of elements for current processor

  /* random number generator initialization */
  srandom(p + 1);

  /* data generation */
  if (p % 2 == 0)
  {
    // Even process - Generate B
    B = (double *) malloc(I * sizeof(double));
    for (i = 0; i < I; i++)
    {
      B[i] = ((double) random())/((double) RAND_MAX + 1);
    }
    // Sort B
    qsort(B, I, sizeof(double), cmpfunc);
  }
  else
  {
    // Odd process - Generate A
    A = (double *) malloc(I * sizeof(double));
    for (i = 0; i < I; i++)
    {
      A[i] = ((double) random())/((double) RAND_MAX + 1);
    }
    // Sort A
    qsort(A, I, sizeof(double), cmpfunc);
  }


  // Start odd-even sort
  evenphase = 1;
  for (step = 0; step < P; step++)
  {
    if (p % 2 == 0)
    {
      // Even process
      if (evenphase)
      {
        // Even phase
        if (p < P - 1)
        {
          // Allocate A to receive
          A = (double *) malloc(Ip[p + 1] * sizeof(double));
          MPI_Recv(A, Ip[p + 1], MPI_DOUBLE, p + 1, 0, MPI_COMM_WORLD,
           MPI_STATUS_IGNORE);
          MPI_Send(B, I, MPI_DOUBLE, p + 1, 0, MPI_COMM_WORLD);

          // Merge A and B
          double *list;
          list = (double *) malloc((I + Ip[p + 1]) * sizeof(double));
          list = merge(A, B, Ip[p + 1], I);

          // Keep first half of list
          memcpy(B, list, I * sizeof(double));
        }
      }
      else
      {
        // Odd phase
        if (p > 0)
        {
          // Allocate A to receive
          A = (double *) malloc(Ip[p - 1] * sizeof(double));
          MPI_Recv(A, Ip[p - 1], MPI_DOUBLE, p - 1, 0, MPI_COMM_WORLD,
           MPI_STATUS_IGNORE);
          MPI_Send(B, I, MPI_DOUBLE, p - 1, 0, MPI_COMM_WORLD);

          // Merge A and B
          double *list;
          list = (double *) malloc((I + Ip[p - 1]) * sizeof(double));
          list = merge(A, B, Ip[p - 1], I);

          // Keep second half of list
          memcpy(B, list + Ip[p - 1], I * sizeof(double));
        }
      }

    }
    else
    {
      // Odd process
      if (evenphase)
      {
        // Even phase
        // Allocate to receive B
        B = (double *) malloc(Ip[p - 1] * sizeof(double));
        MPI_Send(A, I, MPI_DOUBLE, p - 1, 0, MPI_COMM_WORLD);
        MPI_Recv(B, Ip[p - 1], MPI_DOUBLE, p - 1, 0, MPI_COMM_WORLD,
         MPI_STATUS_IGNORE);

        // Merge A and B
        double *list;
        list = (double *) malloc((I + Ip[p - 1]) * sizeof(double));
        list = merge(A, B, I, Ip[p - 1]);

        // Keep second half of list
        memcpy(A, list + Ip[p - 1], I * sizeof(double));
      }
      else
      {
        // Odd phase
        if (p < P - 1)
        {
          // Allocate to receive B
          B = (double *) malloc(Ip[p + 1] * sizeof(double));
          MPI_Send(A, I, MPI_DOUBLE, p + 1, 0, MPI_COMM_WORLD);
          MPI_Recv(B, Ip[p + 1], MPI_DOUBLE, p + 1, 0, MPI_COMM_WORLD,
           MPI_STATUS_IGNORE);

          // Merge A and B
          double *list;
          list = (double *) malloc((I + Ip[p + 1]) * sizeof(double));
          list = merge(A, B, I, Ip[p + 1]);

          // Keep first half of list
          memcpy(A, list, I * sizeof(double));
        }
      }

    }
    evenphase = !evenphase;
  }

  /// Print lists
  if (p == 0)
  {
    // Create file
    fp = fopen(filename, "w");

    for (i = 0; i < I; i++)
    {
      fprintf(fp, "%f\n", B[i]);
    }

    // Close file
    fclose(fp);

    // Send signal to next process
    if (P > 1)
    {
      MPI_Send(&msg, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
    }
  }
  else
  {
    // Wait for signal from previous process
    MPI_Recv(&msg, 1, MPI_INT, p - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    // Open file to append
    fp = fopen(filename, "a");

    for (i = 0; i < I; i++)
    {
      if (p % 2 == 0)
      {
        fprintf(fp, "%f\n", B[i]);
      }
      else
      {
        fprintf(fp, "%f\n", A[i]);
      }
    }

    // Send signal to next process
    if (p != P - 1)
    {
      MPI_Send(&msg, 1, MPI_INT, p + 1, 0, MPI_COMM_WORLD);
    }

    // Close file
    fclose(fp);
  }


  end_time = MPI_Wtime();

  if (p == 0) {
    printf ("runtime: %e on %d processors\n",end_time - start_time, P);
  }
  /* That's it */
  MPI_Finalize();
  exit(0);
}