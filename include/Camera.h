#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "utils.h"

class Camera
{
private:
		const glm::vec3 EYE_;
		const glm::vec3 CENTER_;
		const glm::vec3 UP_;
		const float FOV_;
		const int WIDTH_;
		const int HEIGHT_;
public:
	Camera(
		glm::vec3 eye,
		glm::vec3 center,
		glm::vec3 up,
		float fov,
		int width,
		int height);
	~Camera(){};

	const int width() {return WIDTH_;}
	const int height() {return HEIGHT_;}

	Ray castRay(int pixel_x, int pixel_y, float parameter_x, float parameter_y);
};

#endif // CAMERA_H