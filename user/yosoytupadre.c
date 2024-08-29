#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
    // printf("ID proceso: %d\n", getpid());
    printf("ID padre: %d\n", getppid());
    exit(0);
}