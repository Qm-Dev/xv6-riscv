#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[]) {

    // Sintaxis correcta
    if (argc != 2) {
        printf("Uso correcto: %s <cantidad_procesos>\n", argv[0]);
        exit(1);
    }

    int cantidad_procesos = atoi(argv[1]);

    // Creación de procesos hijos (forks)
    for (int i = 0; i < cantidad_procesos; i++) {
        int val_fork = fork();

        if (val_fork < 0) {
            printf("Error: No se ha logrado realizar el fork correctamente.\n");
            exit(1);
        }

        else if (val_fork == 0) {
            sleep(i);
            printf("Ejecutando proceso %d (ID: %d, P: %d, B: %d)\n", i, getpid(), getpriority(), getboost());
            sleep(10);
            exit(0);
        }
    }

    // Esperar a que todos los procesos hijos terminen
    for (int i = 0; i < 20; i++) {
        wait(0);
    }

  exit(0);

}