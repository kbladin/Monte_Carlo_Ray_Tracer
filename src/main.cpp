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
		glm::vec3(1),
		glm::vec3(1),
		glm::vec3(1),
		40,
		640,
		480);
	// Objects are contained in the Scene object
	Scene s;

	// intensities will hold image data
	float intensities[c.width() * c.height() * 3]; // w * h * rgb
	// intensities need to be converted to rgb pixel data for displaying
	unsigned char pixel_values[c.width() * c.height() * 3]; // w * h * rgb

	// Loop through all pixels to calculate their intensities by ray-tracing
	for (int x = 0; x < c.width(); ++x)
	{
		for (int y = 0; y < c.height(); ++y)
		{
			// Subsampling loop goes here later
			// Later do ray casting in the scene to determine intensities
			int index = (x + y * c.width()) * 3;
			intensities[index + 0] = 0.1; // Red
			intensities[index + 1] = 0.5; // Green
			intensities[index + 2] = 0.8; // Blue
		}
	}

	// Convert to byte data
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