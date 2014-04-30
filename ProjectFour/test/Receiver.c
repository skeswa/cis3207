#include "Msg.h"

#define _XOPEN_SOURCE 600
#include <time.h>
#include <mqueue.h>
#include <signal.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>


bool quit = false;

void sigHandler(int sig);


int main(int argc, char** argv)
{
  if (argc != 2)
  {
    printf("Usage:  %s /qname\n", argv[0]);
    return 1;
  }

  char* name  = argv[1];
  int   flags = O_RDONLY;

  mqd_t qid = mq_open(name, flags);

  if (qid == -1)
  {
    perror("Failed to open the message queue");
  }

  signal(SIGINT, sigHandler);

  while (!quit)
  {
    Msg msg = {0,0,0};
    unsigned int priority = 0;
    struct timespec ts = {time(0) + 5, 0};

    if (mq_timedreceive(qid, (char*) &msg, sizeof(msg), &priority, &ts) != -1)
    {
      printf("msg = (%03d, %03d, %03d)\n", msg.x, msg.y, msg.z);
    }
    else
    {
      printf("error: %s (%d)\n", strerror(errno), errno);
    }
  }

  mq_close(qid);

  return 0;
}


void sigHandler(int sig)
{
  quit = true;
}
