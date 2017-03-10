#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <string.h>

/* This code finds the mean of an input file using 1 or more
 * specified number of threads. For each number, the algorithm will
 * run 5 times to get the average runtime
 *
 * usage: ./meanThreadCode filename no_of_t1 no_of_t2 ...
 */
#define SIZE 524288
#define NUMBER_SIZE 20
#define NUMBER_TIME_AVG 5
void *get_temporal_mean(void *params);	/* thread that computes mean values from the original array*/
void *get_global_mean(void *params);	/* thread that computes global mean from each sub mean*/

int array[SIZE] = {};

double globalMean = 0.0;                 /* final mean value for the whole array*/

typedef struct ArrayWithSize {
    void * array;
    int size;
} ArrayWithSize;

int main (int argc, const char * argv[])
{

    if (argc < 3) {
        printf("usage: %s filename no_of_t1 no_of_t2 ...\n", argv[0]);
        exit(EXIT_FAILURE);
    }

	FILE * fp; // to read file

    int arrayCounter = 0; // to add file inputs into array

    fp = fopen(argv[1], "r");
    if (fp == NULL) {
        printf("Cannot open file\n");
        exit(EXIT_FAILURE);
    }

    char ch;
    char numberBuffer[NUMBER_SIZE];
    int numberBufferCounter = 0;
    int number;

    // convert file input into array of integers
    while ((ch = fgetc(fp))) {
        if (!isspace(ch) && ch != EOF) {
            numberBuffer[numberBufferCounter++] = ch;
        } else {
            // convert string to number
            numberBuffer[numberBufferCounter] = 0;
            number = atoi(numberBuffer);
            // place number in array
            array[arrayCounter++] = number;
            // reset buffer and buffer counter
            memset(numberBuffer, '0', numberBufferCounter);
            numberBufferCounter = 0;
        }

        if (ch == EOF) {
            break;
        }
    }

    // check if file size is correct
    if (SIZE != arrayCounter - 1) {
        printf("Size is wrong. Correct size: %d. Actual size: %d\n", SIZE, arrayCounter - 1);
        exit(EXIT_FAILURE);
    } else {
        printf("File read success!\n");
        printf("Size of array %lu\n", sizeof(array)/sizeof(int));
    }

    // close file
    fclose(fp);

    int number_of_threads;
    int partitionSize;
    double timeArray[NUMBER_TIME_AVG]; // store times for averaging
    double globalMean;
    double timeSpent;
    int argvCount = 2;
    // create aws pointer for global mean
    ArrayWithSize *awsPtr;
    awsPtr = malloc(sizeof(ArrayWithSize));
    // to get the return of joins
    void * join_ret;

    while (argv[argvCount] != NULL) {
        // define number of threads
        number_of_threads = atoi(argv[argvCount++]);
        printf("Number of threads: %d\n", number_of_threads);

        // partitioning the array into numberOfThreads sub_arrays
        partitionSize = SIZE/number_of_threads;
        ArrayWithSize sub_arrays[number_of_threads];
        for (int i = 0; i < number_of_threads; i++) {
            sub_arrays[i].array = array+i*partitionSize;
            sub_arrays[i].size = partitionSize;
        }

        pthread_t workers[number_of_threads];
        // global mean worker
        pthread_t findMean;
        double temp_array[number_of_threads];

        // run five times and get the average time
        for (int t = 0; t < NUMBER_TIME_AVG; t++) {
            // start timer
            clock_t begin = clock();

            /** Run get temporal mean **/
            // start temp mean workers
            for (int i = 0; i < number_of_threads; i++) {
                pthread_create(&workers[i], NULL, get_temporal_mean, &sub_arrays[i]);
            }

            /* now wait for the threads to finish */
            for (int i = 0; i < number_of_threads; i++) {
                pthread_join(workers[i], &join_ret);
                temp_array[i] = *(double *) join_ret;
                free(join_ret);
            }

            /** Run get global mean **/
            awsPtr->array = temp_array;
            awsPtr->size = number_of_threads;

            // start global mean computing thread
            pthread_create(&findMean, NULL, get_global_mean, awsPtr);

            // wait for the final mean computing thread to finish
            pthread_join(findMean, &join_ret);
            globalMean = *(double *) join_ret;
            free(join_ret);

            // end clock
            clock_t end = clock();

            timeArray[t] = (double) (end - begin) / CLOCKS_PER_SEC;
        }
        /** Get average run time **/
        timeSpent = 0;
        for (int i = 0; i < NUMBER_TIME_AVG; i++) {
            timeSpent += timeArray[i];
        }
        timeSpent = timeSpent/NUMBER_TIME_AVG;
        // print out global mean value
        printf("Global Mean Value: %.2f\n", globalMean);
        // print out execution time
        printf("Execution Time: %.5fs\n", timeSpent);
    }
    // free
    free(awsPtr);

    return 0;
}

void *get_temporal_mean(void *params) {
    ArrayWithSize * aws = (ArrayWithSize *) params;
    int * intArray = (int *) aws->array;
    int len = aws->size;
    long sum = 0;
    for (int i = 0; i < len; i++) {
        sum += intArray[i];
    }

    double * avgPtr = malloc(sizeof(double));
    *avgPtr = sum / (double) len;
    pthread_exit(avgPtr);
}

void *get_global_mean(void *params) {
    ArrayWithSize * aws = (ArrayWithSize *) params;
    double * doubleArray = (double *) aws->array;
    int len = aws->size;
    double sum = 0;
    for (int i = 0; i < len; i++) {
        sum += doubleArray[i];
    }
    double * avgPtr = malloc(sizeof(double));
    *avgPtr = sum / (double) len;
    pthread_exit(avgPtr);
}


