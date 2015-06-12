#include <iostream>
#include <cmath>
#include <stdio.h>
#include <time.h>

#include <glm/glm.hpp>
//#include <libiomp/omp.h>

#include "../include/Camera.h"
#include "../include/Scene.h"

int savePPM(
	const char* file_name,
	const int width,
	const int height,
	unsigned char* data)
{
	FILE *fp = fopen(file_name, "wb"); // b - binary mode
	fprintf(fp, "P6\n%d %d\n255\n", width, height);
	fwrite(data, 1, width * height * 3, fp);
	fclose(fp);
	return EXIT_SUCCESS;
}

// Get current date/time, format is YYYY-MM-DD.HH:mm:ss
const std::string currentDateTime() {
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);
    // Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
    // for more information about date/time format
    strftime(buf, sizeof(buf), "%Y-%m-%d-%X", &tstruct);

    return buf;
}

int main(int argc, char const *argv[])
{
  
	static const int WIDTH = 320;
	static const int HEIGHT = 240;
	// The camera is used to cast appropriate initial rays
	Camera c(
		glm::vec3(0, 0, -1), // Eye (position to look at)
		glm::vec3(0, 0, 0), // Center (position of camera)
		glm::vec3(0, 1, 0), // Up vector
		M_PI / 3, // Field of view in radians
		WIDTH, // pixel width
		HEIGHT); // pixel height

	// 3D objects are contained in the Scene object
	Scene s;

	// intensities will hold image data
	SpectralDistribution* intensities = new SpectralDistribution[WIDTH * HEIGHT]; // w * h * rgb
	// intensities need to be converted to rgb pixel data for displaying
	unsigned char* pixel_values = new unsigned char[WIDTH * HEIGHT * 3]; // w * h * rgb

	time_t time_start, time_now;
	time(&time_start);

	// Loop through all pixels to calculate their intensities by ray-tracing
	// This loop could and should be parallellized.
	for (int x = 0; x < WIDTH; ++x)
	{
		for (int y = 0; y < HEIGHT; ++y)
		{
			std::random_device rd;
			std::mt19937 gen(rd());
			std::uniform_real_distribution<float> dis(-0.5, 0.5);

			int index = (x + y * c.width());
			SpectralDistribution sd;

			static const int SUB_SAMPLING = 1;

			for (int i = 0; i < SUB_SAMPLING; ++i)
			{
			// Subsampling loop goes here later
			// Trying to cast a ray
			Ray r = c.castRay(
				x, // Pixel x 
				y, // Pixel y 
				dis(gen), // Parameter x (>= -0.5 and < 0.5), for subsampling
				dis(gen)); // Parameter y (>= -0.5 and < 0.5), for subsampling
			sd += s.traceRay(r, 0);
			}
			intensities[index] = sd / SUB_SAMPLING;
		}
		float percent_finished = (x+1) * 100 / float(c.width());
	    time(&time_now);
		double time_elapsed = difftime(time_now, time_start);
		double time_left = (time_elapsed / percent_finished) *
			(100 - percent_finished);
		int hours = time_left / (60 * 60);
		int minutes = (int(time_left) % (60 * 60)) / 60;
		int seconds = int(time_left) % 60;

		std::cout << percent_finished << " \% finished." << std::endl;
		std::cout << "Estimated time left is " << hours << "h:" << minutes << "m:" << seconds << "s." << std::endl;
	}

	// Convert to byte data
	// This conversion should be more sophisticated later on,
	// Gamma correction and / or transformation to logarithmic scale etc.
	// It should also be dependent on the maximum intensity value.
	float gamma = 0.5;
	for (int x = 0; x < c.width(); ++x)
	{
		for (int y = 0; y < c.height(); ++y)
		{
			int index = (x + y * c.width());
			pixel_values[index * 3 + 0] = char(int(glm::clamp(glm::pow(intensities[index][0],gamma), 0.0f, 1.0f) * 255)); // Red
			pixel_values[index * 3 + 1] = char(int(glm::clamp(glm::pow(intensities[index][1],gamma), 0.0f, 1.0f) * 255)); // Green
			pixel_values[index * 3 + 2] = char(int(glm::clamp(glm::pow(intensities[index][2],gamma), 0.0f, 1.0f) * 255)); // Blue
		}
	}

	std::string file_name = currentDateTime() + ".ppm";
  
	// Save the image data to file
	savePPM(file_name.c_str(), WIDTH, HEIGHT, pixel_values);
	
	delete [] intensities;
	delete [] pixel_values;
  
	// Make a beep sound
	std::cout << '\a';
	
	return 0;
}