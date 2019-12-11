#include "mpi.h"
#include <stdio.h>

int main(int argc, char* argv[]){
  int ProcNum, ProcRank, RecvRank, dest, source, count, tag = 1;
  MPI_Status Status;
  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &ProcNum);
  MPI_Comm_rank(MPI_COMM_WORLD, &ProcRank);
  // printf("%d\n", ProcNum);
  // for ( int i=1; i < ProcNum; i++ ) {
    // printf("%d %d\n", ProcRank, i);
    if ( ProcRank == 0 ){ //Действия, выполняемые только процессом с рангом 0
      // dest = 1;
      // source = 1;
      for ( int i=1; i < ProcNum; i++ ) {
         MPI_Send(&ProcRank, 1, MPI_INT, i, tag, MPI_COMM_WORLD);
         MPI_Recv(&RecvRank, 1, MPI_INT, i, tag, MPI_COMM_WORLD, &Status);
         printf("Task received from %d to %d \n", ProcRank, RecvRank);
       }
    }
    else {// Сообщение, отправляемое всеми процессами, кроме процесса с рангом 0
      MPI_Recv(&RecvRank, 1, MPI_INT, 0, tag, MPI_COMM_WORLD, &Status);
      MPI_Send(&ProcRank, 1, MPI_INT, 0, tag, MPI_COMM_WORLD);
      printf("Task received from %d to %d \n", ProcRank, RecvRank);
    }
    // printf("Task received from %d to %d \n", ProcRank, RecvRank);
  // }
  MPI_Finalize();
return 0;
}
