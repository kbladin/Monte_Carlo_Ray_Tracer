#include "../include/Object3D.h"

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

Mesh::Mesh(Material * material) :
	Object3D(material)
{
	/*
	positions_.push_back(glm::vec3(0.5,0,-4));
	positions_.push_back(glm::vec3(0.8,0,-2));
	positions_.push_back(glm::vec3(0.5,0.3,-4));

	normals_.push_back(glm::vec3(0,0,1));
	normals_.push_back(glm::vec3(0,0,1));
	normals_.push_back(glm::vec3(0,0,1));

	indices_.push_back(0);
	indices_.push_back(1);
	indices_.push_back(2);
	*/

	transform_ = glm::scale(glm::mat4(), glm::vec3(0.2f,0.2f,0.2f));
	transform_ = glm::orientation(glm::vec3(0.3,0.5,-0.7), glm::vec3(0,1,0)) * transform_;
	transform_ = glm::translate(glm::mat4(), glm::vec3(0.4f,0.0f,-4.0f)) * transform_;

	loadOBJ("cube.obj", positions_, uvs_, normals_);

	std::cout << indices_.size() << std::endl;

}

bool Mesh::intersect(IntersectionData* id, Ray r) const
{
	for (int i = 0; i < positions_.size(); i=i+3)
	{
		glm::vec3 p0 = glm::vec3(transform_ * glm::vec4(positions_[i + 0], 1));
		glm::vec3 p1 = glm::vec3(transform_ * glm::vec4(positions_[i + 1], 1));
		glm::vec3 p2 = glm::vec3(transform_ * glm::vec4(positions_[i + 2], 1));

		glm::mat3 M;
		M[0] = -r.direction;
		M[1] = p1 - p0;
		M[2] = p2 - p0;

		glm::vec3 tuv =
			glm::inverse(M) *
			(r.position - p0);

		// To avoid confusion
		// t is the parameter on the ray, u and v are parameters on the plane
		float t = tuv.x;
		float u = tuv.y;
		float v = tuv.z;

		if (u >= 0 && v >= 0 && u + v <= 1 && // Within the boundary
			t >= 0) // t needs to be positive to travel forward on the ray
		{
			glm::vec3 n0 = glm::vec3(transform_ * glm::vec4(normals_[i + 0], 0));
			glm::vec3 n1 = glm::vec3(transform_ * glm::vec4(normals_[i + 1], 0));
			glm::vec3 n2 = glm::vec3(transform_ * glm::vec4(normals_[i + 2], 0));

			// Interpolate to find normal
			glm::vec3 n = (1 - u - v) * n0 + u * n1 + v * n2;
			id->t = t;
			id->normal = glm::normalize(n);
			id->material = material();
			return true;
		}
	}
	return false;
	/*
	for (int i = 0; i < indices_.size(); i=i+3)
	{
		glm::vec3 p0 = glm::vec3(transform_ * glm::vec4(positions_[indices_[i + 0]], 1));
		glm::vec3 p1 = glm::vec3(transform_ * glm::vec4(positions_[indices_[i + 1]], 1));
		glm::vec3 p2 = glm::vec3(transform_ * glm::vec4(positions_[indices_[i + 2]], 1));
		glm::vec3 n0 = glm::vec3(transform_ * glm::vec4(normals_[indices_[i + 0]], 0));
		glm::vec3 n1 = glm::vec3(transform_ * glm::vec4(normals_[indices_[i + 1]], 0));
		glm::vec3 n2 = glm::vec3(transform_ * glm::vec4(normals_[indices_[i + 2]], 0));

		glm::mat3 M;
		M[0] = -r.direction;
		M[1] = p1 - p0;
		M[2] = p2 - p0;

		glm::vec3 tuv =
			glm::inverse(M) *
			(r.position - p0);

		// To avoid confusion
		// t is the parameter on the ray, u and v are parameters on the plane
		float t = tuv.x;
		float u = tuv.y;
		float v = tuv.z;

		if (u >= 0 && v >= 0 && u + v <= 1 && // Within the boundary
			t >= 0) // t needs to be positive to travel forward on the ray
		{
			// Interpolate to find normal
			glm::vec3 n = (1 - u - v) * n0 + u * n1 + v * n2;
			id->t = t;
			id->normal = glm::normalize(n);
			id->material = material();
			return true;
		}
	}
	return false;
	*/
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