# global_illumination
Starting a project in writing a ray tracer algorithm for global illumination of a simple virtual 3D scene.

## TO DO:
* Create scene object classes for object to place in the scene
	* This is probably where most work lies, need intersection definitions and recursive ray tracing.
	* Possibly one base class Object3D with all functions such as reflect() refract() shadowRay() etc.
	* Sphere class
	* Box class
	* Plane class
	* Possibly a TriangleMesh class
* Include an XML parser so that we can load a scene from an XML-file at run time instead of having to recompile when changing the scene
* Convert the simple spectral distributions (three channels) to RGB byte data in a more sophisticated manner (eg. gamma correction or logarithmic scales)
* Implement a field of view algorithm
* Make different wavelengths refract a different amount and increase number of wavelengths. This way we can achieve rainbow effects from refraction.
	* This requires a more thoughtful way of converting spectral distributions to RGB image data