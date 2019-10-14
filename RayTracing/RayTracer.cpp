/*========================================================================
* COSC 363  Computer Graphics (2018)
* Assignment 2
* dsm106
* 15904794
*=========================================================================
*/
#include <iostream>
#include <cmath>
#include <vector>
#include <glm/glm.hpp>
#include "Sphere.h"
#include "SceneObject.h"
#include "Ray.h"
#include <GL/glut.h>
#include "Plane.h"
#include "Triangle.h"
#include "TextureBMP.h"

using namespace std;

const float WIDTH = 20.0;  
const float HEIGHT = 20.0;
const float EDIST = 40.0;
const int NUMDIV = 10000;
const int MAX_STEPS = 5;
const float XMIN = -WIDTH * 0.5;
const float XMAX =  WIDTH * 0.5;
const float YMIN = -HEIGHT * 0.5;
const float YMAX =  HEIGHT * 0.5;
const float ETA = 1.005;

vector<SceneObject*> sceneObjects;  //A global list containing pointers to objects in the scene
TextureBMP backWallTexture; //Wall texture
TextureBMP blackHoleTexture; //Sphere texture

//---The most important function in a ray tracer! ---------------------------------- 
//   Computes the colour value obtained by tracing a ray and finding its 
//     closest point of intersection with objects in the scene.
//----------------------------------------------------------------------------------
glm::vec3 trace(Ray ray, int step)
{
    
	glm::vec3 backgroundCol(0);
	glm::vec3 light(10, 40, -3);
	glm::vec3 ambientCol(0.2);   //Ambient color of light
    glm::vec3 specular(0);
    glm::vec3 colourSum;
    
    
    //------------------------------- other varibales ------------------------------
    float shininess = 15.0;
    float transparency = 0.2;

    
    ray.closestPt(sceneObjects);		//Compute the closest point of intersetion of objects with the ray
    

    if(ray.xindex == -1) return backgroundCol;      //If there is no intersection return background colour
    glm::vec3 materialCol = sceneObjects[ray.xindex]->getColor(); //else return object's colour
    glm::vec3 normalVector = sceneObjects[ray.xindex]->normal(ray.xpt); //Normal vector at point of intersection
    
    //---------------------------------Light vector --------------------------------
    glm::vec3 lightVector = light - ray.xpt;
    float lightDistance = glm::length(lightVector);
    lightVector = glm::normalize(lightVector);
    
    
    //---------------------------------- Dots --------------------------------------
    float lDotn = glm::dot(lightVector, normalVector);
    
    glm::vec3 reflectionVector = glm::reflect(-lightVector, normalVector);
    
    float rDotv = glm::dot(reflectionVector, -ray.dir);
    rDotv = max(rDotv, 0.0f);

    //-------------------------------Textured objects ----------------------------
    
    //pattern Sphere
    if(ray.xindex == 2){
        
        int modxPos = (int)((ray.xpt.x + 5)/2) % 2;
        int modyPos = (int)((ray.xpt.y + 10)/2) % 2;
        
        if((modxPos && modyPos) || (!modxPos && !modyPos)){
            materialCol = glm::vec3(0.12, 0.12, 0.12);
        } else {
            materialCol = glm::vec3(0.15, 0.40, 0.84);
        }
    }
    
    //floor 
    if(ray.xindex == 0){
        
        int modxPos = (int)((ray.xpt.x + 80)/4) % 2;
        int modzPos = (int)((ray.xpt.z + 200)/4) % 2;
        
        if(modxPos && modzPos){
            materialCol = glm::vec3(0);
            
        } else if (!modxPos && !modzPos){
            materialCol = glm::vec3(0.69, 0.76, 0.85);
            
        } else {
            materialCol = glm::vec3(0.15, 0.40, 0.84);
        }
        
    }
    
    //backWall texture
    if(ray.xindex == 1){
        float textCoords = (ray.xpt.x + 80)/160;
        float textCoordt = (ray.xpt.y + 20)/80;
        materialCol = backWallTexture.getColorAt(textCoords, textCoordt);
    }
    
    
    
    //--------------------------------Shadows ------------------------------------
    
    Ray shadow(ray.xpt, lightVector); //shadow ray
    shadow.closestPt(sceneObjects);    
    
    
    if(rDotv >= 0){
        specular = (pow(rDotv, shininess) * glm::vec3(1, 1, 1));
    }
    
    
    if(((lDotn <= 0) || (shadow.xindex >-1)) && (shadow.xdist < lightDistance)){
        if(shadow.xindex == 3 || shadow.xindex == 2){
            colourSum += (ambientCol*materialCol)  + (lDotn * materialCol + specular) * (glm::vec3(0.6) + sceneObjects[3]->getColor()*glm::vec3(0.05));
        } else {
            colourSum = ambientCol * materialCol + specular;
        }
    } else {
        colourSum = ambientCol * materialCol + lDotn * materialCol + specular;
    
    }
    
    //------------------------------ Reflections --------------------------------------
    
    if((ray.xindex >= 4) && step < MAX_STEPS){
        glm::vec3 reflectedDirection = glm::reflect(ray.dir, normalVector);
        Ray reflectedRay(ray.xpt, reflectedDirection);
        glm::vec3 reflectedColour = trace(reflectedRay, step + 1);
        colourSum = colourSum + (.8f * reflectedColour);
    }
    
    //----------------------------- Refractions --------------------------------------
    if((ray.xindex == 3 || ray.xindex == 4) && step < MAX_STEPS){
        
        //transparent sphere
        if(ray.xindex == 4){
            Ray transparent(ray.xpt, ray.dir); //create new ray in the same directin as original ray
            trace(transparent, step+1); //trace ray
        }
        
        glm::vec3 g = glm::refract(ray.dir, normalVector, 1.0f/ETA);
        Ray refractedRay1(ray.xpt, g);
        
        refractedRay1.closestPt(sceneObjects);
        
        if(refractedRay1.xindex == -1){
                return backgroundCol;
        }
        
        glm::vec3 m = sceneObjects[refractedRay1.xindex]->normal(refractedRay1.xpt);
        glm::vec3 h = glm::refract(g, -m, ETA);
        
        Ray refractedRay2(refractedRay1.xpt, h);
        refractedRay2.closestPt(sceneObjects);
        
        if(refractedRay2.xindex == -1){
            return backgroundCol;
        }
        
        glm::vec3 refractedColor = trace(refractedRay2, step+1);
        colourSum = colourSum * transparency + refractedColor*(1-transparency);
        
        return colourSum;
    }

	return colourSum;
}

//Creates two tetrahedrons from triangle planar surfaces 
void makeTetra(){
    glm::vec3 colour(0.27, 0.98, 0.97);
    
    
    Triangle *bottom = new Triangle(glm::vec3(12., 4, -94),
                                    glm::vec3(6., 4, -90),
                                    glm::vec3(5, 4, -98),
                                    colour);
                                    
    Triangle *panel1 = new Triangle(glm::vec3(11., 4, -94),
                                    glm::vec3(5., 4, -98),
                                    glm::vec3(7.5, 10, -95),
                                    colour);
                                    
    Triangle *panel2 = new Triangle(glm::vec3(6., 4, -90),
                                    glm::vec3(5, 4, -98),
                                    glm::vec3(7.5, 10, -95),
                                    colour);
                                    
    Triangle *panel3 = new Triangle(glm::vec3(12, 4, -94),
                                    glm::vec3(6, 4, -90),
                                    glm::vec3(7.5, 10, -95),
                                    colour);
                                    
    Triangle *panel4 = new Triangle(glm::vec3(11., 4, -94),
                                    glm::vec3(5., 4, -98),
                                    glm::vec3(7.5, -2, -95),
                                    colour);
                                    
    Triangle *panel5 = new Triangle(glm::vec3(6., 4, -90),
                                    glm::vec3(5, 4, -98),
                                    glm::vec3(7.5, -2, -95),
                                    colour);
                                    
    Triangle *panel6 = new Triangle(glm::vec3(12, 4, -94),
                                    glm::vec3(6, 4, -90),
                                    glm::vec3(7.5, -2, -95),
                                    colour);
    
    
    sceneObjects.push_back(bottom);
    sceneObjects.push_back(panel1);
    sceneObjects.push_back(panel2);
    sceneObjects.push_back(panel3);
    sceneObjects.push_back(panel4);
    sceneObjects.push_back(panel5);
    sceneObjects.push_back(panel6);
    
    
}

//Creates a cube at a fixed position from six planes
void makeCube(){
    glm::vec3 colour(0.72, 0, 0.3);
    
    Plane *bottom = new Plane(glm::vec3(-5., -21, -85),    //Point A
                              glm::vec3(0., -21, -85),     //Point B
                              glm::vec3(0., -21, -90),     //Point C
                              glm::vec3(-5., -21, -90),    //Point D
                              colour);                     //Colour
                             
    Plane *leftSide = new Plane(glm::vec3(-5., -21, -85),   //Point A
                                glm::vec3(-5., -16, -85),     //Point B
                                glm::vec3(-5., -16, -90),     //Point C
                                glm::vec3(-5., -21, -90),   //Point D
                                colour);                    //Colour
                                
    Plane *rightSide = new Plane(glm::vec3(0., -21, -85),   //Point A
                                glm::vec3(0., -16, -85),      //Point B
                                glm::vec3(0., -16, -90),      //Point C
                                glm::vec3(0., -21, -90),    //Point D
                                colour);                    //Colour
                                
    Plane *top = new Plane(glm::vec3(-5., -16, -85),     //Point A
                            glm::vec3(0., -16, -85),     //Point B
                            glm::vec3(0., -16, -90),     //Point C
                            glm::vec3(-5., -16, -90),    //Point D
                            colour);                   //Colour
                            
    Plane *back = new Plane(glm::vec3(-5., -21, -90),   //Point A
                                glm::vec3(0., -21, -90),     //Point B
                                glm::vec3(0., -16, -90),     //Point C
                                glm::vec3(-5., -16, -90),   //Point D
                                colour);                    //Colour
                                
    Plane *front = new Plane(glm::vec3(-5., -21, -85),         //Point A
                                glm::vec3(0., -21, -85),     //Point B
                                glm::vec3(0., -16, -85),     //Point C
                                glm::vec3(-5., -16, -85),   //Point D
                                colour);                    //Colour
                                
                                
    
    sceneObjects.push_back(back);
    sceneObjects.push_back(bottom);
    sceneObjects.push_back(leftSide);
    sceneObjects.push_back(rightSide);
    sceneObjects.push_back(top);
    sceneObjects.push_back(front);
}

//---The main display module -----------------------------------------------------------
// In a ray tracing application, it just displays the ray traced image by drawing
// each cell as a quad.
//---------------------------------------------------------------------------------------
void display()
{
	float xp, yp;  //grid point
	float cellX = (XMAX-XMIN)/NUMDIV;  //cell width
	float cellY = (YMAX-YMIN)/NUMDIV;  //cell height

	glm::vec3 eye(0., 0., 0.);  //The eye position (source of primary rays) is the origin

	glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

	glBegin(GL_QUADS);  //Each cell is a quad.

	for(int i = 0; i < NUMDIV; i++)  	//For each grid point xp, yp
	{
		xp = XMIN + i*cellX;
		for(int j = 0; j < NUMDIV; j++)
		{
			yp = YMIN + j*cellY;

		    glm::vec3 dir(xp+0.5*cellX, yp+0.5*cellY, -EDIST);	//direction of the primary ray

		    Ray ray = Ray(eye, dir);		//Create a ray originating from the camera in the direction 'dir'
			ray.normalize();				//Normalize the direction of the ray to a unit vector
		    glm::vec3 col = trace (ray, 1); //Trace the primary ray and get the colour value

			glColor3f(col.r, col.g, col.b);
			glVertex2f(xp, yp);				//Draw each cell with its color value
			glVertex2f(xp+cellX, yp);
			glVertex2f(xp+cellX, yp+cellY);
			glVertex2f(xp, yp+cellY);
        }
    }

    glEnd();
    glFlush();
}


//---This function initializes the scene ------------------------------------------- 
//   Specifically, it creates scene objects (spheres, planes, cones, cylinders etc)
//     and add them to the list of scene objects.
//   It also initializes the OpenGL orthographc projection matrix for drawing the
//     the ray traced image.
//----------------------------------------------------------------------------------
void initialize()
{
    glMatrixMode(GL_PROJECTION);
    gluOrtho2D(XMIN, XMAX, YMIN, YMAX);
    glClearColor(0, 0, 0, 1);
    
    //textures
    backWallTexture = TextureBMP((char*)"cosmos.bmp");
    blackHoleTexture = TextureBMP((char*)"blackhole.bmp");

	//-- Create a pointer to a sphere object
	Sphere *reflectiveSphere = new Sphere(glm::vec3(-10.0, -5.0, -110.0), 15.0, glm::vec3(0, 0, 0));
    Sphere *refractingSphere = new Sphere(glm::vec3(12.5, -10.0, -90.0), 7.5, glm::vec3(0.15, 1, 0));
    Sphere *transparentSphere = new Sphere(glm::vec3(12.5, 15.0, -90.0), 5.0, glm::vec3(0));
    Sphere *patternSphere = new Sphere(glm::vec3(-10, 14, -90), 5.0, glm::vec3(1, 1, 0));

    //Floor Plane
    Plane *floor = new Plane(glm::vec3(-80., -20, -40),     //Point A
                             glm::vec3(80., -20, -40),      //Point B
                             glm::vec3(80., -20, -200),     //Point C
                             glm::vec3(-80., -20, -200),    //Point D
                             glm::vec3(0.5, 0.5, 0));       //Colour
                             
    //Back Wall Plane
    Plane *backWall = new Plane(glm::vec3(-200., -20, -200),      //Point A
                                glm::vec3(100., -20, -200),       //Point B
                                glm::vec3(100., 60, -200),       //Point C
                                glm::vec3(-200., 60, -200),      //Point D 
                                glm::vec3(0.5, 0.5, 0));


    //Push textured objects
    sceneObjects.push_back(floor);                //0
    sceneObjects.push_back(backWall);             //1
    sceneObjects.push_back(patternSphere);        //2
    
    //Refractive objects
    sceneObjects.push_back(refractingSphere);     //3

    //tranparent objects
    sceneObjects.push_back(transparentSphere);    //4
    
    //Reflective objects
    makeTetra();                                  //5-11
    sceneObjects.push_back(reflectiveSphere);     //12
    makeCube();                                   //13-18
    
    

    
}



int main(int argc, char *argv[]) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB );
    glutInitWindowSize(600, 600);
    glutInitWindowPosition(20, 20);
    glutCreateWindow("Raytracer");

    glutDisplayFunc(display);
    initialize();

    glutMainLoop();
    return 0;
}
