#define INBOX_QUEUE_NAME "/inbx"    // The name of the server in message queue
#define WAITBOX_QUEUE_NAME "/wtbx"  // The name of the server wat message queue
#define OUTBOX_QUEUE_NAME /otbx     // The name of the client out message queue
#define OUTBOX_QUEUE_NAME_LEN 7     // The size of the name of the cient out message queue

#include "main.h"                   // The header file for this file

mqd_t inbox, waitbox;   // Message queue references
mqd_t outboxes[4];      // More message queue references
pthread_t clients[4];   // Thread refrences for the client
pthread_t _server;      // Thread reference for the server
FILE *lawg;             // The file system log file
                        
int flags;              // Flags for the message queues
mode_t perms;           // Permissions for message queues
struct mq_attr *inAttrs, *waitAttrs; // Attributes for message queues

// Function executes client thread logic
// <-   args = the arguments for the client thread
// ->   
void *client(void *args);
// Function executes server thread logic
// <-   args = the arguments for the server thread
// ->   
void *server(void *args);

// Function generates new message queue attributes
// <-   
// ->   a pointer to mq attributes
struct mq_attr *newAttr() {
    struct mq_attr *a = (struct mq_attr *) malloc(sizeof(struct mq_attr));
    a->mq_flags = 0;
    a->mq_maxmsg = 10;
    a->mq_msgsize = sizeof(Message);
    a->mq_curmsgs = 0;
    return a;
}
// Function creates a new message
// <-   type = type of message, count = how many resources are in play, senderId = is of the sender
// ->   the new message
Message *newMessage(byte type, size_t count, byte senderId) {
    Message *m = (Message *) malloc(sizeof(Message));
    m->type = type;
    m->count = count;
    m->senderId = senderId;
    return m;
}
// Function clones an existing message
// <-   existingMessage = the message to clone
// ->   the cloned message
Message *cloneMessage(Message *existingMessage) {
    return newMessage(existingMessage->type, existingMessage->count, existingMessage->senderId);
}
// Function generates random number between 0 and 1
// <-   
// ->   the random number
float randomFloat() {
    float r = (float) rand() / (float) RAND_MAX;            
    return r;
}

int main() {
    int i;
    char *s;
    ClientArgs *args;
    // Setup the settings for mq
    perms = (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
    flags = O_CREAT | O_RDWR;
    // Attrs for mq
    inAttrs = newAttr();
    waitAttrs = newAttr();
    // We are going to init inbox
    if ((inbox = mq_open(INBOX_QUEUE_NAME, flags, perms, inAttrs)) <= 0) {
        perror("Could not open the inbox queue"); 
        return 1;
    }
    // We are going to init waitbox
    if ((waitbox = mq_open(WAITBOX_QUEUE_NAME, flags, perms, waitAttrs)) <= 0) {
        perror("Could not open the waitbox queue"); 
        return 1;
    }
    // Create log file
    if (!(lawg = fopen("main.log", "a"))) {
        perror("Could not open the log"); 
        return 1;
    }
    // Create the clients
    for (i = 0; i < 4; i++) {
        // Make the outbox string
        s = (char *) malloc(7);
        sprintf(s, "/otbx%d", i);
        s[OUTBOX_QUEUE_NAME_LEN - 1] = 0;
        printf("Creating client %s\n", s);
        // Make the outbox itself
        if ((outboxes[i] = mq_open(s, flags, perms, newAttr())) < 0) {
            perror("Could not open an outbox queue");
            return 1;
        }
        // Set the args
        args = (ClientArgs *) malloc(sizeof(ClientArgs));
        args->type = i % 2;
        args->id = i;
        // Create the threads
        pthread_create(clients + 0, NULL, client, args); 
    }
    // Start running the server
    pthread_create(&_server, NULL, server, NULL);
    // Wait on console output
    getchar();
    return 0;
}

void *client(void *_args) {
    Message *m;
    ClientArgs *args = (ClientArgs *) _args;
    int cycle;
    double waitTime;

    // Set the random seed
    srand(time(NULL));

    for (cycle = 0; cycle < 20; cycle++) {
        printf("Thread #%d beginning cycle %d\n", args->id, cycle);
        // Create dat message doe
        m = (Message *) malloc(sizeof(Message));
        // Set the message
        m->type = MESSAGE_TYPE_GET;
        m->count = (args->type ? 2 : 3) + (cycle % 2);
        m->senderId = args->id;
        // Send ze message yo
        printf("Thread #%d sending resource request for %zu resources\n", args->id, m->count);
        if (mq_send(inbox, (char *) m, sizeof(Message), 0) < 0) {
            perror("Could not send request to server  - exiting");
            return NULL;
        }
        // Wait for server response
        printf("Thread #%d waiting for resources to free up\n", args->id);
        if (mq_receive(outboxes[args->id], (char *) m, sizeof(Message), NULL) < 0) {
            perror("Could not receive go ahead from server - exiting");
            return NULL;
        }
        // Wait for T1
        waitTime = (((args->type ? 4.5 * randomFloat() : 2 * randomFloat()) + (args->type ? 0.5 : 1)) * 1000000);
        printf("Thread #%d holding on to resources for %f seconds\n", args->id, waitTime / 1000000);
        usleep(floor(waitTime));
        // Free the resources
        m = (Message *) malloc(sizeof(Message));
        // Set the message
        m->type = MESSAGE_TYPE_PUT;
        m->count = (args->type ? 2 : 3) + (cycle % 2);
        m->senderId = args->id;
        // Send ze message yo
        if (mq_send(inbox, (char *) m, sizeof(Message), 0) < 0) {
            perror("Could not send free to server  - exiting");
            return NULL;
        }

        waitTime = (args->type ? 2000000 : 3000000);
        printf("Thread #%d waiting to request resources for %f seconds\n", args->id, waitTime / 1000000);
        usleep(floor(waitTime));
    }

    printf("Thread #%d has completed 20 cycles - now exiting\n", args->id);
    return NULL;
}

void *server(void *_args) {
    Message *m = newMessage(0, 0, 0);
    Message *_m = newMessage(0, 0, 0);
    Message *goAhead = newMessage(MESSAGE_TYPE_GOAHEAD, 0, 0);
    int messageLength;
    int count = 10;
    byte isWaiting = 0;
    
    while ((messageLength = mq_receive(inbox, (char *) m, sizeof(Message), NULL)) > 0) {
        printf("Server received a message of type %d and size %d\n", m->type, messageLength);
        if (m->type == MESSAGE_TYPE_PUT) {
            printf("Server got a PUT from thread #%d\n", m->senderId);
            // Return the resources
            count += m->count; 
            printf("Server bumped the count by %zu to %d\n", m->count, count);
            // Check wait queue
            if (isWaiting) {
                while (_m->count <= count) {
                    printf("Server releasing thread #%d from the wait since count is big enough\n", _m->senderId);
                    // The waiting message is taken care of now - send the go ahead
                    mq_send(outboxes[_m->senderId], (char *) goAhead, sizeof(Message), 0);
                    // Do the subtraction
                    count -= _m->count;
                    printf("Server dropped the count by %zu to %d\n", _m->count, count);
                    // Check the wait queue
                    if (waitAttrs->mq_curmsgs == 0) {
                        printf("Server exhausted the wait queue\n");
                        // We exhausted the waitqueue
                        isWaiting = 0;
                        break;
                    } else {
                        // We need to grab the next message in the q
                        printf("Server is moving on to the next message in the queue\n");
                        mq_receive(waitbox, (char *) _m, sizeof(Message), NULL);
                    }
                }
                // Otherwise we keep waiting
            } else {
                // Catch new things in the queue - we havent started waiting yet
                if (waitAttrs->mq_curmsgs > 0) {
                    printf("Server is adding a new message to the wait queue\n");
                    // We need to grab the next message in the q
                    mq_receive(waitbox, (char *) _m, sizeof(Message), NULL); 
                    isWaiting = 1;
                }
            }
        } else if (m->type == MESSAGE_TYPE_GET) {
            printf("Server got a GET from thread #%d\n", m->senderId);
            if (m->count > count) {
                printf("Server doesn't have the resources - thread#%d being put in waiting queue\n", m->senderId);
                mq_send(waitbox, (char *) m, sizeof(Message), 0);
                // So we don't overwrite m for the send
                m = cloneMessage(m);
            } else {
                // We can take care of this here and now
                count -= m->count;
                printf("Server dropped the count by %zu to %d\n", m->count, count);
                // The waiting message is taken care of now - send the go ahead
                mq_send(outboxes[m->senderId], (char *) goAhead, sizeof(Message), 0);
            }
        }
    }

    if (messageLength < 0) perror("There was an issue reading messages on the server");
    else if (messageLength == 0) printf("Got a 0 sized message - and that isn't a thing...\n");
    
    return NULL;
}
