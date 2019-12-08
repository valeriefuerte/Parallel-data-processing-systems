#include <stdio.h>
#include "mpi.h"
#include <stdlib.h>

int main(int argc, char *argv[])
{
    int result, sum = 0;
    int ProcNum, ProcRank;

    int m = 3;
    int n = 3;
    int array[m][n];
    for(int i = 0; i < m; i++){
    	for(int j = 0; j < n; j++){
    	    array[i][j] = rand() % 10;
            printf("%d ", array[i][j]);
    	}
        printf("\n");
    }

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD,&ProcNum);
    MPI_Comm_rank(MPI_COMM_WORLD,&ProcRank); 

    int step = m*n/ProcNum;
    int* pointer = (*array + ProcRank*step);
    int Num = step;
    if(ProcRank == ProcNum - 1){
        Num = Num + m*n % ProcNum;
    }
    for(int i = 0; i < Num; i++){
	   sum += pointer[i];
    }

    MPI_Reduce(&sum, &result, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    if (ProcRank == 0)
    {   
        printf("Result: %d\n", result);    
    }

    MPI_Finalize();
    return 0;
}