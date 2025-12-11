#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

int primo(long int n) {
    long int i;

    if (n < 2) return 0;
    if (n == 2) return 1;
    if (n % 2 == 0) return 0;

    for (i = 3; i <= (long int) sqrt((double)n); i += 2) {
        if (n % i == 0) return 0;
    }
    return 1;
}

int main(int argc, char *argv[]) {
    int rank, size;
    long int n;
    long int i;
    int local_total = 0;
    int global_total = 0;
    double inicio, fim;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Processo 0 lê o valor de N
    if (rank == 0) {
        if (argc < 2) {
            n = 0;
        } else {
            n = strtol(argv[1], NULL, 10);
        }
    }

    // Envia N para todos os processos
    MPI_Bcast(&n, 1, MPI_LONG, 0, MPI_COMM_WORLD);

    if (n <= 0) {
        MPI_Finalize();
        return 0;
    }

    MPI_Barrier(MPI_COMM_WORLD);
    inicio = MPI_Wtime();

    // Processo 0 conta o 2
    if (rank == 0 && n >= 2) {
        local_total = 1;
    }

    // Cada processo começa em um ímpar diferente
    long int first = 3 + 2L * rank;
    long int step  = 2L * size;

    for (i = first; i <= n; i += step) {
        if (primo(i)) local_total++;
    }

    // Soma todos os resultados no processo 0
    MPI_Reduce(&local_total, &global_total, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    fim = MPI_Wtime();

    // ✅ Imprime SOMENTE o tempo
    if (rank == 0) {
        printf("tempo: %.6f\n", fim - inicio);
    }

    MPI_Finalize();
    return 0;
}

