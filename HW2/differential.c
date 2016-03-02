/* Reaction-diffusion equation in 1D
* Solution by Jacobi iteration
* simple MPI implementation
*
* C Michael Hanke 2006-12-12
*/

#define MIN(a,b) ((a) < (b) ? (a) : (b))

/* Use MPI */
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

/* define problem to be solved */
#define N 1000   /* number of inner grid points */
#define SMX 1000000 /* number of iterations */
#define PI 3.14159265
#define filename "u_result_3.txt"

/* implement coefficient functions */
double r(const double x)
{
  // Create a function of x
  // return - x * x; // 1
  // return - pow(x, 4) - 3; // 2
  return - pow(x, 4) - 3; // 3
}


double f(const double x)
{
  // Create a function of x
  // return x * x + 3 * x + 34; // 1
  // return -108 * PI * PI * sin(6 * PI * x) - (pow(x, 4) + 3) * 3 * sin(6 * PI * x); // 2
  return -100000 * PI * PI * sin(100 * PI * x) - (pow(x, 4) + 3) * 10 * sin(100 * PI * x); // 3
}

/* We assume linear data distribution. The formulae according to the lecture
  are:
  L = N / P;
  R = N % P;
  I = (N + P - p - 1) / P;    (number of local elements)
  n = p * L + MIN(p, R) + i; (global index for given (p, i)
  Attention: We use a small trick for introducing the boundary conditions:
  - The first ghost point on p = 0 holds u(0);
  - the last ghost point on p = P-1 holds u(1).
  Hence, all local vectors hold I elements while u has I + 2 elements.
*/

int main(int argc, char *argv[])
{
  /* local variables */
  int P, p, I, step, L, R, i, msg = 1, n;
  double *unew, *u, h, *rr, *ff;
  FILE *fp;

  h = 1.0 / (N + 1.0);

  /* Initialize MPI */
  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &P);
  MPI_Comm_rank(MPI_COMM_WORLD, &p);

  if (N < P)
  {
    fprintf(stdout, "Too few discretization points...\n");
    exit(1);
  }

  /* Compute local indices for data distribution */
  L = N / P;
  R = N % P;
  I = (N + P - p - 1) / P;

  /* arrays */
  unew = (double *) malloc(I * sizeof(double));
  /* Note: The following allocation includes additionally:
      - boundary conditions are set to zero
      - the initial guess is set to zero */
  u = (double *) calloc(I + 2, sizeof(double));
  rr = (double *) malloc(I * sizeof(double));
  ff = (double *) malloc(I * sizeof(double));

  for (i = 0; i < I; i++)
  {
    n = p * L + MIN(p, R) + i;
    rr[i] = r(n * h);
    ff[i] = f(n * h);
  }

  /* Jacobi iteration */
  for (step = 0; step < SMX; step++)
  {
    /* RB communication of overlap */
    if (p % 2 == 0)
    {
      // RED!!!
      if (p != P - 1)
      {
        MPI_Send(&unew[I - 2], 1, MPI_DOUBLE, p + 1, 0, MPI_COMM_WORLD);
        MPI_Recv(&u[I + 1], 1, MPI_DOUBLE, p + 1, 0, MPI_COMM_WORLD,
         MPI_STATUS_IGNORE);
      }
      if (p != 0)
      {
        MPI_Send(&unew[1], 1, MPI_DOUBLE, p - 1, 0, MPI_COMM_WORLD);
        MPI_Recv(&u[0], 1, MPI_DOUBLE, p - 1, 0, MPI_COMM_WORLD,
         MPI_STATUS_IGNORE);
      }
    }
    else
    {
      // BLACK!!!
      MPI_Recv(&u[0], 1, MPI_DOUBLE, p - 1, 0, MPI_COMM_WORLD,
       MPI_STATUS_IGNORE);
      MPI_Send(&unew[1], 1, MPI_DOUBLE, p - 1, 0, MPI_COMM_WORLD);
      if (p != P - 1)
      {
        MPI_Recv(&u[I + 1], 1, MPI_DOUBLE, p + 1, 0, MPI_COMM_WORLD,
         MPI_STATUS_IGNORE);
        MPI_Send(&unew[I - 2], 1, MPI_DOUBLE, p + 1, 0, MPI_COMM_WORLD);
      }
    }

    /* local iteration step */
    for (i = 0; i < I; i++)
    {
      unew[i] = (u[i] + u[i + 2] - h * h * ff[i]) / (2.0 - h * h * rr[i]);
    }
    memcpy(u + 1, unew, I * sizeof(double));
  }

  /* output for graphical representation */
  /* Instead of using gather (which may lead to excessive memory requirements
    on the master process) each process will write its own data portion. This
    introduces a sequentialization: the hard disk can only write (efficiently)
    sequentially. Therefore, we use the following strategy:
    1. The master process writes its portion. (file creation)
    2. The master sends a signal to process 1 to start writing.
    3. Process p waits for the signal from process p-1 to arrive.
    4. Process p writes its portion to disk. (append to file)
    5. process p sends the signal to process p+1 (if it exists).
  */

  if (p == 0)
  {
    // Create file
    fp = fopen(filename, "w");

    // Write to file
    // u(0) = 0
    fprintf(fp, "0 ");
    for (i = 0; i < I; i++)
    {
      fprintf(fp, "%f ", unew[i]);
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

    // Write to file
    for (i = 0; i < I; i++)
    {
      fprintf(fp, "%f ", unew[i]);
    }

    // Send signal to next process
    if (p != P - 1)
    {
      MPI_Send(&msg, 1, MPI_INT, p + 1, 0, MPI_COMM_WORLD);
    }
    else
    {
      // u(1) = 0
      fprintf(fp, "0");
    }

    // Close file
    fclose(fp);
  }

  /* That's it */
  MPI_Finalize();
  exit(0);
}