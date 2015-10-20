#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "utils.h"

class Camera
{
public:
	glm::vec3 eye;
	glm::vec3 center;
	glm::vec3 up;
	float fov;
	glm::mat4 VP_inv;

	const int WIDTH;
	const int HEIGHT;

	Camera(
		const glm::vec3 eye,
		const glm::vec3 center,
		const glm::vec3 up,
		const float fov,
		const int width,
		const int height);
	~Camera(){};

	Ray castRay(
		const int pixel_x, // [0, WIDTH_ - 1]
		const int pixel_y, // [0, HEIGHT_ - 1]
		const float parameter_x, // [-0.5, 0.5]
		const float parameter_y); // [-0.5, 0.5]

	int getWidth();
	int getHeight();
};

#endif // CAMERA_H