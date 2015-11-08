#include <iostream>
#include <fstream>
#include <cmath>
#include <stdio.h>
#include <time.h>

#include <glm/glm.hpp>
#include <omp.h>

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
    strftime(buf, sizeof(buf), "%Y-%m-%d-%X", &tstruct);

    return buf;
}

int main(int argc, char const *argv[])
{
	time_t time_start, time_now, rendertime_start;
	time(&time_start);

	static const int WIDTH = 1024 / 1;
	static const int HEIGHT = 768 / 1;
	static const int SUB_SAMPLING_CAUSTICS = 10;
	static const int SUB_SAMPLING_MONTE_CARLO = 500;
	static const int SUB_SAMPLING_DIRECT_SPECULAR = 100;
	static const int NUMBER_OF_PHOTONS_EMISSION = 2000000;

	// The camera is used to cast appropriate initial rays
	Camera c(
		glm::vec3(0, 0, 3.2), // Eye (position of camera)
		glm::vec3(0, 0, 0), // Center (position to look at)
		glm::vec3(0, 1, 0), // Up vector
		M_PI / 3, // Field of view in radians
		WIDTH, // pixel width
		HEIGHT); // pixel height
	glm::vec3 camera_plane_normal = glm::normalize(c.center - c.eye);

	// 3D objects are contained in the Scene object
	Scene s(argv[1]);

	// irradiance_values will hold image data
	SpectralDistribution* irradiance_values = new SpectralDistribution[c.WIDTH * c.HEIGHT];
	// irradiance_values need to be converted to rgb pixel data for displaying
	unsigned char* pixel_values =
		new unsigned char[c.WIDTH * c.HEIGHT * 3]; // w * h * rgb

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> dis(-0.5, 0.5);

	std::cout << "Building photon map." << std::endl;
	s.buildPhotonMap(NUMBER_OF_PHOTONS_EMISSION);

	float rendering_percent_finished = 0;
	std::cout << "Rendering started!" << std::endl;
	std::cout << rendering_percent_finished << " \% finished." << std::endl;

	time(&rendertime_start);

	double prerender_time = difftime(rendertime_start, time_start);

	// Loop through all pixels to calculate their irradiance_values by ray-tracing
	for (int x = 0; x < c.WIDTH; ++x)
	{
		// Parallellize the for loop with openMP.
		#pragma omp parallel for
		for (int y = 0; y < c.HEIGHT; ++y)
		{
			int index = (x + y * c.WIDTH);
			SpectralDistribution sd;
			if (SUB_SAMPLING_DIRECT_SPECULAR)
			{
				for (int i = 0; i < SUB_SAMPLING_DIRECT_SPECULAR; ++i)
				{
					Ray r = c.castRay(
						x, // Pixel x
						(c.HEIGHT - y - 1), // Pixel y 
						dis(gen), // Parameter x (>= -0.5 and < 0.5), for subsampling
						dis(gen)); // Parameter y (>= -0.5 and < 0.5), for subsampling
					sd += s.traceRay(r, Scene::WHITTED_SPECULAR) * glm::dot(r.direction, camera_plane_normal);
				}
				irradiance_values[index] += sd / SUB_SAMPLING_DIRECT_SPECULAR * (2 * M_PI);				
			}
			sd = SpectralDistribution();
			if (SUB_SAMPLING_CAUSTICS)
				{
				for (int i = 0; i < SUB_SAMPLING_CAUSTICS; ++i)
				{
					Ray r = c.castRay(
						x, // Pixel x
						(c.HEIGHT - y - 1), // Pixel y 
						dis(gen), // Parameter x (>= -0.5 and < 0.5), for subsampling
						dis(gen)); // Parameter y (>= -0.5 and < 0.5), for subsampling
					sd += s.traceRay(r, Scene::CAUSTICS) * glm::dot(r.direction, camera_plane_normal);
				}
				irradiance_values[index] += sd / SUB_SAMPLING_CAUSTICS * (2 * M_PI);
			}
			sd = SpectralDistribution();
			if (SUB_SAMPLING_MONTE_CARLO)
				{
				for (int i = 0; i < SUB_SAMPLING_MONTE_CARLO; ++i)
				{
					Ray r = c.castRay(
						x, // Pixel x
						(c.HEIGHT - y - 1), // Pixel y 
						dis(gen), // Parameter x (>= -0.5 and < 0.5), for subsampling
						dis(gen)); // Parameter y (>= -0.5 and < 0.5), for subsampling
					sd += s.traceRay(r, Scene::MONTE_CARLO) * glm::dot(r.direction, camera_plane_normal);
				}
				irradiance_values[index] += sd / SUB_SAMPLING_MONTE_CARLO * (2 * M_PI);
			}
		}

		// To show how much time we have left.
		rendering_percent_finished = (x+1) * 100 / float(c.WIDTH);
	  	time(&time_now);
		double rendering_time_elapsed = difftime(time_now, rendertime_start);
		double rendering_time_left = (rendering_time_elapsed / rendering_percent_finished) *
			(100 - rendering_percent_finished);

		double total_time_elapsed = prerender_time + rendering_time_elapsed;
		double total_time_estimate = total_time_elapsed + rendering_time_left;
		double total_time_left = total_time_estimate - total_time_elapsed;

		int hours = total_time_left / (60 * 60);
		int minutes = (int(total_time_left) % (60 * 60)) / 60;
		int seconds = int(total_time_left) % 60;

		std::cout << rendering_percent_finished << " \% of rendering finished." << std::endl;
		std::cout << "Estimated time left is "
			<< hours << "h:"
			<< minutes << "m:"
			<< seconds << "s." << std::endl;
	}

	// To show how much time it actually took to render.
	time(&time_now);
	double time_elapsed = difftime(time_now, time_start);
	int hours_elapsed = time_elapsed / (60 * 60);
	int minutes_elapsed = (int(time_elapsed) % (60 * 60)) / 60;
	int seconds_elapsed = int(time_elapsed) % 60;

	int hours_prerender = prerender_time / (60 * 60);
	int minutes_prerender = (int(prerender_time) % (60 * 60)) / 60;
	int seconds_prerender = int(prerender_time) % 60;

	std::string rendering_time_string =
	 	  std::to_string(hours_elapsed) + "h:"
		+ std::to_string(minutes_elapsed) + "m:"
		+ std::to_string(seconds_elapsed) + "s";

	std::string prerender_time_string =
		  std::to_string(hours_prerender) + "h:"
		+ std::to_string(minutes_prerender) + "m:"
		+ std::to_string(seconds_prerender) + "s";

	std::cout << "Rendering time : " << rendering_time_string << std::endl;

	// Convert to byte data
	// Gamma correction
	float gamma = 1 / 2.2;
	for (int x = 0; x < c.WIDTH; ++x)
	{
		for (int y = 0; y < c.HEIGHT; ++y)
		{
			int index = (x + y * c.WIDTH);
			pixel_values[index * 3 + 0] = char(int(glm::clamp(
				glm::pow(irradiance_values[index][0],gamma), 0.0f, 1.0f) * 255)); // Red
			pixel_values[index * 3 + 1] = char(int(glm::clamp(
				glm::pow(irradiance_values[index][1],gamma), 0.0f, 1.0f) * 255)); // Green
			pixel_values[index * 3 + 2] = char(int(glm::clamp(
				glm::pow(irradiance_values[index][2],gamma), 0.0f, 1.0f) * 255)); // Blue
		}
	}

	std::string date_time = currentDateTime();
	std::string file_name = date_time + ".ppm";
  
	// Save the image data to file
	savePPM(file_name.c_str(), WIDTH, HEIGHT, pixel_values);

	// Save information in a text file
	std::ofstream myfile;
	myfile.open (date_time + ".txt");
	myfile << "Rendered file information:\n\n";
	myfile << "File name                    : " + date_time + ".ppm\n";
	myfile << "Rendering time               : " + rendering_time_string +"\n";
	myfile << "Prerendering time            : " + prerender_time_string +"\n";
	myfile << "Resolution                   : " + std::to_string(WIDTH) + " x " + std::to_string(HEIGHT) + "\n";
	myfile << "Caustic sub sampling         : " + std::to_string(SUB_SAMPLING_CAUSTICS) + "\n";
	myfile << "Monte Carlo sub sampling     : " + std::to_string(SUB_SAMPLING_MONTE_CARLO) + "\n";
	myfile << "Direct specular sub sampling : " + std::to_string(SUB_SAMPLING_DIRECT_SPECULAR) + "\n";
	myfile << "Emitted photons              : " + std::to_string(NUMBER_OF_PHOTONS_EMISSION) + "\n";
	myfile << "Photons in scene             : " + std::to_string(s.getNumberOfPhotons()) + "\n";
	myfile << "Objects in scene             : " + std::to_string(s.getNumberOfObjects()) + "\n";
	myfile << "Spheres in scene             : " + std::to_string(s.getNumberOfSpheres()) + "\n";
	myfile << "Triangles in scene           : " + std::to_string(s.getNumberOfTriangles()) + "\n";
	myfile << "Gamma                        : " + std::to_string(gamma) + "\n";
	myfile.close();

	// Clean up
	delete [] irradiance_values;
	delete [] pixel_values;
  
	// Make a beep sound
	std::cout << '\a';
	return EXIT_SUCCESS;
}