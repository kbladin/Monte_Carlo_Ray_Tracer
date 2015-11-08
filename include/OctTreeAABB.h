#ifndef OCT_TREE_AABB
#define OCT_TREE_AABB

#include <vector>

#include <glm/glm.hpp>
#include "utils.h"

class Mesh;

// Axis aligned bounding box.
struct AABB
{
	bool intersect(Ray r) const;
	bool intersectTriangle(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2) const;

	glm::vec3 min_;
	glm::vec3 max_;
};

// A node of an octree. Has eight child nodes
class OctNodeAABB
{
public:
	OctNodeAABB(
		OctNodeAABB* parent,
		int depth,
		Mesh* mesh,
		glm::vec3 aabb_min,
		glm::vec3 aabb_max);
	~OctNodeAABB();

	bool intersect(IntersectionData* id, Ray r) const;

protected:
	Mesh* mesh_;
	AABB aabb_;
	std::vector<unsigned int> triangle_indices_;

	OctNodeAABB* children_[8]; // Child nodes
	// In order:
	// children_[0] = left bottom far
	// children_[1] = right bottom far
	// children_[2] = left top far
	// children_[3] = right top far
	// children_[4] = left bottom near
	// children_[5] = right bottom near
	// children_[6] = left top near
	// children_[7] = right top near
};

// An octree containing axis aligned bounding boxes
class OctTreeAABB : public OctNodeAABB
{
public:
	OctTreeAABB(Mesh* mesh);
	~OctTreeAABB();
private:
};

#endif