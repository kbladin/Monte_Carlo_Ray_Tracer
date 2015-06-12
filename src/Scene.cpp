#include "../include/Scene.h"

#include <iostream>
#include <random>

// --- Object3D class functions --- //

Object3D::Object3D(Material* material) : 
	material_(material)
{}

Material Object3D::material() const
{
	if (material_)
	{
		return *material_;
	}
	else
	{
		return Material();
	}
}

// --- Sphere class functions --- //

Sphere::Sphere(glm::vec3 position, float radius, Material* material) : 
	Object3D(material), POSITION_(position), RADIUS_(radius)
{}

bool Sphere::intersect(IntersectionData* id, Ray r) const
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
		if (t < 0) // if we are inside the sphere
		{
			// the intersection is on the inside of the sphere
			t = -p_half + sqrt(to_square);
		}
		n = r.position + t*r.direction - POSITION_;
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

bool Plane::intersect(IntersectionData* id, Ray r) const
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

glm::vec3 Plane::getPointOnSurface(float u, float v) const
{
	glm::vec3 v1 = P1_ - P0_;
	glm::vec3 v2 = P2_ - P0_;
	return P0_ + u * v1 + v * v2;
}

// --- LightSource class functions --- //

LightSource::LightSource(
	glm::vec3 p0,
	glm::vec3 p1,
	glm::vec3 p2,
	float emittance,
	SpectralDistribution color) :
	emitter_(p0, p1, p2, NULL),
	emittance(emittance),
	color(color)
{}

bool LightSource::intersect(LightSourceIntersectionData* light_id, Ray r)
{
	IntersectionData id;
	if(emitter_.intersect(&id, r))
	{
		light_id->normal = id.normal;
		light_id->t = id.t;
		light_id->emittance = emittance;
		light_id->color = color;
		return true;
	}
	else
		return false;
}

glm::vec3 LightSource::getPointOnSurface(float u, float v)
{
	return emitter_.getPointOnSurface(u, v);
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

	mirror_->color_diffuse[0] = 0.2;
	mirror_->color_diffuse[1] = 1;
	mirror_->color_diffuse[2] = 0.2;
	mirror_->color_specular[0] = 1;
	mirror_->color_specular[1] = 1;
	mirror_->color_specular[2] = 1;
	mirror_->reflectance = 1;
	mirror_->specular_reflectance = 0.5;
	mirror_->polish_power = 1000;

	glass_->color_diffuse[0] = 0.5;
	glass_->color_diffuse[1] = 0.5;
	glass_->color_diffuse[2] = 0.5;
	glass_->color_specular[0] = 1;
	glass_->color_specular[1] = 1;
	glass_->color_specular[2] = 1;
	glass_->reflectance = 0;
	glass_->specular_reflectance = 0;
	glass_->transmissivity = 1;
	glass_->refraction_index = 2;
	glass_->clearness_power = 1000;
	glass_->polish_power = 1000;

	*air_ = Material::air();

	diffuse_red_->color_diffuse[0] = 1;
	diffuse_red_->color_diffuse[1] = 0.2;
	diffuse_red_->color_diffuse[2] = 0.2;
	diffuse_red_->reflectance = 1;

	diffuse_green_->color_diffuse[0] = 0.2;
	diffuse_green_->color_diffuse[1] = 1;
	diffuse_green_->color_diffuse[2] = 0.2;
	diffuse_green_->reflectance = 1;

	diffuse_blue_->color_diffuse[0] = 0.2;
	diffuse_blue_->color_diffuse[1] = 0.2;
	diffuse_blue_->color_diffuse[2] = 1;
	diffuse_blue_->reflectance = 1;

	diffuse_white_->color_diffuse[0] = 1;
	diffuse_white_->color_diffuse[1] = 1;
	diffuse_white_->color_diffuse[2] = 1;
	diffuse_white_->reflectance = 1;

	diffuse_gray_->color_diffuse[0] = 0.5;
	diffuse_gray_->color_diffuse[1] = 0.5;
	diffuse_gray_->color_diffuse[2] = 0.5;
	diffuse_gray_->reflectance = 1;
	

	SpectralDistribution lamp_color;
	lamp_color[0] = 1;
	lamp_color[1] = 1;
	lamp_color[2] = 1;
	float lamp_size = 0.6;
	lamps_.push_back(new LightSource(
		glm::vec3(-lamp_size / 2,1 - 0.01,-5 + (1 - lamp_size / 2)), // P0
		glm::vec3(lamp_size / 2,1 - 0.01,-5 + (1 - lamp_size / 2)), // P1
		glm::vec3(-lamp_size / 2,1 - 0.01,-5 + (1 + lamp_size / 2)), // P2
		3, // Emittance
		lamp_color));

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

	objects_.push_back(new Sphere(glm::vec3(0.5,-0.7,-4), 0.3, mirror_));
	objects_.push_back(new Sphere(glm::vec3(-0.5,-0.5,-4), 0.3, glass_));
	objects_.push_back(new Sphere(glm::vec3(0.2,0.2,-4.5), 0.2, diffuse_blue_));
}

Scene::~Scene()
{
	delete gen_;
	delete dis_;

	for (int i = 0; i < objects_.size(); ++i)
	{
		delete objects_[i];
	}
	for (int i = 0; i < lamps_.size(); ++i)
	{
		delete lamps_[i];
	}

	delete mirror_;
	delete glass_;
	delete diffuse_red_;
	delete diffuse_green_;
	delete diffuse_blue_;
	delete diffuse_white_;
	delete diffuse_gray_;
	delete air_;
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

bool Scene::intersectLamp(LightSourceIntersectionData* light_id, Ray r)
{
	LightSourceIntersectionData lamp_id_smallest_t;
	lamp_id_smallest_t.t = 100000; // Ugly solution

	LightSource* intersecting_lamp = NULL;
	for (int i = 0; i < lamps_.size(); ++i)
	{
		LightSourceIntersectionData id_local;
		if (lamps_[i]->intersect(&id_local,r) && id_local.t < lamp_id_smallest_t.t)
		{
			lamp_id_smallest_t = id_local;
			intersecting_lamp = lamps_[i];
		}
	}
	if (intersecting_lamp)
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
		if (intersecting_object && id_smallest_t.t < lamp_id_smallest_t.t)
		{
			return false;
		}
		else
		{
			*light_id = lamp_id_smallest_t;
			return true;	
		}
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
	float rand1 = pow( glm::asin((*dis_)(*gen_)) / (M_PI / 2),
		power);
	float rand2 = (*dis_)(*gen_);

	// inclination has cosine powered distribution and azimuth has uniform
	// distribution
	float inclination = glm::acos(1 - 2 * rand1);
	float azimuth = 2 * M_PI * rand2;

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

SpectralDistribution Scene::traceRay(Ray r, int iteration)
{
	IntersectionData id;
	LightSourceIntersectionData lamp_id;
	if (intersectLamp(&lamp_id, r))
	{
		return lamp_id.color * lamp_id.emittance;
	}
	else if (intersect(&id, r))
	{
		bool end_here = iteration >= 1;
		Ray recursive_ray = r;
		// To make sure it does not intersect with itself again
		glm::vec3 offset = id.normal * 0.000001f;
		bool inside = false;
		if (glm::dot(id.normal, r.direction) > 0)
		// The ray is inside an object
		{
			inside = true;
		}
		// Russian roulette
		//float roll_die = 0.5;
		float roll_reflect = id.material.reflectance * id.material.reflectance / (id.material.reflectance + id.material.transmissivity);
		float roll_refract = id.material.transmissivity / (id.material.reflectance + id.material.transmissivity);
		float roll = (*dis_)(*gen_); // [0 , 1]
		if (roll_reflect && roll < roll_reflect)
		{ // The ray is reflected
			recursive_ray.position = r.position + id.t * r.direction +
				(inside ? -offset : offset);

			// Russian roulette
			float roll_specular = id.material.specular_reflectance;
			float roll = (*dis_)(*gen_); // [0 , 1]
			if (roll_specular && roll < roll_specular)
			{ // The ray is reflected specularly
				if (iteration >= 5)
					return SpectralDistribution();
				glm::vec3 perfect_reflection = glm::reflect(r.direction, id.normal);
				// Add some randomization to the direction vector
				recursive_ray.direction = shake(perfect_reflection, id.material.polish_power);
				// Recursively trace the reflected ray
				return traceRay(recursive_ray, iteration + 1) * id.material.color_specular;
			}
			else
			{ // The ray is reflected diffusely
				SpectralDistribution L_local;
				// Cast shadow rays
				static const int n_samples = 10;
				for (int i = 0; i < lamps_.size(); ++i)
				{
					for (int j = 0; j < n_samples; ++j)
					{
						Ray shadow_ray = recursive_ray;
						glm::vec3 differance = lamps_[i]->getPointOnSurface((*dis_)(*gen_),(*dis_)(*gen_)) - shadow_ray.position;
						shadow_ray.direction = glm::normalize(differance);

						float brdf = 1 / (2 * M_PI); // Dependent on inclination and azimuth
						float cos_angle = glm::dot(shadow_ray.direction, id.normal);

						LightSourceIntersectionData shadow_id;

						if(intersectLamp(&shadow_id, shadow_ray) && glm::dot(shadow_id.normal, shadow_ray.direction) < 0)
						{
							L_local += shadow_id.color * shadow_id.emittance * brdf * cos_angle * id.material.color_diffuse * 1 / glm::pow(glm::length(differance), 2);
						}
					}
				}
				L_local /= n_samples;
				if (end_here)
				{
					return L_local;
				}
				else
				{
					SpectralDistribution L_indirect;
					static const int n_samples = 1;
					for (int i = 0; i < n_samples; ++i)
					{
						// helper is just a random vector and can not possibly be
						// a zero vector since id.normal is normalized
						glm::vec3 helper = id.normal + glm::vec3(1,1,1);
						glm::vec3 tangent = glm::normalize(glm::cross(id.normal, helper));

						float rand1 = (*dis_)(*gen_);
						float rand2 = (*dis_)(*gen_);

						// Uniform distribution
						float inclination = glm::acos(1 - rand1);//glm::acos(1 -  2 * (*dis_)(*gen_));
						float azimuth = 2 * M_PI * rand2;
						// Change the actual vector
						glm::vec3 random_direction = id.normal;
						random_direction = glm::normalize(glm::rotate(
							random_direction,
							inclination,
							tangent));
						random_direction = glm::normalize(glm::rotate(
							random_direction,
							azimuth,
							id.normal));

						float brdf = 1 / (2 * M_PI); // Dependent on inclination and azimuth
						float estimator = 1 / (2 * M_PI);

						float cos_angle = glm::dot(random_direction, id.normal);

						recursive_ray.direction = random_direction;
						L_indirect += traceRay(recursive_ray, iteration + 1) * brdf * cos_angle / estimator * id.material.color_diffuse;
					}
					return L_indirect / n_samples + L_local;
				}
			}
		}
		else if (roll_refract && roll < roll_reflect + roll_refract)
		{ // The ray is refracted (and reflected according to Fresnels equations)
			if (iteration >= 5)
				return SpectralDistribution();
			glm::vec3 normal = inside ? -id.normal : id.normal;
			glm::vec3 perfect_refraction = glm::refract(
				r.direction,
				normal,
				r.material.refraction_index / id.material.refraction_index);
			glm::vec3 perfect_reflection = glm::reflect(r.direction, id.normal);
			if (perfect_refraction != glm::vec3(0))
			{ // Refraction and reflection
				// Schlicks approximation to Fresnels equations.
				float n1 = r.material.refraction_index;
				float n2 = id.material.refraction_index;
				float R_0 = pow((n1 - n2)/(n1 + n2), 2);
				float R = R_0 + (1 - R_0) * pow(1 - glm::dot(normal, -r.direction),5);

				Ray recursive_ray_reflected = recursive_ray;
				Ray recursive_ray_refracted = recursive_ray;

				if (inside)
				{
					// Reflected ray
					// Change the material the ray is travelling in
					recursive_ray_reflected.material = id.material;
					recursive_ray_reflected.position = r.position + id.t * r.direction -offset;
					// Refracted ray
					// Change the material the ray is travelling in
					recursive_ray_refracted.material = *air_;
					recursive_ray_refracted.position = r.position + id.t * r.direction +offset;
				}
				else
				{
					// Reflected ray
					// Change the material the ray is travelling in
					recursive_ray_reflected.material = *air_;
					recursive_ray_reflected.position = r.position + id.t * r.direction +offset;
					// Refracted ray
					// Change the material the ray is travelling in
					recursive_ray_refracted.material = id.material;
					recursive_ray_refracted.position = r.position + id.t * r.direction -offset;
				}
				SpectralDistribution to_return;
				// Add some randomization to the direction vectors
				recursive_ray_reflected.direction = shake(perfect_reflection, id.material.polish_power);
				recursive_ray_refracted.direction = shake(perfect_refraction, id.material.clearness_power);

				// Recursively trace the refracted rays
				SpectralDistribution reflected_part = traceRay(recursive_ray_reflected, iteration + 1) * id.material.color_specular * R;
				SpectralDistribution refracted_part= traceRay(recursive_ray_refracted, iteration + 1) * (1 - R);
				return reflected_part + refracted_part;
			}
			else
			{ // Brewster angle reached, complete specular reflection
				if (inside)
					recursive_ray.position = r.position + id.t * r.direction - offset;
				else
					recursive_ray.position = r.position + id.t * r.direction + offset;
				// Add some randomization to the direction vector
				recursive_ray.direction = shake(perfect_reflection, id.material.polish_power);
				// Recursively trace the reflected ray
				return traceRay(recursive_ray, iteration + 1) * id.material.color_specular;
			}
		}
	}
	return SpectralDistribution();
}