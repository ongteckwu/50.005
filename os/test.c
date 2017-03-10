#include <errno.h>
#include <string.h>   //str
#include <stdio.h>
#include <unistd.h>   //pid
#include <stdlib.h>   //malloc
#include <fcntl.h>

#define INELIGIBLE 0
#define READY 1
#define RUNNING 2
#define FINISHED 3

#define MAX_LENGTH 1024
#define MAX_CHILDREN 10
#define MAX_NODES 50

#define CREATE_FLAGS (O_WRONLY | O_CREAT | O_TRUNC)
#define CREATE_PERMS (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

int makeargv(const char *s, const char *delimiters, char ***argvp) {
  int error;
  int i;
  int numtokens;
  const char *snew;
  char *t;

  if ((s == NULL) || (delimiters == NULL) || (argvp == NULL)) {
    errno = EINVAL;
    return -1;
  }
  *argvp = NULL;
  snew = s + strspn(s, delimiters);
  if ((t = (char*)malloc(strlen(snew) + 1)) == NULL)
    return -1;
  strcpy(t,snew);
  numtokens = 0;
  if (strtok(t, delimiters) != NULL)
    for (numtokens = 1; strtok(NULL, delimiters) != NULL; numtokens++) ;

  if ((*argvp = (char**)malloc((numtokens + 1)*sizeof(char *))) == NULL) {
    error = errno;
    free(t);
    errno = error;
    return -1;
  }

  if (numtokens == 0)
    free(t);
  else {
    strcpy(t,snew);
    **argvp = strtok(t,delimiters);
    for (i=1; i<numtokens; i++)
      *((*argvp) +i) = strtok(NULL,delimiters);
  }
  *((*argvp) + numtokens) = NULL;
  return numtokens;
}

void freemakeargv(char **argv) {
  if (argv == NULL)
    return;
  if (*argv != NULL)
    free(*argv);
  free(argv);
}

typedef struct node {
    int id;
    char prog[MAX_LENGTH];
    char input[MAX_LENGTH];
    char output[MAX_LENGTH];
    int children[MAX_CHILDREN];
    int num_children;
    int num_parents;
    int num_parents_finished;
    int status;
    pid_t pid;
} node_t;

/**
 * Parse the input line at s, and populate the node at n, which will
 * have id set to id.
 * Return 0 on success or -1 and set errno on failure.
 */
static int parse_input_line(char *s, int id, node_t *n) {

  char **strings;
  char **child_list;
  int c = 0;

  /* Split the line on ":" delimiters */
  if (makeargv(s, ":", &strings) == -1) {
    return -1;
  }

  /* Parse the space-delimited child list */
  if (makeargv(strings[1], " ", &child_list) == -1) {
    freemakeargv(strings);
    return -1;
  }

  strcpy(n->prog, strings[0]);
  // fprintf(stderr,"n=%d n->prog=%s\n",id,n->prog);
  strcpy(n->input, strings[2]);
  // fprintf(stderr,"n=%d n->input=%s\n",id,n->input);
  strcpy(n->output, strings[3]);
  // fprintf(stderr,"n=%d n->output=%s\n",id,n->output);
  n->id = id;
  // fprintf(stderr,"n=%d n->id=%d\n",id,id);
  n->num_children = 0;

  //if strcmp is not equal none
  if (strcmp(child_list[c], "none") != 0) {
    for (c = 0; child_list[c] != NULL; c++) {
      n->children[c] = atoi(child_list[c]);
      // fprintf(stderr,"n=%d n->child[%d]=%d\n",id,c,n->children[c]);
      n->num_children++;
    }
  }
  // fprintf(stderr,"n=%d n->numchildren=%d\n",id,n->num_children);

  //start all nodes as ineligible
  n->status = INELIGIBLE;
  // fprintf(stderr,"n=%d n->status=%d\n",id,n->status);

  return 0;
}

/**
 * Parse the file at filename, and populate the array at n.
 * Return the number of nodes parsed on success, or -1 and set errno on
 * failure.
 */
static int parse_graph_file(char *filename, node_t *n) {

  FILE *f;
  char buf[MAX_LENGTH];
  int errno_copy;
  int id = 0;

  if ((f = fopen(filename,"r")) == NULL) {
    return -1;
  }

  while (fgets(buf, MAX_LENGTH, f) != NULL) {

    /* fgets retained the trailing newline; get rid of it */
    strtok(buf, "\n");

    if (parse_input_line(buf, id, n) == 0) {
      n++;
      id++;
    }
    else {
      return -1;
    }
  }

  /*
   * Make sure that we actually reached the end of the file, and close it.  The
   * error-handling seems a bit odd here, but it's just because we want to
   * retain any errno value that occurred during fgets rather than stomping on
   * it with a possible fclose errno value.
   */
  if (ferror(f)) {
    errno_copy = errno;
    fclose(f);  /* Ignore error; it's not what we want to report */
    errno = errno_copy;
    return -1;
  }
  if (fclose(f) == EOF) {
    return -1;
  }

  /* Otherwise return the number of nodes parsed */
  printf ("the number of nodes=%d\n", id);
  return id;

}

/**
* The function inserts into the node's num_parents field by counting the number of parents it has.
* The num_parents attribute is important to decide whether it's status is READY or not.
* returns 0 when function is successful.
*/



int populateParents(node_t *nodes, int num) {
  int i;
  int j;
  for (i=0; i< num; i++) {
    for (j=0; j < nodes[i].num_children; j++) {
      nodes[nodes[i].children[j]].num_parents += 1;
    }
    //print number of parents per node
    // fprintf(stderr,"n=%d n->num_parents=%d\n",nodes[i].id,nodes[i].num_parents);
  }
  return 0;
}


/**
* This function executes one single node through the forking of the main thread.
* It first tokenizes commands for the node.prog through the string tokenizer.
* This is followed by the checking of the input and output file descriptors, which can be changed
* depending on whether they are specified or not.
* Finally, the execvp system call is used to run the node.prog. The child process is ended
* and joined back to the main after the execvp function.
* the function returns -1 when there is an error, else it returns 0 if it is successful
*/


int execute_node(node_t node) {
  int noChildren = node.num_children;
  int input;
  int output;

  //string tokenize command
  int idx = 0;
  char *comIn[MAX_LENGTH]= {};
  comIn[idx] = strtok(node.prog, " \n");

  while(comIn[idx] !=  NULL) {
    idx = idx + 1;
    comIn[idx] = strtok(NULL, " \n");
  }

  //file descriptor for input
  if (strncmp(node.input,"stdin",5) != 0 ) {
    input = open(node.input, O_RDONLY);
    if (input < 0 ) {
      perror("Failed to open file");
      return -1;
    }
    if (dup2( input , STDIN_FILENO) <0 ) {
      perror("Failed to dup the 2 file descriptors for input!");
      return -1;
    }
  }

  //file descriptor for output
  if (strncmp(node.output,"stdout",6) != 0 ){
    output = open(node.output, CREATE_FLAGS, CREATE_PERMS);
    if (output < 0 ) {
      perror("Failed to create file to write to");
      return -1;
    }
    if (dup2(output,STDOUT_FILENO) < 0) {
      perror("Failed to dup the 2 file descriptors for output!");
      return -1;
    }
  }
  //execvp the command. if successful, program will exit from the execution of command. else throw error.
  execvp(comIn[0],comIn);
  perror("Failed to execute the node prog!");
  return -1;
}

/**
* This function accepts the array of node objects, and a count of the number of nodes to be executed.
* It executes all the nodes within the array, through the action of forking a child from the main process.
* Each node is first checked for their status on whether they are READY to run. If not, it will go to the next node
* It runs execute_node for each function for the forking and running of the node.prog.
* If there is an error in the function, this will return -1. If successful, it returns 0.
*/

int execute_all_nodes(node_t* nodes, int num) {
  int totalNodes = num;
  int numNodesFinished = 0;
  int i;
  int j;
  while(numNodesFinished < totalNodes) {

    //check if parents are done, then change state to ready
    for(i = 0; i < num ; i++ ) {
      if (nodes[i].status == INELIGIBLE) {
        if (nodes[i].num_parents_finished == nodes[i].num_parents)
          nodes[i].status = READY;
      }
    }

    //For them to fork concurrently first before waiting
    for(i = 0; i<num;i++) {
      if (nodes[i].status == READY) {
        pid_t child_pid;
        child_pid = fork();
        if (child_pid < 0) {
          perror("There was an error forking the process!");
          return -1;
        }

        if (child_pid == 0) {
          //is child
          if (execute_node(nodes[i]) < 0) {
            perror("There was an error running execute_node.");
            return -1;
          }
        } else {
          //main thread has child_pid, pid for the child.
          nodes[i].status = RUNNING;
          nodes[i].pid = child_pid;
        }
      }
    }

    for(i = 0; i<num;i++) {
      if (nodes[i].status == RUNNING ) {
        if (wait(NULL) == -1) {
          perror("There was an error waiting for the process to end!");
          return -1;
        } else {
          //successfully waited for forked child to complete
          nodes[i].status = FINISHED;
          numNodesFinished++;
          //update nodes's childs
          for(j=0;j < nodes[i].num_children; j++) {
            nodes[nodes[i].children[j]].num_parents_finished++;
          }
        }
      }
    }
  }
  return 0;
}


/**
* The main function that processmgt.c runs.
* Firstly, it parses the arguments and checks if there is an argument for the filename of the textfile to be processed.
* Secondly, if parses the content of the textfile and converts them into an array of node objects.
* populateParents is run to populate the numParents field of each node.
* The main bulk of the program is in the execute_all_nodes function where all the nodes defined in the textfile is run.
* Any error will return in EXIT_FAILURE.
*/

int main(int argc, char **argv) {
  node_t nodes[MAX_NODES];
  int num;
  int i =0;

  if (argc != 2){
    fprintf(stderr, "usage: %s graph-file\n", argv[0]);
    return EXIT_FAILURE;
  }
  if ((num = parse_graph_file(argv[1], nodes)) == -1) {
    perror("Failed to parse graph file\n");
    return EXIT_FAILURE;
  }

  populateParents(nodes, num);

  printf("Beginning running of processes!\n");
  if (execute_all_nodes(nodes,num) == -1) {
    perror("There was an error running execute_all_nodes\n");
    return EXIT_FAILURE;
  }

  printf("This is the end of executing the program!\n");
  return EXIT_SUCCESS;
}