#include "main.h"

mqd_t inbox;
mqd_t outboxes[4];
pthread_t clients[4];
pthread_t _server;
FILE *lawg;

void *client(void *args);
void *server(void *args); 

int main() {
    int i;
    char *s;
    ClientArgs *args;
    // We are going to init inbox
    inbox = mq_open("/inbox", O_CREAT, 777, NULL);
    // Create log file
    if (!(lawg = fopen("main.log", "a"))) {
        perror("Could not open the log.\n"); 
        return 1;
    }
    // Create the clients
    for (i = 0; i < 4; i++) {
        // Make the outbox string
        s = (char *) malloc(9);
        sprintf(s, "/outbox%d", i);
        s[8] = 0;
        // Make the outbox itself
        outboxes[i] = mq_open(s, O_CREAT, 777, NULL);
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
    byte buffer[1];
    ClientArgs *args = (ClientArgs *) _args;
    int cycle;
    for (cycle = 0; cycle < 20; cycle++) {
        // Create dat message doe
        m = (Message *) malloc(sizeof(Message));
        // Set the message
        m->type = MESSAGE_TYPE_PUT;
        m->count = (args->type ? 2 : 3) + (cycle % 2);
        m->senderId = args->id;
        // Send ze message yo
        if (mq_send(inbox, (char *) m, sizeof(Message), 0) < 0) {
            perror("Could not send request to server  - exiting");
            return NULL;
        }
        // Wait for server response
        if (mq_receive(outboxes[args->id], (char *) buffer, 1, NULL) < 0) {
            perror("Could not receive go ahead from server - exiting");
            return NULL;
        };
        // Wait for T1
        usleep(((args->type ? 4.5 * rand() : 2 * rand()) + (args->type ? 0.5 : 1)) * 1000000);
        // Free the resources
        m = (Message *) malloc(sizeof(Message));
        // Set the message
        m->type = MESSAGE_TYPE_GIVE;
        m->count = (args->type ? 2 : 3) + (cycle % 2);
        m->senderId = args->id;
        // Send ze message yo
        if (mq_send(inbox, (char *) m, sizeof(Message), 0) < 0) {
            perror("Could not send free to server  - exiting");
            return NULL;
        }
        // Wait for server response
        // Wait for T2
        usleep(args->type ? 2000000 : 3000000);
    }
    return NULL;
}

void *server(void *_args) {
    return NULL;
}
