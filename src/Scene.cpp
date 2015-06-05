#include "../include/Scene.h"

#include <iostream>

// --- Sphere class functions --- //

Sphere::Sphere(glm::vec3 position, float radius) : 
	POSITION_(position), RADIUS_(radius)
{}

bool Sphere::intersect(glm::vec3* intersection_position, Ray r)
{
	// if to_square is negative we have imaginary solutions,
	// hence no intersection
	// p_half comes from the p-q formula (p/2)
	float p_half = glm::dot((r.position - POSITION_), r.direction);
	float to_square = 
		pow(p_half, 2) + 
		pow(RADIUS_, 2) - 
		pow(glm::length(r.position - POSITION_), 2);
	float t; // parameter that tells us where on the ray the intersection is
	if (to_square < 0)
	// No intersection points
	{
		return false;
	}
	else // if (to_square > 0) or (to_square == 0)
	// One or two intersection points, if two intersection points,
	// we choose the closest one
	{
		t = -p_half + sqrt(to_square);
		// This is the ignored intersection point (back of the sphere):
		//float t2 = -p_half - sqrt(to_square);
	}
	*intersection_position = r.position + t * r.direction;
	return true;
}

// --- Scene class functions --- //

Scene::Scene ()
{
	sphere_ = new Sphere(glm::vec3(1,1,-7), 0.2);
}

Scene::~Scene()
{
	delete sphere_;
}

SpectralDistribution Scene::traceRay(Ray r)
{
	SpectralDistribution sd;
	glm::vec3 intersection_position;
	if (sphere_->intersect(&intersection_position, r))
	{
		sd.data[0] = 0;
		sd.data[1] = 0.5;
		sd.data[2] = 0;
	}
	else
	{
		sd.data[0] = r.direction.x;
		sd.data[1] = r.direction.y;
		sd.data[2] = r.direction.z;
	}
	return sd;
}