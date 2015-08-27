#include "../include/Object3D.h"

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
	if (material_)
	{
		return *material_;
	}
	else
	{
		return Material();
	}
}

// --- Mesh class functions --- //

Mesh::Mesh(glm::mat4 transform, const char* file_path, Material * material) :
	Object3D(material)
{
	transform_ = transform;
	loadOBJ(file_path, positions_, uvs_, normals_);

	std::cout << "Building octree for mesh." << std::endl;
	ot_aabb_ = new OctTreeAABB(3, this);
	std::cout << "Octree built." << std::endl;
}

bool Mesh::intersect(IntersectionData* id, Ray r) const
{
	return ot_aabb_->intersect(id, r);
}

// This function is taken from www.opengl-tutorial.org
bool Mesh::loadOBJ(
	const char * path, 
	std::vector<glm::vec3> & out_vertices, 
	std::vector<glm::vec2> & out_uvs,
	std::vector<glm::vec3> & out_normals
){
	printf("Loading OBJ file %s...\n", path);

	std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
	std::vector<glm::vec3> temp_vertices; 
	std::vector<glm::vec2> temp_uvs;
	std::vector<glm::vec3> temp_normals;


	FILE * file = fopen(path, "r");
	if( file == NULL ){
		printf("Impossible to open the file ! Are you in the right path ? See Tutorial 1 for details\n");
		getchar();
		return false;
	}

	while( 1 ){

		char lineHeader[128];
		// read the first word of the line
		int res = fscanf(file, "%s", lineHeader);
		if (res == EOF)
			break; // EOF = End Of File. Quit the loop.

		// else : parse lineHeader
		
		if ( strcmp( lineHeader, "v" ) == 0 ){
			glm::vec3 vertex;
			fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z );
			temp_vertices.push_back(vertex);
		}else if ( strcmp( lineHeader, "vt" ) == 0 ){
			glm::vec2 uv;
			fscanf(file, "%f %f\n", &uv.x, &uv.y );
			uv.y = -uv.y; // Invert V coordinate since we will only use DDS texture, which are inverted. Remove if you want to use TGA or BMP loaders.
			temp_uvs.push_back(uv);
		}else if ( strcmp( lineHeader, "vn" ) == 0 ){
			glm::vec3 normal;
			fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z );
			temp_normals.push_back(normal);
		}else if ( strcmp( lineHeader, "f" ) == 0 ){
			std::string vertex1, vertex2, vertex3;
			unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
			int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2] );
			if (matches != 9){
				printf("File can't be read by our simple parser :-( Try exporting with other options\n");
				return false;
			}
			vertexIndices.push_back(vertexIndex[0]);
			vertexIndices.push_back(vertexIndex[1]);
			vertexIndices.push_back(vertexIndex[2]);
			uvIndices    .push_back(uvIndex[0]);
			uvIndices    .push_back(uvIndex[1]);
			uvIndices    .push_back(uvIndex[2]);
			normalIndices.push_back(normalIndex[0]);
			normalIndices.push_back(normalIndex[1]);
			normalIndices.push_back(normalIndex[2]);
		}else{
			// Probably a comment, eat up the rest of the line
			char stupidBuffer[1000];
			fgets(stupidBuffer, 1000, file);
		}

	}

	// For each vertex of each triangle
	for( unsigned int i=0; i<vertexIndices.size(); i++ ){

		// Get the indices of its attributes
		unsigned int vertexIndex = vertexIndices[i];
		unsigned int uvIndex = uvIndices[i];
		unsigned int normalIndex = normalIndices[i];
		
		// Get the attributes thanks to the index
		glm::vec3 vertex = temp_vertices[ vertexIndex-1 ];
		glm::vec2 uv = temp_uvs[ uvIndex-1 ];
		glm::vec3 normal = temp_normals[ normalIndex-1 ];
		
		// Put the attributes in buffers
		out_vertices.push_back(vertex);
		out_uvs     .push_back(uv);
		out_normals .push_back(normal);
	
	}

	return true;
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
	T = r.position - P0_;
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

glm::vec3 Plane::getNormal() const
{
	glm::vec3 v1 = P1_ - P0_;
	glm::vec3 v2 = P2_ - P0_;
	return glm::normalize(glm::cross(v1, v2));
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

std::vector<Ray> LightSource::shootLightRay()
{
	std::vector<Ray> rays;

	// Move random code out later
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> dis(0, 1);

	Ray first;
	first.position = getPointOnSurface(dis(gen), dis(gen));

	// Get a uniformly distributed vector
	glm::vec3 normal = emitter_.getNormal();
	glm::vec3 tangent = emitter_.getFirstTangent();
	// rand1 is a random number from the cosine estimator
	float rand1 = glm::asin(dis(gen));// (*dis_)(*gen_);
	float rand2 = dis(gen);

	// Uniform distribution
	float inclination = glm::acos(1 - rand1);//glm::acos(1 -  2 * (*dis_)(*gen_));
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

	first.direction = random_direction;
	first.material = Material::air();
	
	return rays;
}