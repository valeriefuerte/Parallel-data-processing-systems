#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

// #define N 10// array size
// int A[N] = {0,2,1,5,4,3,7,6,8,9}; // this is a dummy array that should only be initialized on rank == ROOT

int handling(int *array, int count)
{
	return array[0];
}

int main(int argc, char *argv[]) {

	int size;
	int rank;
	const int VERY_LARGE_INT = 999999;
	int tag = 1;

	int N = 10;
	int A[N];
	for(int i = 0; i < N; i++){
	    A[i] = rand() % 10;
        // printf("%d", A[i]);
    }
    // printf("\n");

	MPI_Init(&argc, &argv);

	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	int m = 0, k = N - 1;
	int n;

	int *array = (int *)malloc(N * sizeof(int));
	int after_handling = 0;
	int result[size];

	if (rank == 0) {
		if (size == 2) {
			for(int dest = 1; dest < size; ++dest)
			{
				MPI_Send(&A[(dest-1)*k], k, MPI_INT, dest, tag, MPI_COMM_WORLD);
				printf("P0 sent %d elements to P%d.\n", k, dest);
			}
		}
		else
			for(int dest = 1; dest < size; ++dest)
			{
				n = m + (k-1)/2; //кусок массива, который отрезается
				// printf("m: %d\n (k+1)/2: %d\n", m, n);
				k = N - n; //k - то, что осталось
				m = n + 1; //m - указывает на начало передаваемой части				
				MPI_Send(&A[(dest-1)*k], k, MPI_INT, dest, tag, MPI_COMM_WORLD);
				printf("P0 sent %d elements to P%d.\n", k, dest);
			}
	}

	/* Every other rank is receiving one message: from 0 into array */
	else {
		MPI_Recv(array, k, MPI_INT, MPI_ANY_SOURCE, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		after_handling = handling(array, m);
		// printf("Smth from P%d is %d.\n", rank, after_handling);
		MPI_Send(&after_handling, 1, MPI_INT, 0, tag, MPI_COMM_WORLD);
	}
	
	MPI_Status Status;
	if(rank == 0)
	{
		int tmp;
		for(int i = 1; i < size; i++){
			MPI_Recv(&tmp, 1, MPI_INT, MPI_ANY_SOURCE, tag, MPI_COMM_WORLD, &Status);
			int index = Status.MPI_SOURCE;
			result[index] = tmp;
		}
		printf("Result:\n");
		for(int i = 1; i < size; i++){
			printf("%d\n", result[i]);
		}
	}

	MPI_Finalize();
	return 0;
}