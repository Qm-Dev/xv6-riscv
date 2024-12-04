#define MAX_MSG_SIZE 128

typedef struct message{
   int sender_pid;
   char content[MAX_MSG_SIZE];
} message;