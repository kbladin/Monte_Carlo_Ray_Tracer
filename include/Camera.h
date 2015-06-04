#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>

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
	~Camera();

	const int width() {return WIDTH_;}
	const int height() {return HEIGHT_;}

	void doSomething();
};

#endif // CAMERA_H