/*
 * Author:      Carl Johnson
 * Course:      CSc 422 Parallel & Distributed Programming
 * Assignment:  HW1 Programming Exercise (a) Sequentual Quicksort
 * Professor:   Patrick Homer
 * Date:        1/31/2018
*/

/* sortSeq -- sorts the limes in a given text file using
 * a recursive inplementation of quicksort, based on the
 * CLRS quicksort algorithm.
 * The path of the file to be sorted is provided as the first
 * command-line argument.
*/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

// Macro to define initial size of the array
#define INIT_ARRAY_SZ 128

// Prototype declaration for main program functions
void quicksort(char**, int, int);
int partition(char**, int, int);
int selectPivot(char**, int, int);
void swapStr(char**, int, int);
char **extendArray(char**, int, int);


int main (int argc, char *argv[]) {

	// Exit if file does not open
	if (argc != 2) {
		fprintf(stderr, "Error: Exactly 1 argument required:\n%s [filename]\n",
				argv[0]);
		exit(1);
	}

	// Open file for reading
	FILE *inputFile = fopen(argv[1], "r");

	// Exit if file does not open
	if (inputFile == NULL) {
		fprintf(stderr, "The file \'%s\' does not exist.\n", argv[1]);
		exit(1);
	}

	// Create an array to hold lines from the file
	char **linesArray = malloc(INIT_ARRAY_SZ * sizeof(char*));
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

	// Keep track of start time for sort runtime calculation
	struct timeval startTime, endTime;
	int seconds, micros;
	gettimeofday(&startTime, NULL);

	// Sort the array using quicksort
	quicksort(linesArray, 0, totalLines - 1);

	// Print runtime info to stderr for performance testing
	gettimeofday(&endTime, NULL);
	seconds = endTime.tv_sec  - startTime.tv_sec;
	micros  = endTime.tv_usec - startTime.tv_usec;
	if ( endTime.tv_usec < startTime.tv_usec ) {
		micros += 1000000;
		seconds--;
	}
	fprintf(stderr, "runtime: %d seconds, %d microseconds\n", seconds, micros);


	// Print all lines of the array in order
	int i;
	for (i = 0; i < totalLines; i++) {
		printf("%s\n", linesArray[i]);
		free(linesArray[i]);
	}

	free(linesArray);

	exit(0);
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
  	return mid; // index of median value
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
