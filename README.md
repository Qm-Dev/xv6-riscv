# Sistemas Operativos 2024/2 - Tarea 2 - Benjamín Herrera
El presente documento expone el paso a paso para la modificación en la programación de procesos del sistema xv6-riscv.
## Pasos previos
Previamente, y al igual que en tareas pasadas:

1. Se creó y se cambió a una nueva rama exclusivamente para esta tarea: `git checkout -b Benjamin_Herrera_t2`
2. En la carpeta de `user`, se creó el archivo (de momento vacío) `proctest.c`, el cual corresponderá al nuevo programa ejecutable que permitirá probar los cambios realizados.

## Modificación de la estructura del proceso
Como parte de las modificaciones requeridas, se añadió un campo para prioridad y otro campo para boost en la estructura del proceso, ubicado en el archivo `proc.h` dentro de la carpeta `kernel` del sistema operativo (líneas 95-96).

Cabe también destacar que la prioridad debe ser inicializada en 0, mientras que el boost en 1. Para esto, modificamos ahora el archivo `proc.c`, especificamente la función `allocproc()`, la cual se encarga de asignar y preparar nuevos procesos. La asignación de valores para los campos de prioridad y boost ocurre en las líneas 128 y 129, después de que el proceso ya haya sido asignado y se encuentre listo para inicializarse (`state = USED`).

## Modificación de la prioridad
Como parte del detalle de esta entrega, se solicita que cada vez que un proceso ingrese, necesariamente se deba aumentar la prioridad de todos los procesos existentes que puedan ser ejecutados (exceptuando los zombies). Para esto, continuaremos modificando el archivo `proc.c`, específicamente la función `scheduler()`.

1. Se implementa la lógica `Prioridad += Boost` (línea 467). A medida que el **valor de la prioridad** vaya siendo **cercano a cero**, mayor prioridad tendrá el proceso, y por tanto será más pronto en ser ejecutado.
2. Se implementan los casos donde el boost cambia a partir del valor alcanzado por la prioridad: si la prioridad alcanza 9, el boost tomará como valor -1 (470-472); si la prioridad llega a 0, el boost tomará valor 1 (473-475).

## Pruebas
`proctest.c` es el programa donde se llevarán a cabo las pruebas para verificar el funcionamiento de las modificaciones previamente descritas en este documento.

### Llamadas de sistema adicionales
Para verificar la implementación de los campos de prioridad y boost, se crearon dos llamadas de sistema adicionales (similar a lo realizado previamente en la Tarea 1):

1. En el archivo `user/user.h`, se definieron las llamadas `getpriority` y `getboost` (líneas 25-26).
2. En el archivo `usys.pl`, se agregaron los esbozos de las llamadas previamente creadas (líneas 39-40).
3. En el archivo `kernel/syscall.h`, estas llamadas a sistema le fueron asignados los números `22` y `23` respectivamente (líneas 23-24).
4. En el archivo `kernel/syscall.c`, se implementó el prototipado de las funciones para dichas llamadas de sistema (líneas 104-105, 131-132).
5. Por último, en `kernel/sysproc.c`, se implementó el código como tal de las llamadas (95-99, 101-105). Similar a la llamada `getpid`, estas dos llamadas realizan un retorno del valor asociado a la estructura del proceso (`priority` y `boost`, los cuales fueron previamente agregados cuando modificamos la estructura del proceso).

Con las llamadas de sistema ya creadas, ahora podremos corroborar el funcionamiento correcto de la prioridad y el boost.

### Programa de prueba
El programa de prueba se compone de la siguiente forma:
```C
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
```
* El usuario debe señalar la cantidad de procesos a crear mediante el fork. Por ejemplo, si desea probar con 20 procesos, la invocación del programa en la consola de xv6 debiera ser `$ proctest 20`.
  * El máximo número de procesos que permite este programa es de 61. Intentar con un número mayor ejecutará el programa, pero este finalizará una vez terminado el último proceso (proceso 61, o proceso 60 acorde al `printf`).
* `val_fork` corresponde al valor que retorna `fork()` (0 indica que es el proceso hijo, lo cual señala que el fork ha tenido éxito. Un valor menor a 0 indica un error en la operación.)
* La salida presenta PID (ID), prioridad (P) y boost (B) asociado al proceso creado.
* Previamente, la salida aparecía mezclada a medida que se ejecutaban procesos. Para evitar esto, se añadió `sleep(i)`, lo cual permite mostrar una salida más ordenada.

### Ejemplo de salida
El siguiente es un ejemplo de salida con 20 procesos del programa de prueba descrito previamente:
```shell
$ proctest 20
Ejecutando proceso 0 (ID: 4, P: 1, B: 1)
Ejecutando proceso 1 (ID: 5, P: 2, B: 1)
Ejecutando proceso 2 (ID: 6, P: 3, B: 1)
Ejecutando proceso 3 (ID: 7, P: 4, B: 1)
Ejecutando proceso 4 (ID: 8, P: 5, B: 1)
Ejecutando proceso 5 (ID: 9, P: 6, B: 1)
Ejecutando proceso 6 (ID: 10, P: 7, B: 1)
Ejecutando proceso 7 (ID: 11, P: 8, B: 1)
Ejecutando proceso 8 (ID: 12, P: 9, B: -1)
Ejecutando proceso 9 (ID: 13, P: 8, B: -1)
Ejecutando proceso 10 (ID: 14, P: 7, B: -1)
Ejecutando proceso 11 (ID: 15, P: 6, B: -1)
Ejecutando proceso 12 (ID: 16, P: 5, B: -1)
Ejecutando proceso 13 (ID: 17, P: 4, B: -1)
Ejecutando proceso 14 (ID: 18, P: 3, B: -1)
Ejecutando proceso 15 (ID: 19, P: 2, B: -1)
Ejecutando proceso 16 (ID: 20, P: 1, B: -1)
Ejecutando proceso 17 (ID: 21, P: 0, B: 1)
Ejecutando proceso 18 (ID: 22, P: 1, B: 1)
Ejecutando proceso 19 (ID: 23, P: 2, B: 1)
```

