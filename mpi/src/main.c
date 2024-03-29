#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "image.h"

int main(int argc, char *argv[]) {

	int rank;
	int nProcesses;

	// The image that is going to be blurred
	IMAGE *image = NULL;

	// The resulting image
	IMAGE *result = NULL;

	// The used filter
	FILTER *filter;

	// Info
	char image_file_name[50];
	char result_file_name[50];
	int radius;
	double sigma;

	MPI_Init(&argc, &argv);
	MPI_Status stat;

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &nProcesses);

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

	if (rank == 0) {
		// Load image
		printf("Loading image...\n");
		image = image_load(image_file_name);
	}
	
	// Create filter
	printf("Creating filter...\n");
	filter = filter_create_gauss(radius, sigma);

	// Apply filter
	printf("Appling filter...\n");
	result = apply_filter(image, filter, rank, nProcesses);

	MPI_Barrier(MPI_COMM_WORLD);
	if (rank == 0) {
		// Write image to disk
		printf("Writing image to disk...\n");
		image_write(result, result_file_name);

		// Free memory
		image_free(image);
		image_free(result);
	}

	MPI_Barrier(MPI_COMM_WORLD);
	filter_free(filter);

	printf("DONE!\n");
	MPI_Finalize();
	return 0;
}
