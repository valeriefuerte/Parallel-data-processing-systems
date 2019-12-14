#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include "mpi.h"

#define N 4/* number of rows and columns in matrix */

main(int argc, char **argv)
{
  int ProcNum,rank,source,dest,rows,offset;

  struct timeval start, stop;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &ProcNum);
  MPI_Status status;

  double a[N][N],b[N][N],c[N][N];

  if (rank == 0) {
    for (int i=0; i<N; i++) {
      for (int j=0; j<N; j++) {
        a[i][j]= rand() % 10;
        b[i][j]= rand() % 10;
      }
    }

    gettimeofday(&start, 0);

    /* send matrix data to the worker tasks */
    rows = N/(ProcNum-1);
    offset = 0;

    for (dest=1; dest<=ProcNum-1; dest++)
    {
      MPI_Send(&offset, 1, MPI_INT, dest, 1, MPI_COMM_WORLD);
      MPI_Send(&rows, 1, MPI_INT, dest, 1, MPI_COMM_WORLD);
      MPI_Send(&a[offset][0], rows*N, MPI_DOUBLE,dest,1, MPI_COMM_WORLD);
      MPI_Send(&b, N*N, MPI_DOUBLE, dest, 1, MPI_COMM_WORLD);
      offset = offset + rows;
    }

    /* wait for results from all worker tasks */
    for (int i=1; i<=ProcNum-1; i++)
    {
      source = i;
      MPI_Recv(&offset, 1, MPI_INT, source, 2, MPI_COMM_WORLD, &status);
      MPI_Recv(&rows, 1, MPI_INT, source, 2, MPI_COMM_WORLD, &status);
      MPI_Recv(&c[offset][0], rows*N, MPI_DOUBLE, source, 2, MPI_COMM_WORLD, &status);
    }

    gettimeofday(&stop, 0);

    printf("The A matrix:\n");
    for (int i=0; i<N; i++) {
      for (int j=0; j<N; j++)
        printf("%6.2f   ", a[i][j]);
      printf ("\n");
    }

    printf("The B matrix:\n");
    for (int i=0; i<N; i++) {
      for (int j=0; j<N; j++)
        printf("%6.2f   ", b[i][j]);
      printf ("\n");
    }

    printf("The result matrix:\n");
    for (int i=0; i<N; i++) {
      for (int j=0; j<N; j++)
        printf("%6.2f   ", c[i][j]);
      printf ("\n");
    }

    fprintf(stdout,"Time = %.6f\n\n",
         (stop.tv_sec+stop.tv_usec*1e-6)-(start.tv_sec+start.tv_usec*1e-6));

  }

  /* Matrix multiplication */
  if (rank > 0) {
    source = 0;
    MPI_Recv(&offset, 1, MPI_INT, source, 1, MPI_COMM_WORLD, &status);
    MPI_Recv(&rows, 1, MPI_INT, source, 1, MPI_COMM_WORLD, &status);
    MPI_Recv(&a, rows*N, MPI_DOUBLE, source, 1, MPI_COMM_WORLD, &status);
    MPI_Recv(&b, N*N, MPI_DOUBLE, source, 1, MPI_COMM_WORLD, &status);

    for (int k=0; k<N; k++)
      for (int i=0; i<rows; i++) {
        c[i][k] = 0.0;
        for (int j=0; j<N; j++)
          c[i][k] = c[i][k] + a[i][j] * b[j][k];
      }


    MPI_Send(&offset, 1, MPI_INT, 0, 2, MPI_COMM_WORLD);
    MPI_Send(&rows, 1, MPI_INT, 0, 2, MPI_COMM_WORLD);
    MPI_Send(&c, rows*N, MPI_DOUBLE, 0, 2, MPI_COMM_WORLD);
  }

  MPI_Finalize();
}
