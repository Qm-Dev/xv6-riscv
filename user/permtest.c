#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fcntl.h" // O_RDONLY, O_RDWR y O_WRONLY

void mostrar_contenido_archivo() {
    int archivo;
    char buffer[256];
    archivo = open("test_permisos.txt", O_RDONLY);
    if (archivo < 0) {
        printf("Ocurrió un error al abrir el archivo.\n");
        exit(1);
    }
    if (read(archivo, buffer, 256) < 0) {
        printf("Ocurrió un error al leer el archivo.\n");
        exit(1);
    }
    printf("\nContenido del archivo:\n%s\n\n", buffer);
    close(archivo);
}

int main() {
    int archivo;
    char *nombre_archivo = "test_permisos.txt";

    // Crear el archivo (permiso 3: lectura y escritura)
    archivo = open(nombre_archivo, O_CREATE | O_RDWR);
    if (archivo < 0) {
        printf("Ocurrió un error al crear el archivo.\n");
        exit(1);
    }
    printf("Archivo de testeo creado exitosamente.\n");
    if (write(archivo, "Nueva linea de texto (creación).\n", 33) != 33) {
        printf("Ocurrió un error al escribir en el archivo recién creado.\n");
        exit(1);
    }
    else {
        printf("Texto escrito exitosamente.\n");
        mostrar_contenido_archivo();
    }
    close(archivo);

    // Cambiar el permiso a solo lectura (permiso 1); intentar abrir en modo escritura
    if (chmod(nombre_archivo, 1) < 0) {
        printf("Ocurrió un error al cambiar los permisos del archivo.\n");
        exit(1);
    }
    printf("El permiso del archivo de testeo ha cambiado a solo lectura.\n");
    archivo = open(nombre_archivo, O_WRONLY);
    if (write(archivo, "Nueva linea de texto (solo lectura).\n", 36) != 36) {
        printf("Ocurrió un error al escribir en el archivo: El archivo solo permite lectura.\n");
        mostrar_contenido_archivo();
        // exit(1); Si no se comenta esta línea, el programa finaliza.
    }

    // Restaurar permisos de escritura y lectura; volver a escribir
    if (chmod(nombre_archivo, 3) < 0) {
        printf("Ocurrió un error al cambiar los permisos del archivo.\n");
        exit(1);
    }
    printf("El permiso del archivo de testeo ha cambiado a lectura y escritura.\n");
    archivo = open(nombre_archivo, O_CREATE | O_RDWR);
    if (write(archivo, "Nueva linea de texto (lectura y escritura).\n", 43) != 43) {
        printf("Ocurrió un error al escribir en el archivo.\n");
        exit(1);
    }
    else {
        printf("El texto del archivo ha sido modificado exitosamente.\n");
        mostrar_contenido_archivo();
    }

    // Establecer inmutabilidad (permiso 5)
    if (chmod(nombre_archivo, 5) < 0) {
        printf("Ocurrió un error al cambiar los permisos del archivo.\n");
        exit(1);
    }
    printf("El archivo de testeo es ahora inmutable.\n");
    archivo = open(nombre_archivo, O_WRONLY);
    if (write(archivo, "Nueva linea de texto (inmutable).\n", 33) != 33) {
        printf("Ocurrió un error al escribir en el archivo: El archivo es inmutable.\n");
        mostrar_contenido_archivo();
    }
    if (chmod(nombre_archivo, 3) < 0) {
        printf("Ocurrió un error al intentar cambiar los permisos del archivo inmutable.\n");
        mostrar_contenido_archivo();
        // exit(1);
    }
    exit(0);

}