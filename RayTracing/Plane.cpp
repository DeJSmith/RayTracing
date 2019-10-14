/*----------------------------------------------------------
* COSC363  Ray Tracer
*
*  The Plane class
*  This is a subclass of Object, and hence implements the
*  methods intersect() and normal().
-------------------------------------------------------------*/

#include "Plane.h"
#include <math.h>


/**
* Checks if a point pt is inside the current polygon
* Implement a point inclusion test using 
* member variables a, b, c, d.
*/
bool Plane::isInside(glm::vec3 pt)
{
	glm::vec3 norm = normal(pt);
	
	glm::vec3 veca1 = glm::vec3(pt - a);
	glm::vec3 veca2 = glm::vec3(b - a);
	float aCross = glm::dot(glm::cross(veca2, veca1), norm);
	
	glm::vec3 vecb1 = glm::vec3(pt - b);
	glm::vec3 vecb2 = glm::vec3(c - b);
	float bCross = glm::dot(glm::cross(vecb2, vecb1), norm);
	
	glm::vec3 vecc1 = glm::vec3(pt - c);
	glm::vec3 vecc2 = glm::vec3(d - c);
	float cCross = glm::dot(glm::cross(vecc2, vecc1), norm);
	
	glm::vec3 vecd1 = glm::vec3(pt - d);
	glm::vec3 vecd2 = glm::vec3(a - d);
	float dCross = glm::dot(glm::cross(vecd2, vecd1), norm);
	
	if(aCross > 0 && bCross > 0 && cCross > 0 && dCross > 0) {
		return true;
	} else {
		return false;
	}

}

/**
* Plane's intersection method.  The input is a ray (pos, dir). 
*/
float Plane::intersect(glm::vec3 posn, glm::vec3 dir)
{
	glm::vec3 n = normal(posn);
	glm::vec3 vdif = a - posn;
	float vdotn = glm::dot(dir, n);
	if(fabs(vdotn) < 1.e-4) return -1;
    float t = glm::dot(vdif, n)/vdotn;
	if(fabs(t) < 0.0001) return -1;
	glm::vec3 q = posn + dir*t;
	if(isInside(q)) return t;
    else return -1;
}

/**
* Returns the unit normal vector at a given point.
* Compute the plane's normal vector using 
* member variables a, b, c, d.
* The parameter pt is a dummy variable and is not used.
*/
glm::vec3 Plane::normal(glm::vec3 pt)
{
	glm::vec3 n = glm::vec3(0); //vector cross product
	n = glm::cross((b - a),(d- a));
	n = glm::normalize(n);

    return n;
}



