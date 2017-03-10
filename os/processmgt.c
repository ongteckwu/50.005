#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <sys/wait.h>
#include <fcntl.h>

#define INELIGIBLE 0
#define READY 1
#define RUNNING 2
#define FINISHED 3

#define MAX_LENGTH 1024
#define MAX_PARENTS 10
#define MAX_CHILDREN 10
#define MAX_NODES 50

#define CREATE_FLAGS (O_WRONLY | O_CREAT | O_TRUNC)
#define CREATE_PERMS (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

typedef struct node {
  int id; // corresponds to line number in graph text file
  char prog[MAX_LENGTH]; // program name
  char *args[MAX_LENGTH/2 + 1]; // argument list for program
  int num_args; // num of arguments
  char input[MAX_LENGTH]; // in filename e.g. stdin
  char output[MAX_LENGTH]; // out filename
  int parents[MAX_PARENTS]; // parents ids
  int num_parents;
  int children[MAX_CHILDREN]; // children ids
  int num_children;
  int status; // INELIGIBLE/ READY/ RUNNING/ FINISHED
  pid_t pid; // Process id when it's running
} node_t;

typedef struct edge {
  int i; // in vertex
  int o; // out vertex
} edge_t;

/**
 * Compare two ints
 */
int cmpfunc(const void * a, const void * b)
{
  return ( *(int*)a - *(int*)b );
}

/**
 * Search for tokens in the string s, separated by the characters in 
 * delimiters. Populate the string array at *tokens.
 *
 * Return the number of tokens parsed on success, or -1 and set errno on 
 * failure.
 */
int parse_tokens(const char *s, const char *delimiters, char ***tokens) {
  const char *s_new;
  char *t;
  int num_tokens;
  int errno_copy;

  /* Check arguments */
  if ((s == NULL) || (delimiters == NULL) || (tokens == NULL)) {
    errno = EINVAL;
    return -1;
  }

  /* Clear token array */
  *tokens = NULL;

  /* Ignore initial segment of s that only consists of delimiters */
  s_new = s + strspn(s, delimiters);

  /* Make a copy of s_new (strtok modifies string) */
  t = (char *) malloc(strlen(s_new) + 1);
  if (t == NULL) {
    return -1;    
  }
  strcpy(t, s_new);

  /* Count number of tokens */
  num_tokens = 0;
  if (strtok(t, delimiters) != NULL) {
    for (num_tokens = 1; strtok(NULL, delimiters) != NULL; num_tokens++) ;
  }

  /* Allocate memory for tokens */
  *tokens = (char**) malloc((num_tokens + 1)*sizeof(char *));
  if (*tokens == NULL) {
    errno_copy = errno;
    free(t);  // ignore errno from free
    errno = errno_copy;  // retain errno from malloc
    return -1;
  }

  /* Parse tokens */
  if (num_tokens == 0) {
    free(t);
  } else {
    strcpy(t, s_new);
    **tokens = strtok(t, delimiters);
    for (int i=1; i<num_tokens; i++) {
      *((*tokens) +i) = strtok(NULL, delimiters);      
    }
  }
  *((*tokens) + num_tokens) = NULL;  // end with null pointer

  return num_tokens;
}

void free_parse_tokens(char **tokens) {
  if (tokens == NULL) {
    return;    
  }
  
  if (*tokens != NULL) {
    free(*tokens);    
  }

  free(tokens);
}

/**
 * Parse the input line at line, and populate the node at node, which will
 * have id set to id.
 * 
 * Return 0 on success or -1 and set errno on failure.
 */
int parse_input_line(char *line, int id, node_t *node) {
  char **strings;  // string array
  char **arg_list;  // string array
  char **child_list;  // string array
  int a;

  /* Split the line on ":" delimiters */
  if (parse_tokens(line, ":", &strings) < 0) {
    perror("Failed to parse node information");
    return -1;
  }

  /* Parse the space-delimited argument list */
  if (parse_tokens(strings[0], " ", &arg_list) < 0) {
    perror("Failed to parse argument list");
    free_parse_tokens(strings);
    return -1;
  }

  /* Parse the space-delimited child list */
  if (parse_tokens(strings[1], " ", &child_list) < 0) {
    perror("Failed to parse child list");
    free_parse_tokens(strings);
    return -1;
  }

  /* Set node id */
  node->id = id;
  fprintf(stderr, "... id = %d\n", node->id);

  /* Set program name */
  strcpy(node->prog, arg_list[0]);
  fprintf(stderr, "... prog = %s\n", node->prog);

  /* Set program arguments */
  for (a = 0; arg_list[a] != NULL; a++) {
    node->args[a] = arg_list[a];
    node->num_args++;
    fprintf(stderr, "... arg[%d] = %s\n", a, node->args[a]);
  }
  node->args[a] = NULL;
  fprintf(stderr, "... arg[%d] = %s\n", a, node->args[a]);

  /* Set input file */
  strcpy(node->input, strings[2]);
  fprintf(stderr, "... input = %s\n", node->input);
  
  /* Set output file */
  strcpy(node->output, strings[3]);
  fprintf(stderr, "... output = %s\n", node->output);
    
  /* Set child nodes */
  node->num_children = 0;
  if (strcmp(child_list[0], "none") != 0) {
    for (int c = 0; child_list[c] != NULL; c++) {
      if (c < MAX_CHILDREN) {
        if (atoi(child_list[c]) != id) {
          node->children[c] = atoi(child_list[c]);
          fprintf(stderr, "... child[%d] = %d\n", c, node->children[c]);
          node->num_children++;
        } else {
          perror("Node cannot be a child of itself");
          return -1;
        }
      } else {
        perror("Exceeded maximum number of children per node");
        return -1;
      }
    }
  }
  fprintf(stderr, "... num_children = %d\n", node->num_children);

//  free_parse_tokens(strings);
//  free_parse_tokens(child_list);
//  free_parse_tokens(arg_list);

  return 0;
}

/**
 * Parse the file at file_name, and populate the array at n.
 * 
 * Return the number of nodes parsed on success, or -1 and set errno on
 * failure.
 */
int parse_graph_file(char *file_name, node_t *node) {
  FILE *f;
  char line[MAX_LENGTH];
  int id = 0;
  int errno_copy;

  /* Open file for reading */
  fprintf(stderr, "Opening file...\n");
  f = fopen(file_name, "r");
  if (f == NULL) {
    perror("Failed to open file");
    return -1;
  }

  /* Read file line by line */
  fprintf(stderr, "Reading file...\n");
  while (fgets(line, MAX_LENGTH, f) != NULL) {
    strtok(line, "\n");  // remove trailing newline

    /* Parse line */
    fprintf(stderr, "Parsing line %d...\n", id);
    if (parse_input_line(line, id, node) == 0) {
      node++;  // increment pointer to point to next node in array
      id++;  // increment node ID
      if (id >= MAX_NODES) {
        perror("Exceeded maximum number of nodes");
        return -1;
      }
    } else {
      perror("Failed to parse input line");
      return -1;
    }
  }

  /* Handle file reading errors and close file */
  if (ferror(f)) {
    errno_copy = errno;
    fclose(f);  // ignore errno from fclose
    errno = errno_copy;  // retain errno from fgets
    perror("Error reading file");
    return -1;
  }

  /* If no file reading errors, close file */
  if (fclose(f) == EOF) {
    perror("Error closing file");
    return -1;  // stream was not successfully closed
  }
  
  /* If no file closing errors, return number of nodes parsed */  
  return id;
}

/**
 * Parses the process tree represented by nodes and determines the parent(s)
 * of each node.
 *
 * Returns 0
 */
int parse_node_parents(node_t *nodes, int num_nodes) {
  /**
    * Loops through each node and then loops through each node again
    * and check whether the current node is a child of the second loop's
    * current node. If so, add the second loop's current node into the
    * parent list of the first loop's current node.
   */

  node_t *current_node = nodes;
  node_t *second_current_node;
  int current_id;
  int *search_result;
  int p;

  for (int i = 0; i < num_nodes; i++) {
    // restart second current node and set current_id
    current_id = current_node->id;
    second_current_node = nodes;
    p = 0; // parent list index
    for (int j = 0; j < num_nodes; j++) {
      search_result = (int *) bsearch(&current_id, second_current_node->children,
                              second_current_node->num_children, sizeof(int), cmpfunc);

      if (search_result != NULL) {
        // current_node exists in children
        // add second_current_node's id to current_node's parent list
        current_node->parents[p++] = second_current_node->id;
        current_node->num_parents++;
      }
      // go to next node
      second_current_node++;
    }
    // go to next node
    current_node++;

  }

  return 0;

}

/**
 * Checks the status of each node in the process tree represented by nodes and 
 * verifies whether it can progress to the next stage in the cycle:
 *
 * INELIGIBLE -> READY -> RUNNING -> FINISHED
 *
 * IF INELIGIBLE && parents are done
 *  set to READY
 *
 *  IF RUNNING & is done
 *  set to FINISHED
 * Returns the number of nodes that have finished running, or -1 if there was 
 * an error.
 */
int parse_node_status(node_t *nodes, int num_nodes) {
  int * parents;
  int parents_done;

  // Set INELIGIBLE to READY
  for (int i = 0; i < num_nodes; i++) {
    if (nodes[i].status == INELIGIBLE) {
      if (nodes[i].num_parents == 0) {
        // if no parent
        nodes[i].status = READY;
      }
      else {
        // if not, check if all parents are done
        parents_done = 1;
        parents = nodes[i].parents;
        for (int j = 0; j < nodes[i].num_parents; j++) {
          if (nodes[parents[j]].status != FINISHED) {
            parents_done = 0;
            break;
          }
        }

        if (parents_done) {
          nodes[i].status = READY;
        }
      }
    }
  }

  pid_t result;
  int wait_status;

  // set to FINISH if finished RUNNING
  for (int i = 0; i < num_nodes; i++) {
    if (nodes[i].status == RUNNING) {
      result = waitpid(nodes[i].pid, &wait_status, WNOHANG);
      if (result < 0) {
        switch(errno) {
          case ECHILD: {
            perror("Process does not exist!");
            return -1;
          }
          default: {
            perror("Something wrong with waitpid...");
            return -1;
          }
        }
      }
      else if (result > 0) {
        // node done
        fprintf(stderr, "line %d done\n", i);
        nodes[i].status = FINISHED;
      }
    }
  }

  // Get finished count
  int finished_count = 0;
  for (int i = 0; i < num_nodes; finished_count += (nodes[i++].status == FINISHED));
  return finished_count;
}

/**
 * Prints the process tree represented by nodes to standard error.
 *
 * Returns 0 if printed successfully.
 *
 */
int print_process_tree(node_t *nodes, int num_nodes) {
  /**
   * Perform a topological sort and print the edges of each row
   * For nodes with no parents, s-><node num> will be printed.
   * For nodes with parents, <parent num>-><node num> will be printed.
   */
  int num_nodes_done = 0;
  edge_t * edges_to_print[num_nodes];
  edge_t * edge; // placeholder
  int parent;
  int child;
  int num_edges_to_print;
  int num_of_parents_not_visited[num_nodes];
  int temp_not_visited[num_nodes];

  // populate number of parents not visited
  for (int i = 0; i < num_nodes; i++) {
    num_of_parents_not_visited[i] = nodes[i].num_parents;
  }

  // for state change invariance
  memcpy(temp_not_visited, num_of_parents_not_visited, num_nodes * sizeof(int));

  // populate number of parents not visited
//  for (int i = 0; i < num_nodes; i++) {
//    printf("%d ", num_of_parents_not_visited[i]);
//  }
//
//  printf("\n");

  while (num_nodes_done < num_nodes) {
    num_edges_to_print = 0; // reset number of edges to print
    // get nodes with no parents
    for (int i = 0; i < num_nodes; i++) {
      if (num_of_parents_not_visited[i] == 0) {
        num_nodes_done++;
        temp_not_visited[i]--; // set to -1 so that it won't be revisited

        // create the edges to be printed
        if (nodes[i].num_parents == 0) {
          // if no parent
          edge = (edge_t *) malloc(sizeof(edge));
          edge->i = -1;
          edge->o = i;
          edges_to_print[num_edges_to_print++] = edge;
        } else {

          for (int j = 0; j < nodes[i].num_parents; j++) {
            edge = (edge_t *) malloc(sizeof(edge));
            parent = nodes[i].parents[j];
            edge->i = parent;
            edge->o = i;
            edges_to_print[num_edges_to_print++] = edge;
          }
        }

        // reduce parent_visited number of all nodes whose parent is the current node by 1
        for (int j = 0; j < nodes[i].num_children; j++) {
          child = nodes[i].children[j];
          temp_not_visited[child]--;
        }
      }
    }

    if (num_edges_to_print > 0) {
      // print edges
      for (int i = 0; i < num_edges_to_print; i++) {
        edge = edges_to_print[i];
        if (edge->i != -1) {
          fprintf(stderr, "%d->%d | ", edge->i, edge->o);
        } else { // no parent
          fprintf(stderr, "s->%d | ", edge->o);
        }
        // free edge
        free(edge);
      }
      fprintf(stderr, "\n");
    }

    // do a state change for not_visited
    memcpy(num_of_parents_not_visited, temp_not_visited, num_nodes * sizeof(int));
  }

  return 0;

}

/**
 * Takes in a graph file and executes the programs in parallel.
 */
int main(int argc, char *argv[]) {
  node_t nodes[MAX_NODES];
  int num_nodes;
  int num_nodes_finished;

  /* Check command line arguments */
  if (argc < 2) {
    printf("Usage: ./processmgt <filename>\n");
    exit(EXIT_FAILURE);
  }

  char * filename = argv[1];
  /* Parse graph file */
  fprintf(stderr, "Parsing graph file...\n");
  if ((num_nodes = parse_graph_file(filename, nodes)) < 0) {
    perror("Parse graph failure");
    return EXIT_FAILURE;
  };

  /* Parse nodes for parents */
  fprintf(stderr, "Parsing node parents...\n");

  parse_node_parents(nodes, num_nodes);

  /* Print process tree */
  fprintf(stderr, "\nProcess tree:\n");

  print_process_tree(nodes, num_nodes);

  /* Run processes */
  fprintf(stderr, "Running processes...\n");

  // set all statuses to INELIGIBLE
  for (int i = 0; i < num_nodes; i++) {
    nodes[i].status = INELIGIBLE;
  }
  num_nodes_finished = 0;
  node_t * current_node;

  while (num_nodes_finished < num_nodes) {
    if ((num_nodes_finished = parse_node_status(nodes, num_nodes)) < 0) {
      perror("Error in check node status. ");
      return EXIT_FAILURE;
    }

    // If processes are READY, set them to RUNNING
    pid_t child_pid;
    for (int i = 0; i < num_nodes; i++) {
      current_node = nodes + i;
      if (current_node->status == READY) {
        // run process
        child_pid = fork();
        if (child_pid == 0) {
          // child
          // check if stdin and stdout need to be replaced
          int input;
          int output;

          if (strncmp(current_node->input, "stdin", 5) != 0) {
            // open file and dup2 file descriptor on 0
            if ((input = open(current_node->input, O_RDONLY)) < 0) {
              perror("File not found or file does not exist");
              return EXIT_FAILURE;
            }
            if (dup2(input, STDIN_FILENO) < 0) {
              perror("Something from with opening file for read");
              return EXIT_FAILURE;
            }

          }

          if (strncmp(current_node->output, "stdout", 6) != 0) {
            // open file and dup2 file descriptor on 1
            if ((output = open(current_node->output, CREATE_FLAGS, CREATE_PERMS)) < 0) {
              perror("File not found or file does not exist");
              return EXIT_FAILURE;
            }
            if (dup2(output, STDOUT_FILENO) < 0) {
              perror("Something from with opening file for write");
              return EXIT_FAILURE;
            }
          }

          execvp(current_node->prog, current_node->args);
          perror("Failed to execute the node prog!");
          return EXIT_FAILURE;

        } else if (child_pid < 0) {
          switch(errno) {
            case EAGAIN: {
              perror("Limit of total number of processes has reached.");
              return EXIT_FAILURE;
            }
            case ENOMEM: {
              perror("Insufficient swap space");
              return EXIT_FAILURE;
            }
          }
        } else {
          // parent
          current_node->status = RUNNING;
          current_node->pid = child_pid;
        }

      }
    }
  }

  if (num_nodes_finished < 0) {
    perror("Error executing processes");
    return EXIT_FAILURE;
  }


  fprintf(stderr, "All processes finished. Exiting.\n");
  return EXIT_SUCCESS;
}
