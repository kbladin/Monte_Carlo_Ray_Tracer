#include "../include/Camera.h"

#include <iostream>

// Function for debugging
std::ostream& operator<<(std::ostream& out, const glm::vec3 v){
   return out << "[" << v.x << " , " << v.y << " , " << v.z  << "]";
}

Camera::Camera (
	glm::vec3 eye,
	glm::vec3 center,
	glm::vec3 up,
	float fov,
	int width,
	int height) :

	EYE_(eye),
	CENTER_(center),
	UP_(up),
	FOV_(fov),
	WIDTH_(width),
	HEIGHT_(height)
{}

Ray Camera::castRay(
	int pixel_x,
	int pixel_y,
	float parameter_x,
	float parameter_y)
{
	Ray r;
	if (pixel_x < 0 || pixel_x > WIDTH_ - 1 ||
		pixel_y < 0 || pixel_y > HEIGHT_ - 1 ||
		parameter_x < -0.5 || parameter_x > 0.5 ||
		parameter_y < -0.5 || parameter_y > 0.5
		)
	{
		std::cout << "ERROR : Invalid arguments in castRay()" << std::endl;
		r.position = glm::vec3(0);
		r.direction = glm::vec3(0);
	}
	else
	{
		// View and perspective matrices are used in the unProject() function
		glm::mat4 V = glm::lookAt(EYE_, CENTER_, UP_);
		float aspect = float(WIDTH_) / HEIGHT_;
		glm::mat4 P = glm::perspective(FOV_, aspect, 0.1f, 100.0f);

		// The unProject() function returns a vector in world-space which
		// defines a direction out of the frustum depending on which pixel
		// we shoot the ray from. "from" will be on the near-viewplane
		// and "to" will be on the far-viewplane.
		glm::vec3 from = glm::unProject(
			glm::vec3(pixel_x + parameter_x, pixel_y + parameter_y, 0.0f),
			V,
			P,
			glm::vec4(0, 0, WIDTH_, HEIGHT_));
		glm::vec3 to = glm::unProject(
			glm::vec3(pixel_x + parameter_x, pixel_y + parameter_y, 1.0f),
			V,
			P,
			glm::vec4(0, 0, WIDTH_, HEIGHT_));

		glm::vec3 direction = glm::normalize(to - from);

		r.position = from;
		r.direction = direction;
		r.material = Material::air();
	}
	return r;
}