#include "../include/Scene.h"

#include <iostream>

// --- Object3D class functions --- //

Object3D::Object3D(Material material) : 
	material_(material)
{

}

// --- Sphere class functions --- //

Sphere::Sphere(glm::vec3 position, float radius, Material material) : 
	Object3D(material), POSITION_(position), RADIUS_(radius)
{}

bool Sphere::intersect(float* t, Ray r)
{
	// if to_square is negative we have imaginary solutions,
	// hence no intersection
	// p_half comes from the p-q formula (p/2)
	float p_half = glm::dot((r.position - POSITION_), r.direction);
	float to_square = 
		pow(p_half, 2) + 
		pow(RADIUS_, 2) - 
		pow(glm::length(r.position - POSITION_), 2);
	float t_local; // parameter that tells us where on the ray the intersection is
	if (to_square < 0)
	// No intersection points
		return false;
	else // if (to_square > 0) or (to_square == 0)
	// One or two intersection points, if two intersection points,
	// we choose the closest one
	{
		t_local = -p_half - sqrt(to_square);
		// This is the ignored intersection point (back of the sphere):
		//float t_local2 = -p_half + sqrt(to_square);
	}
	if (t_local >= 0) // t needs to be positive to travel forward on the ray
	{
		*t = t_local;
		return true;
	}
	return false;
}

// --- Plane class functions --- //

Plane::Plane(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, Material material) : 
	Object3D(material), P0_(p0), P1_(p1), P2_(p2)
{}

bool Plane::intersect(float* t, Ray r)
{
	glm::mat3 M;
	M[0] = -r.direction;
	M[1] = P1_ - P0_;
	M[2] = P2_ - P0_;

	glm::vec3 tuv = glm::inverse(M) * (r.position - P0_);

	// To avoid confusion
	// t is the parameter on the ray, u and v are parameters on the plane
	float t_local = tuv.x;
	float u = tuv.y;
	float v = tuv.z;

	if (u >= 0 && u <= 1 && v >= 0 && v <= 1 && // Within the boundary
		t_local >= 0) // t needs to be positive to travel forward on the ray
	{
		*t = t_local;
		return true;
	}
	else
		return false;
}

// --- Scene class functions --- //

Scene::Scene ()
{
	Material sphere_material;
	Material plane_material;
	sphere_material.color = glm::vec3(0,0.5,0);
	plane_material.color = glm::vec3(0.5,0,0);
	objects_.push_back(new Sphere(glm::vec3(0.7,0.7,-5), 0.5, sphere_material));
	objects_.push_back(new Plane(
		glm::vec3(-0.7,-0.7,-4.7), // P0
		glm::vec3(0.7,-0.7,-4.7), // P1
		glm::vec3(-0.7,0.7,-4.7), // P2
		plane_material));
}

Scene::~Scene()
{
	for (int i = 0; i < objects_.size(); ++i)
	{
		delete objects_[i];
	}
}

Object3D* Scene::intersect(Ray r)
{
	float smallest_t = 100000;
	Object3D* to_return = NULL;
	for (int i = 0; i < objects_.size(); ++i)
	{
		float t;
		if (objects_[i]->intersect(&t,r) && t < smallest_t)
		{
			smallest_t = t;
			to_return = objects_[i];
		}
	}
	return to_return;
}

SpectralDistribution Scene::traceRay(Ray r)
{
	SpectralDistribution sd;
	Object3D* intersected_object = intersect(r);
	if (intersected_object)
	{
		sd.data[0] = intersected_object->material_.color.r;
		sd.data[1] = intersected_object->material_.color.g;
		sd.data[2] = intersected_object->material_.color.b;
	}
	else
	{
		sd.data[0] = 0;
		sd.data[1] = 0;
		sd.data[2] = 0;
	}
	return sd;
}