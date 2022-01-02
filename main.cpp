#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#  include <OpenGL/gl.h>
#  include <OpenGL/glu.h>
#  include <GLUT/glut.h>
#else
#  include <GL/gl.h>
#  include <GL/glu.h>
#  include <GL/freeglut.h>
#endif
#define _USE_MATH_DEFINES
#include <cmath>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <iostream>
#include <list>
#include <map>		// map
#include <utility>	// pair, make pair
#include <algorithm> // min list
#include <string>
#include "SceneObject.h"
#include "Libraries/Eigen/Dense"
#include "PPM.h"

using namespace std;
using namespace Eigen;

// variable for file name
char fileName[32];

/* PARTICLE SYSTEM */
int const fps = 30;
int const winW = 600;
int const winH = 600;

// scene rotation
float const rotAngleScene = 1.0f; 	// change angle by x degrees per arrow key
float sceneRot[] = {0, 0, 0}; 			// scene rotation

// camera
float const constCamMove = 1.0f; // how much camera moves with WASD
Vector3f camPos(12, 6, 12);
Vector3f targetPos(0, 0, 0);

bool anglingCamera = false;
Vector3f raycastStart(0, 0, 0); // stores close points of raycast
Vector3f raycastEnd(0, 0, 0);	// stores far points of raycast
const float cameraPanSpeed = 0.2f;

// mouse position
float mousePos[2] = {0.0, 0.0};

// Objects and Selection
list<SceneObject> terrain;
list<SceneObject> lights;
list<SceneObject> objects;
bool selected = false;
SceneObject* selectedObject = nullptr;

// Object Parameters when Adding one with 'E'
// 1 through 5 for different shapes, materials
int materialNum = 1; // [1-5], use setMaterial
int shapeNum = 1;	// [1-5], use setShape
int textureNum = 0;	// [0-3]; use setTexture

// Textures
const int NUMBER_OF_TEXTURES = 3;
char *textureFiles[NUMBER_OF_TEXTURES] =  {
		"assets/marble.ppm", 
		"assets/carpet.ppm", 
		"assets/battlecruiser.ppm"
	};	
GLuint loadedTextures[NUMBER_OF_TEXTURES + 1]; // binding loaded textures into this array; 0 is no texture, 1 to MAX is something

GLubyte *img_data[NUMBER_OF_TEXTURES];
int img_width[NUMBER_OF_TEXTURES];
int img_height[NUMBER_OF_TEXTURES];
// texture files to load

// Interaction with Object
enum InteractionMode {
	Translate, Rotate, Scale, Camera
};
InteractionMode currentMode = Translate;

// on interactions with obj, how much to move
const float translateConst = 0.5f;
const float rotateConst = 5.0f;
const float scaleConst = 0.25f;


// pre-defining some functions
void printText(); // prints command list
void setTexture(int);
void useTexture(int);
void setMaterial(int);
void setShape(int);

// Getting texture data
void initializeTextures() {
    glGenTextures(NUMBER_OF_TEXTURES+1, loadedTextures);
	for (int i = 0; i < NUMBER_OF_TEXTURES; i++) {
		img_data[i] = LoadPPM(textureFiles[i], &img_width[i], &img_height[i]);
		glBindTexture(GL_TEXTURE_2D, loadedTextures[i+1]);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img_width[i], img_height[i], 0, GL_RGB, GL_UNSIGNED_BYTE, img_data[i]);
	}
	glBindTexture(GL_TEXTURE_2D, 0);
    
}

// Creates scene objects to represent lights (for interaction)
void initializeLights() {
    glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);


	float light0Amb[] = {0.4, 0.4, 0.4, 1};
	float light0Diff[] = {0.2, 0.8, 0.2, 1};
	float light0Spec[] = {0.2, 0.2, 0.2, 1};
	Material light0Mat(0, light0Amb, light0Diff, light0Spec, 25);
	SceneObject light0("LIGHT0", Vector3f(-5, 3, -3), Vector3f(0, 0, 0), Vector3f(0.5, 0.5, 0.5), light0Mat, Sphere);
	lights.push_back(light0);

	float light1Amb[4] = {0.4, 0.4, 0.4, 1};
	float light1Diff[4] = {0.6, 0.1, 0.1, 1};
	float light1Spec[4] = {0.9, 0.2, 0.2, 1};
	Material light1Mat(0, light1Amb, light1Diff, light1Spec, 25);
	SceneObject light1("LIGHT1", Vector3f(5, 2, -5), Vector3f(0, 0, 0), Vector3f(0.5, 0.5, 0.5), light1Mat, Sphere);
	lights.push_back(light1);
}

// Creates scene objects for wall and floor and adds them to list
void initializeTerrain() {
	SceneObject floor("Terrain", Vector3f(0, -1, 0), Vector3f(0, 0, 0), Vector3f(20, 1, 20), Material::Purple(), Cube);
	SceneObject leftWall("Terrain", Vector3f(0, 3.5, -10.0), Vector3f(0, 0, 0), Vector3f(20, 10, 1), Material::Tan(), Cube);
	SceneObject rightWall("Terrain", Vector3f(-10.0, 3.5, 0), Vector3f(0, 0, 0), Vector3f(1, 10, 20), Material::Tan(), Cube);
	terrain.push_back(floor);
	terrain.push_back(leftWall);
	terrain.push_back(rightWall);
}
// select an object
void select(SceneObject &obj) {
	if (obj.id == "Terrain")
		return;
	// otherwise
	selectedObject = &obj;
	selected = true;
}

// deselect objects
void deselect() {
	selectedObject = nullptr;
	selected = false;
}

void setShape(int num) {
	shapeNum = num;
	cout << "Selected Shape: " << SceneObject::toString(SceneObject::getObjType(shapeNum)) << endl;
}

void setMaterial(int num) {
	materialNum = num;
	cout << "Selected Material: " << Material::toString(Material::getMaterial(materialNum)) << endl;
}

void setTexture(int num) {
	if (num >= NUMBER_OF_TEXTURES + 1)
		return;
	textureNum = num;
	string textureName;
	if (num == 0)
		textureName = "None";
	else
		textureName = textureFiles[num-1];
	cout << "Selected Texture: " << textureName << endl;
}

void useTexture(int num) {
	glBindTexture(GL_TEXTURE_2D, loadedTextures[num]);
}

// add an existing object to the list
void addObject(SceneObject& obj) {
	objects.push_back(obj);
	select(objects.back());
}

// create an object from current parameters
void addObject() {
	Vector3f pos(0,0,0);
	Vector3f rot(0,0,0);
	Vector3f scale(1,1,1);
	Material mat = Material::getMaterial(materialNum);
	ObjType type = SceneObject::getObjType(shapeNum);
	SceneObject obj(pos, rot, scale, mat, type);
	addObject(obj);
}

// creates and adds some objects to the screen
void initializeObjects() {
	SceneObject obj1(Vector3f(0, 1, 0), Vector3f(0, 0, 0), Vector3f(1, 1, 1), Material::Water(), Cube);
	objects.push_back(obj1);
}

// removing the selected object - only removing objects, can't remove lights or terrain
void removeObject() {
	if(selected) {
		for (auto it = objects.begin(); it != objects.end();) {
			// it is pointer, we dereference it to SceneObject with *it, then get address of the SceneObject
			// same with selectedObject
			// if the address is the same, then we will remove this object
			if (&(*it) == &(*selectedObject)) {
				deselect();
				objects.erase(it);
				break;
			}
			++it;
		}
	}
}

// clear list
void removeAllObjects() {
	deselect();
	objects.clear();
}

// applies to selected object
void applyMaterialToObj() {
	if (selected) {
		selectedObject->material = Material::getMaterial(materialNum);
	}
}

void applyShapeToObj() {
	if (selected) {
		selectedObject->type = SceneObject::getObjType(shapeNum);
	}
}

// ------INTERACTING WITH SELECTED OBJECTS --------------

// InteractionMode parsing
string parseInteractionMode(InteractionMode mode) {
	switch(mode) {
		case Rotate:
			return "Rotate";
		case Scale:
			return "Scale";
		case Translate:
			return "Translate";
		case Camera:
			return "Camera";
		default:
			return "Error";
	}
}


// Takes in a vector 'vec', and a rotation vector in degrees of x y z. Rotates vec by the given angles.
void rotateVector(Vector3f &vec, Vector3f rotVector) {
    // first turn rotation vector into quaternion
    AngleAxisf xAngle(rotVector(0) * M_PI / 180, Vector3f::UnitX());
    AngleAxisf yAngle(rotVector(1) * M_PI / 180, Vector3f::UnitY());
    AngleAxisf zAngle(rotVector(2) * M_PI / 180, Vector3f::UnitZ());
    Quaternion<float> q = xAngle * yAngle * zAngle;

    // Now, turn into a rotation matrix
    Matrix3f rotationMatrix = q.toRotationMatrix();
    
    // And multiply the two
    vec = rotationMatrix * vec;
    // and now vec is rotated;
}

// rotate camera using arrow keys giving the values
void rotateCamera(float x, float y, float z) {
	Vector3f rotation = Vector3f(x, y, z);
	rotateVector(camPos, rotation);
	rotateVector(targetPos, rotation);
}

// this should move the camera along the x, y, z axis of the normal it is looking at
void moveCamera(float x, float y, float z, boolean moveBoth) {
	// modifier vector
	Vector3f mod(x, y, z);
	// move camera based on x,y,z with respect to the normal
	Vector3f cameraNorm = (targetPos - camPos).normalized();
	// cross product w/ arbitrary 'up' vector to get left/right 
	Vector3f cameraRight = cameraNorm.cross(Vector3f(0, 1, 0)).normalized();
	// cross product of two vectors gives 90 degree to both; in this case, up or down
	Vector3f cameraUp = cameraRight.cross(cameraNorm).normalized();

	targetPos += (cameraRight * mod(0)); 
	targetPos += (cameraUp * mod(1)); 
	targetPos += (cameraNorm * mod(2)); 

	if (moveBoth) {
		camPos += (cameraRight * mod(0)); 
		camPos += (cameraUp * mod(1)); 
		camPos += (cameraNorm * mod(2)); 
	}

}

// for mouse angling; taking in delta mouse movement since last motion
void updateCamera(int deltaX, int deltaY) {
	// take in mouse movement
	// apply change
	float x = deltaX * cameraPanSpeed;
	float y = deltaY * cameraPanSpeed;
	float z = 0;

	// move camera based on x,y,z with respect to the normal
	moveCamera(x, y, z, false);
}


// InteractionMode setting
void setMode(InteractionMode mode) {
	currentMode = mode;
	anglingCamera = false;
	cout << "Setting interaction mode to: " << parseInteractionMode(mode) << endl;
}

// Interaction Handlers
void moveObject(float x, float y, float z) {
	selectedObject->position(0) += x * translateConst;
	selectedObject->position(1) += y * translateConst;
	selectedObject->position(2) += -z * translateConst;
}

void rotateObject(float x, float y, float z) {
	selectedObject->rotation(0) += x * rotateConst;
	selectedObject->rotation(1) += y * rotateConst;
	selectedObject->rotation(2) += z * rotateConst;
}

void scaleObject(float x, float y, float z) {
	selectedObject->scale(0) += x * scaleConst;
	selectedObject->scale(1) += y * scaleConst;
	selectedObject->scale(2) += z * scaleConst;
}

// Sending inputs to correct mode handler
void interactObject(float x, float y, float z) {
	// if controlling camera...
	if (currentMode == Camera) {
		// move both target and camera position
		moveCamera(x, y, z, true);
		return;
	}

	// else, we are trying to interact with an object
	if (!selected || selectedObject == nullptr)
		return;
	
	switch(currentMode) {
		case Translate:
			moveObject(x, y, z);
			break;
		case Rotate:
			rotateObject(x, y, z);
			break;
		case Scale:
			scaleObject(x, y, z);
			break;
		default:
			break;
	}
}


// Given screen coordinates, we generate a normalized ray direction
Vector3f raycast(int mouseX, int mouseY) {	
	GLdouble model[16], proj[16];
	GLint view[4];
	glGetDoublev(GL_MODELVIEW_MATRIX, model);
	glGetDoublev(GL_PROJECTION_MATRIX, proj);
	glGetIntegerv(GL_VIEWPORT, view);

	GLdouble nearX, nearY, nearZ;		// stores coordinates closeby
	GLdouble farX, farY, farZ;			// stores coordinates in the far plane
	gluUnProject(mouseX, mouseY, 0.0, model, proj, view, &nearX, &nearY, &nearZ);
	gluUnProject(mouseX, mouseY, 1.0, model, proj, view, &farX, &farY, &farZ);	
	raycastStart = Vector3f((float) nearX, (float) nearY, (float) nearZ);
	raycastEnd = Vector3f((float) farX, (float) farY, (float) farZ);
	// diff between far point and close point
	Vector3f ray = (raycastEnd - raycastStart).normalized(); // mouse ray vector, normalized
	return ray;
}

// Given a ray origin and direction, and a list of objects, check for any object that the ray hits
map<float, SceneObject*> getObjectsHit(Vector3f rayOrigin, Vector3f rayDirection, list<SceneObject> &listObj) {
	map<float, SceneObject*> collided;
	for (SceneObject& object : listObj) {
		// get list of intersection points for this object
		list<float> distances = object.handleRay(rayOrigin, rayDirection);
		if(distances.size() > 1) {
			// getting closest distance of intersection
			float minimum = distances.front();
			for (auto dist: distances) 
				if (dist < minimum) minimum = dist;

			// store this object, with the key being the minimum distance from ray to obj
			pair<float, SceneObject*> pair(minimum, &object);
			collided.insert(pair);
		}
	}
	// this contains a map of all objects hit, along with the distance (key) to said object
	return collided;
}

// On Left Mouse Button, do something...
void handleClickLeft(map<float, SceneObject*> &collided) {
	// map contains collisions; get first one since key is sorted by min value
	if (collided.size() > 0) {
		// select closest
		SceneObject* objPointer = collided.begin()->second;

		// if clicked on an already selected object
		if (selectedObject == objPointer) {
			// selected same object
			
		} else {
			// selected new object
			if (objPointer->id == "Terrain")
				deselect();
			else
				select(*objPointer);
		}
	} else {
		// nothing hit
		deselect();
	}
}

// On Right Mouse Button
void handleClickRight(map<float, SceneObject*> &collided) {
	if (collided.size() > 0) {
		// select closest
		SceneObject* objPointer = collided.begin()->second;

		// if clicked on an already selected object
		if (selectedObject == objPointer) {
			removeObject();
		} else {
			// selected new object
			if (objPointer->id == "Terrain")
				deselect();
			else
				select(*objPointer);
		}
	} else {
		// nothing hit
		deselect();
	}

}

void load(char *fileName){

	// Open the file to read, first check it exists
	FILE * file = fopen(fileName, "r");

		if (!file)
			{
					cout <<  "File does not exist" << endl;
					return;
			}

	// clear the scene
	removeAllObjects();

	// Get all the rows and construct the objects
	while (!feof(file)){
			SceneObject obj = SceneObject();
			int matId;
			fscanf(file, "%f %f %f %f %f %f %f %f %f %d %d\n",
							&obj.position[0], &obj.position[1], &obj.position[2],
       &obj.rotation[0], &obj.rotation[1], &obj.rotation[2],
       &obj.scale[0], &obj.scale[1], &obj.scale[2],
       &matId, (int*)&obj.type);
					obj.material = Material::getMaterial(matId);
					objects.push_back(obj);
			}

	// close file and confirm loaded
	fclose(file);
			cout << "Scene has been loaded" << endl;
}


void save(char *fileName){

 // Open file to begin write
      FILE * file = fopen(fileName, "w");

      // iterate through scene objects and write data
      list<SceneObject>::iterator it;
      for (it = objects.begin(); it != objects.end(); it++)
      {

      fprintf(file, "%f %f %f %f %f %f %f %f %f %d %d\n",
                  it->position[0], it->position[1], it->position[2],
                  it->rotation[0], it->rotation[1], it->rotation[2],
                  it->scale[0], it->scale[1], it->scale[2],
                  Material::getMaterialInt(it->material), it->type);
      }
      
      // Close teh file
      fclose(file);

      // Confirmation sent to the console
      cout << "Scene has been saved" << endl;
}

// Handling changing parameters using 1-5, shift1-5 for shape and materials
void handleNumKey(unsigned char key) {
	if (key == '1') setShape(1);
	if (key == '2') setShape(2);
	if (key == '3') setShape(3);
	if (key == '4') setShape(4);
	if (key == '5') setShape(5);
	if (key == '!') setMaterial(1);
	if (key == '@') setMaterial(2);
	if (key == '#') setMaterial(3);
	if (key == '$') setMaterial(4);
	if (key == '%') setMaterial(5);
	if (key == '6') setTexture(0);
	if (key == '7') setTexture(1);
	if (key == '8') setTexture(2);
	if (key == '9') setTexture(3);
}

void keyboard(unsigned char key, int x, int y) {
	switch (key)
	{
		case 'p':	// see commands
			printText();
			break;
		case 'q': 	// quit
			exit (0);
			break;

		// changing parameters when creating object
		case '1':	case '2':	case '3':	case '4':	case '5':		// shape
		case '!':	case '@':	case '#':	case '$':	case '%':		// colour
		case '6':	case '7':	case '8':	case '9':	case '0':		// texture
			handleNumKey(key);
			break;
		// spawns object with above parameters
		case 'e': case 'E':
			addObject();
			break;

		// applies the colour of the material to your selected object
		case 'm': case 'M':
			applyMaterialToObj();
			// applyMaterialToObject(x, y); // not too sure about asg here; wants raycast, but we are colouring selected object already?
			break;
		case 'n': case 'N':
			applyShapeToObj();
			// applyShapeToObject(x, y); // not too sure about asg here; wants raycast, but we are colouring selected object already?
			break;

		// ========INTERACTIONS=========
		// interacting along z axis
		case 'w': case 'W':
			interactObject(0, 0, 1);
			break;
		case 's': case 'S':
			interactObject(0, 0, -1);
			break;

		// interacting along x axis
		case 'a': case 'A':
			interactObject(-1, 0, 0);
			break;
		case 'd': case 'D':
			interactObject(1, 0, 0);
			break;

		// interacting along y axis
		case 'r': case 'R':
			interactObject(0, 1, 0);
			break;
		case 'f': case 'F':
			interactObject(0, -1, 0);
			break;

		// setting interaction mode
		case 'z': case 'Z':
			setMode(Translate);
			break;
		case 'x': case 'X':
			setMode(Rotate);
			break;
		case 'c': case 'C':
			setMode(Scale);
			break;
		case 'v': case 'V':
			setMode(Camera);
			break;
		case 32: // spacebar to reset
			cout << "Clearing objects" << endl;
			removeAllObjects();
			break;
		case ',':
			cout << "," << endl;
			break;
		case '.':
			cout << "." << endl;
			break;
		case '/':
			cout << "/" << endl;
			break;
		case 'k': case 'K':
			cout << "Saving objects..." << endl;
			cout << "Enter a filename (ending with txt): ";
			cin >> fileName;
			save(fileName);
			break;
		case 'l': case 'L':
			cout << "Loading objects..." << endl;
			cout << "Enter a filename (ending with txt): ";
			cin >> fileName;
			load(fileName);
			break;

		default:
			break;
	}

	glutPostRedisplay();
}

void kbdSpecial(int key, int x, int y) {
	switch(key) {
		// ARROW KEYS FOR SCENE ROTATION
		case GLUT_KEY_UP:
			rotateCamera(0, 0, rotAngleScene);
			break;
		case GLUT_KEY_DOWN:
			rotateCamera(0, 0, -rotAngleScene);
			break;
		case GLUT_KEY_RIGHT:
			rotateCamera(0, rotAngleScene, 0);
			break;
		case GLUT_KEY_LEFT:
			rotateCamera(0, -rotAngleScene, 0);
			break;
		default:
			break;
	}
	glutPostRedisplay();
}

// void gluttimerfunc; updates particle systems
void update(int val) {

	glutPostRedisplay(); 		// request update
	glutTimerFunc(1000 / fps, update, 0); // setting timer for next fps update
}

/* Input - Mouse Callback */
void mouse(int button, int state, int x, int y){
	mousePos[0] = x; 
	mousePos[1] = winH - y;
	if (state == GLUT_DOWN) {
		// generate ray direction from mouse click
		Vector3f rayDirection = raycast(mousePos[0], mousePos[1]);
		if (currentMode != Camera) {
			map<float, SceneObject*> collided = getObjectsHit(raycastStart, rayDirection, objects);	// find items in list 'objects' that ray hits
			map<float, SceneObject*> collidedLights = getObjectsHit(raycastStart, rayDirection, lights);	// find items in list 'lights' that ray hits
			map<float, SceneObject*> collidedTerrain = getObjectsHit(raycastStart, rayDirection, terrain);	// find items in list 'terrain' that ray hits
			collided.insert(collidedLights.begin(), collidedLights.end());
			collided.insert(collidedTerrain.begin(), collidedTerrain.end());
			if (button == GLUT_LEFT_BUTTON) {
				handleClickLeft(collided); // sending list of objects hit to handler
			}
			if (button == GLUT_RIGHT_BUTTON) {
				handleClickRight(collided); // sending list of objects hit to handler
			}
		} else if (currentMode == Camera) {
			anglingCamera = true;
			// do something here to change the angle
		}
	} else if (state == GLUT_UP) {

		anglingCamera = false;
	}
}

// Callback for active and passive mouse movement 
void motion(int x, int y){

	if (anglingCamera) {
		updateCamera(x - mousePos[0], (winH - y) - mousePos[1]);
	}
	mousePos[0] = x;
	mousePos[1] = winH - y;

}

// draws a scene object based on its stats
void drawObject(SceneObject obj) {
	// update materials based on obj mat here
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, obj.material.ambMat);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, obj.material.diffMat);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, obj.material.specMat);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, obj.material.shine);
	glColor3fv(obj.material.colour);
	glPushMatrix();
		glTranslatef(obj.position(0), obj.position(1), obj.position(2));

		glRotatef(obj.rotation(0), 1, 0, 0);
		glRotatef(obj.rotation(1), 0, 1, 0);
		glRotatef(obj.rotation(2), 0, 0, 1);
		
		// fix to cone to offset position
		if (obj.type == Cone) glTranslatef(0, 0, -obj.scale(2)/2);

		glScalef(obj.scale(0), obj.scale(1), obj.scale(2));

		//enable texture coordinate generation for glutsolidshapes
		glEnable(GL_TEXTURE_GEN_S); 
		glEnable(GL_TEXTURE_GEN_T);
		// Check obj type to draw shape
		glFrontFace(GL_CCW); // special case for teapot
		switch(obj.type) {
			case Cube:
				glutSolidCube(1);
				break;
			case Sphere:
				glutSolidSphere(0.5, 16, 16);
				break;
			case Cone:
				glutSolidCone(0.5, 1, 16, 16);
				break;
			case Torus:
				// inner ring, outer ring
				glutSolidTorus(0.2, 0.3, 16, 16);
				break;
			case Teapot:
				glFrontFace(GL_CW); // special case for teapot
				glutSolidTeapot(0.5);
				break;
			default: 
				glutSolidDodecahedron();
				break;
		}

	glPopMatrix();
}

void drawObject(SceneObject obj, int texture) {
	useTexture(texture);
	drawObject(obj);
}

// sets light parameters then draws the object to represent it
void drawLight(SceneObject light) {
	float pos[] = {light.position(0), light.position(1), light.position(2), 1};
	if (light.id == "LIGHT0") {
		glLightfv(GL_LIGHT0, GL_POSITION, pos);
		glLightfv(GL_LIGHT0, GL_AMBIENT, light.material.ambMat);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, light.material.diffMat);
		glLightfv(GL_LIGHT0, GL_SPECULAR, light.material.specMat);
	} else if (light.id == "LIGHT1") {
		glLightfv(GL_LIGHT1, GL_POSITION, pos);
		glLightfv(GL_LIGHT1, GL_AMBIENT, light.material.ambMat);
		glLightfv(GL_LIGHT1, GL_DIFFUSE, light.material.diffMat);
		glLightfv(GL_LIGHT1, GL_SPECULAR, light.material.specMat);
	} else {
		cout << "Unhandled LIGHT id: " << light.id << endl;
	}
	drawObject(light, 0);
}

// given an object that is selected, draw a wireframe around it
void drawWireframeObject(SceneObject obj) {
	// green wireframe material
    float amb[] = {0, 1, 0, 1};
    float diff[] = {0, 1, 0, 1};
    float spec[] = {0, 1, 0, 1};
    float shine = 150;

	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, amb);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diff);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spec);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shine);

	// no texture for wireframe to ensure it always appears to user
	useTexture(0);
	glPushMatrix();
		glTranslatef(obj.position(0), obj.position(1), obj.position(2));

		glRotatef(obj.rotation(0), 1, 0, 0);
		glRotatef(obj.rotation(1), 0, 1, 0);
		glRotatef(obj.rotation(2), 0, 0, 1);
		
		glScalef(obj.scale(0), obj.scale(1), obj.scale(2));
		// render a sphere wireframe around sphere to show raysphere interactions
		if (obj.type == Sphere)
			glutWireSphere(0.505, 16, 16);
		else
			glutWireCube(1.01);
	glPopMatrix();
}

// draws the entire scene
void drawScene() {

	// drawing light, and rendering the spheres at their location
	for (auto light : lights) {
		drawLight(light);
	}

	// drawing floor, walls; using three different materials
	int i = 1;
	for (auto obj : terrain) {
		drawObject(obj, i);
		i++;
	}

	// render all scene objects
	for (auto obj : objects) {
		drawObject(obj, textureNum);
	}

	// render wireframe
	if(selected && selectedObject != nullptr) {
		drawWireframeObject(*selectedObject);
	}
}

void init(void)
{
	glClearColor(0, 0, 0, 0);
	glColor3f(1, 1, 1);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
//	glOrtho(-5, 5, -5, 5, -5, 80);
	gluPerspective(45, 1, 1, 100);

	initializeTextures();

	initializeLights();

	
	initializeTerrain();
	initializeObjects();


	// prints command list
	printText();
}

/* display function - GLUT display callback function
 *		clears the screen, sets the camera position, draws the ground plane and movable box
 */
void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	// camera
	glLoadIdentity();
	gluLookAt(camPos(0), camPos(1), camPos(2), targetPos(0), targetPos(1), targetPos(2), 0,1,0);

	drawScene();

	glutSwapBuffers();
}

/* main function - program entry point */
int main(int argc, char** argv)
{
	glutInit(&argc, argv);		//starts up GLUT
	
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH );
    
	glutInitWindowSize(winW, winH);
	glutInitWindowPosition(100, 100);

	glutCreateWindow("A4");	//creates the window

	glutDisplayFunc(display);	//registers "display" as the display callback function
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(kbdSpecial);
	glutMouseFunc(mouse);
	
	// both motions call same callback
	glutMotionFunc(motion);
	glutPassiveMotionFunc(motion);
	
	glutTimerFunc(1000 / fps, update, 0);

	// Enable Textures
	glEnable(GL_TEXTURE_2D);

	glEnable(GL_DEPTH_TEST);
    
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glCullFace(GL_BACK);

    
	init();

	glutMainLoop();				//starts the event loop

	return(0);					//return may not be necessary on all compilers
}


void printText() {
	cout << "\n===Commands List===\n"
		<< "1-5 - Selects the shape to be used" << endl
			<< "    N - Applies currently selected shape to a selected object" << endl
		<< "SHIFT + 1-5 - Selects the material to be used" << endl
			<< "    M - Applies currently selected material to a selected object" << endl
		<< "6-9 - Applies the texture for all interactable objects" << endl
			<< "    6 = None, 7-9 selects a textures " << endl
			<< "    Floor and Walls each show 1 texture " << endl
		<< "E - Adds an object into the scene" << endl
		<< "LMB - Selects an interactable object (not floor, walls)" << endl
		<< "RMB - if done on a selected object, removes that object" << endl
		<< "    Clicking outside of an interactable object will deselect" << endl
		<< "Spacebar - removes all interactable objects in the scene" << endl
		// Selected Object Interaction 
		<< "Z X C V - Changes interaction mode with the selected object to Translate, Rotate, Scale, or Camera" << endl 
		<< "W S - Interacts or moves camera along the z axis" << endl 
		<< "A D - Interacts or moves camera along the x axis" << endl 
		<< "R F - Interacts or moves camera along the y axis" << endl 
		<< "LMB/RMB Down - If in Camera Mode, turn the camera" << endl 
		<< "K - Prompts the system to save the objects in the scene to  a file" << endl 
		<< "L - Prompts the system to load objects from a file" << endl 
			<< "    Enter this in the terminal, not the program" << endl 

		<< "Up & Down Arrow Keys - Rotates the scene around the z-axis" << endl
		<< "Left & Right Arrow Keys - Rotates the scene around the y-axis" << endl
		<< "Q - Quits and exits the program" << endl
		<< "Press 'p' at any time to view the commands again." << endl
		<< endl;
}