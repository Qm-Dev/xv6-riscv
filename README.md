# Sistemas Operativos 2024/2 - Tarea 4 - Benjamín Herrera
El presente documento presenta el paso a paso para la implementación de un sistema de permisos básicos en el sistema operativo xv6-riscv, permitiendo la modificación de archivos acorde a los accesos entregados como también la adición de un permiso especial de inmutabilidad.

## Pasos previos
Previamente, y al igual que en tareas pasadas:

* Se creó y se cambió a una nueva rama exclusivamente para esta tarea: `git checkout -b Benjamin_Herrera_t4`
* En la carpeta de `user`, se creó el archivo `permtest.c`, el cual contiene el programa de prueba.
    * El Makefile fue editado, añadiéndole `$U/_permtest\` con tal de que reconociera este nuevo programa (línea 142).

## Implementación
La implementación aborda ambas partes pedidas en el enunciado original.
### Modificación del inodo
* `file.h`: Se agregó el campo de `permission` para señalar el permiso asociado.
```c
struct inode {
  uint dev;           // Device number
  uint inum;          // Inode number
  int ref;            // Reference count
  struct sleeplock lock; // protects everything below here
  int valid;          // inode has been read from disk?

  short type;         // copy of disk inode
  short major;
  short minor;
  short nlink;
  uint size;
  uint addrs[NDIRECT+1];
  uint permission;
};
```
* `sysfile.h`: Se declara en `create` el permiso que tendrán por defecto los inodos una vez creados (3: lectura y escritura)
```c
// ...
ip->permission = 3;
// ...
```

### Creación de la llamada chmod
1. `user.h`: Se declara la nueva llamada (25).
```h
int chmod(const char*, int);
```
2. `usys.pl`: Se añade la entrada asociada (39).
```pl
entry("chmod");
```
3. `syscall.h`: Se define el número 22 para chmod (23).
```h
#define SYS_chmod  22
```
4. `syscall.c`: Se realiza el prototipado y el nuevo mapeo (104, 130).
```c
// ...
extern uint64 sys_chmod(void);
// ...
[SYS_chmod]   sys_chmod,
// ...
```
5. `sysfile.c`: Se escribe la lógica de la llamada (532-564).
```c
uint64
sys_chmod(void)
{
  char ruta_archivo[MAXPATH];
  int nuevo_permiso;
  struct inode *ip;
  argint(1, &nuevo_permiso);

  // Corroborar validez de los valores en los args ingresados
  if (argstr(0, ruta_archivo, MAXPATH) < 0 || nuevo_permiso < 0){
    return -1;
  }
  begin_op();
  if ((ip = namei(ruta_archivo)) == 0){
    end_op();
    return -1;
  }
  ilock(ip);

  // Cambiar el permiso del archivo (excepto aquellos inmutables)
  if (ip->permission == 5) {
    iunlockput(ip);
    end_op();
    return -1;
  }
  ip->permission = nuevo_permiso;
  iupdate(ip);
  iunlockput(ip);

  // Finalizar operación
  end_op();
  return 0;
}
```
Debido a que estoy trabajando al mismo tiempo ambas partes de la tarea, destacar que el siguiente bloque de código es el encargado de abordar la inmutabilidad (551-556), fundamental para la tarea 2:
```c
// Cambiar el permiso del archivo (excepto aquellos inmutables)
if (ip->permission == 5) {
    iunlockput(ip);
    end_op();
    return -1;
}
```
En adición a esto, se señala en la línea 396:
```c
f->writable = (ip->permission != 5) || (omode & O_WRONLY) || (omode & O_RDWR);
```

## Pruebas de funcionamiento
A continuación, se presenta el código utilizado para probar los cambios realizados en esta tarea (`permtest.c`):
```c
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
```
Se utilizan dos métodos en el programa: `mostrar_contenido_archivo`, el cual muestra en consola el contenido que posee el archivo de prueba utilizado y `main` donde se llevan a cabo las operaciones relacionadas a cambiar el permiso del archivo de prueba e intentar escribir en este.

***Nota: El nombre del archivo de prueba (test_permisos.txt) está hardcodeado solo por motivos de probar la implementación descrita en este documento.***

La salida esperada de este programa debiera verse de la siguiente forma:

```shell
xv6 kernel is booting

hart 2 starting
hart 1 starting
init: starting sh
$ permtest
Archivo de testeo creado exitosamente.
Texto escrito exitosamente.

Contenido del archivo:
Nueva linea de texto (creación).

El permiso del archivo de testeo ha cambiado a solo lectura.
Ocurrió un error al escribir en el archivo: El archivo solo permite lectura.

Contenido del archivo:
Nueva linea de texto (creación).

El permiso del archivo de testeo ha cambiado a lectura y escritura.
El texto del archivo ha sido modificado exitosamente.

Contenido del archivo:
Nueva linea de texto (lectura y escritura).

El archivo de testeo es ahora inmutable.
Ocurrió un error al escribir en el archivo: El archivo es inmutable.

Contenido del archivo:
Nueva linea de texto (lectura y escritura).

Ocurrió un error al intentar cambiar los permisos del archivo inmutable.

Contenido del archivo:
Nueva linea de texto (lectura y escritura).
```



## Comentarios
* La llamada de sistema `chmod` no estaba siendo reconocida al ejecutar QEMU, por lo cual no ejecutaba tampoco el sistema xv6-riscv. Al depurar, me di cuenta que esto ocurría ya que `chmod` es una syscall asociada a archivos y no procesos (esto ya que a lo largo del semestre trabajamos con llamadas de sistemas asociadas a estos últimos exceptuando en esta tarea). La solución a esto fue mover la lógica de la llamada de `sysproc.c` a `sysfile.c`, lo cual permitió que QEMU ejecutara xv6-riscv sin problemas. Si bien esto puede ser resuelto incorporando los módulos requeridos, luego de esta explicación es sabido el por qué esta sería una pésima idea de llevar a cabo.
```shell
kernel/sysproc.c: In function 'sys_chmod':
kernel/sysproc.c:115:5: error: invalid use of undefined type 'struct inode'
  115 |   ip->permission = nuevo_permiso;
      |     ^~
make: *** [<builtin>: kernel/sysproc.o] Error 1
```
