#include "image.h"
#include "filter.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <semaphore.h>
#include <pthread.h>
#include <omp.h>

int write_pos;
int *ready_lines;


void image_load(const char *image_name) {
	// Declare image structure
	image = (IMAGE*) malloc( sizeof(IMAGE) );

	// Open file
	FILE *file = fopen(image_name, "r");
	if(!file) {
		return ;
	}

	// Read image info
	fscanf(file, "%s", image->header);
	fscanf(file, "%d %d %d", &(image->width), &(image->height), &(image->color_depth));

	// Alocate memory for pixels
	image->pixels = (pixel**) malloc(image->height * sizeof(pixel*));
	pthread_mutex_unlock(&read_initialisation);

	int i, j, k;
	for(i = 0; i < image->height; i++) {
		image->pixels[i] = (pixel*) malloc(image->width * sizeof(pixel));
	}

	// Read pixels
	for(i = 0; i < image->height; i++) {

		for (j = 0; j < image->width; j++) {
			fscanf(file,
				   "%c%c%c",
				   &(image->pixels[i][j].R),
				   &(image->pixels[i][j].G),
				   &(image->pixels[i][j].B));
		}
		if(i < filter->radius || i >= 2 * filter->radius)
			sem_post(&read_semaphore);
	}
	for (k = 0; k < filter->radius + 960; ++k) {
		sem_post(&read_semaphore);
	}
	// Close file
	fclose(file);
}

int image_write(const char *file_name)
{
	// Open file
	FILE *file = fopen(file_name, "w");
	if (!file)
		return 0;


	pthread_mutex_lock(&result_initialisation);
	// Write image info
	fprintf(file,
			"%s\n%d %d\n%d",
			result->header,
			result->width,
			result->height,
			result->color_depth);

	// Write pixels
	int j;
	while (1) {
		sem_wait(&write_semaphore);
		if (write_pos >= result->height)
			break;

		while (ready_lines[write_pos] == 1) {

			for (j = 0; j < result->width; j++)
				fprintf(file,
						"%c%c%c",
						result->pixels[write_pos][j].R,
						result->pixels[write_pos][j].G,
						result->pixels[write_pos][j].B);
			write_pos++;

		}
	}
	// Write EOF
	fprintf(file, "%d", EOF);

	// Close file
	fclose(file);

	return 1;
}


IMAGE *image_create_blank(IMAGE *source) {
	// Declare
	IMAGE *empty_image = (IMAGE*) malloc( sizeof(IMAGE) );

	//Copy info(except pixels)
	strcpy(empty_image->header, source->header);
	empty_image->height = source->height;
	empty_image->width = source->width;
	empty_image->color_depth = source->color_depth;

	// Allocate memory for pixels
	empty_image->pixels = (pixel**) malloc(empty_image->height * sizeof(pixel*));
	int i;
	for(i = 0; i < empty_image->height; i++)
		empty_image->pixels[i] = (pixel*) malloc(empty_image->width * sizeof(pixel));

	return empty_image;
}

void image_free(IMAGE *image) {
	// Free image pixels
	int i;
	for(i = 0; i < image->height; i++)
		free(image->pixels[i]);
	free(image->pixels);

	// Free image
	free(image);
}

// apply_to_pixel is a private function (local scope in image.c; used in apply_filer.c)
void apply_to_pixel(int x, int y, IMAGE *original, IMAGE *result, FILTER *filter) {
	if(x<filter->radius || y<filter->radius || x>=original->width-filter->radius || y>=original->height-filter->radius) {
		result->pixels[y][x] = original->pixels[y][x];
		return;
	}

	int i, j;
	pixel res;
	res.R = res.G = res.B = 0;
	double fil;

	#pragma omp parallel for shared(res)
	for(i = -filter->radius; i <= filter->radius; i++) 
		for(j = -filter->radius; j <= filter->radius; j++) {
			fil = filter->matrix[i+filter->radius][j+filter->radius];
			res.R += fil * original->pixels[y+i][x+j].R;
			res.G += fil * original->pixels[y+i][x+j].G;
			res.B += fil * original->pixels[y+i][x+j].B;
		}
	
	result->pixels[y][x].R = res.R;
	result->pixels[y][x].G = res.G;
	result->pixels[y][x].B = res.B;
}

void apply_filter() {
	pthread_mutex_lock(&read_initialisation);

	result = image_create_blank(image);
	ready_lines = (int *) calloc(image->height, sizeof(int));
	pthread_mutex_unlock(&result_initialisation);

	int x, y;
//	FILE*f = fopen("/home/dragos/log", "w");
//	fprintf(f, "TOTAL %d\n", image->width);

	#pragma omp parallel for shared(x, result)
	for (x = 0; x < image->width; x++) {
//		fprintf(f, "WAIT FOR READ %d\n", x);
		sem_wait(&read_semaphore);
//		printf("APPLY FILTER %d\n", x);
		for(y = 0; y < image->height; y++) {
			apply_to_pixel(x, y, image, result, filter);
		}
		ready_lines[y] = 1;
		sem_post(&write_semaphore);
//		fprintf(f, "DONE %d\n", x);
	}
//	fprintf(f, "DONE\n");
}
