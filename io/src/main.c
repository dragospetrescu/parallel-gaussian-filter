#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "image.h"

void read() {
	printf("READING");
}

void create() {
	printf("CREATE");
}


int main(int argc, char *argv[]) {
	// Info
	char image_file_name[50];
	char result_file_name[50];
	int radius;
	double sigma;

	// Arguments: argv[0]="path", argv[1]="image_name.ppm", argv[2]="result_image_name.ppm" argv[3]="radius" argv[4]="sigma"
	if(argc == 5) {	//If enought arguments given take the info from the them
		//Original image file name
		strcpy(image_file_name, argv[1]);

		// Result image file name
		strcpy(result_file_name, argv[2]);

		// Convert radius
		radius = atoi(argv[3]);
	
		// Convert sigma
		sigma = atof(argv[4]);
	} else { // Read info from keyboard
		// Original image file name
		printf("Original image name: ");
		scanf("%s", image_file_name);
		
		// Result image file name
		printf("Result image name: ");
		scanf("%s", result_file_name);

		// Read radius
		printf("Radius: ");
		scanf("%d", &radius);

		// Read sigma
		printf("Sigma: ");
		scanf("%lf", &sigma);
	}

	pthread_t read_thread, filter_thread;

	pthread_create(&read_thread,NULL, read, NULL);
	pthread_create(&filter_thread,NULL, create, NULL);


	// Load image
	printf("Loading image...\n");
	image = image_load(image_file_name);

	pthread_join(read_thread,NULL);
	pthread_join(filter_thread, NULL);

	// Create filter
	printf("Creating filter...\n");
	filter = filter_create_gauss(radius, sigma);

	// Apply filter
	printf("Appling filter...\n");
	result = apply_filter(image, filter);

	// Write image to disk
	printf("Writing image to disk...\n");
	image_write(result, result_file_name);

	// Free memory
	image_free(image);
	image_free(result);
	filter_free(filter);

	printf("DONE!\n");

	return 0;
}
