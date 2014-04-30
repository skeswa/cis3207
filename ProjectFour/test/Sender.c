#include "Msg.h"

#include <mqueue.h>
#include <stdbool.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>    // for srand(), rand()
#include <time.h>      // for time()
#include <errno.h>
#include <string.h>


bool quit = false;

void sigHandler(int sig)
{
  quit = true;
}


int main(int argc, char** argv)
{
  if (argc != 2)
  {
    printf("Usage:  %s /qname\n", argv[0]);
    return 1;
  }

  char*  name  = argv[1];
  int    flags = O_RDWR | O_CREAT;
  mode_t mode  = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;

  struct mq_attr attr;
  attr.mq_flags   = 0;
  attr.mq_maxmsg  = 10;
  attr.mq_msgsize = sizeof(Msg);
  attr.mq_curmsgs = 0;

  mqd_t qid = mq_open(name, flags, mode, &attr);

  if (qid == -1)
  {
    printf("error: %s (%d)\n", strerror(errno), errno);
    return 1;
  }

  signal(SIGINT, sigHandler);

  srand(time(0));

  printf("Press enter to send a message...");
  getchar();

  do
  {
    Msg msg = {rand() % 100, rand() % 100, rand() % 100};

    printf("sending msg = (%03d, %03d, %03d)\n", msg.x, msg.y, msg.z);

    if (mq_send(qid, (char*) &msg, sizeof(Msg), 0) == -1)
    {
      perror("Failed to send msg");
    }

    printf("Press enter to send another message...");
    getchar();

  } while (!quit);

  mq_close(qid);
  mq_unlink(argv[1]);

  return 0;
}
