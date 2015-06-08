#include "../include/Scene.h"

#include <iostream>

// --- Object3D class functions --- //

Object3D::Object3D(Material* material) : 
	material_(material)
{}

// --- Sphere class functions --- //

Sphere::Sphere(glm::vec3 position, float radius, Material* material) : 
	Object3D(material), POSITION_(position), RADIUS_(radius)
{}

bool Sphere::intersect(IntersectionData* id, Ray r)
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
	glm::vec3 n; // normal of the intersection point on the surface
	if (to_square < 0)
	// No intersection points
		return false;
	else // if (to_square > 0) or (to_square == 0)
	// One or two intersection points, if two intersection points,
	// we choose the closest one that gives a positive t
	{
		t = -p_half - sqrt(to_square); // First the one on the front face
		n = r.position + t*r.direction - POSITION_;
		if (t < 0) // if we are inside the sphere
		{
			// the intersection is on the inside of the sphere
			t = -p_half + sqrt(to_square);
			n = POSITION_ - (r.position + t*r.direction);
		}
	}
	if (t >= 0) // t needs to be positive to travel forward on the ray
	{
		id->t = t;
		id->normal = n;
		id->material = material();
		return true;
	}
	return false;
}

// --- Plane class functions --- //

Plane::Plane(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, Material* material) : 
	Object3D(material), P0_(p0), P1_(p1), P2_(p2)
{}

bool Plane::intersect(IntersectionData* id, Ray r)
{
	glm::mat3 M;
	M[0] = -r.direction;
	M[1] = P1_ - P0_;
	M[2] = P2_ - P0_;

	glm::vec3 tuv = glm::inverse(M) * (r.position - P0_);

	// To avoid confusion
	// t is the parameter on the ray, u and v are parameters on the plane
	float t = tuv.x;
	float u = tuv.y;
	float v = tuv.z;

	if (u >= 0 && u <= 1 && v >= 0 && v <= 1 && // Within the boundary
		t >= 0) // t needs to be positive to travel forward on the ray
	{
		glm::vec3 n = glm::cross(u * (P1_ - P0_), v * (P2_ - P0_));
		if (glm::dot(r.direction, n) > 0)
			n = -n; // intersection on the back side of the plane
		
		id->t = t;
		id->normal = n;
		id->material = material();
		return true;
	}
	else
		return false;
}

// --- Scene class functions --- //

Scene::Scene ()
{
	sphere_material_ = new Material();
	plane_material_ = new Material();

	sphere_material_->color_diffuse.data[0] = 0;
	sphere_material_->color_diffuse.data[1] = 0.5;
	sphere_material_->color_diffuse.data[2] = 0;

	plane_material_->color_diffuse.data[0] = 0.5;
	plane_material_->color_diffuse.data[1] = 0;
	plane_material_->color_diffuse.data[2] = 0;

	objects_.push_back(new Sphere(glm::vec3(0.7,0.7,-5), 0.5, sphere_material_));
	objects_.push_back(new Plane(
		glm::vec3(-0.7,-0.7,-4.7), // P0
		glm::vec3(0.7,-0.7,-4.7), // P1
		glm::vec3(-0.7,0.7,-4.7), // P2
		plane_material_));
}

Scene::~Scene()
{
	for (int i = 0; i < objects_.size(); ++i)
	{
		delete objects_[i];
	}
	delete sphere_material_;
	delete plane_material_;
}

bool Scene::intersect(IntersectionData* id, Ray r)
{
	IntersectionData id_smallest_t;
	id_smallest_t.t = 100000; // Ugly solution

	Object3D* intersecting_object = NULL;
	for (int i = 0; i < objects_.size(); ++i)
	{
		IntersectionData id_local;
		if (objects_[i]->intersect(&id_local,r) && id_local.t < id_smallest_t.t)
		{
			id_smallest_t = id_local;
			intersecting_object = objects_[i];
		}
	}
	if (intersecting_object)
	{
		*id = id_smallest_t;
		return true;
	}
	return false;
}

SpectralDistribution Scene::traceRay(Ray r)
{
	SpectralDistribution sd;
	IntersectionData id;
	if (intersect(&id, r))
	{
		sd.data[0] = id.material.color_diffuse.data[0];
		sd.data[1] = id.material.color_diffuse.data[1];
		sd.data[2] = id.material.color_diffuse.data[2];
	}
	else
	{
		sd.data[0] = 0;
		sd.data[1] = 0;
		sd.data[2] = 0;
	}
	return sd;
}