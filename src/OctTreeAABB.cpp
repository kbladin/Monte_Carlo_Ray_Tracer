#include "../include/OctTreeAABB.h"
#include "../include/Object3D.h"

#include "../external_libraries/common_include/boxOverlap.h"

#include <iostream>

// --- AABB class functions --- //

bool AABB::intersect(Ray r) const
{
	glm::vec3 origin = r.origin;
	glm::vec3 direction = r.direction;

	// r.dir is unit direction vector of ray
	glm::vec3 dirfrac(1.0f / direction.x, 1.0f / direction.y, 1.0f / direction.z);
	// lb is the corner of AABB with minimal coordinates - left bottom, 
	// rt is maximal corner
	// r.org is the origin of ray
	float t1 = (min_.x - origin.x)*dirfrac.x;
	float t2 = (max_.x - origin.x)*dirfrac.x;
	float t3 = (min_.y - origin.y)*dirfrac.y;
	float t4 = (max_.y - origin.y)*dirfrac.y;
	float t5 = (min_.z - origin.z)*dirfrac.z;
	float t6 = (max_.z - origin.z)*dirfrac.z;

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
	glm::vec3 center_point = (min_ + max_) / 2.0f;
	glm::vec3 scale = (max_ - center_point) / 1.0f;

	// Convert to format used by triBoxOverlap()
	float boxcenter[3] = {center_point[0], center_point[1], center_point[2]};
	float boxhalfsize[3] = {scale[0], scale[1], scale[2]};
	float triverts[3][3] = {{p0.x, p0.y, p0.z}, {p1.x, p1.y, p1.z}, {p2.x, p2.y, p2.z}};

	return triBoxOverlap( boxcenter, boxhalfsize, triverts) == 1;
}

// --- OctNodeAABB class functions --- //

OctNodeAABB::OctNodeAABB(
	OctNodeAABB* parent,
	int depth,
	Mesh* mesh,
	glm::vec3 aabb_min,
	glm::vec3 aabb_max)
{
	aabb_.min_ = aabb_min;
	aabb_.max_ = aabb_max;
	mesh_ = mesh;
	// Find which triangles are in this AABB
	// If it has no parent this is the root node
	std::vector<unsigned int>& index_list =
		!parent ?
		mesh->indices_ :
		parent->triangle_indices_;
	
	// Check parents triangles and see which of them is in this node
	for (int i = 0; i < index_list.size(); i=i+3)
	{
		if (aabb_.intersectTriangle(
			mesh->positions_[index_list[i + 0]],
			mesh->positions_[index_list[i + 1]],
			mesh->positions_[index_list[i + 2]]))
		{ // Insert the triangle in this node
			triangle_indices_.push_back(index_list[i + 0]);
			triangle_indices_.push_back(index_list[i + 1]);
			triangle_indices_.push_back(index_list[i + 2]);
		}
	}
	
	if (depth == 0 || triangle_indices_.size() <= 3 * 16)
	{ // Base case
		for (int i=0; i<8; i++)
			children_[i] = NULL;
	}
	else
	{ // Continue recursion, create more children
		glm::vec3 child_aabb_min;
		glm::vec3 child_aabb_max;
		for (int i = 0; i < 8; ++i)
		{
			child_aabb_min = glm::vec3(
				i%2 	== 0 ? aabb_min.x : (aabb_min.x + aabb_max.x) / 2,
				(i/2)%2 == 0 ? aabb_min.y : (aabb_min.y + aabb_max.y) / 2,
				(i/4)%2 == 0 ? aabb_min.z : (aabb_min.z + aabb_max.z) / 2);
			child_aabb_max = glm::vec3(
				i%2 	== 0 ? (aabb_min.x + aabb_max.x) / 2 : aabb_max.x,
				(i/2)%2 == 0 ? (aabb_min.y + aabb_max.y) / 2 : aabb_max.y,
				(i/4)%2 == 0 ? (aabb_min.z + aabb_max.z) / 2 : aabb_max.z);
			children_[i] = new OctNodeAABB(
				this,
				depth - 1,
				mesh,
				child_aabb_min,
				child_aabb_max);
		}
	}
}

OctNodeAABB::~OctNodeAABB()
{
	for (int i = 0; i < 8; ++i)
	{
		if (children_[i])
			delete children_[i];
	}
}

bool OctNodeAABB::intersect(IntersectionData* id, Ray r) const
{
	if (triangle_indices_.size() == 0)
		// No triangles in this node
		return false;
	else if (children_[0] == NULL)
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

		// Check intersection for all triangles in this node
		for (int i = 0; i < triangle_indices_.size(); i=i+3)
		{
			// Möller–Trumbore intersection algorithm for triangle

			p0 = mesh_->positions_[triangle_indices_[i + 0]];
			p1 = mesh_->positions_[triangle_indices_[i + 1]];
			p2 = mesh_->positions_[triangle_indices_[i + 2]];

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
			T = r.origin - p0;

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
				glm::vec3 n0 = mesh_->normals_[triangle_indices_[i + 0]];
				glm::vec3 n1 = mesh_->normals_[triangle_indices_[i + 1]];
				glm::vec3 n2 = mesh_->normals_[triangle_indices_[i + 2]];

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
		IntersectionData closest_id;
		IntersectionData id_tmp;
		closest_id.t = 1000000000;
		bool intersect = false;
		for (int i = 0; i < 8; ++i)
			if(children_[i]->aabb_.intersect(r))
				if (children_[i]->intersect(&id_tmp, r))
					if (id_tmp.t < closest_id.t)
					{
						closest_id = id_tmp;
						intersect = true;
					}
		if (intersect)
		{
			*id = closest_id;
			return true;
		}
	}
	return false;
}

// --- OctTreeAABB class functions --- //

OctTreeAABB::OctTreeAABB(Mesh* mesh) : 
	OctNodeAABB(
		NULL,
		8, // Maximum depth of tree
		mesh,
		mesh->getMinPosition(),
		mesh->getMaxPosition())
{
}

OctTreeAABB::~OctTreeAABB()
{
}