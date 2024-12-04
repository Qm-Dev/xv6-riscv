# Sistemas Operativos 2024/2 - Tarea 5 - Benjamín Herrera
El presente documento presenta el paso a paso para la implementación de un sistema simple de comunicación entre procesos (IPC) en xv6-riscv.

## Pasos previos
Previamente, y al igual que en tareas pasadas:

* Se creó y se cambió a una nueva rama exclusivamente para esta tarea: `git checkout -b Benjamin_Herrera_t5`
* En la carpeta de `user`, se creó el archivo `msgtest.c`, el cual contiene el programa de prueba.
    * El Makefile fue editado, añadiéndole `$U/_msgtest\` con tal de que reconociera este nuevo programa (línea 142).

## Implementación
Como buscamos implementar IPC, será necesario crear tanto la estructura del mensaje que será enviado entre procesos, como también la de la cola global, quien contendrá estos.

### Estructura del mensaje
Se creó el archivo `message.h` dentro de `kernel`, el cual contiene la estructura del mensaje. Esta estructura es la misma dejada en el enunciado original de la tarea por el docente, con la diferencia de estar sujeta a la variable global `MAX_MSG_SIZE`, que determina el tamaño que tendrá el contenido del mensaje.
```h
#define MAX_MSG_SIZE 128

typedef struct message{
   int sender_pid;
   char content[MAX_MSG_SIZE];
} message;
```

### Implementación de la cola
1. En `defs.h`, se creó la definición de la cola de mensajes (`msg_queue`). La cola está sujeta a la variable global `MSG_QUEUE_SIZE`, la cual señala el tamaño de esta (cuantos mensajes podrá albergar) y al archivo `message.h` previamente creado; seguido de ello, se declaran `msg_queue_head` y `msg_queue_tail`, variables que determinan el principio y final de la cola. Seguido de esto, se declara el método `init_msg_queue`, responsable de inicializar la cola una vez compilado el sistema operativo. (191-199)
```h
// Definir la cola global de mensajes
#define MSG_QUEUE_SIZE 64
#include "message.h"

extern message msg_queue[MSG_QUEUE_SIZE];
extern int msg_queue_head;
extern int msg_queue_tail;

void            init_msg_queue(void);
```
2. En `proc.h`, se declara un spinlock asociado a la cola, esto acorde a lo solicitado en el enunciado original. Esto evita que múltiples procesos puedan acceder al mismo tiempo a la cola de mensajes.
```h
extern struct spinlock msg_queue_lock;
```
3. En `proc.c`, se declara la cola globalmente. Adicionalmente, se incorpora el lock previamente creado. (698-703)
```c
// Declaración global de la cola
struct message msg_queue[MSG_QUEUE_SIZE];
int msg_queue_head = 0;
int msg_queue_tail = 0;

struct spinlock msg_queue_lock;
```
Además, se escribe la lógica a seguir una vez llamado el método encargado de inicializar la cola. (705-712)
```c
// Método para la inicialización de la cola
void
init_msg_queue(void)
{
   initlock(&msg_queue_lock, "msg_queue_lock");
   msg_queue_head = 0;
   msg_queue_tail = 0;
}
```
4. En `main.c`, se llama al método previamente señalado (42). De esta forma, hemos logrado implementar la cola exitosamente en el sistema, sin embargo aún falta por crear las llamadas de sistema asociadas (`sys_send` y `sys_receive`).
```c
void
main()
{
  if(cpuid() == 0){
    // ...
  } else {
    // ...
    init_msg_queue();
  }

  scheduler();        
}
```

### Llamadas sys_send y sys_receive
El proceso de creación de llamadas de sistema es similar al ya visto y documentando en tareas pasadas:
1. `user.h`: Se declaran las nuevas llamadas (25-26).
```h
int send(int, char*);
int receive(char*);
```
2. `usys.pl`: Se añade la entrada asociada (39-40).
```pl
entry("send");
entry("receive");
```
3. `syscall.h`: Se define el número 22 y 23 para las llamadas previamente creadas (23-24).
```h
#define SYS_send  22
#define SYS_receive 23
```
4. `syscall.c`: Se realiza el prototipado y el nuevo mapeo (104-105, 130-131).
```c
// ...
extern uint64 sys_send(void);
extern uint64 sys_receive(void);
// ...
[SYS_send]    sys_send,
[SYS_receive] sys_receive,
// ...
```
5. `sysfile.c`: Se escribe la lógica de ambas llamadas (507-).
```c
uint64
sys_send(void)
{
  // Declarar variables
  int receiver_pid;
  uint64 msg_dir;
  char msg_content[MAX_MSG_SIZE];

  // Obtener args
  argint(0, &receiver_pid);
  argaddr(1, &msg_dir);

  // Corroborar validez de los args
  if (receiver_pid < 0 || msg_dir < 0)
    return -1;

  // Verificar validez del mensaje
  if (copyin(myproc()->pagetable, msg_content, msg_dir, sizeof(char) * MAX_MSG_SIZE) < 0)
    return -1;
  
  // Lock. Otros procesos no podrán acceder a la cola
  acquire(&msg_queue_lock);

  // La cola está llena
  if (msg_queue_tail == MSG_QUEUE_SIZE) {
      release(&msg_queue_lock);
      return -1;
  }

  // Guardar el mensaje en la cola (ID proceso remitente, contenido del mensaje)
  msg_queue[msg_queue_tail].sender_pid = myproc()->pid;
  safestrcpy(msg_queue[msg_queue_tail].content, msg_content, MAX_MSG_SIZE);
  msg_queue_tail++;

  // Despertar procesos bloqueados y levantar el lock
  wakeup(&msg_queue);
  release(&msg_queue_lock);

  return 0;
}

uint64
sys_receive(void)
{
  // Declarar y obtener variables
  uint64 msg_dir;
  char message[MAX_MSG_SIZE];
  argaddr(0, &msg_dir);

  // Corroborar validez del arg
  if (msg_dir < 0)
    return -1;

  // Lock.
  acquire(&msg_queue_lock);

  // Cola vacía
  while (msg_queue_head == msg_queue_tail) {
    sleep(&msg_queue, &msg_queue_lock);
  }

  // Obtener datos del mensaje (ID proceso remitente, contenido del mensaje)
  int sender_pid = msg_queue[msg_queue_head].sender_pid;
  safestrcpy(message, msg_queue[msg_queue_head].content, MAX_MSG_SIZE);
  msg_queue_head = (msg_queue_head + 1) % MSG_QUEUE_SIZE;
  
  release(&msg_queue_lock);

  // Verificar validez del mensaje
  if (copyout(myproc()->pagetable, msg_dir, message, MAX_MSG_SIZE) < 0)
    return -1;

  return sender_pid;
}
```


## Pruebas de funcionamiento
A continuación se presenta el código utilizado para probar los cambios realizados en esta tarea (`msgtest.c`):
```c
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
```
La salida esperada de este programa debiera verse de la siguiente forma:
```shell
xv6 kernel is booting

hart 1 starting
hart 2 starting
init: starting sh
$ msgtest
Mensaje N°0 enviado.
Mensaje recibido: Mensaje N°0
Mensaje N°1 enviado.
Mensaje recibido: Mensaje N°1
Mensaje N°2 enviado.
Mensaje recibido: Mensaje N°2
Mensaje N°3 enviado.
Mensaje recibido: Mensaje N°3
Mensaje N°4 enviado.
Mensaje recibido: Mensaje N°4
Mensaje N°5 enviado.
Mensaje recibido: Mensaje N°5
Mensaje N°6 enviado.
Mensaje recibido: Mensaje N°6
Mensaje N°7 enviado.
Mensaje recibido: Mensaje N°7
Mensaje N°8 enviado.
Mensaje recibido: Mensaje N°8
Mensaje N°9 enviado.
Mensaje recibido: Mensaje N°9
```

## Comentarios
* Al definirse la estructura del mensaje en un archivo aparte (`message.h`), las importaciones fueron ciertamente difíciles de figurar. Finalmente, solo era necesario incluirla en el archivo `defs.h`, ya que si deseabamos incluirla en más archivos, xv6-riscv arrojaría errores de compilación similares al siguiente, indicando conflictos de redefinición:
```shell
In file included from kernel/proc.c:8:
kernel/message.h:3:16: error: redefinition of 'struct message'
    3 | typedef struct message{
      |                ^~~~~~~
In file included from kernel/defs.h:193,
                 from kernel/proc.c:7:
kernel/message.h:3:16: note: originally defined here
    3 | typedef struct message{
      |                ^~~~~~~
kernel/message.h:6:3: error: conflicting types for 'message'; have 'struct message'
    6 | } message;
      |   ^~~~~~~
kernel/message.h:6:3: note: previous declaration of 'message' with type 'message'
    6 | } message;
      |   ^~~~~~~
kernel/proc.c:699:16: error: conflicting types for 'msg_queue'; have 'struct message[64]'
  699 | struct message msg_queue[MSG_QUEUE_SIZE];
      |                ^~~~~~~~~
kernel/defs.h:195:16: note: previous declaration of 'msg_queue' with type 'message[64]'
  195 | extern message msg_queue[MSG_QUEUE_SIZE];
      |                ^~~~~~~~~
make: *** [<builtin>: kernel/proc.o] Error 1
```
* Al intentar incluir `<string.h>` en el programa de prueba, xv6-riscv no logró ser ejecutado, arrojando varias excepciones de conflicto.
`<string.h>` permite utilizar `strcpy` y `strcat`, funciones manipuladoras de strings (copiar de una cadena a otra y concatenar cadenas respectivamente). Producto de esto, se crearon funciones localmente en el programa de prueba.
```shell
In file included from user/msgtest.c:7:
/usr/local/riscv64-unknown-elf/include/string.h:30:10: error: conflicting types for 'memcmp'; have 'int(const void *, const void *, size_t)' {aka 'int(const void *, const void *, long unsigned int)'}
   30 | int      memcmp (const void *, const void *, size_t);
      |          ^~~~~~
In file included from user/msgtest.c:6:
./user/user.h:40:5: note: previous declaration of 'memcmp' with type 'int(const void *, const void *, uint)' {aka 'int(const void *, const void *, unsigned int)'}
   40 | int memcmp(const void *, const void *, uint);
      |     ^~~~~~
/usr/local/riscv64-unknown-elf/include/string.h:31:10: error: conflicting types for 'memcpy'; have 'void *(void * restrict,  const void * restrict,  size_t)' {aka 'void *(void * restrict,  const void * restrict,  long unsigned int)'}
   31 | void *   memcpy (void *__restrict, const void *__restrict, size_t);
      |          ^~~~~~
./user/user.h:41:7: note: previous declaration of 'memcpy' with type 'void *(void *, const void *, uint)' {aka 'void *(void *, const void *, unsigned int)'}
   41 | void *memcpy(void *, const void *, uint);
      |       ^~~~~~
/usr/local/riscv64-unknown-elf/include/string.h:32:10: error: conflicting types for 'memmove'; have 'void *(void *, const void *, size_t)' {aka 'void *(void *, const void *, long unsigned int)'}
   32 | void *   memmove (void *, const void *, size_t);
      |          ^~~~~~~
./user/user.h:31:7: note: previous declaration of 'memmove' with type 'void *(void *, const void *, int)'
   31 | void *memmove(void*, const void*, int);
      |       ^~~~~~~
/usr/local/riscv64-unknown-elf/include/string.h:33:10: error: conflicting types for 'memset'; have 'void *(void *, int,  size_t)' {aka 'void *(void *, int,  long unsigned int)'}
   33 | void *   memset (void *, int, size_t);
      |          ^~~~~~
./user/user.h:38:7: note: previous declaration of 'memset' with type 'void *(void *, int,  uint)' {aka 'void *(void *, int,  unsigned int)'}
   38 | void* memset(void*, int, uint);
      |       ^~~~~~
/usr/local/riscv64-unknown-elf/include/string.h:35:10: error: conflicting types for 'strchr'; have 'char *(const char *, int)'
   35 | char    *strchr (const char *, int);
      |          ^~~~~~
./user/user.h:32:7: note: previous declaration of 'strchr' with type 'char *(const char *, char)'
   32 | char* strchr(const char*, char c);
      |       ^~~~~~
/usr/local/riscv64-unknown-elf/include/string.h:41:10: error: conflicting types for 'strlen'; have 'size_t(const char *)' {aka 'long unsigned int(const char *)'}
   41 | size_t   strlen (const char *);
      |          ^~~~~~
./user/user.h:37:6: note: previous declaration of 'strlen' with type 'uint(const char *)' {aka 'unsigned int(const char *)'}
   37 | uint strlen(const char*);
      |      ^~~~~~
make: *** [<builtin>: user/msgtest.o] Error 1
```

* La función `itoa` encargada de transformar de entero a caracter no es estándar en el lenguaje C, por lo que similar a la situación previamente descrita, se creó una función localmente.

* La salida del programa de prueba se mostraba desordenada una vez ejecutada:
```shell
$ msgtest
MeMnensaje recibsiadjoe N:� �M0e nesnaviajdeo .N�
�0M
eMnesanje sNaj�e� 1r eecnivbiidaod:o .M
enMseajnes aNj�e� 1N
�Me�n2s aejnev iraedcoi.b
iMdeon:s aMjeen sNa�j�e3  N�e�n2v
iMaednos.a
jeM ernescaijbe N�i�d4o :e nMvenisajaed oN.�
�3
MenMseaje nrsajee cNib�i�do5: Men senaje N�v�4i
adoM.ensa
je reMcibideon:s aMjee nNs�a�j6e  Nenv°5
iaMendsoa.j
e recMiebnisaje N°7 enviado.
Mensaje N°8d oe:n vMieando.
Mesnsaaje Nje °9 Nenvi°a6do
.M
ensaje recibido: Mensaje N°7
Mensaje recibido: Mensaje N°8
Mensaje recibido: Mensaje N°9
$
```
Para intentar solucionar esto, se agregó `sleep(i)` al inicio de los ciclos for del escritor y lector (un parche que había funcionado en una tarea pasada para el mismo problema presentado). Sin embargo, los resultados seguían siendo similares:
```shell
$ msgtest
MensMaejnesa jNe° 0r eecibido: Mensaje N°0
nviado.
MeMnesnasjaej eN �r�e1c iebnivdioa:d oM.e
nsaje N°1
MeMnesansjea jNe� �r2e ceinbviidaod:o .
Mensaje N°2
MenMseajnes aNj�e� 3r eecnivbiiaddoo:.
Mensaje N°3
MensajMee nNs�a�j4e  enrveicadiob.i
do: Mensaje N°4
MenMseanjsea Nj°e5  ernevciiabiddoo.:
 Mensaje N°5
MensajeM eNn�s�a6 jenvei ardeoc.
ibido: Mensaje N°6
MeMnesanjsea jNe� �r7e ceinvbiiaddoo.
: Mensaje N°7
Mensaje N°8M enesnavjie ardeoc.i
bido: Mensaje N°8
MenMseanjsea jN°9 een vreiadcoi.b
ido: Mensaje N°9
$
```
Finalmente, empecé a jugar con el argumento del `sleep`. Mediante el nuevo argumento `i+x`, donde `i` corresponde a la iteración actual del for y `x` un número cualquiera, por medio de prueba y error comencé a ver si la salida se hacía más legible al aumentar el `x` de un proceso en una unidad más alta que el otro. Esto dió resultados favorables con `i+1` e `i+2`. Este parche, si bien no es robusto, logra su cometido de obtener una salida más legible.
```shell
$ msgtest
Mensaje N°0 enviado.
Mensaje recibido: Mensaje N°0
Mensaje N°1 enviado.
Mensaje recibido: Mensaje N°1
Mensaje N°2 enviado.
Mensaje recibido: Mensaje N°2
Mensaje N°3 enviado.
Mensaje recibido: Mensaje N°3
Mensaje N°4 enviado.
Mensaje recibido: Mensaje N°4
Mensaje N°5 enviado.
Mensaje recibido: Mensaje N°5
Mensaje N°6 enviado.
Mensaje recibido: Mensaje N°6
Mensaje N°7 enviado.
Mensaje recibido: Mensaje N°7
Mensaje N°8 enviado.
Mensaje recibido: Mensaje N°8
Mensaje N°9 enviado.
Mensaje recibido: Mensaje N°9
$
```