# Global Illumination

This project is a program for rendering virtual 3D scenes with a global illumination algorithm called Monte Carlo ray tracing.

The algorithm recursively trace light rays from the camera out in the scene and bounces them around on / through the surface of objects.

![](doc_images/2015-08-25-20/39/12.ppm "Rendered image")

## Things it has

* Some predefined object classes (spheres, planes).
* Possibility to load triangle meshes in to the scene.
	* Mesh objects are given an axis aligned bounding box for faster rendering.
* Material properties for 3D objects
* Monte Carlo ray tracing algorithm, simulates many light phenomena:
	* Color bleeding
	* Caustics
	* Soft shadows
	* Reflection
		* Currently not considering the Fresnel effect.
	* Refraction
		* Uses Schlick's approximation to Fresnels equations for reflected part.
* Simple paralellization using openMP.

## Things I would like to add

* Change the algorithm in to a bidirectional path tracing algorithm.
* Stopping rays according to the Russian roulette method.
* Other optimizations (reduce cash misses, simplify code, precomputing, better paralelization technique).
* Include an XML parser so that it is possible load a scene from an XML-file at run time instead of having to recompile when changing the scene.
* Implement a field of view algorithm.
* Make different wavelengths refract a different amount and increase number of wavelengths. This way it is possible to achieve rainbow effects from refraction.
	* This requires a more thoughtful way of converting spectral distributions to RGB image data.
	* Probably way to costly.
* Make the triangle mesh rendering faster.
	* Maybe with the use of oct trees.
