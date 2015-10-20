#include "../include/Object3D.h"

#include "../external_libraries/common_include/objloader.h"
#include "../external_libraries/common_include/vboindexer.h"

#include <random>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

// --- Object3D class functions --- //

Object3D::Object3D(Material* material) : 
	material_(material)
{}

Material Object3D::material() const
{
	return material_ ? *material_ : Material();
}

// --- Mesh class functions --- //

Mesh::Mesh(glm::mat4 transform, const char* file_path, Material * material) :
	Object3D(material)
{
	transform_ = transform;

	std::vector<glm::vec3> tmp_positions;
	std::vector<glm::vec2> tmp_uvs;
	std::vector<glm::vec3> tmp_normals;

	if(!loadOBJ(file_path, tmp_positions, tmp_uvs, tmp_normals))
		exit (EXIT_FAILURE);
	for (int i = 0; i < tmp_positions.size(); ++i)
	{
		tmp_positions[i] = glm::vec3(transform_ * glm::vec4(tmp_positions[i], 1));
		tmp_normals[i] = glm::vec3(transform_ * glm::vec4(tmp_normals[i], 0));
	}
	indexVBO(
		tmp_positions,
		tmp_uvs,
		tmp_normals,

		indices_,
		positions_,
		uvs_,
		normals_);


	std::cout << "Building octree for mesh." << std::endl;
	ot_aabb_ = new OctTreeAABB(this);
	std::cout << "Octree built." << std::endl;
}

bool Mesh::intersect(IntersectionData* id, Ray r) const
{
	return ot_aabb_->intersect(id, r);
}

glm::mat4 Mesh::getTransform() const
{
	return transform_;
}

glm::vec3 Mesh::getMinPosition() const
{
	glm::vec3 min = positions_[0];
	for (int i = 1; i < positions_.size(); ++i)
	{
		glm::vec3 p = positions_[i];
		min.x = p.x < min.x ? p.x : min.x;
		min.y = p.y < min.y ? p.y : min.y;
		min.z = p.z < min.z ? p.z : min.z;
	}
	return min;
}

glm::vec3 Mesh::getMaxPosition() const
{
	glm::vec3 max = positions_[0];
	for (int i = 1; i < positions_.size(); ++i)
	{
		glm::vec3 p = positions_[i];
		max.x = p.x > max.x ? p.x : max.x;
		max.y = p.y > max.y ? p.y : max.y;
		max.z = p.z > max.z ? p.z : max.z;
	}
	return max;
}

int Mesh::getNumberOfTriangles() const
{
	return indices_.size() / 3;
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
	float p_half = glm::dot((r.origin - POSITION_), r.direction);
	float to_square = 
		pow(p_half, 2) + 
		pow(RADIUS_, 2) - 
		pow(glm::length(r.origin - POSITION_), 2);
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
		n = r.origin + t*r.direction - POSITION_;
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

glm::vec3 Sphere::getPointOnSurface(float u, float v) const
{
	// Uniform over a sphere
	float inclination = glm::acos(1 - 2 * u);
	float azimuth = 2 * M_PI * v;

	glm::vec3 random_direction = glm::vec3(1,0,0);
	random_direction = glm::normalize(glm::rotate(
		random_direction,
		inclination,
		glm::vec3(0,1,0)));
	random_direction = glm::normalize(glm::rotate(
		random_direction,
		azimuth,
		glm::vec3(1,0,0)));

	return POSITION_ + random_direction * RADIUS_;
}

// --- Plane class functions --- //

Plane::Plane(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, Material* material) : 
	Object3D(material),
	P0_(p0),
	P1_(p1),
	P2_(p2),
	NORMAL_(glm::normalize(glm::cross(p0 - p1, p0 - p2))),
	AREA_(glm::length(glm::cross(p0 - p1, p0 - p2)))
{}

bool Plane::intersect(IntersectionData* id, Ray r) const
{
	// Möller–Trumbore intersection algorithm

	glm::vec3 e1, e2;  //Edge1, Edge2
	glm::vec3 P, Q, T;
	float det, inv_det, u, v;
	float t;

	// Find vectors for two edges sharing P0_
	e1 = P1_ - P0_;
	e2 = P2_ - P0_;
	// Begin calculating determinant - also used to calculate u parameter
	P = glm::cross(r.direction, e2);
	// if determinant is near zero, ray lies in plane of triangle
	det = glm::dot(e1, P);
	// NOT CULLING
	if(det > -0.00001 && det < 0.00001) return false;
		inv_det = 1.f / det;

	// calculate distance from P0_ to ray origin
	T = r.origin - P0_;
	Q = glm::cross(T, e1);

	// Calculate u parameter and test bound
	u = glm::dot(T, P) * inv_det;
	v = glm::dot(r.direction, Q) * inv_det;

	// The intersection lies outside of the plane
	if(u < 0.f || u > 1.f || v < 0.f || v > 1.f) return false;

	t = glm::dot(e2, Q) * inv_det;

	if(t > 0.00001) { //ray intersection
	id->t = t;
	id->normal = glm::normalize(glm::cross(e1, e2));
	id->material = material();
	return true;
	}

	// No hit, no win
	return false;
}

glm::vec3 Plane::getPointOnSurface(float u, float v) const
{
	glm::vec3 v1 = P1_ - P0_;
	glm::vec3 v2 = P2_ - P0_;
	return P0_ + u * v1 + v * v2;
}

float Plane::getArea() const
{
	return AREA_;
}

glm::vec3 Plane::getNormal() const
{
	return NORMAL_;
}

glm::vec3 Plane::getFirstTangent() const
{
	return glm::normalize(P1_ - P0_);
}

// --- LightSource class functions --- //

LightSource::LightSource(
	glm::vec3 p0,
	glm::vec3 p1,
	glm::vec3 p2,
	float flux,
	SpectralDistribution color) :
	emitter_(p0, p1, p2, NULL),
	radiosity(flux / emitter_.getArea() * color)
{}

bool LightSource::intersect(LightSourceIntersectionData* light_id, Ray r)
{
	IntersectionData id;
	if(emitter_.intersect(&id, r))
	{
		light_id->normal = id.normal;
		light_id->t = id.t;
		light_id->radiosity = radiosity;
		light_id->area = getArea();
		return true;
	}
	else
		return false;
}

glm::vec3 LightSource::getPointOnSurface(float u, float v)
{
	return emitter_.getPointOnSurface(u, v);
}

float LightSource::getArea() const
{
	return emitter_.getArea();
}

glm::vec3 LightSource::getNormal() const
{
	return emitter_.getNormal();
}

Ray LightSource::shootLightRay()
{
	// Move random code out later
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> dis(0, 1);

	Ray r;
	r.origin = getPointOnSurface(dis(gen), dis(gen));

	// Get a uniformly distributed vector
	glm::vec3 normal = emitter_.getNormal();
	glm::vec3 tangent = emitter_.getFirstTangent();
	// rand1 is a random number from the cosine estimator
	float rand1 = dis(gen);//glm::asin(dis(gen));// (*dis_)(*gen_);
	float rand2 = dis(gen);

	// Uniform distribution
	float inclination = acos(sqrt(rand1));//glm::acos(1 - rand1);//glm::acos(1 -  2 * (*dis_)(*gen_));
	float azimuth = 2 * M_PI * rand2;
	// Change the actual vector
	glm::vec3 random_direction = normal;
	random_direction = glm::normalize(glm::rotate(
		random_direction,
		inclination,
		tangent));
	random_direction = glm::normalize(glm::rotate(
		random_direction,
		azimuth,
		normal));

	r.direction = random_direction;
	r.material = Material::air();
	
	return r;
}