#ifndef FILTER_H
#define FILTER_H

#define PI 3.14159265

typedef enum {
	FILTER_AVG, 
	FILTER_GAUSS
} filter_type;

typedef struct {
	int radius;
	double **matrix;
	int type;
} FILTER;

typedef struct {
	unsigned char R;
	unsigned char G;
	unsigned char B;
} pixel;

//.ppm image
typedef struct {
	char header[3];
	int width, height;
	int color_depth;
	pixel **pixels;
} IMAGE;

FILTER *filter_create_avg(int radius);

FILTER *filter_create_gauss(int radius, double sigma);

void filter_print(FILTER *filter);

void filter_free(FILTER *filter);

// The image that is going to be blurred
IMAGE *image;

// The resulting image
IMAGE *result;

// The used filter
FILTER *filter;

int read_lines = 0;
int written_lines = 0;
int filtered_lines = 0;

#endif /*FILTER_H*/
