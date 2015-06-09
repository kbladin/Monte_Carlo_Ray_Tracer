#include "../include/Scene.h"

#include <iostream>
#include <random>

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
		}
	}
	if (t >= 0) // t needs to be positive to travel forward on the ray
	{
		id->t = t;
		id->normal = glm::normalize(n);
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
		id->t = t;
		id->normal = glm::normalize(n);
		id->material = material();
		return true;
	}
	else
		return false;
}

// --- Scene class functions --- //

Scene::Scene ()
{
	gen_ = new std::mt19937(rd_());
	dis_ = new std::uniform_real_distribution<float>(0, 1);
    
	mirror_ = new Material();
	glass_ = new Material();
	diffuse_red_ = new Material();
	diffuse_green_ = new Material();
	diffuse_blue_ = new Material();
	diffuse_white_ = new Material();
	diffuse_gray_ = new Material();
	air_ = new Material();
	lamp_ = new Material();

	mirror_->color_diffuse[0] = 0;
	mirror_->color_diffuse[1] = 0;
	mirror_->color_diffuse[2] = 0;
	mirror_->color_specular[0] = 1;
	mirror_->color_specular[1] = 1;
	mirror_->color_specular[2] = 1;
	mirror_->specular_reflectance = 1;
	mirror_->polish_power = 20;

	glass_->color_diffuse[0] = 1;
	glass_->color_diffuse[1] = 1;
	glass_->color_diffuse[2] = 1;
	glass_->specular_reflectance = 0;
	glass_->transmissivity = 1;
	glass_->refraction_index = 1.3;
	glass_->clearness_power = 20;

	*air_ = Material::air();

	lamp_->color_diffuse[0] = 1;
	lamp_->color_diffuse[1] = 1;
	lamp_->color_diffuse[2] = 1;
	lamp_->emittance = 10;

	diffuse_red_->color_diffuse[0] = 0.5;
	diffuse_red_->color_diffuse[1] = 0;
	diffuse_red_->color_diffuse[2] = 0;

	diffuse_green_->color_diffuse[0] = 0;
	diffuse_green_->color_diffuse[1] = 0.5;
	diffuse_green_->color_diffuse[2] = 0;

	diffuse_blue_->color_diffuse[0] = 0;
	diffuse_blue_->color_diffuse[1] = 0;
	diffuse_blue_->color_diffuse[2] = 0.5;

	diffuse_white_->color_diffuse[0] = 1;
	diffuse_white_->color_diffuse[1] = 1;
	diffuse_white_->color_diffuse[2] = 1;

	diffuse_gray_->color_diffuse[0] = 0.5;
	diffuse_gray_->color_diffuse[1] = 0.5;
	diffuse_gray_->color_diffuse[2] = 0.5;
	
	// Back
	objects_.push_back(new Plane(
		glm::vec3(-1,-1,-5), // P0
		glm::vec3(1,-1,-5), // P1
		glm::vec3(-1,1,-5), // P2
		diffuse_white_));
	// Left
	objects_.push_back(new Plane(
		glm::vec3(-1,-1,-5), // P0
		glm::vec3(-1,1,-5), // P1
		glm::vec3(-1,-1,-0), // P2
		diffuse_red_));
	// Right
	objects_.push_back(new Plane(
		glm::vec3(1,-1,-5), // P0
		glm::vec3(1,-1,-0), // P1
		glm::vec3(1,1,-5), // P2
		diffuse_green_));
	// Roof
	objects_.push_back(new Plane(
		glm::vec3(-1,1,-5), // P0
		glm::vec3(1,1,-5), // P1
		glm::vec3(-1,1,-0), // P2
		diffuse_white_));
	// Floor
	objects_.push_back(new Plane(
		glm::vec3(-1,-1,-5), // P0
		glm::vec3(-1,-1,-0), // P1
		glm::vec3(1,-1,-5), // P2
		diffuse_white_));

	objects_.push_back(new Sphere(glm::vec3(0.5,-0.5,-4), 0.3, mirror_));
	objects_.push_back(new Sphere(glm::vec3(-0.3,-0.5,-3), 0.2, glass_));
	objects_.push_back(new Sphere(glm::vec3(0,1,-4), 0.2, lamp_));
}

Scene::~Scene()
{
	delete gen_;
	delete dis_;

	for (int i = 0; i < objects_.size(); ++i)
	{
		delete objects_[i];
	}
	delete mirror_;
	delete glass_;
	delete diffuse_red_;
	delete diffuse_green_;
	delete diffuse_blue_;
	delete diffuse_white_;
	delete diffuse_gray_;
	delete air_;
	delete lamp_;
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

glm::vec3 Scene::shake(glm::vec3 r, float power)
{
	// shaked will be a vector close to r but randomized to be different
	// depending on power. power small => more randomization (cosine distribution)
	glm::vec3 shaked = r;
	// helper is just a random vector and can not possibly be
	// a zero vector since r is normalized 
	glm::vec3 helper = r + glm::vec3(1,1,1);
	glm::vec3 tangent = glm::normalize(glm::cross(r, helper));

	// Add randomized inclination and azimuth
	// Inclination is random with cosine distribution
	float inclination = pow( glm::asin(2 * (*dis_)(*gen_) - 1) / (M_PI / 2),
		power) * (M_PI / 2);
	// Azimuth is random with uniform distribution
	float azimuth = 2 * M_PI * (*dis_)(*gen_);

	// Change the actual vector
	shaked = glm::rotate(
		shaked,
		inclination,
		tangent);
	shaked = glm::rotate(
		shaked,
		azimuth,
		r);

	// Normalize to avoid accumulating errors
	return glm::normalize(shaked);
}

SpectralDistribution Scene::traceRay(Ray r)
{
	SpectralDistribution sd;
	IntersectionData id;
	if (intersect(&id, r))
	{
		if (id.material.emittance)
		{
			sd = id.material.color_diffuse * id.material.emittance;
			return sd;
		}

		bool inside = false;
		if (glm::dot(id.normal, r.direction) > 0)
		// The ray is inside an object
		{
			inside = true;
		}

		// To make sure it does not intersect with itself again
		glm::vec3 offset = id.normal * 0.001f;

		if (id.material.transmissivity &&
			(*dis_)(*gen_) < id.material.transmissivity)
		// The ray is transmitted through the material
		{
			glm::vec3 perfect_refraction = glm::refract(
				r.direction,
				id.normal,
				r.material.refraction_index / id.material.refraction_index);
			if (perfect_refraction == glm::vec3(0))
			// Specular reflection
			{
				r.position = r.position + id.t * r.direction +
					(inside ? -offset : offset);

				glm::vec3 perfect_reflection = glm::reflect(r.direction, id.normal);
				// Add some randomization to the direction vector
				r.direction = shake(perfect_reflection, id.material.polish_power);
				// Recursively trace the reflected ray
				return traceRay(r) * id.material.color_specular;
			}
			else
			// Refraction
			{
				r.position = r.position + id.t * r.direction +
					(inside ? offset : -offset);
				// Add some randomization to the direction vector
				r.direction = shake(perfect_refraction, id.material.clearness_power);
				// Change the material the ray is travelling in
				if (inside)
					r.material = *air_;
				else
					r.material = id.material;
				// Recursively trace the refracted ray
				return traceRay(r) * id.material.color_diffuse;
			}
		}
		else
		// The ray is reflected out of the material
		{
			r.position = r.position + id.t * r.direction +
				(inside ? -offset : offset);

			if (id.material.specular_reflectance &&
			(*dis_)(*gen_) < id.material.specular_reflectance)
			// The ray will be reflected specularly
			{
				glm::vec3 perfect_reflection = glm::reflect(r.direction, id.normal);
				// Add some randomization to the direction vector
				r.direction = shake(perfect_reflection, id.material.polish_power);
				// Recursively trace the reflected ray
				return traceRay(r) * id.material.color_specular;
			}
			else
			// The ray will be reflected diffusely
			{
				if (inside)
				{
					sd[0] = 0;
					sd[1] = 0;
					sd[2] = 0;
				} else
				{
					r.direction = shake(id.normal, 1);
					return traceRay(r) * id.material.color_diffuse;
				}
			}
		}
	}
	else
	{
		sd[0] = 0;
		sd[1] = 0;
		sd[2] = 0;
	}
	return sd;
}