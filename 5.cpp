#include <mpi.h>
#include <iostream>

#include "matrix_functions.h"

int ProcNum; 
int ProcRank;
int matrix_size = 3;
double *A;
double *B;
double *C;

double multiply_matrix(double *A, double *B,  double *C, int size) //Parallel Ribbonn algo
{
    double *bufA, *bufB, *bufC;
    int new_size = size;

    MPI_Status Status;

    int proc_size = new_size/ProcNum; //process part size
    int proc_elem = proc_size*new_size; //process part element

    bufA = new double[proc_elem];
    bufB = new double[proc_elem];
    bufC = new double[proc_elem];

    for (int i = 0; i < proc_elem; i++)
    {
        bufC[i] = 0;
    }
        
    double start_time = MPI_Wtime(); //возвращает количество секунд, представляя полное время по отношению к некоторому моменту времени в прошлом. 
    MPI_Scatter(A, proc_elem, MPI_DOUBLE, bufA, proc_elem, MPI_DOUBLE, 0, MPI_COMM_WORLD); //рассылка
    MPI_Scatter(B, proc_elem, MPI_DOUBLE, bufB, proc_elem, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    int NextProc = ProcRank + 1;
    if ( ProcRank == ProcNum - 1 ) NextProc = 0;

    int PrevProc = ProcRank - 1;
    if ( ProcRank == 0 ) PrevProc = ProcNum - 1;

    int PrevNum = ProcRank;
    for (int p = 0; p < ProcNum; p++)
    {
        for (int i = 0; i < proc_size; i++)
        {
            for (int j = 0; j < size; j++)
            {
                double tmp = 0;
                for (int k = 0; k < proc_size; k++)
                    tmp += bufA[PrevNum * proc_size + i * size + k] * bufB[k * size + j];
                bufC[i * size + j] += tmp;
            }
        }

        PrevNum -= 1;

        if (PrevNum < 0)
            PrevNum = ProcNum - 1;

        //Совмещенные прием и передача
        MPI_Sendrecv_replace(bufB, proc_elem, MPI_DOUBLE, NextProc, 0, PrevProc, 0, MPI_COMM_WORLD, &Status);
    }

    MPI_Gather(bufC, proc_elem, MPI_DOUBLE, C, proc_elem, MPI_DOUBLE, 0, MPI_COMM_WORLD); //сборка данных
    
    double end_time = MPI_Wtime();
    return end_time - start_time;
}

void InitProcess (double* &A,double* &B,double* &C ,int &Size) {
    MPI_Comm_size(MPI_COMM_WORLD, &ProcNum);
    MPI_Comm_rank(MPI_COMM_WORLD, &ProcRank);

    //Broadcasts a message from the process with rank "root" to all other processes of the communicator
    MPI_Bcast(&Size, 1, MPI_INT, 0, MPI_COMM_WORLD);
    
    if (ProcRank == 0) {
        A = new double [Size*Size];
        B = new double [Size*Size];
        C = new double [Size*Size];
        RandInit (A, Size); RandInit (B, Size);
    }
}


int main(int argc, char **argv) {
    double beg, end, serial;

    MPI_Init (&argc, &argv);  
    InitProcess (A, B, C, matrix_size);
    
    double parallel = multiply_matrix(A, B, C, matrix_size);

    if (ProcRank == 0) 
    {
        printf("Parallel algorithm: %f\n", parallel);
         
        double sequential = matrix_s_multiply(A, B, C, matrix_size); //Sequential matrix multiplication
        printf("Sequential algorithm: %f\n", sequential);
        
        printMatrixes(A, B, C, matrix_size);
    }
    MPI_Finalize();
    delete [] A;
    delete [] B;
    delete [] C;  
}
