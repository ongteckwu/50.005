/**
 * CSE lab project 3 -- C version
 * 
**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int numberOfCustomers;
int numberOfResources;

int available[10]; // array containing available resources
int maximum[10][10]; // rows: process; columns: resources
int allocation[10][10]; // rows: process; columns: resources
int need[10][10]; // rows: process; columns: resources



void initBank(int resources[], int resourcesNumber, int customerNumber) {
	// TODO: set numberOfCustomers and numberOfResources
	numberOfCustomers = customerNumber;
	numberOfResources = resourcesNumber;
	// TODO: init available/maximum/allocation/need
	for (int i = 0; i < numberOfResources; i++) {
		available[i] = resources[i];
	}
}

void printHeaders() {
	printf("Resources   ");
	for (int i = 0; i < numberOfResources; i++) {
		printf("%4d", i);
	}
	printf("\n");
}

void printResources(int resourceArray[][10]) {
	for (int i = 0; i < numberOfCustomers; i++) {
		printf("Customer %3d", i);
		for (int j = 0; j < numberOfResources; j++) {
			printf("%4d", resourceArray[i][j]);
		}
		printf("\n");
	}
}

void showState() {
	// TODO: print the current state with a tidy format
	printf("\nCurrent state: \n");
	// TODO: print available
	printf("Available: \n");
	printHeaders();
	printf("            ");
	for (int i = 0; i < numberOfResources; i++) {
		printf("%4d", available[i]);
	}
	printf("\n");
	// TODO: print maximum
	printf("\nMaximum: \n");
	printHeaders();
	printResources(maximum);
	// TODO: print allocation
	printf("\nAllocation: \n");
	printHeaders();
	printResources(allocation);
	// TODO: print need
	printf("\nNeed: \n");
	printHeaders();
	printResources(need);
	printf("\n");
}

int addCustomer(int customerId, int maximumDemand[]) {
	// TODO: add customer, update maximum and need
	int size = numberOfResources;
	memcpy(maximum[customerId], maximumDemand, size * sizeof(int));
	memcpy(need[customerId], maximumDemand, size * sizeof(int));
	return 1;
}


int compareArrays(int * array1, int* array2, int size) {
	// if array1 is absolutely smaller than or equal to array2, return 1
	// else, return 0
	for (int i = 0; i < size; i++) {
		if (array1[i] > array2[i]) {
			return 0;
		}
	}
	return 1;
}

void addArrays(int * array1, int* array2, int size) {
	// add array 2 to array 1
	for (int i = 0; i < size; i++) {
		array1[i] += array2[i];
	}
}

int checkSafe(int customerId, int request[]) {
	// TODO: check if the new state is safe
	// returns 1 if safe
	// returns 0 if unsafe
	printf("Checking safety...\n");
	int work[numberOfResources];
	int finish[numberOfCustomers];
	memset(finish, 0, numberOfCustomers * sizeof(int));
	int finishNumber = 0;
	memcpy(work, available, numberOfResources * sizeof(int));
	int toContinue = 0;
	while (finishNumber < numberOfCustomers) {
		for (int i = 0; i < numberOfCustomers; i++) {
			if (!finish[i] && compareArrays(need[i], work, numberOfResources)) {
				addArrays(work, allocation[i], numberOfResources);
				finish[i] = 1;
				finishNumber++;
				toContinue = 1;
			}
		}
		if (toContinue) {
			toContinue = 0;
		} else {
			// non can proceed, return unsafe
			return 0;
		}
	}
	// safe to proceed
	return 1;
}


int requestResources(int customerId, int request[]) {

	// TODO: print the request
	printf("Customer %d requesting request...\n", customerId);
	for (int i = 0; i < numberOfResources; i++) {
		printf("Resource %d: %d\n", i, request[i]);
	}
	// TODO: judge if request larger than need
	int * needArray = need[customerId];
	for (int i = 0; i < numberOfResources; i++) {
		if (request[i] > needArray[i]) {
			printf("Request %d is larger than the need: %d vs %d\n", i, request[i], needArray[i]);
			return 0;
		}
	}
	// TODO: judge if request larger than avaliable
	for (int i = 0; i < numberOfResources; i++) {
		if (request[i] > available[i]) {
			printf("Request %d is larger than the available: %d vs %d\n", i, request[i], available[i]);
			return 0;
		}
	}

	// TODO: judge if the new state is safe if grants this request (for question 2)
	// temporarily change available, allocation[customerId], and need[customerId]
	for (int i = 0; i < numberOfResources; i++) {
		available[i] -= request[i];
		allocation[customerId][i] += request[i];
		need[customerId][i] -= request[i];
	}

	int isSafe = checkSafe(customerId, request);

	// TODO: request is granted, update state
	if (!isSafe) {
		for (int i = 0; i < numberOfResources; i++) {
			available[i] += request[i];
			allocation[customerId][i] -= request[i];
			need[customerId][i] += request[i];
		}
		printf("It's unsafe!\n");
		return 0;
	}
	printf("It's safe!\n");
	return 1;

}


int releaseResources(int customerId, int release[]) {
	// TODO: deal with release (:For simplicity, we do not judge the release request, just update directly)
	return 0;
}


int main (int argc, const char * argv[]) 
{	// Test code for Question 1
	numberOfCustomers = 5;
	numberOfResources = 3;
	int resource[3] = {10, 5, 7};
	initBank(resource, sizeof(resource)/sizeof(int),numberOfCustomers);
	int maximumDemand[5][3] = {
		{7, 5, 3}, 
		{3, 2, 2}, 
		{9, 0, 2},
		{2, 2, 2},
		{4, 3, 3}
		};
	for(int i = 0; i < numberOfCustomers; i++) {
		addCustomer(i,maximumDemand[i]);
	}
	
	int request[5][3] = {
		{0, 1, 0},
		{2, 0, 0},
		{3, 0, 2},
		{2, 1, 1},
		{0, 0, 2}
	};
   requestResources(0,request[0]);
   requestResources(1,request[1]);
   requestResources(2,request[2]);
   requestResources(3,request[3]);
   requestResources(4,request[4]);
   showState();

   // Test code for Question 2, please comment out the following part when test Question 1
   int newRequest1[3] = {1, 0, 2};
   requestResources(1, newRequest1);
   showState();
   int newRequest2[3] = {0, 2, 0};
   requestResources(0, newRequest2);
   showState();
   // End of test code for Question 2
   return 0;
}



