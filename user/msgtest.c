#define MSG_SIZE 128
#define MAX_ITER 10

#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

// Convertir entero a string
int itoa(int n, char *s) {
    int i, sign;
    if ((sign = n) < 0) n = -n;
    i = 0;
    do {
        s[i++] = n % 10 + '0';
    } while ((n /= 10) > 0);
    if (sign < 0) s[i++] = '-';
    s[i] = '\0';
    return i;
}

// Copiar un string a otro
void local_strcpy(char *dest, char *src) {
    while ((*dest++ = *src++) != '\0');
}

// Concatenar strings
void local_strcat(char *dest, char *src) {
    while (*dest != '\0') dest++;
    while ((*dest++ = *src++) != '\0');
}

// Lógica del escritor
int writer(void) {
    for (int i = 0; i < MAX_ITER; i++) {
        sleep(i+1);
        char msg[MSG_SIZE];
        char num[12];

        // Crear el mensaje
        itoa(i, num);
        local_strcpy(msg, "Mensaje N°");
        local_strcat(msg, num);

        if (send(getpid(), msg) < 0) {
            printf("Ocurrió un error al intentar enviar el mensaje N°%d\n", i);
        }
        else {
            printf("Mensaje N°%d enviado.\n", i);
        }
    }
    exit(0);
}

// Lógica del lector
int reader(void) {
    for (int i = 0; i < MAX_ITER; i++) {
        sleep(i+2);
        char msg[MSG_SIZE];

        if (receive(msg) < 0) {
            printf("Ocurrió un error al intentar recibir el mensaje N°%d\n", i);
        } else {
            printf("Mensaje recibido: %s\n", msg);
        }
    }
    exit(0);
}

int main(void) {
    if (fork() == 0) {
        writer();
    } else {
        reader();
    }
    exit(0);
}