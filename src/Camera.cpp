#include "../include/Camera.h"

#include <iostream>

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
{

}

Camera::~Camera (){

}

void Camera::doSomething(){
	std::cout << "Something" << std::endl;
}