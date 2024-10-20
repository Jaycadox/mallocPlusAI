#include <stdio.h>
#include "mallocPlusAI.h"

int main (void) {
	if (initMallocPlusAI("http://localhost:1234", "neural-chat-7b-v3-1")) {
		fprintf(stderr, "failed to start AI malloc");
		return 1;
	}
	/* setMallocPlusAIBearerAuth("TOKEN"); */
	setMallocPlusAIVerbose(true);
	setMallocPlusAITemperature(1.0); // We live on the edge

	int *arr = mallocPlusAI("int[5]");
	arr[0] = 0;
	arr[1] = 1;
	arr[2] = 2;
	arr[3] = 3;
	arr[4] = 4;
	printf("No segfault :)");

	freeMallocPlusAI();
	return 0;
}
