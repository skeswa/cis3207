#include <stdlib.h>
#include <mqueue.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/queue.h>
#include <time.h>
#include <math.h>

#define MESSAGE_TYPE_PUT    1
#define MESSAGE_TYPE_GET    0
#define MESSAGE_TYPE_GOAHEAD 3

#define MQ_DEF_MSGSIZE 1024
#define MQ_DEF_MAXMSG 16

typedef unsigned char byte;

typedef struct {
    byte type;
    size_t count;
    byte senderId;
} Message;

typedef struct {
    byte type;
    byte id;
} ClientArgs;





