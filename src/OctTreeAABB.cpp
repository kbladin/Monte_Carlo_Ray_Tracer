#include "../include/OctTreeAABB.h"
#include "../include/Object3D.h"

#include "../external_libraries/common_include/TriangleCube.h"

#include <iostream>

// --- AABB class functions --- //

bool AABB::intersect(Ray r) const
{
	glm::vec3 origin =
		glm::vec3(glm::inverse(transform) *
		glm::vec4(r.position, 1));
	glm::vec3 direction =
		glm::vec3(glm::inverse(transform) *
		glm::vec4(r.direction, 0));

	// r.dir is unit direction vector of ray
	glm::vec3 dirfrac(1.0f / direction.x, 1.0f / direction.y, 1.0f / direction.z);
	// lb is the corner of AABB with minimal coordinates - left bottom, 
	// rt is maximal corner
	// r.org is the origin of ray
	float t1 = (min.x - origin.x)*dirfrac.x;
	float t2 = (max.x - origin.x)*dirfrac.x;
	float t3 = (min.y - origin.y)*dirfrac.y;
	float t4 = (max.y - origin.y)*dirfrac.y;
	float t5 = (min.z - origin.z)*dirfrac.z;
	float t6 = (max.z - origin.z)*dirfrac.z;

	float tmin = glm::max(
		glm::max(glm::min(t1, t2), glm::min(t3, t4)),
		glm::min(t5, t6));
	float tmax = glm::min(
		glm::min(glm::max(t1, t2), glm::max(t3, t4)),
		glm::max(t5, t6));

	// if tmax < 0, ray (line) is intersecting AABB, but whole AABB is behing us
	if (tmax < 0)
	{
	    //*t = tmax;
	    return false;
	}

	// if tmin > tmax, ray doesn't intersect AABB
	if (tmin > tmax)
	{
	    //*t = tmax;
	    return false;
	}

	//*t = tmin;
	return true;
	return false;
}

bool AABB::intersectTriangle(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2) const
{
	// Transform triangle to compare with a unit cube centered in origin.
	glm::vec3 center_point = (min + max) / 2.0f;
	glm::vec3 scale = (max - center_point) / 2.0f;

	p0 -= center_point;
	p1 -= center_point;
	p2 -= center_point;

	p0 *= scale;
	p1 *= scale;
	p2 *= scale;

	// Convert to format used in function t_c_intersection()
	Point3 p0_point3;
	Point3 p1_point3;
	Point3 p2_point3;

	p0_point3.x = p0.x;
	p0_point3.y = p0.y;
	p0_point3.z = p0.z;

	p1_point3.x = p1.x;
	p1_point3.y = p1.y;
	p1_point3.z = p1.z;

	p2_point3.x = p2.x;
	p2_point3.y = p2.y;
	p2_point3.z = p2.z;

	Triangle3 triangle3;
	triangle3.v1 = p0_point3;
	triangle3.v2 = p1_point3;
	triangle3.v3 = p2_point3;

	// Do the actual test
	return t_c_intersection(triangle3) == INSIDE;
}

// --- OctNodeAABB class functions --- //

OctNodeAABB::OctNodeAABB(
	int depth,
	Mesh* mesh,
	glm::vec3 aabb_min,
	glm::vec3 aabb_max)
{
	mesh_ = mesh;
	aabb_.min = aabb_min;
	aabb_.max = aabb_max;
	aabb_.transform = mesh->getTransform();

	if (depth == 0)
	{ // Base case
		left_bottom_far_ = 		NULL;
		right_bottom_far_ = 	NULL;
		left_top_far_ = 		NULL;
		right_top_far_ = 		NULL;
		left_bottom_near_ = 	NULL;
		right_bottom_near_ = 	NULL;
		left_top_near_ = 		NULL;
		right_top_near_ = 		NULL;
	}
	else
	{ // Continue recursion
		left_bottom_far_ = 		new OctNodeAABB(
			depth - 1,
			mesh,
			glm::vec3(
				aabb_min.x,
				aabb_min.y,
				aabb_min.z),
			glm::vec3(
				(aabb_min.x + aabb_max.x) / 2,
				(aabb_min.y + aabb_max.y) / 2,
				(aabb_min.z + aabb_max.z) / 2));
		right_bottom_far_ = 	new OctNodeAABB(
			depth - 1,
			mesh,
			glm::vec3(
				(aabb_min.x + aabb_max.x) / 2,
				aabb_min.y,
				aabb_min.z),
			glm::vec3(
				aabb_max.x,
				(aabb_min.y + aabb_max.y) / 2,
				(aabb_min.z + aabb_max.z) / 2));
		left_top_far_ = 		new OctNodeAABB(
			depth - 1,
			mesh,
			glm::vec3(
				aabb_min.x,
				(aabb_min.y + aabb_max.y) / 2,
				aabb_min.z),
			glm::vec3(
				(aabb_min.x + aabb_max.x) / 2,
				aabb_max.y,
				(aabb_min.z + aabb_max.z) / 2));
		right_top_far_ = 		new OctNodeAABB(
			depth - 1,
			mesh,
			glm::vec3(
				(aabb_min.x + aabb_max.x) / 2,
				(aabb_min.y + aabb_max.y) / 2,
				aabb_min.z),
			glm::vec3(
				aabb_max.x,
				aabb_max.y,
				(aabb_min.z + aabb_max.z) / 2));
		left_bottom_near_ = 	new OctNodeAABB(
			depth - 1,
			mesh,
			glm::vec3(
				aabb_min.x,
				aabb_min.y,
				(aabb_min.z + aabb_max.z) / 2),
			glm::vec3(
				(aabb_min.x + aabb_max.x) / 2,
				(aabb_min.y + aabb_max.y) / 2,
				aabb_max.z));
		right_bottom_near_ = 	new OctNodeAABB(
			depth - 1,
			mesh,
			glm::vec3(
				(aabb_min.x + aabb_max.x) / 2,
				aabb_min.y,
				(aabb_min.z + aabb_max.z) / 2),
			glm::vec3(
				aabb_max.x,
				(aabb_min.y + aabb_max.y) / 2,
				aabb_max.z));
		left_top_near_ = 		new OctNodeAABB(
			depth - 1,
			mesh,
			glm::vec3(
				aabb_min.x,
				(aabb_min.y + aabb_max.y) / 2,
				(aabb_min.z + aabb_max.z) / 2),
			glm::vec3(
				(aabb_min.x + aabb_max.x) / 2,
				aabb_max.y,
				aabb_max.z));
		right_top_near_ = 		new OctNodeAABB(
			depth - 1,
			mesh,
			glm::vec3(
				(aabb_min.x + aabb_max.x) / 2,
				(aabb_min.y + aabb_max.y) / 2,
				(aabb_min.z + aabb_max.z) / 2),
			glm::vec3(
				aabb_max.x,
				aabb_max.y,
				aabb_max.z));
	}

	// Find which triangles are in this AABB
	// A bit of a waste to check all triangles but this way the node does not
	// have to have a pointer to its parent.
	for (int i = 0; i < mesh->indices_.size(); i=i+3)
	{
		if (aabb_.intersectTriangle(
			mesh->positions_[mesh->indices_[i + 0]],
			mesh->positions_[mesh->indices_[i + 1]],
			mesh->positions_[mesh->indices_[i + 2]]))
		{ // Insert the triangle in this node
			triangle_indices_.push_back(mesh->indices_[i + 0]);
			triangle_indices_.push_back(mesh->indices_[i + 1]);
			triangle_indices_.push_back(mesh->indices_[i + 2]);
		}
	}
}

OctNodeAABB::~OctNodeAABB()
{
	if(left_bottom_far_) delete left_bottom_far_;
	if(right_bottom_far_) delete right_bottom_far_;
	if(left_top_far_) delete left_top_far_;
	if(right_top_far_) delete right_top_far_;
	if(left_bottom_near_) delete left_bottom_near_;
	if(right_bottom_near_) delete right_bottom_near_;
	if(left_top_near_) delete left_top_near_;
	if(right_top_near_) delete right_top_near_;
}

bool OctNodeAABB::intersect(IntersectionData* id, Ray r) const
{
	if (triangle_indices_.size() == 0)
		return false;
	else if (left_bottom_far_ == NULL)
	{ // Reached a leaf node
		float t_smallest = 10000000;
		bool intersect = false;

		glm::vec3 p0;
		glm::vec3 p1;
		glm::vec3 p2;

		glm::vec3 e1, e2;  //Edge1, Edge2
		glm::vec3 P, Q, T;
		float det, inv_det, u, v;
		float t;

		for (int i = 0; i < triangle_indices_.size(); i=i+3)
		{
			// Möller–Trumbore intersection algorithm for triangle

			p0 = glm::vec3(aabb_.transform * glm::vec4(mesh_->positions_[triangle_indices_[i + 0]], 1));
			p1 = glm::vec3(aabb_.transform * glm::vec4(mesh_->positions_[triangle_indices_[i + 1]], 1));
			p2 = glm::vec3(aabb_.transform * glm::vec4(mesh_->positions_[triangle_indices_[i + 2]], 1));

			// Find vectors for two edges sharing p0
			e1 = p1 - p0;
			e2 = p2 - p0;
			// Begin calculating determinant - also used to calculate u parameter
			P = glm::cross(r.direction, e2);
			// if determinant is near zero, ray lies in plane of triangle
			det = glm::dot(e1, P);
			// NOT CULLING
			if(det > -0.00001 && det < 0.00001) {
				continue;
			}
			
			inv_det = 1.f / det;

			//calculate distance from V1 to ray origin
			T = r.position - p0;

			//Calculate u parameter and test bound
			u = glm::dot(T, P) * inv_det;
			//The intersection lies outside of the triangle
			if(u < 0.f || u > 1.f) {
				continue;
			}

			//Prepare to test v parameter
			Q = glm::cross(T, e1);

			//Calculate V parameter and test bound
			v = glm::dot(r.direction, Q) * inv_det;
			//The intersection lies outside of the triangle
			if(v < 0.f || u + v  > 1.f) {
				continue;
			}

			t = glm::dot(e2, Q) * inv_det;

			if(t > 0.00001 && t < t_smallest) { //ray intersection
				t_smallest = t;
				glm::vec3 n0 = glm::vec3(aabb_.transform * glm::vec4(mesh_->normals_[triangle_indices_[i + 0]], 0));
				glm::vec3 n1 = glm::vec3(aabb_.transform * glm::vec4(mesh_->normals_[triangle_indices_[i + 1]], 0));
				glm::vec3 n2 = glm::vec3(aabb_.transform * glm::vec4(mesh_->normals_[triangle_indices_[i + 2]], 0));

				// Interpolate to find normal
				glm::vec3 n = (1 - u - v) * n0 + u * n1 + v * n2;
				id->t = t;
				id->normal = glm::normalize(n);
				id->material = mesh_->material();
				intersect = true;
			}
		}
		return intersect;
	}
	else
	{ // Check intersection with all the child nodes
		if(left_bottom_far_->aabb_.intersect(r))
			return left_bottom_far_->intersect(id, r);
		else if(right_bottom_far_->aabb_.intersect(r))
			return right_bottom_far_->intersect(id, r);
		else if(left_top_far_->aabb_.intersect(r))
			return left_top_far_->intersect(id, r);
		else if(right_top_far_->aabb_.intersect(r))
			return right_top_far_->intersect(id, r);
		else if(left_bottom_near_->aabb_.intersect(r))
			return left_bottom_near_->intersect(id, r);
		else if(right_bottom_near_->aabb_.intersect(r))
			return right_bottom_near_->intersect(id, r);
		else if(left_top_near_->aabb_.intersect(r))
			return left_top_near_->intersect(id, r);
		else if(right_top_near_->aabb_.intersect(r))
			return right_top_near_->intersect(id, r);
	}
	return false;
}

// --- OctTreeAABB class functions --- //

OctTreeAABB::OctTreeAABB(
	int depth,
	Mesh* mesh) : 
	OctNodeAABB(
		depth,
		mesh,
		mesh->getMinPosition(),
		mesh->getMaxPosition())
{
}

OctTreeAABB::~OctTreeAABB()
{
}