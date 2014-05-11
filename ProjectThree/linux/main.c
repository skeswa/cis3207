#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>

#define PRODUCER_SPEED      50000       /* producer thread sleep time in micro seconds */
#define CONSUMER_SPEED      80000       /* consumer thread sleep time in micro seconds */
#define BUFFER_SIZE         20          /* total number of slots */
#define NUM_PRUDUCERS       3           /* total number of producers */
#define NUM_CONSUMERS       4           /* total number of consumers */
#define NUM_ITEMS           30          /* number of items produced/consumed */


void *Producer(void *args);
void *Consumer(void *args);

typedef unsigned char piece;
typedef struct {
    piece buf[BUFFER_SIZE];
    int in;       // Next empty slot
    int out;      // Next full slot
    sem_t full; 
    sem_t empty;
    sem_t mutex;
} Buffer;

Buffer buffer;

int main() {
    pthread_t pids[NUM_PRUDUCERS], cids[NUM_CONSUMERS];
    int i, *iRef;
    void *ret;
    // Setup the sems
    sem_init(&buffer.full, 0, 0);
    sem_init(&buffer.empty, 0, BUFFER_SIZE);
    sem_init(&buffer.mutex, 0, 1);
    srand(time(NULL));
    // Create the threads
    for (i = 0; i < NUM_PRUDUCERS; i++) {  
        iRef = (int *) malloc(sizeof(int));
        *iRef = i;
        pthread_create(pids + i, NULL, Producer, (void*) iRef);
    }
    for (i = 0; i < NUM_CONSUMERS; i++) {  

        iRef = (int *) malloc(sizeof(int));
        *iRef = i;
        pthread_create(cids + i, NULL, Consumer, (void*) iRef);
    }
    // Join the threads
    for (i = 0; i < NUM_PRUDUCERS; i++) {  
        pthread_join(*(pids + i), &ret);
    }
    for (i = 0; i < NUM_CONSUMERS; i++) {  
        pthread_join(*(cids + i), &ret);
    }
    // Finish up
    exit(0);
}

void *Producer(void *args) {
    int i;
    piece item;
    int id;

    id = *((int *) args);
    for (i = 0; i < NUM_ITEMS * NUM_CONSUMERS; i++) {
        usleep(PRODUCER_SPEED);
        item = (piece) (rand() % 255);   
        /* If there are no empty slots, wait */
        printf("Producer #%d:\tWaiting for one of the slots to open up\n", id);
        sem_wait(&buffer.empty);
        /* If another thread uses the buffer, wait */
        printf("Producer #%d:\tWaiting for control of the buffer\n", id);
        sem_wait(&buffer.mutex);
        buffer.buf[buffer.in] = item;
        buffer.out = buffer.in;
        buffer.in = (buffer.in + 1) % BUFFER_SIZE;
        printf("Producer #%d:\tPutting '%d' in the buffer\n", id, item);
        /* Release the buffer */
        printf("Producer #%d:\tRelinquishing control of the buffer\n", id);
        sem_post(&buffer.mutex);
        /* Increment the number of full slots */
        sem_post(&buffer.full);
    }
    printf("Producer #%d:\tFinished producing %d items\n", id, NUM_ITEMS * NUM_CONSUMERS);

    return NULL;
}

void *Consumer(void *args) {
    int i;
    piece item;
    int id;

    id = *((int *) args);
    for (i = 0; i < NUM_ITEMS * NUM_PRUDUCERS; i++) {
        usleep(CONSUMER_SPEED);
        /* If there are no full slots, wait */
        printf("Consumer #%d:\tWaiting for one of the slots to fill up\n", id);
        sem_wait(&buffer.full);
        /* If another thread uses the buffer, wait */
        printf("Consumer #%d:\tWaiting for control of the buffer\n", id);
        sem_wait(&buffer.mutex);
        item = buffer.buf[buffer.out];
        printf("Consumer #%d:\tGot '%d' from the buffer\n", id, item);
        buffer.out = (buffer.out + 1) % BUFFER_SIZE;
        /* Release the buffer */
        printf("Consumer #%d:\tRelinquishing control of the buffer\n", id);
        sem_post(&buffer.mutex);
        /* Increment the number of full slots */
        sem_post(&buffer.empty);
    }
    printf("Consumer #%d:\tFinished consuming %d items\n", id, NUM_ITEMS * NUM_PRUDUCERS);

    return NULL;
}