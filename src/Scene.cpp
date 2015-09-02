#include "../include/Scene.h"

#include "../external_libraries/common_include/pugixml.h"
#include "../include/xmlTraverser.h"

#include <iostream>
#include <string>
#include <random>

// --- Scene class functions --- //

Scene::Scene (const char* file_path)
{
	if (!file_path)
	{
		std::cout << "No scene file specified. Please use a scene xml file as argument" << std::endl;
		exit (EXIT_FAILURE);
	}

	gen_ = new std::mt19937(rd_());
	dis_ = new std::uniform_real_distribution<float>(0, 1);

    pugi::xml_document doc;
    std::cout << "Loading XML file." << std::endl;
    pugi::xml_parse_result result = doc.load_file(file_path);
    std::cout << "Result: " << result.description() << std::endl;

    if (!result)
		exit (EXIT_FAILURE);
	
	scene_traverser walker;
	walker.scene = this;

	std::cout << "Creating scene from XML file." << std::endl;
	doc.traverse(walker);
    std::cout << "Scene created!" << std::endl;
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
	for(std::map<std::string, Material* >::iterator it = materials_.begin();
		it != materials_.end();
		it++) {
		delete it->second;
	}
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

/*
SpectralDistribution Scene::traceBidirectionalDiffuseRay(
	Ray r,
	IntersectionData id,
	int iteration)
{
	// Shoot Light ray



	return SpectralDistribution();
}

std::vector<std::pair< Ray, IntersectionData > > Scene::forwardTraceLightRay(
	Ray r,
	int iteration)
{
	std::vector<std::pair< Ray, IntersectionData > > data;
	IntersectionData id;
	if (intersect(&id, r))
	{
		data.push_back(std::pair< Ray, IntersectionData >(r, id));

		IntersectionData id;
		LightSourceIntersectionData lamp_id;
		if (intersectLamp(&lamp_id, r)) // Ray hitted light source
			return data; //lamp_id.color * lamp_id.emittance;
		else if (intersect(&id, r))
		{ // Ray hitted another object
			// To make sure it does not intersect with itself again
			glm::vec3 offset = id.normal * 0.0001f;
			bool inside = false;
			if (glm::dot(id.normal, r.direction) > 0) // The ray is inside an object
				inside = true;
			
			float transmissivity = id.material.transmissivity;
			float reflectance = id.material.reflectance;
			float specularity = id.material.specular_reflectance;

			SpectralDistribution total;
			if (1 - transmissivity)
			{ // Completely or partly reflected
				Ray recursive_ray = r;
				// New position same in both cases, can be computed once outside
				// of trace functions
				recursive_ray.position = r.position + id.t * r.direction +
					(inside ? -offset : offset);
				SpectralDistribution specular_part =
					specularity ?
						traceSpecularRay(
							recursive_ray,
							id,
							iteration) * specularity :
						SpectralDistribution();
				SpectralDistribution diffuse_part =
					(1 - specularity) ?
						traceDiffuseRay(
							recursive_ray,
							id,
							iteration) * (1 - specularity) :
						SpectralDistribution();
				total +=
					(specular_part + diffuse_part) *
					(1 - transmissivity) *
					reflectance;
			}
			if (transmissivity)
			{ // Completely or partly transmissive
				SpectralDistribution transmitted_part =
					transmissivity ?
						traceRefractedRay(r, id, iteration, offset, inside) :
						SpectralDistribution();
				total += transmitted_part * transmissivity;
			}
			return total;
		}
		return SpectralDistribution();


	}
}

std::vector<std::pair< Ray, IntersectionData > > Scene::forwardTraceDiffuseRay(
	Ray r,
	IntersectionData id,
	int iteration)
{
	// Start by adding the local illumination part (shadow rays)
	SpectralDistribution total_diffuse = traceLocalDiffuseRay(r, id);
	if (!(iteration >= 2)) // Do not end here
		// Add the indirect illumination part (Monte Carlo sampling)
		total_diffuse += traceIndirectDiffuseRay(r, id, iteration);
	return total_diffuse;
}

std::vector<std::pair< Ray, IntersectionData > > Scene::forwardTraceIndirectDiffuseRay(
	Ray r,
	IntersectionData id,
	int iteration)
{
	SpectralDistribution L_indirect;
	static const int n_samples = 1;
	for (int i = 0; i < n_samples; ++i)
	{
		// helper is just a random vector and can not possibly be
		// a zero vector since id.normal is normalized
		glm::vec3 helper = id.normal + glm::vec3(1,1,1);
		glm::vec3 tangent = glm::normalize(glm::cross(id.normal, helper));

		// rand1 is a random number from the cosine estimator
		float rand1 = glm::asin((*dis_)(*gen_));// (*dis_)(*gen_);
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

		float cos_angle = glm::dot(random_direction, id.normal);
		float estimator = cos_angle / M_PI;// 1 / (2 * M_PI);

		r.direction = random_direction;
		L_indirect += traceRay(r, iteration + 1) * brdf * cos_angle / estimator * id.material.color_diffuse;
	}
	return L_indirect / n_samples;
}

std::vector<std::pair< Ray, IntersectionData > > Scene::forwardTraceRefractedRay(
	Ray r,
	IntersectionData id,
	int iteration,
	glm::vec3 offset,
	bool inside)
{
	if (iteration >= 5)
		return SpectralDistribution();
	
	Ray recursive_ray = r;

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
			offset = -offset;
		
		// Reflected ray
		// Change the material the ray is travelling in
		recursive_ray_reflected.material = *air_;
		recursive_ray_reflected.position = r.position + id.t * r.direction +offset;
		// Refracted ray
		// Change the material the ray is travelling in
		recursive_ray_refracted.material = id.material;
		recursive_ray_refracted.position = r.position + id.t * r.direction -offset;
		
		SpectralDistribution to_return;
		// Add some randomization to the direction vectors
		recursive_ray_reflected.direction = shake(perfect_reflection, id.material.polish_power);
		recursive_ray_refracted.direction = shake(perfect_refraction, id.material.clearness_power);

		// Recursively trace the refracted rays
		SpectralDistribution reflected_part = traceRay(recursive_ray_reflected, iteration + 1) * id.material.color_specular * R;
		SpectralDistribution refracted_part= traceRay(recursive_ray_refracted, iteration + 1) * (1 - R) * id.material.color_diffuse;
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
*/

/*
Photon Scene::tracePhoton(Ray r, int iteration)
{
	IntersectionData id;
	LightSourceIntersectionData lamp_id;
	if (intersectLamp(&lamp_id, r)){ // Ray hitted light source
		Photon to_return;
		to_return.position = r.position + r.direction * lamp_id.t;
		r.t * lamp_id.radiosity;
	}
	if (intersect(&id, r))
	{ // Ray hit another object
		// To make sure it does not intersect with itself again
		glm::vec3 offset = id.normal * 0.0001f;
		bool inside = false;
		if (glm::dot(id.normal, r.direction) > 0) // The ray is inside an object
			inside = true;
		
		float transmissivity = id.material.transmissivity;
		float reflectance = id.material.reflectance;
		float specularity = id.material.specular_reflectance;

		SpectralDistribution total;
		if (1 - transmissivity)
		{ // Completely or partly reflected
			Ray recursive_ray = r;
			// New position same in both cases, can be computed once outside
			// of trace functions
			recursive_ray.position = r.position + id.t * r.direction +
				(inside ? -offset : offset);
			SpectralDistribution specular_part =
				specularity ?
					traceSpecularRay(
						recursive_ray,
						id,
						iteration) * specularity :
					SpectralDistribution();
			SpectralDistribution diffuse_part =
				(1 - specularity) ?
					traceDiffuseRay(
						recursive_ray,
						id,
						iteration) * (1 - specularity) :
					SpectralDistribution();
			total +=
				(specular_part + diffuse_part) *
				(1 - transmissivity) *
				reflectance;
		}
		if (transmissivity)
		{ // Completely or partly transmissive
			SpectralDistribution transmitted_part =
				traceRefractedRay(r, id, iteration, offset, inside);
			total += transmitted_part * transmissivity;
		}
		return total;
	}
	return SpectralDistribution();
}
*/

SpectralDistribution Scene::traceDiffuseRay(
	Ray r,
	IntersectionData id,
	int iteration)
{
	// Start by adding the local illumination part (shadow rays)
	SpectralDistribution total_diffuse = traceLocalDiffuseRay(r, id);
	if (!(iteration >= 2)) // Do not end here
		// Add the indirect illumination part (Monte Carlo sampling)
		total_diffuse = total_diffuse * 0.5 + traceIndirectDiffuseRay(r, id, iteration) * 0.5;
	return total_diffuse;
}

SpectralDistribution Scene::traceLocalDiffuseRay(
	Ray r,
	IntersectionData id)
{
	SpectralDistribution L_local;
	// Cast shadow rays
	// We devide up the area light source in to n_samples area parts.
	// Used to define the solid angle
	static const int n_samples = 10;
	for (int i = 0; i < lamps_.size(); ++i)
	{
		for (int j = 0; j < n_samples; ++j)
		{
			Ray shadow_ray = r;
			glm::vec3 differance = lamps_[i]->getPointOnSurface((*dis_)(*gen_),(*dis_)(*gen_)) - shadow_ray.position;
			shadow_ray.direction = glm::normalize(differance);

			float brdf = 1 / (2 * M_PI); // Dependent on inclination and azimuth
			float estimator = 1 / (2 * M_PI); // Dependent on inclination and azimuth
			float cos_theta = glm::dot(shadow_ray.direction, id.normal);

			LightSourceIntersectionData shadow_ray_id;

			if(intersectLamp(&shadow_ray_id, shadow_ray))
			{
				float cos_alpha = glm::dot(shadow_ray_id.normal, -shadow_ray.direction);
				float light_solid_angle = shadow_ray_id.area / n_samples * glm::clamp(cos_alpha, 0.0f, 1.0f) / glm::pow(glm::length(differance), 2);

				L_local +=
					(shadow_ray_id.color * shadow_ray_id.radiosity) * // Radiosity
					brdf * cos_theta / estimator *
					id.material.color_diffuse *
					light_solid_angle;// * 1 / glm::pow(glm::length(differance), 2) * glm::clamp(cos_alpha, 0.f, 1.f);
			}
		}
	}
	return L_local /= n_samples;
}

SpectralDistribution Scene::traceIndirectDiffuseRay(
	Ray r,
	IntersectionData id,
	int iteration)
{
	SpectralDistribution L_indirect;
	static const int n_samples = 1;
	for (int i = 0; i < n_samples; ++i)
	{
		// helper is just a random vector and can not possibly be
		// a zero vector since id.normal is normalized
		glm::vec3 helper = id.normal + glm::vec3(1,1,1);
		glm::vec3 tangent = glm::normalize(glm::cross(id.normal, helper));

		// rand1 is a random number from the cosine estimator
		float rand1 = glm::asin((*dis_)(*gen_));// (*dis_)(*gen_);
		float rand2 = (*dis_)(*gen_);

		// Uniform distribution over a half sphere
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

		float cos_angle = glm::dot(random_direction, id.normal);
		float estimator = cos_angle / M_PI;// 1 / (2 * M_PI);

		r.direction = random_direction;

		L_indirect += traceRay(r, iteration + 1) * brdf * cos_angle / estimator * id.material.color_diffuse;
	}
	return (L_indirect[0] < 1 && L_indirect[1] < 1 && L_indirect[2] < 2 ) ? L_indirect / n_samples : SpectralDistribution();
	//return L_indirect / n_samples;
}

SpectralDistribution Scene::traceSpecularRay(
	Ray r,
	IntersectionData id,
	int iteration)
{
	SpectralDistribution specular = SpectralDistribution();
	if (!(iteration >= 5)) // Do not end here
	{		
		// Add some randomization to the direction vector
		r.direction = glm::reflect(r.direction, id.normal);// shake(perfect_reflection, id.material.polish_power);
		// Recursively trace the reflected ray
		specular += traceRay(r, iteration + 1) * id.material.color_specular;
	}
	return specular;
}

SpectralDistribution Scene::traceRefractedRay(
	Ray r,
	IntersectionData id,
	int iteration,
	glm::vec3 offset,
	bool inside)
{
	if (iteration >= 5)
		return SpectralDistribution();
	
	Ray recursive_ray = r;

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
			offset = -offset;
		
		// Reflected ray
		// Change the material the ray is travelling in
		recursive_ray_reflected.material = Material::air();
		recursive_ray_reflected.position = r.position + id.t * r.direction +offset;
		// Refracted ray
		// Change the material the ray is travelling in
		recursive_ray_refracted.material = id.material;
		recursive_ray_refracted.position = r.position + id.t * r.direction -offset;
		
		SpectralDistribution to_return;
		// Add some randomization to the direction vectors
		recursive_ray_reflected.direction = perfect_reflection; // shake(perfect_reflection, id.material.polish_power);
		recursive_ray_refracted.direction = perfect_refraction; // shake(perfect_refraction, id.material.clearness_power);

		// Recursively trace the refracted rays
		SpectralDistribution reflected_part = traceRay(recursive_ray_reflected, iteration + 1) * id.material.color_specular * R;
		SpectralDistribution refracted_part= traceRay(recursive_ray_refracted, iteration + 1) * (1 - R) * id.material.color_diffuse;
		return reflected_part + refracted_part;
	}
	else
	{ // Brewster angle reached, complete specular reflection
		if (inside)
			recursive_ray.position = r.position + id.t * r.direction - offset;
		else
			recursive_ray.position = r.position + id.t * r.direction + offset;
		// Add some randomization to the direction vector
		recursive_ray.direction = perfect_reflection; // shake(perfect_reflection, id.material.polish_power);
		// Recursively trace the reflected ray
		return traceRay(recursive_ray, iteration + 1) * id.material.color_specular;
	}
}

SpectralDistribution Scene::traceRay(Ray r, int iteration)
{
	IntersectionData id;
	LightSourceIntersectionData lamp_id;
	if (intersectLamp(&lamp_id, r)) // Ray hitted light source
		return lamp_id.color * lamp_id.radiosity;
	else if (intersect(&id, r))
	{ // Ray hit another object
		// To make sure it does not intersect with itself again
		glm::vec3 offset = id.normal * 0.0001f;
		bool inside = false;
		if (glm::dot(id.normal, r.direction) > 0) // The ray is inside an object
			inside = true;
		
		float transmissivity = id.material.transmissivity;
		float reflectance = id.material.reflectance;
		float specularity = id.material.specular_reflectance;

		SpectralDistribution total;
		if (1 - transmissivity)
		{ // Completely or partly reflected
			Ray recursive_ray = r;
			// New position same in both cases, can be computed once outside
			// of trace functions
			recursive_ray.position = r.position + id.t * r.direction +
				(inside ? -offset : offset);
			SpectralDistribution specular_part =
				specularity ?
					traceSpecularRay(
						recursive_ray,
						id,
						iteration) * specularity :
					SpectralDistribution();
			SpectralDistribution diffuse_part =
				(1 - specularity) ?
					traceDiffuseRay(
						recursive_ray,
						id,
						iteration) * (1 - specularity) :
					SpectralDistribution();
			total +=
				(specular_part + diffuse_part) *
				(1 - transmissivity) *
				reflectance;
		}
		if (transmissivity)
		{ // Completely or partly transmissive
			SpectralDistribution transmitted_part =
				traceRefractedRay(r, id, iteration, offset, inside);
			total += transmitted_part * transmissivity;
		}
		return total;
	}
	return SpectralDistribution();
}
