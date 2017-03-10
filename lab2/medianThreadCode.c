
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

/* This code finds the median of an input file using 1 or more
 * specified number of threads. For each number, the algorithm will
 * run 5 times to get the average runtime
 *
 * usage: ./medianThreadCode filename no_of_t1 no_of_t2 ...
 */

#define SIZE    524288
//#define NUMBER_OF_THREADS	128
#define NUMBER_SIZE 20
#define NUMBER_TIME_AVG 5

void *sorter(void *params);    /* thread that performs sorting for subthread*/
void *merger(void *params);    /* thread that performs merging for final result */
void *find_median(void *params);    /* thread that computes median of the sorted array */
int compare(const void *a, const void *b);
int *merge(int *intArray, int size, int partitionSize);

int array[SIZE] = {};

typedef struct ArrayWithSize {
    void *array;
    int size;
    int secondarySize;
} ArrayWithSize;

double median = 0.0;

int main(int argc, const char *argv[]) {
    if (argc < 3) {
        printf("usage: %s filename no_of_t1 no_of_t2 ...\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    FILE *fp; // to read file

    int arrayCounter = 0; // to add file inputs into array

    fp = fopen(argv[1], "r");
    if (fp == NULL) {
        printf("Cannot open file\n");
        exit(EXIT_FAILURE);
    }

    int number;
    while (fscanf(fp, "%d\n", &array[arrayCounter++]) == 1 && arrayCounter <= SIZE); // read file


    // check if file size is correct
    if (SIZE != arrayCounter - 1) {
        printf("Size is wrong. Correct size: %d. Actual size: %d\n", SIZE, arrayCounter - 1);
        exit(EXIT_FAILURE);
    } else {
        printf("File read success!\n");
        printf("Size of array %lu\n", sizeof(array) / sizeof(int));
    }

    // close file

    fclose(fp);
    int number_of_threads;
    int partitionSize;
    double timeArray[NUMBER_TIME_AVG]; // store times for averaging
    double globalMedian;
    double timeSpent;
    int argvCount = 2;
    // create aws pointer for global mean
    ArrayWithSize *awsPtr;
    awsPtr = malloc(sizeof(ArrayWithSize));
    void *join_ret;
    while (argv[argvCount] != NULL) {
        // define number of threads
        int number_of_threads = atoi(argv[2]);

        // TODO: partition the array list into N sub-arrays, where N is the number of threads
        // define number of threads
        number_of_threads = atoi(argv[argvCount++]);
        printf("Number of threads: %d\n", number_of_threads);

        // partitioning the array into numberOfThreads sub_arrays
        partitionSize = SIZE / number_of_threads;
        ArrayWithSize sub_arrays[number_of_threads];
        for (int i = 0; i < number_of_threads; i++) {
            sub_arrays[i].array = array + i * partitionSize;
            sub_arrays[i].size = partitionSize;
        }

        pthread_t workers[number_of_threads];
        pthread_t merging_thread;
        pthread_t median_thread;

        // run five times and get the average time
        for (int t = 0; t < NUMBER_TIME_AVG; t++) {
            // start timer
            clock_t begin = clock();

            /** Run sorting **/

            // start temp mean workers
            for (int i = 0; i < number_of_threads; i++) {
                pthread_create(&workers[i], NULL, sorter, &sub_arrays[i]);
            }

            /* now wait for the threads to finish */
            for (int i = 0; i < number_of_threads; i++) {
                pthread_join(workers[i], NULL);
            }

            /** Run merge **/

            awsPtr->array = array;
            awsPtr->size = number_of_threads;
            awsPtr->secondarySize = partitionSize;

            pthread_create(&merging_thread, NULL, merger, awsPtr);

            // in-place merge
            pthread_join(merging_thread, NULL);

            /** Run find median **/

            awsPtr->size = SIZE;

            pthread_create(&median_thread, NULL, find_median, awsPtr);

            // wait for the median computing thread to finish
            pthread_join(median_thread, &join_ret);
            globalMedian = *(int *) join_ret;
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

        timeSpent = timeSpent / NUMBER_TIME_AVG;
        // print out global mean value
        printf("Global Median Value: %.2f\n", globalMedian);
        // print out execution time
        printf("Execution Time: %.5fs\n", timeSpent);
    }
    return 0;
}


// You can use any sorting algorithm
void *sorter(void *params) {
    ArrayWithSize *aws = (ArrayWithSize *) params;
    int *intArray = aws->array;
    int size = aws->size;
    qsort(intArray, size, sizeof(int), compare);
    pthread_exit(NULL);
}

// You can use any merging algorithm
void *merger(void *params) {
    ArrayWithSize *aws = (ArrayWithSize *) params;
    int *intArray = aws->array;
    int size = aws->size;
    int partitionSize = aws->secondarySize;
    merge(intArray, size, partitionSize);
    pthread_exit(NULL);
}

int *merge(int *intArray, int size, int partitionSize)  {
    if (size <= 1) {
        return intArray;
    }

    int firstSize = size / 2;
    int secondSize = size - firstSize;
    int *firstArray = merge(intArray, firstSize, partitionSize);
    int *secondArray = merge(intArray + (firstSize * partitionSize), secondSize, partitionSize);
    int firstArrayCount = 0;
    int secondArrayCount = 0;
    int maxFirstArrayCount = partitionSize * firstSize - 1;
    int maxSecondArrayCount = partitionSize * secondSize - 1;
    int * temp_array = malloc(size * partitionSize * sizeof(int));
    int temp_array_counter = 0;
    // merge
    while (firstArrayCount <= maxFirstArrayCount && secondArrayCount <= maxSecondArrayCount) {
        if (firstArray[firstArrayCount] < secondArray[secondArrayCount])
            temp_array[temp_array_counter++] = firstArray[firstArrayCount++];
        else
            temp_array[temp_array_counter++] = secondArray[secondArrayCount++];
    }

    for (;firstArrayCount <= maxFirstArrayCount; temp_array[temp_array_counter++] = firstArray[firstArrayCount++]);
    for (;secondArrayCount <= maxSecondArrayCount; temp_array[temp_array_counter++] = secondArray[secondArrayCount++]);

    memcpy(firstArray, temp_array, partitionSize * size * sizeof(int));

    free(temp_array);
    return firstArray;

}

void *find_median(void *params) {
    ArrayWithSize *aws = (ArrayWithSize *) params;
    int *intArray = aws->array;
    int size = aws->size;
    int * medianPtr = malloc(sizeof(int));
    *medianPtr = array[size/2];
    pthread_exit(medianPtr);
}

int compare(const void *a, const void *b) {
    return (*(int *) a - *(int *) b);
}
