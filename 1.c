#include "mpi.h"
#include <stdio.h>

int main(int argc, char* argv[]){
  int ProcNum, ProcRank, RecvRank;
  MPI_Status Status;
  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &ProcNum);
  MPI_Comm_rank(MPI_COMM_WORLD, &ProcRank);
  for ( int i=1; i < ProcNum; i++ ) {
    if ( ProcRank == 0 ){ //Действия, выполняемые только процессом с рангом 0
      printf ("Message is sent from process %3d\n", ProcRank);   
      MPI_Recv(&RecvRank, 1, MPI_INT, i, MPI_ANY_TAG, MPI_COMM_WORLD, &Status);//add proc rang of sender i
      printf("Message is received by process %3d\n", RecvRank);     
    }
    else {// Сообщение, отправляемое всеми процессами, кроме процесса с рангом 0
      MPI_Send(&ProcRank, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }
  }
  MPI_Finalize();
return 0;
}
