#include <mpi.h>
#include "image.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

IMAGE *image_load(const char *image_name) {
	// Declare image structure
	IMAGE *image = (IMAGE*) malloc( sizeof(IMAGE) );

	// Open file
	FILE *file = fopen(image_name, "r");
	if(!file)
		return NULL;

	// Read image info
	fscanf(file, "%s", image->header);
	fscanf(file, "%d %d %d", &(image->width), &(image->height), &(image->color_depth));

	// Alocate memory for pixels
	image->pixels = (pixel**) malloc(image->height * sizeof(pixel*));
	int i, j;
	for(i = 0; i < image->height; i++)
		image->pixels[i] = (pixel*) malloc(image->width * sizeof(pixel));

	// Read pixels
	for(i = 0; i < image->height; i++)
		for(j = 0; j < image->width; j++)
			fscanf(file, "%c%c%c", &(image->pixels[i][j].R), &(image->pixels[i][j].G), &(image->pixels[i][j].B));

	// Close file
	fclose(file);

	return image;
}

int image_write(IMAGE *image, const char *file_name) {
	// Open file
	FILE *file = fopen(file_name, "w");
	if(!file)
		return 0;
	
	// Write image info
	fprintf(file, "%s\n%d %d\n%d", image->header, image->width, image->height, image->color_depth);

	// Write pixels
	int i, j;
	for(i = 0; i < image->height; i++)
		for(j = 0; j < image->width; j++)
			fprintf(file, "%c%c%c", image->pixels[i][j].R, image->pixels[i][j].G, image->pixels[i][j].B);

	// Write EOF
	fprintf(file, "%d", EOF);

	// Close file
	fclose(file);

	return 1;
}

IMAGE *image_create_blank(IMAGE *source) {
	// Declare
	IMAGE *image = (IMAGE*) malloc( sizeof(IMAGE) );

	//Copy info(except pixels)
	strcpy(image->header, source->header);
	image->height = source->height;
	image->width = source->width;
	image->color_depth = source->color_depth;

	// Allocate memory for pixels
	image->pixels = (pixel**) malloc(image->height * sizeof(pixel*));
	int i;
	for(i = 0; i < image->height; i++)
		image->pixels[i] = (pixel*) malloc(image->width * sizeof(pixel));

	return image;
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

IMAGE *apply_filter(IMAGE *original, FILTER *filter, int rank, int nProcesses) {
	int x, y, i;
	IMAGE *result = NULL; 
	pixel **pixels = NULL;
	pixel *pixelsResult = NULL;
	int currentProc = 1;
	int shouldReceive = 0;
	int exit = 0;
	int width = 0;

	MPI_Barrier(MPI_COMM_WORLD);

	MPI_Datatype mpi_pixel;
	MPI_Type_contiguous(3, MPI_BYTE, &mpi_pixel);
  	MPI_Type_commit(&mpi_pixel);

	if (rank == 0) {
		width = original->width;

		for (i = 1; i < nProcesses; i++) {
			MPI_Send(&width, 1, MPI_INT, i, 1, MPI_COMM_WORLD);
		}

		result = image_create_blank(original);
		for (y = 0; y < filter->radius; y++) {
			for (x = 0; x < original->width; x++) {
				result->pixels[y][x] = original->pixels[y][x];
			}
		}

		for (y = original->height - filter->radius; y < original->height; y++) {
			for (x = 0; x < original->width; x++) {
				result->pixels[y][x] = original->pixels[y][x];
			}
		}

		
		for (y = 0; y < original->height - 2 * filter->radius; y++) {

			MPI_Send(&exit, 1, MPI_INT, currentProc, 2, MPI_COMM_WORLD);

			if (shouldReceive) {
				MPI_Recv(&result->pixels[filter->radius + y - 3][0], original->width* 3, MPI_BYTE, currentProc, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			}

			MPI_Send(&original->pixels[y][0], filter->radius*original->width* 3, MPI_BYTE, currentProc, 1, MPI_COMM_WORLD);
			MPI_Send(&original->pixels[y+filter->radius][0], original->width* 3, MPI_BYTE, currentProc, 1, MPI_COMM_WORLD);
			MPI_Send(&original->pixels[y+filter->radius+1][0], filter->radius*original->width* 3, MPI_BYTE, currentProc, 1, MPI_COMM_WORLD);

			currentProc++;
			if (currentProc == nProcesses) {
				currentProc = 1;
				shouldReceive = 1;
			}
		}

		exit = 1;

		for (currentProc = 1; currentProc < nProcesses; currentProc++) {
			MPI_Recv(&result->pixels[filter->radius + y - 3][0], (2*filter->radius + 1)*original->width*3, MPI_BYTE, currentProc, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			MPI_Send(&exit, 1, MPI_INT, currentProc, 2, MPI_COMM_WORLD);
			y++;
		}

		return result;
	} else {

		MPI_Recv(&width, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

		pixels = (pixel**) malloc((2*filter->radius + 1) * sizeof(pixel*));
		for (i = 0; i < (2*filter->radius + 1); i++)
			pixels[i] = (pixel*) malloc(width * sizeof(pixel));

		pixelsResult = (pixel*) malloc(width * sizeof(pixel));

		int p;
		MPI_Status stat1;

		while(1) {
			MPI_Recv(&exit, 1, MPI_INT, 0, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			if (exit == 1) {
				break;
			}

			MPI_Status stat;
			int err;

			MPI_Recv(&pixels[0][0], filter->radius*width*3, MPI_BYTE, 0, 1, MPI_COMM_WORLD, &stat);
			MPI_Recv(&pixels[filter->radius][0], width*3, MPI_BYTE, 0, 1, MPI_COMM_WORLD, &stat);
			MPI_Recv(&pixels[filter->radius+1][0], filter->radius*width*3, MPI_BYTE, 0, 1, MPI_COMM_WORLD, &stat);

			MPI_Get_count(&stat, MPI_INT, &err);
			int i, j;

			double fil;
			y = filter->radius;

			for (x = 0; x < width; x++) {
				pixel res;
				res.R = res.G = res.B = 0;


				if (x < filter->radius || x >= width - filter->radius) {
					pixelsResult[x] = pixels[y][x];
					continue;
				}

				for(i = -filter->radius; i <= filter->radius; i++) {

					for(j = -filter->radius; j <= filter->radius; j++) {
						fil = filter->matrix[i+filter->radius][j+filter->radius];
						res.R += fil * pixels[y+i][x+j].R;
						res.G += fil * pixels[y+i][x+j].G;
						res.B += fil * pixels[y+i][x+j].B;
					}
				}

				pixelsResult[x].R = res.R;
				pixelsResult[x].G = res.G;
				pixelsResult[x].B = res.B;
			}

			MPI_Send(pixelsResult, width*3, MPI_BYTE, 0, 1, MPI_COMM_WORLD);
		}

	}

	return result;
}
