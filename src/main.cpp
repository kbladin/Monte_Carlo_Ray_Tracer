#include <iostream>

#include <glm/glm.hpp>

#include "../include/Camera.h"
#include "../include/Scene.h"

int savePPM(
	const char* file_name,
	const int width,
	const int height,
	unsigned char* data)
{
	FILE *fp = fopen(file_name, "wb"); /* b - binary mode */
	fprintf(fp, "P6\n%d %d\n255\n", width, height);
	fwrite(data, 1, width * height * 3, fp);
	fclose(fp);
	return EXIT_SUCCESS;
}

int main(int argc, char const *argv[])
{
	// The camera is used to cast appropriate initial rays
	Camera c(
		glm::vec3(0, 0, -1), // Eye (position to look at)
		glm::vec3(0, 0, 0), // Center (position of camera)
		glm::vec3(0, 1, 0), // Up vector
		M_PI / 4, // Field of view in radians
		800, // pixel width
		600); // pixel height

	// 3D objects are contained in the Scene object
	Scene s;

	// intensities will hold image data
	float intensities[c.width() * c.height() * 3]; // w * h * rgb
	// intensities need to be converted to rgb pixel data for displaying
	unsigned char pixel_values[c.width() * c.height() * 3]; // w * h * rgb

	// Loop through all pixels to calculate their intensities by ray-tracing
	// This loop could and should be parallellized.
	for (int x = 0; x < c.width(); ++x)
	{
		for (int y = 0; y < c.height(); ++y)
		{
			// Subsampling loop goes here later
			// Trying to cast a ray
			Ray r = c.castRay(
				x, // Pixel x 
				y, // Pixel y 
				0, // Parameter x (>= -0.5 and < 0.5), for subsampling
				0); // Parameter y (>= -0.5 and < 0.5), for subsampling
			SpectralDistribution sd = s.traceRay(r);

			int index = (x + y * c.width()) * 3;
			intensities[index + 0] = sd.data[0]; // Red
			intensities[index + 1] = sd.data[1]; // Green
			intensities[index + 2] = sd.data[2]; // Blue
		}
	}

	// Convert to byte data
	// This conversion should be more sophisticated later on,
	// Gamma correction and / or transformation to logarithmic scale etc.
	// It should also be dependent on the maximum intensity value.
	for (int x = 0; x < c.width(); ++x)
	{
		for (int y = 0; y < c.height(); ++y)
		{
			int index = (x + y * c.width()) * 3;
			pixel_values[index + 0] = char(int (intensities[index + 0] * 255)); // Red
			pixel_values[index + 1] = char(int (intensities[index + 1] * 255)); // Green
			pixel_values[index + 2] = char(int (intensities[index + 2] * 255)); // Blue
		}
	}

	// Save the image data to file
	savePPM("test.ppm", c.width(), c.height(), pixel_values);

	return 0;
}