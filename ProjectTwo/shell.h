#define CHAR_PIPE         '|'
#define CHAR_CARROT_RIGHT '>'
#define CHAR_CARROT_LEFT  '<'

#define READ_STATUS_WAITING 0
#define READ_STATUS_PROG    1
#define READ_STATUS_ARGS    2
#define READ_STATUS_IN      3
#define READ_STATUS_OUT     4
#define READ_STATUS_PIPE    5


// Represents a process - has information about its execution
typedef struct Process {
  FILE *in;             // Where stdin is piped
  FILE *out;            // Where stdout is piped 
  int status;           // Execution status
  char *prog;           // The program string
  char **argv;          // Arguments for execution
  pid_t pid;            // The id of the process
  struct Process *next; // Linked list of piped processes
} Process;
