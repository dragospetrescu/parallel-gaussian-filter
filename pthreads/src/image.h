#ifndef IMAGE_H
#define IMAGE_H

#include "filter.h"

//Loads an .ppm image from a given file
void image_load(const char *image_name);

//Writes the given image to the "image->image_name" file
int image_write(const char *file_name);

//
IMAGE *image_create_blank(IMAGE *source);

//Free
void image_free(IMAGE *image);

//Apply a filter to the image
void apply_filter();

#endif /*IMAGE_H*/
