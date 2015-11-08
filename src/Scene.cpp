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

SpectralDistribution Scene::traceDiffuseRay(
	Ray r,
	int render_mode,
	IntersectionData id,
	int iteration)
{
	r.has_intersected = true;
	// Start by adding the local illumination part (shadow rays)
	SpectralDistribution total_diffuse = traceLocalDiffuseRay(r, render_mode, id);
	//if (!(iteration >= 2)) // Do not end here
		// Add the indirect illumination part (Monte Carlo sampling)
		total_diffuse = total_diffuse + traceIndirectDiffuseRay(r, render_mode, id, iteration);
	return total_diffuse;
}

SpectralDistribution Scene::traceLocalDiffuseRay(
	Ray r,
	int render_mode,
	IntersectionData id)
{
	SpectralDistribution L_local;
	// Cast shadow rays
	// We divide up the area light source in to n_samples area parts.
	// Used to define the solid angle
	static const int n_samples = 1;
	for (int i = 0; i < lamps_.size(); ++i)
	{
		for (int j = 0; j < n_samples; ++j)
		{
			Ray shadow_ray = r;
			glm::vec3 differance = lamps_[i]->getPointOnSurface((*dis_)(*gen_),(*dis_)(*gen_)) - shadow_ray.origin;
			shadow_ray.direction = glm::normalize(differance);

			SpectralDistribution brdf;// = id.material.color_diffuse / (2 * M_PI); // Dependent on inclination and azimuth
			float cos_theta = glm::dot(shadow_ray.direction, id.normal);

			LightSourceIntersectionData shadow_ray_id;

			if (id.material.diffuse_roughness)
			{
				brdf = evaluateOrenNayarBRDF(
					-r.direction,
					shadow_ray.direction,
					id.normal,
					id.material.color_diffuse * id.material.reflectance * (1 -id.material.specular_reflectance),
					id.material.diffuse_roughness);
			}
			else
				brdf = evaluateLambertianBRDF(
					-r.direction,
					shadow_ray.direction,
					id.normal,
					id.material.color_diffuse * id.material.reflectance * (1 - id.material.specular_reflectance));

			if(intersectLamp(&shadow_ray_id, shadow_ray))
			{
				float cos_light_angle = glm::dot(shadow_ray_id.normal, -shadow_ray.direction);
				float light_solid_angle = shadow_ray_id.area / n_samples * glm::clamp(cos_light_angle, 0.0f, 1.0f) / glm::pow(glm::length(differance), 2) / (M_PI * 2);

				L_local +=
					brdf *
					shadow_ray_id.radiosity *
					cos_theta *
					light_solid_angle
					;
			}
		}
	}
	return L_local;
}

SpectralDistribution Scene::traceIndirectDiffuseRay(
	Ray r,
	int render_mode,
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
		float rand1 = (*dis_)(*gen_);
		float rand2 = (*dis_)(*gen_);

		// Uniform distribution over a hemisphere
		float inclination = acos(sqrt(rand1));//glm::acos(1 - rand1);//glm::acos(1 -  2 * (*dis_)(*gen_));
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

		float cos_angle = glm::dot(random_direction, id.normal);
		float g = cos_angle / M_PI;

		SpectralDistribution brdf;
		if (id.material.diffuse_roughness)
		{
			brdf = evaluateOrenNayarBRDF(
				-r.direction,
				random_direction,
				id.normal,
				id.material.color_diffuse * id.material.reflectance * (1 - id.material.specular_reflectance),
				id.material.diffuse_roughness);
		}
		else
		{
			brdf = evaluateLambertianBRDF(
				-r.direction,
				random_direction,
				id.normal,
				id.material.color_diffuse * id.material.reflectance * (1 - id.material.specular_reflectance));
		}

		r.direction = random_direction;
		r.radiance *= M_PI * brdf; // Importance, M_PI is because of the importance sampling
		L_indirect += traceRay(r, render_mode, iteration + 1) * M_PI * brdf;
	}
	return L_indirect / n_samples;
}

SpectralDistribution Scene::traceSpecularRay(
	Ray r,
	int render_mode,
	IntersectionData id,
	int iteration)
{
	r.has_intersected = true;
	SpectralDistribution specular = SpectralDistribution();

	r.direction = glm::reflect(r.direction, id.normal);
	SpectralDistribution brdf = evaluatePerfectBRDF(id.material.color_specular * id.material.reflectance * id.material.specular_reflectance);
	r.radiance *= brdf;
	// Recursively trace the reflected ray
	specular += traceRay(r, render_mode, iteration + 1) * brdf;
	return specular;
}

SpectralDistribution Scene::traceRefractedRay(
	Ray r,
	int render_mode,
	IntersectionData id,
	int iteration,
	glm::vec3 offset,
	bool inside)
{
	Ray recursive_ray = r;
	recursive_ray.has_intersected = true;

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
		recursive_ray_reflected.origin = r.origin + id.t * r.direction +offset;
		// Refracted ray
		// Change the material the ray is travelling in
		recursive_ray_refracted.material = id.material;
		recursive_ray_refracted.origin = r.origin + id.t * r.direction -offset;
		
		SpectralDistribution to_return;
		recursive_ray_reflected.direction = perfect_reflection;
		recursive_ray_refracted.direction = perfect_refraction;

		SpectralDistribution brdf_specular = evaluatePerfectBRDF(id.material.color_specular * id.material.reflectance * id.material.specular_reflectance * R);
		SpectralDistribution brdf_refractive = evaluatePerfectBRDF(id.material.color_diffuse * id.material.reflectance * id.material.specular_reflectance * (1 - R));


		recursive_ray_reflected.radiance *= brdf_specular;
		recursive_ray_refracted.radiance *= brdf_refractive;

		// Recursively trace the refracted rays
		SpectralDistribution reflected_part = traceRay(recursive_ray_reflected, render_mode, iteration + 1) * brdf_specular;
		SpectralDistribution refracted_part = traceRay(recursive_ray_refracted, render_mode, iteration + 1) * brdf_refractive;
		return reflected_part + refracted_part;
	}
	else
	{ // Brewster angle reached, complete specular reflection
		if (inside)
			recursive_ray.origin = r.origin + id.t * r.direction - offset;
		else
			recursive_ray.origin = r.origin + id.t * r.direction + offset;

		SpectralDistribution brdf_specular = evaluatePerfectBRDF(id.material.color_specular * id.material.reflectance * id.material.specular_reflectance);
		recursive_ray.direction = perfect_reflection;
		recursive_ray.radiance *= brdf_specular;
		// Recursively trace the reflected ray
		return traceRay(recursive_ray, render_mode, iteration + 1) * brdf_specular;
	}
}

SpectralDistribution Scene::traceRay(Ray r, int render_mode, int iteration)
{
	IntersectionData id;
	LightSourceIntersectionData lamp_id;

	if (intersectLamp(&lamp_id, r)) // Ray hit light source
		switch (render_mode)
		{
			case WHITTED_SPECULAR :
				return lamp_id.radiosity / (M_PI * 2);
			default :
				return SpectralDistribution();
		}
	else if (intersect(&id, r))
	{ // Ray hit another object
		// Russian roulette
		float random = (*dis_)(*gen_);
		//float non_termination_probability = glm::max((1 - float(iteration) / 10), 0.5f);
		float non_termination_probability = iteration == 0 ? 1.0 : 0.8;
		if (random > non_termination_probability || iteration > 20)
			return SpectralDistribution();
		//if (iteration >= 4)
		//	return SpectralDistribution();

		// To make sure it does not intersect with itself again
		glm::vec3 offset = id.normal * 0.00001f;
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
			// New position same in both cases, can be computed once, outside
			// of trace functions
			recursive_ray.origin = r.origin + id.t * r.direction +
				(inside ? -offset : offset);
			SpectralDistribution specular_part =
				specularity ?
					traceSpecularRay(
						recursive_ray,
						render_mode,
						id,
						iteration) :
					SpectralDistribution();
			SpectralDistribution diffuse_part;
			switch (render_mode)
			{
				case PHOTON_MAPPING :
				{
					if (r.has_intersected)
					{
						Photon p;
						p.position = recursive_ray.origin;
						p.direction_in = -r.direction;

						float photon_area = Photon::RADIUS * Photon::RADIUS * M_PI;
						// The projected area should be photon area times cos theta,
						// This is avoided both here and later to avoid numerical problem
						// when dividing with small numbers.
						float projected_area = photon_area;// * glm::dot(p.direction_in, id.normal);
						float solid_angle = M_PI;
			
						p.delta_flux = recursive_ray.radiance / non_termination_probability * projected_area * solid_angle;

						KDTreeNode to_insert;
						to_insert.p = p;

						photon_map_.insert(to_insert);
					}
					break;
				}
				case CAUSTICS :
				{
					KDTreeNode ref_node;
					ref_node.p.position = r.origin + r.direction * id.t + offset;

					std::vector<KDTreeNode> closest_photons;
					photon_map_.find_within_range(ref_node,Photon::RADIUS,std::back_insert_iterator<std::vector<KDTreeNode> >(closest_photons));
					
					SpectralDistribution photon_radiance;
					for (int i = 0; i < closest_photons.size(); ++i)
					{
						SpectralDistribution brdf;
						if (id.material.diffuse_roughness)
						{
							brdf = evaluateOrenNayarBRDF(
								-r.direction,
								closest_photons[i].p.direction_in,
								id.normal,
								id.material.color_diffuse * id.material.reflectance * (1 - id.material.specular_reflectance),
								id.material.diffuse_roughness);
						}
						else
						{
							brdf = evaluateLambertianBRDF(
								-r.direction,
								closest_photons[i].p.direction_in,
								id.normal,
								id.material.color_diffuse * id.material.reflectance * (1 - id.material.specular_reflectance));
						}

						float distance = glm::length(closest_photons[i].p.position - ref_node.p.position);
						// The area of the photon if its inclination angle
						// is 90 degrees and the surface is flat.
						//float cos_theta = glm::max(glm::dot(closest_photons[i].p.direction_in, id.normal), 0.0f);
						float photon_area = Photon::RADIUS * Photon::RADIUS * M_PI;
						float projected_area = photon_area;// * cos_theta;
						photon_radiance +=
							// flux / area / steradian = radiance
							closest_photons[i].p.delta_flux *
							(glm::length(distance) < Photon::RADIUS ? 1 : 0)
							/ (projected_area * 2 * M_PI)
							//
							* brdf // The brdf is part of the integral of the rendering equation
							* (2 * M_PI); // Integration over the whole hemisphere get us back to radiance
					}

					diffuse_part = photon_radiance;
					break;
				}
				case WHITTED_SPECULAR :
				{
					diffuse_part = SpectralDistribution();
					break;
				}
				case MONTE_CARLO :
				{
					diffuse_part =
						(1 - specularity) ?
							traceDiffuseRay(
								recursive_ray,
								render_mode,
								id,
								iteration) :
							SpectralDistribution();
					break;
				}
				default :
				{
					diffuse_part = SpectralDistribution();
					break;
				}
			}

			total +=
				(specular_part + diffuse_part) *
				(1 - transmissivity);
		}
		if (transmissivity)
		{ // Completely or partly transmissive
			SpectralDistribution transmitted_part =
				traceRefractedRay(r, render_mode, id, iteration, offset, inside);
			total += transmitted_part * transmissivity;
		}
		return total / non_termination_probability;
	}
	return SpectralDistribution();
}

void Scene::buildPhotonMap(const int n_photons)
{
	if (lamps_.size())
	{
		SpectralDistribution total_flux = SpectralDistribution();
		float total_flux_norm = 0;
		for (int i = 0; i < lamps_.size(); ++i)
		{
			total_flux_norm += lamps_[i]->radiosity.norm() * lamps_[i]->getArea();
			total_flux += lamps_[i]->radiosity * lamps_[i]->getArea();
		}
		for (int k = 0; k < 100; ++k)
		{
			#pragma omp parallel for
			for (int i = 0; i < n_photons / 100; ++i)
			{
				// Pick a light source. Bigger flux => Bigger chance to be picked.
				int picked_light_source = 0;
				float accumulating_chance = 0;
				float random = (*dis_)(*gen_);
				for (int i = 0; i < lamps_.size(); ++i)
				{
					float interval =
						lamps_[i]->radiosity.norm() *
						lamps_[i]->getArea() /
						total_flux_norm;
					if (random > accumulating_chance && random < accumulating_chance + interval)
					{ // This lamp got picked
						picked_light_source = i;
						break;
					}
					else
						accumulating_chance += interval; 
				}

				Ray r = lamps_[picked_light_source]->shootLightRay();
				r.has_intersected = false;
				// Compute delta_flux based on the flux of the light source
				SpectralDistribution delta_flux = total_flux / n_photons;
				float photon_area = Photon::RADIUS * Photon::RADIUS * M_PI;
				float solid_angle = (M_PI * 2);
				r.radiance = delta_flux / (photon_area * solid_angle);
				traceRay(r, PHOTON_MAPPING);
			}
			std::cout << k << "\% of photon map finished." << std::endl;
		}
		std::cout << "Number of photons in scene: " << photon_map_.size() << std::endl;
		std::cout << "Optimizing kd tree" << std::endl;
		photon_map_.optimize();
	}
	else
	{
		std::cout << "No lightsource in the scene. Could not build photon map." << std::endl;
	}
}

int Scene::getNumberOfTriangles()
{
	int n_triangles = 0;
	for (int i = 0; i < objects_.size(); ++i)
	{
		if(Mesh* m = dynamic_cast<Mesh*>(objects_[i])) {
		   // old was safely casted to NewType
		   n_triangles += m->getNumberOfTriangles();
		}
	}
	return  n_triangles;
}

int Scene::getNumberOfObjects()
{
	return objects_.size();
}

int Scene::getNumberOfSpheres()
{
	int n_spheres = 0;
	for (int i = 0; i < objects_.size(); ++i)
	{
		if(Sphere* s = dynamic_cast<Sphere*>(objects_[i])) {
		   // old was safely casted to NewType
		   n_spheres ++;
		}
	}
	return  n_spheres;
}

int Scene::getNumberOfPhotons()
{
	return photon_map_.size();
}

