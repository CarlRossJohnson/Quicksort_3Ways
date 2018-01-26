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
char **extendArray(char**, int, int);
void quicksort(char**, int, int);
int partition(char**, int, int);
int selectPivot(char**, int, int);
void swapStr(char**, int, int);


int main (int argc, char *argv[]) {

	// Keep track of start time for later runtime calculation
	struct timeval startTime, endTime;
	int seconds, micros;

	gettimeofday(&startTime, NULL);

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


	while (getline(&buf, &bufLen, inputFile) != EOF) {
		// If the array is full, resize it
		if (lineIndex >= arrayLen) {
			int newArrayLen = arrayLen * 2;
			linesArray = extendArray(linesArray, arrayLen, newArrayLen);
			if (linesArray == NULL) {
				fprintf(stderr, "ERROR: Out of memory!\n");
				exit(1);
			}
			arrayLen = newArrayLen;
		}

		// Remove final newline characters to standardize lines
		int lineLen = strlen(buf);
		if (buf[lineLen - 1] == '\n') {
			buf[lineLen - 1] = '\0';
		}


		linesArray[lineIndex] = buf;
		buf = NULL;
		lineIndex++;
	}

	// New variale for total lines in array for code clarity
	int totalLines = lineIndex;

	// Sort the array using quicksort
	quicksort(linesArray, 0, totalLines - 1);

	// Print all lines of the array in order
	int i;
	for (i = 0; i < totalLines; i++) {
		printf("%s\n", linesArray[i]);
	}

	// Print runtime info to stderr for performance testing
	gettimeofday(&endTime, NULL);
	seconds = endTime.tv_sec  - startTime.tv_sec;
	micros  = endTime.tv_usec - startTime.tv_usec;
	if ( endTime.tv_usec < startTime.tv_usec ) {
		micros += 1000000;
		seconds--;
	}
	fprintf(stderr, "runtime: %d seconds, %d microseconds\n", seconds, micros);

	exit(0);
}

/* void quicksort(char **strArray, int lower, int upper) -- Using the
 * quicksort algorithm from CLRS, recursivelt sorts all elements in
 * strArray.
*/
void quicksort(char **strArray, int lower, int upper) {
	if (lower < upper) {
		int pivot = partition(strArray, lower, upper);
		quicksort(strArray, lower, pivot - 1);
		quicksort(strArray, pivot + 1, upper);
	}
}

/* void partition(char **strArray, int lower, int upper) --
 * Using the partition algorithm from CLRS, partitions all
 * elements between indexes lower and upper in strArray.
*/
int partition(char **strArray, int lower, int upper) {
	int eqLines = 0;
	int pivot = selectPivot(strArray, lower, upper);
	swapStr(strArray, pivot, upper);
	char* pivotStr = strArray[upper];

	int j;
	int strcmpRetVal;
	int currIndex = lower - 1;

	for (j = lower; j <= upper - 1; j++) {
		strcmpRetVal = strcmp(strArray[j], pivotStr);
		if (strcmpRetVal < 0) {
			currIndex++;
			swapStr(strArray, currIndex, j);
		}
		else if (strcmpRetVal == 0) {
			eqLines++;
			if (eqLines % 2 == 0) {
				currIndex++;
				swapStr(strArray, currIndex, j);
			}
		}
	}
	swapStr(strArray, currIndex + 1, upper);
	return currIndex + 1;
}

/* swapStr(char **strArray, int indexA, int indexB) --
 * Swaps the string pointers at the given indexes with one another
*/
void swapStr(char **strArray, int indexA, int indexB) {
	char* temp = strArray[indexA];
	strArray[indexA] = strArray[indexB];
	strArray[indexB] = temp;
}

/* selectPivot(char **strArray, int lower, int upper) --
 * Uses the 'median of 3' approach to select an optimal
 * pivot bewteen lower and upper for partitioning.
 * If the range is 10 or smaller, simply returns upper index.
*/
int selectPivot(char **strArray, int lower, int upper) {
	if (upper - lower <= 10)
		return upper;

	int mid = (upper + lower) / 2;

	if (strcmp(strArray[mid], strArray[lower]) < 0)
		swapStr(strArray, lower, mid);
	if (strcmp(strArray[upper], strArray[lower]) < 0)
		swapStr(strArray, lower, upper);
	if (strcmp(strArray[upper], strArray[mid]) < 0)
		swapStr(strArray, mid, upper);

	return mid; // index of median value
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
	char **newArray = malloc(newLen * sizeof(char*));
	if (newArray == NULL) {
		// malloc() failed. Return NULL
		return NULL;
	}

	int i;
	for (i = 0; i < oldLen; i++)
	{
		newArray[i] = oldArray[i];
	}

	free(oldArray);

	return newArray;
 }
