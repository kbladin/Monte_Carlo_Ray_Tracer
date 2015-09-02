# Global Illumination

This project is a program for rendering virtual 3D scenes with a global illumination algorithm called Monte Carlo ray tracing.

The algorithm recursively trace light rays from the camera out in the scene and bounces them around on / through the surface of objects.

![](data/rendered_images/rendered_image.ppm "Rendered image")

## Things it has

* Some predefined object classes (spheres, planes).
* Possibility to load triangle meshes in to the scene.
	* Octree data structure used to partition triangles for faster rendering.
* Material properties for 3D objects
	* Diffuse color
	* Specular color
	* Reflectance [0, 1]
	* Specular reflectance [0 (diffuse), 1 (mirror)]
	* Transmissivity [0, 1]
	* Refraction_index [1 (air), 2.4 (diamond)]
* Monte Carlo ray tracing algorithm, simulates many light phenomena:
	* Color bleeding
	* Caustics
	* Soft shadows
	* Reflection
		* Currently not considering the Fresnel effect.
	* Refraction
		* Uses Schlick's approximation to Fresnels equations for reflected part.
* Simple paralellization using openMP.
* Using the XML parser pugixml to be able to load XML files describing the scenes.

## Things I would like to add

* Change the algorithm in to a bidirectional path tracing algorithm.
* Stopping rays according to the Russian roulette method.
* Other optimizations (reduce cash misses, simplify code, precomputing, better paralellization technique).
* Implement a field of view algorithm.
* Make different wavelengths refract a different amount and increase number of wavelengths. This way it is possible to achieve rainbow effects from refraction.
	* This requires a more thoughtful way of converting spectral distributions to RGB image data.
	* Probably way to costly.