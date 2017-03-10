Process Management (DAG Traversal)
========================================================================

Contents: **Purpose of the program** | **How to compile the program** | **What exactly the program does** 


Introduction
------------------------------------------------------------------------

This is the source code of a process management tree for traversing a DAG of processes.



Installing
------------------------------------------------------------------------

Compilation:
  
   gcc processmgt.c -o processmgt
   
Running the program:
 
  ./processmgt <graph-file>


Setting up a graph-file to run
------------------------------------------------------------------------

Once your processmgt.c has compiled, you will need a graph-file for it to run.

To make a proper graph file as shown below:

    sleep 10:1:stdin:stdout
    echo "Process P1 running. Dependency to P4 is cleared.":4:stdin:out1.txt
    sleep 15:3:stdin:stdout
    echo "Process P3 running. Dependency to P4 is cleared.":4:stdin:out2.txt
    cat out1.txt out2.txt:5:stdin:cat-out.txt
    grep 3:6:cat-out.txt:grep-out.txt
    wc -l:none:grep-out.txt:wc-out.txt

Make sure that your graph-file is in the format of `<program name with arguments:list of children ID's:input file:output file>`.

Make sure that your graph file is legal according to the above format. Otherwise the program will not run properly.


What exactly the program does
------------------------------------------------------------------------

The program reads the `graph-file` line by line separating the node arguments by `:` and 
identifying the inputs, outputs, commands that are supposed to be run at every stage as well as the child nodes.
The nodes are created in the form of a struct node_t.

Next, the parent nodes of each node are generated and placed in the same struct.

The process tree is then printed.

All nodes first start of with the status INELIGIBLE. Once the parents of a node have finished running or if the node has no parent, 
the status of the node will be set to READY. All READY nodes will then be set to RUNNING once the process associated with the node
is executed. Once execution is done, the status will be set to FINISH. The flow of status is as follows:

INELIGIBLE -> READY -> RUNNING -> FINISH

The program ends when all processes have completed their run.
