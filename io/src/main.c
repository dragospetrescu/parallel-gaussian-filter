#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include "image.h"

// The image that is going to be blurred
IMAGE *image;

// The resulting image
IMAGE *result;

// The used filter
FILTER *filter;

sem_t read_semaphore;
sem_t write_sempaphore;
pthread_mutex_t read_initialisation = PTHREAD_MUTEX_INITIALIZER;

void* func_thread_image_load(void *input) {
	printf("Loading image... %s\n", (char *)input);
	image_load((char *)input);
}

void* func_thread_apply_filter(void *input) {
	printf("Appling filter...\n");
	apply_filter();
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

	int error = sem_init(&read_semaphore, 0, 0);
	if(error != 0) {
		printf("ERROR\n");
		return 1;
	}
	error = sem_init(&write_sempaphore, 0, 0);
	if(error != 0) {
		printf("ERROR\n");
		return 1;
	}
	pthread_mutex_lock(&read_initialisation);

	// Create filter
	printf("Creating filter...\n");
	filter = filter_create_gauss(radius, sigma);

	pthread_t load_image_thread;
	pthread_t apply_filter_thread;
	pthread_create(&load_image_thread, NULL, func_thread_image_load, image_file_name);
	pthread_create(&apply_filter_thread, NULL, func_thread_apply_filter, NULL);

	pthread_join(load_image_thread, NULL);
	pthread_join(apply_filter_thread, NULL);

	// Write image to disk
	printf("Writing image to disk...\n");
	image_write(result_file_name);

	// Free memory
	image_free(image);
	image_free(result);
	filter_free(filter);

	printf("DONE!\n");

	return 0;
}
