#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
    // Corregir sintaxis
    if (argc != 2) {
        printf("Uso correcto: ancestros <int>\n");
        exit(1);
    }
    else {
        int pid = atoi(argv[1]);
        printf("ID Proceso: %d\n", getancestor(pid));
        exit(0);
    }
}