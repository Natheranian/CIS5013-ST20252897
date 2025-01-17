#pragma once

#include "core.h"

// Model an arcball / pivot camera looking at the origin (0, 0, 0).  The camera by default looks down the negative z axis (using a right-handed coordinate system).  Therefore 'forwards' is along the -z axis.  The camera is actually right/left handed agnostic.  The encapsulated frustum however needs to know the differences for the projection matrix and frustum plane calculations

class ArcballCamera {

private:

	float				theta, phi; // spherical coordinates theta (rotation around the x axis) and phi (rotation around the y axis).  <theta, phi> are stored in degrees.  Zero degree rotation on <theta, phi> places the camera on the +z axis.  A positive phi represents counter-clockwise rotation around the y axis in a right-hand coordinate system.  A positive theta represents a counter-clockwise rotation around the x axis in a right-handed coordinate system
	float				radius; // radius of the camera's spherical coordinate model.  This lies in the interval [0, +inf]

	// Projection / frustum values
	float				fovY, aspect, nearPlane, farPlane;

	// derived values

	// camera position
	//glm::vec4			cameraPos;

	// camera orientation basis(matrix) derived from <theta, phi>
	//glm::mat4			R;

	// view transform matrix for camera's current position and orientation - maps from world to eye coordinate space
	glm::mat4			viewMatrix;

	// projection transform matrix
	glm::mat4			projectionMatrix;


	//
	// Private API
	//

	// update position, orientation, view and projection matrices when camera rotation and radius are modified
	void calculateDerivedValues();

public:

	// Constructors

	ArcballCamera(); // initialise camera parameters so it is placed at the origin looking down the -z axis (for a right-handed camera) or +z axis (for a left-handed camera)

	ArcballCamera(float theta, float phi, float radius, float fovy, float aspect, float nearPlane, float farPlane);
	// create a camera with orientation <theta, phi> representing Euler angles specified in degrees and Euclidean distance 'init_radius' from the origin.  The frustum / viewplane projection coefficients are defined in init_fovy, specified in degrees spanning the entire vertical field of view angle, init_aspect (w/h ratio), init_nearPlane and init_farPlane.  If init_farPlane = 0.0 (as determined by equalf) then the resulting frustum represents an infinite perspective projection.  This is the default


	// Accessor methods for stored properties

	// return the pivot rotation around the x axis (theta) in degrees
	float getTheta(); 

	// return the pivot rotation around the y axis (phi) in degrees
	float getPhi(); 

	// rotate by angles dTheta, dPhi given in degrees
	void rotateCamera(float dTheta, float dPhi);

	// return the camera radius (distance from origin)
	float getRadius();

	// scale camera radius by s.  s is assumed to lie in the interval (0, +inf].  s < 1.0 reduces the radius while s > 1.0 increases the radius
	void scaleRadius(float s);

	// increment camera radius by i.  The camera radius cannot have a value < 0.0 so the resulting radius lies in the interval [0, +inf].
	void incrementRadius(float i);

	float getFovY();

	void setFovY(float fovY);

	float getAspect();

	void setAspect(float aspect);

	float getNearPlaneDistance();

	void setNearPlaneDistance(float nearPlaneDistance);

	float getFarPlaneDistance();

	void setFarPlaneDistance(float farPlaneDistance);

	void resetCamera(float aspect);
	
	
	// Accessor methods for derived values

	//glm::vec4 getPosition(); // return the camera location in world coordinate space.  The radius of the camera's position in spherical coordinates is the l2 norm of the returned position vector

	//glm::mat4 getOrientationBasis(); // return a const reference to the camera's orientation matrix in world coordinate space

	glm::mat4 viewTransform(); // return a const reference to the view transform matrix for the camera

	glm::mat4 projectionTransform(); // return a const reference the projection transform for the camera.  This is a pass-through method and calls projectionMatrix on the encapsulated ViewFrustum

};
