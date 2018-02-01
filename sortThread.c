/* Author:      Carl Johnson
 * Course:      CSc 422 Parallel & Distributed Programming
 * Assignment:  HW1 Programming Exercise (a) Multi-Threaded QuickSort
 * Professor:   Patrick Homer
 * Date:        1/31/2018
 */

/* sortThread -- sorts the limes in a given text file using
 * a multi-threaded implementation of quicksort, based on the
 * CLRS quicksort algorithm.
 * The number of threads to create is the first command line argument.
 * The path of the file to be sorted is the second command-line argument.
*/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/types.h>

// Macro to define initial size of the array
#define INIT_ARRAY_SZ 128

/*
 * threadParams -- a struct to hold the parameters
 * for the quicksort and merge functions
 * to allow them to be called by pthread_create().
 * For the quicksort function, the fields
 * ourputArray and mid are left unused.
*/
struct threadParams {
	char **inputArray;
	char **outputArray;
	int lower;
	int mid;
	int upper;
};

// Prototype declaration for main program functions
char **multiThreadSort(char**, int, int);
void *threadQuicksort(void*);
void *threadMerge(void*);
void merge(char**, char **, int, int, int);
void quicksort(char**, int, int);
int partition(char**, int, int);
int selectPivot(char**, int, int);
void swapStr(char**, int, int);
char **extendArray(char**, int, int);



int main (int argc, char *argv[]) {

	// Exit if file does not open
  	if (argc != 3) {
  		fprintf(stderr, "Error: Exactly 2 arguments required:\n%s <numThreads> <fileName>\n",
  				argv[0]);
  		exit(1);
  	}

	// Set number of threads
	int numThreads = atoi(argv[1]);

	// Exit if number of threads is not a power of 2 (max 16)
	if (numThreads != 1 &&
		numThreads != 2 &&
		numThreads != 4 &&
		numThreads != 8 &&
		numThreads != 16) {
			fprintf(stderr, "Error: numThreads must be power of 2 (max 16)\n");
	  		exit(1);
		}

  	// Open file for reading
	FILE *inputFile = fopen(argv[2], "r");

  	// Exit if file does not open
  	if (inputFile == NULL) {
  		fprintf(stderr, "The file \'%s\' does not exist.\n", argv[2]);
  		exit(1);
  	}

	// Create an array to hold lines from the file
	char **linesArray = malloc(INIT_ARRAY_SZ * sizeof(char*));

	// Check for unsuccessful malloc
	if (linesArray == NULL) {
		fprintf(stderr, "ERROR: Out of memory!\n");
		exit(1);
	}

  	// Declare variables for our file-reading loop
  	int arrayLen = INIT_ARRAY_SZ;
  	char *buf = NULL;
  	size_t bufLen = 0;
  	int lineIndex = 0;


	/* Read all lines from the file into an
	 * array of strings, extending the array as needed*/
  	while (getline(&buf, &bufLen, inputFile) != EOF) {

  		// If the array is full, resize it
  		if (lineIndex >= arrayLen) {
  			int newArrayLen = arrayLen * 2;
  			linesArray = extendArray(linesArray, arrayLen, newArrayLen);

			// Check for unsuccessful malloc
  			if (linesArray == NULL) {
  				fprintf(stderr, "ERROR: Out of memory!\n");
  				exit(1);
  			}

			// Reset array length variable
  			arrayLen = newArrayLen;
  		}

  		// Remove final newline characters to standardize lines
  		int lineLen = strlen(buf);
  		if (buf[lineLen - 1] == '\n') {
  			buf[lineLen - 1] = '\0';
  		}

		// Add string pointer to the array
  		linesArray[lineIndex] = buf;
  		buf = NULL;
  		lineIndex++;
  	}

	// Cleanup memory from file input
	free(buf);
	fclose(inputFile);

  	// New variale for total lines in array for code clarity
  	int totalLines = lineIndex;

	/* Keep track of start time for
	 * sort runtime calculation */
	struct timeval startTime, endTime;
	int seconds, micros;
	gettimeofday(&startTime, NULL);

	// Sort the array
	if (totalLines >= numThreads) {
		// Sort the array using threaded quicksort
		linesArray = multiThreadSort(linesArray, totalLines, numThreads);
	}
	else {
		// Sort the array using sequential quicksort
	  	quicksort(linesArray, 0, totalLines - 1);
	}

	/* Print runtime info to stderr for performance testing */
  	gettimeofday(&endTime, NULL);
  	seconds = endTime.tv_sec  - startTime.tv_sec;
  	micros  = endTime.tv_usec - startTime.tv_usec;
  	if ( endTime.tv_usec < startTime.tv_usec ) {
  		micros += 1000000;
  		seconds--;
  	}
  	fprintf(stderr, "runtime: %d seconds, %d microseconds\n", seconds, micros);

  	// Print all lines of the array in order,
	// freeing each line from memory after print
	int i;
  	for (i = 0; i < totalLines; i++) {
  		printf("%s\n", linesArray[i]);
		free(linesArray[i]);
	}

	// Free the array of poitners
	free(linesArray);

	exit(0);
}

/*
 * char **multiThreadSort(char **linesArray, int totalLines, int numThreads) --
 * Breaks linesArray indexes (0 - totalLines) into a number of ranges
 * specified by numThreads, then, using quicksort, sorts each of those
 * ranges alphabetically by the string they point to using separate threads.
 * Afterwards, merges those ranges back together using spearate threads
 * until all string poitners (0 - totalLines) are sorted.
 * Returns a pointer to the sorted array.
*/
char **multiThreadSort(char **linesArray, int totalLines, int numThreads) {

	// Sort/merge variables
	int i, result;

	// Array to hold all
	struct threadParams **paramList = malloc(numThreads * sizeof(struct threadParams*));

	// Check for unsuccessful malloc
	if (linesArray == NULL) {
		fprintf(stderr, "ERROR: Out of memory!\n");
		exit(1);
	}

	// pthread variable declarations
	pthread_t threadID[numThreads];
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);

	// Break up the array and sort each part
	for (i = 0; i < numThreads; i++) {
		paramList[i] = malloc(sizeof(struct threadParams));
		paramList[i]->inputArray = linesArray;
		paramList[i]->lower = i * totalLines / numThreads;
		paramList[i]->upper = (i + 1) * totalLines / numThreads - 1;

		result = pthread_create(&threadID[i], &attr, threadQuicksort,(void *) paramList[i]);

		if (result != 0) {
			fprintf(stderr, "pthread_create failed, result = %d\n", result);
			exit(1);
		}
	}

	// Wait for all threads to exit
	for (i = 0; i < numThreads; i++) {

		result = pthread_join(threadID[i], NULL);

		if ( result != 0 ) {
			fprintf(stderr, "A: join with worker %ld failed, error = %d\n",
					(long) threadID[i], result);
			exit(1);
		}
	}

	// Set up auxiliary array for merging and
	// swapping between two arrays
	char** inputArray = linesArray;
	char** outputArray = malloc(totalLines * sizeof(char*));
	char** temp;

	// Merge until all strings are sorted
	int numMerges = numThreads / 2;
	while (numMerges > 0) {

		// Create new threads for merging
		for (i = 0; i < numMerges; i++) {
			paramList[i] = malloc(sizeof(struct threadParams));
			paramList[i]->inputArray = inputArray;
			paramList[i]->outputArray = outputArray;
			paramList[i]->lower = i * totalLines / numMerges;
			paramList[i]->mid = ((i * totalLines / numMerges) +
						((i + 1) * totalLines / numMerges)) / 2;
			paramList[i]->upper = (i + 1) * totalLines / numMerges - 1;

			result = pthread_create(&threadID[i], &attr, threadMerge, (void *)paramList[i]);

			if (result != 0) {\
				fprintf(stderr, "pthread_create failed, result = %d\n", result);
				exit(1);
			}

		}

		// Wait for all threads to exit
		for (i = 0; i < numMerges; i++) {

			result = pthread_join(threadID[i], NULL);

			if ( result != 0 ) {
				fprintf(stderr, "join with worker %ld failed, error = %d\n",
						(long) threadID[i], result);
				exit(1);
			}
		}

		// Swap input and output array pointers
		temp = inputArray;
		inputArray = outputArray;
		outputArray = temp;

		// Reset number of lerges left
		numMerges = numMerges / 2;
	}

	// Cleanup memory from merge operations
	free(paramList);
	free(outputArray);

	// Return the sorted array
	return inputArray;
}

/*
 * void *threadQuicksort(void *arg) -- A middleman method
 * for calling quicksort in a separate thread.
 * Sets the arguments for merge() from the struct pointer
 * specified by arg.
*/
void *threadQuicksort(void *arg) {
	// Cast arg to threadParams*
	struct threadParams *params = (struct threadParams*) arg;

	// Call merge() with the given arguments
	quicksort(params->inputArray, params->lower, params->upper);

	// Free params from memory
	free(params);

	// Exit thread
	pthread_exit( NULL );
}

/*
 * void *threadMerge(void *arg) -- A middleman method
 * for calling merge in a separate thread.
 * Sets the arguments for merge() from the struct pointer
 * specified by arg.
*/
void *threadMerge(void *arg) {
	// Cast arg to threadParams*
	struct threadParams *params = (struct threadParams*) arg;

	// Call merge() with the given arguments
	merge(params->inputArray, params->outputArray, params->lower, params->mid, params->upper);

	// Free params from memory
	free(params);

	// Exit thread
	pthread_exit( NULL );
}

/* void merge(char **inputArray, char **outputArray, int lower, int mid, int upper) --
 * Precondition: The stirng pointers in the index ranges lower to mid-1 (inclusive)
 * are sorted relative to each other in inputArray,
 * and the stirng pointers in the index ranges mid to upper (inclusive)
 * are sorted relative to each other in inputArray.
 *
 * This method copies all pointers in sorted order from each range in inputArray
 * to the index range lower to upper (inclusive) in outputArray.
*/
void merge(char **inputArray, char **outputArray, int lower, int mid, int upper) {

	// Set up loop variables
	int i = lower;
	int j = mid;
	int k;

	// Merge values
	for (k = lower; k <= upper; k++) {
		if (i < mid && j <= upper) {
			if (strcmp(inputArray[i], inputArray[j]) <= 0) {
				outputArray[k] = inputArray[i];
				i++;
			}
			else {
				outputArray[k] = inputArray[j];
				j++;
			}
		}
		else if (i >= mid) {
			outputArray[k] = inputArray[j];
			j++;
		}
		else {
			outputArray[k] = inputArray[i];
			i++;
		}
  	}
}

/* void quicksort(char **strArray, int lower, int upper) -- Using the
 * quicksort algorithm from CLRS, recursively sorts all elements
 * between and including indexes upper and lower in strArray.
*/
void quicksort(char **strArray, int lower, int upper) {
	if (lower < upper) {
		// Partition range
		int pivot = partition(strArray, lower, upper);
		// Call quickosrt again on each range around pivot
  		quicksort(strArray, lower, pivot - 1);
		quicksort(strArray, pivot + 1, upper);
	}
}

/* void partition(char **strArray, int lower, int upper) --
 * Using the partition algorithm from CLRS, partitions all
 * elements between and including indexes lower and upper
 * in strArray around a pivot chosen by the 'median of 3'
 * method (implemented in selectPivot()).
 * Returns pivot index
*/
int partition(char **strArray, int lower, int upper) {

	//Select the optimal pivot index
  	int pivot = selectPivot(strArray, lower, upper);

	// Move pivot to the end
  	swapStr(strArray, pivot, upper);

	// Save value of pivot string for comparison
  	char* pivotStr = strArray[upper];

	// Variable to keep track of how many strings equivalent
	// to the pivot we encounter
	int eqLines = 0;

  	int j;
  	int strcmpRetVal;
  	int currIndex = lower - 1;

	// Move the values relative to the pivot
  	for (j = lower; j <= upper - 1; j++) {
  		strcmpRetVal = strcmp(strArray[j], pivotStr);
  		if (strcmpRetVal < 0) {
  			currIndex++;
  			swapStr(strArray, currIndex, j);
  		}
  		else if (strcmpRetVal == 0) {
			// Alternate which side equivalent lines are moved
  			eqLines++;
  			if (eqLines % 2 == 0) {
  				currIndex++;
  				swapStr(strArray, currIndex, j);
  			}
  		}
  	}

	// Move pivot back
  	swapStr(strArray, currIndex + 1, upper);

	// Return picot index
  	return currIndex + 1;
}

/* selectPivot(char **strArray, int lower, int upper) --
 * Uses the 'median of 3' approach to select
 * (and return the index of) an optimal pivot between
 * and including lower and upperin strArray for partitioning.
 * Also sorts 3 of the string pointers relative to
 * each other within the array, which will aid future sorting.
 * If the range is 10 or smaller, simply returns upper index.
*/
int selectPivot(char **strArray, int lower, int upper) {

	// Abort if range is too small
 	if (upper - lower <= 10)
  		return upper;

	// Set midpoint
  	int mid = (upper + lower) / 2;

	// Sort the three values
  	if (strcmp(strArray[mid], strArray[lower]) < 0)
  		swapStr(strArray, lower, mid);
  	if (strcmp(strArray[upper], strArray[lower]) < 0)
  		swapStr(strArray, lower, upper);
  	if (strcmp(strArray[upper], strArray[mid]) < 0)
  		swapStr(strArray, mid, upper);

	// Return index of median value
  	return mid;
}

/* swapStr(char **strArray, int indexA, int indexB) --
 * Swaps the string pointers at indexA and indexB in
 * strArray with one another.
*/
void swapStr(char **strArray, int indexA, int indexB) {
	char* temp = strArray[indexA];
  	strArray[indexA] = strArray[indexB];
  	strArray[indexB] = temp;
}

/*
 * extendArray(char **oldArray, int oldLen, int newLen) --
 * Accepts a character pointer array (oldArray) of length oldLen,
 * creates a new, bigger array with the size newLen, and
 * copies all elements from oldArray to the new array.
 * Returns a pointer to the new array.
 * Returns NULL if memory allocation fails for the new array.
*/
char **extendArray(char **oldArray, int oldLen, int newLen) {

	// Create new bigger array
	char **newArray = malloc(newLen * sizeof(char*));
	// Return NULL if malloc fails
	if (newArray == NULL) {
		// malloc() failed. Return NULL
		return NULL;
	}

	//Copy all values over
  	int i;
  	for (i = 0; i < oldLen; i++)
  	{
  		newArray[i] = oldArray[i];
  	}

	// Free old array
  	free(oldArray);

	// Return new array
  	return newArray;
}
