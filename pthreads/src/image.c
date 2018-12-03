#include "image.h"
#include "filter.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <semaphore.h>
#include <pthread.h>

int write_pos;

int *ready_lines;

void image_load(const char *image_name)
{
	// Declare image structure
	image = (IMAGE *) malloc(sizeof(IMAGE));

	// Open file
	FILE *file = fopen(image_name, "r");
	if (!file) {
		return;
	}

	// Read image info
	fscanf(file, "%s", image->header);
	fscanf(file,
		   "%d %d %d",
		   &(image->width),
		   &(image->height),
		   &(image->color_depth));

	// Alocate memory for pixels
	image->pixels = (pixel **) malloc(image->height * sizeof(pixel *));
	pthread_mutex_unlock(&read_initialisation);

	int i, j, k;
	for (i = 0; i < image->height; i++) {
		image->pixels[i] = (pixel *) malloc(image->width * sizeof(pixel));
	}

	// Read pixels
	for (i = 0; i < image->height; i++) {

		for (j = 0; j < image->width; j++) {
			fscanf(file,
				   "%c%c%c",
				   &(image->pixels[i][j].R),
				   &(image->pixels[i][j].G),
				   &(image->pixels[i][j].B));
		}
		if (i < filter->radius || i >= 2 * filter->radius)
			sem_post(&read_semaphore);
	}
	for (k = 0; k < filter->radius; ++k) {
		sem_post(&read_semaphore);
	}
	for (k = 0; k < NUM_THREADS; ++k) {
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

IMAGE *image_create_blank(IMAGE *source)
{
	// Declare
	IMAGE *empty_image = (IMAGE *) malloc(sizeof(IMAGE));

	//Copy info(except pixels)
	strcpy(empty_image->header, source->header);
	empty_image->height = source->height;
	empty_image->width = source->width;
	empty_image->color_depth = source->color_depth;

	// Allocate memory for pixels
	empty_image->pixels =
		(pixel **) malloc(empty_image->height * sizeof(pixel *));
	int i;
	for (i = 0; i < empty_image->height; i++)
		empty_image->pixels[i] =
			(pixel *) malloc(empty_image->width * sizeof(pixel));

	return empty_image;
}

void image_free(IMAGE *image)
{
	// Free image pixels
	int i;
	for (i = 0; i < image->height; i++)
		free(image->pixels[i]);
	free(image->pixels);

	// Free image
	free(image);
}

// apply_to_pixel is a private function (local scope in image.c; used in apply_filer.c)
void
apply_to_pixel(int x, int y, IMAGE *original, IMAGE *result, FILTER *filter)
{
	if (x < filter->radius || y < filter->radius
		|| x >= original->width - filter->radius
		|| y >= original->height - filter->radius) {
		result->pixels[y][x] = original->pixels[y][x];
		return;
	}

	int i, j;
	pixel res;
	res.R = res.G = res.B = 0;
	double fil;

	for (i = -filter->radius; i <= filter->radius; i++)
		for (j = -filter->radius; j <= filter->radius; j++) {
			fil = filter->matrix[i + filter->radius][j + filter->radius];
			res.R += fil * original->pixels[y + i][x + j].R;
			res.G += fil * original->pixels[y + i][x + j].G;
			res.B += fil * original->pixels[y + i][x + j].B;
		}

	result->pixels[y][x].R = res.R;
	result->pixels[y][x].G = res.G;
	result->pixels[y][x].B = res.B;
}

void *apply_filter_thread_function(void *input)
{
	int y;
	while (1) {
		sem_wait(&read_semaphore);

		pthread_mutex_lock(&filter_mutex);
		y = filter_index;
		filter_index++;
		pthread_mutex_unlock(&filter_mutex);

		if (y >= image->height) {
			break;
		}

		int x;
		for (x = 0; x < image->width; x++)
			apply_to_pixel(x, y, image, result, filter);

		ready_lines[y] = 1;
		sem_post(&write_semaphore);
	}

}

void apply_filter()
{
	int i;
	pthread_mutex_lock(&read_initialisation);

	result = image_create_blank(image);

	ready_lines = (int *) calloc(image->height, sizeof(int));
	pthread_mutex_unlock(&result_initialisation);

	pthread_t apply_filter_threads[NUM_THREADS];

	for (i = 0; i < NUM_THREADS; ++i) {
		pthread_t apply_filter_thread;
		pthread_create(&apply_filter_thread,
					   NULL,
					   apply_filter_thread_function,
					   NULL);
		apply_filter_threads[i] = apply_filter_thread;

	}

	for (i = 0; i < NUM_THREADS; ++i) {
		pthread_join(apply_filter_threads[i], NULL);
	}
	sem_post(&write_semaphore);
}

