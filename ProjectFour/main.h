#include <stdlib.h>
#include <mqueue.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

#define MESSAGE_TYPE_PUT    1
#define MESSAGE_TYPE_GIVE   0

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





