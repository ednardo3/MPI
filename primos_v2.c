#include <mpi.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#define BLOCO 500000
#define TAG_WORK 1
#define TAG_RESULT 2
#define TAG_END 3

int eh_primo(long x) {
    if (x < 2) return 0;
    for (long i = 2; i <= sqrt(x); i++)
        if (x % i == 0) return 0;
    return 1;
}

long conta_primos(long inicio, long fim) {
    long count = 0;
    for (long i = inicio; i <= fim; i++)
        if (eh_primo(i)) count++;
    return count;
}

int main(int argc, char *argv[]) {
    long N;
    int rank, size;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank == 0) {
        // -----------------------------
        // MESTRE
        // -----------------------------
        N = atol(argv[1]);

        long inicio = 2;
        long fim;
        long total = 0;
        long blocos_enviados = 0;

        double t0 = MPI_Wtime();

        // Envia um bloco inicial para cada trabalhador
        for (int worker = 1; worker < size; worker++) {
            if (inicio > N) break;

            fim = inicio + BLOCO - 1;
            if (fim > N) fim = N;

            MPI_Send(&inicio, 1, MPI_LONG, worker, TAG_WORK, MPI_COMM_WORLD);
            MPI_Send(&fim,    1, MPI_LONG, worker, TAG_WORK, MPI_COMM_WORLD);

            inicio = fim + 1;
            blocos_enviados++;
        }

        // Recebe resultados e envia novos blocos
        while (blocos_enviados > 0) {
            long resultado;
            MPI_Status status;

            MPI_Recv(&resultado, 1, MPI_LONG, MPI_ANY_SOURCE, TAG_RESULT, MPI_COMM_WORLD, &status);
            total += resultado;

            int worker = status.MPI_SOURCE;

            if (inicio <= N) {
                fim = inicio + BLOCO - 1;
                if (fim > N) fim = N;

                MPI_Send(&inicio, 1, MPI_LONG, worker, TAG_WORK, MPI_COMM_WORLD);
                MPI_Send(&fim,    1, MPI_LONG, worker, TAG_WORK, MPI_COMM_WORLD);

                inicio = fim + 1;
                blocos_enviados++;
            } else {
                long dummy = 0;
                MPI_Send(&dummy, 1, MPI_LONG, worker, TAG_END, MPI_COMM_WORLD);
                blocos_enviados--;
            }
        }

        double t1 = MPI_Wtime();
        printf("tempo: %lf\n", t1 - t0);
        printf("total de primos: %ld\n", total);
    }

    else {
        // -----------------------------
        // TRABALHADORES
        // -----------------------------
        while (1) {
            long inicio, fim;
            MPI_Status status;

            MPI_Recv(&inicio, 1, MPI_LONG, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

            if (status.MPI_TAG == TAG_END)
                break;

            MPI_Recv(&fim, 1, MPI_LONG, 0, TAG_WORK, MPI_COMM_WORLD, &status);

            long resultado = conta_primos(inicio, fim);

            MPI_Send(&resultado, 1, MPI_LONG, 0, TAG_RESULT, MPI_COMM_WORLD);
        }
    }

    MPI_Finalize();
    return 0;
}

