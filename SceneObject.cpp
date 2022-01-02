#define _USE_MATH_DEFINES

#include "SceneObject.h"
#include <list>
#include <cmath>
#include <iostream>

using namespace std;
using namespace Eigen;

SceneObject::SceneObject() {
    SceneObject(Vector3f(0, 0, 0), Vector3f(0, 0, 0), Vector3f(1, 1, 1), Material::RandomMat(), Cube);
}

SceneObject::SceneObject(Vector3f pos, Vector3f rot, Vector3f sca, Material mat, ObjType type) {
    this->id = "obj";
    this->position = pos;
    this->rotation = rot;
    this->scale = sca;
    this->material = mat;
    this->type = type;
}

SceneObject::SceneObject(std::string name, Vector3f pos, Vector3f rot, Vector3f sca, Material mat, ObjType type) {
    this->id = name;
    this->position = pos;
    this->rotation = rot;
    this->scale = sca;
    this->material = mat;
    this->type = type;
}

// takes in an objtype, and returns the string of that enum for printing
std::string SceneObject::toString(ObjType type) {
    switch(type){
        case Cube: 
            return "Cube";
        case Sphere: 
            return "Sphere";
        case Cone: 
            return "Cone";
        case Torus: 
            return "Torus";
        case Teapot: 
            return "Teapot";
        default:
            return "Other";
    }
}

// takes in an int, and returns the appropriate enum type
ObjType SceneObject::getObjType(int num) {
    switch(num){
        case 1: 
            return Cube;
        case 2: 
            return Sphere;
        case 3: 
            return Cone;
        case 4: 
            return Torus;
        case 5: 
            return Teapot;
        default:
            return Other;
    }
}

// takes in an objtype, and returns an int to represent it
int SceneObject::getObjTypeInt(ObjType type) {
    switch(type){
        case Cube: 
            return 1;
        case Sphere: 
            return 2;
        case Cone: 
            return 3;
        case Torus: 
            return 4;
        case Teapot: 
            return 5;
        default:
            return 1;
    }
}

// takes in a ray origin and direction, and checks if that ray hits this scene object
std::list<float> SceneObject::handleRay(Vector3f rayOrigin, Vector3f rayDirection) {
    switch (this->type) {

        // special raysphere interaction when dealing with spheres
        case Sphere:
            return raysphere(rayOrigin, rayDirection);
            break;
        
        // all other cases, use raybox
        default:
            return raybox(rayOrigin, rayDirection);
            break;
    }
}

// an intersection between ray and plane, and returns distance
float SceneObject::rayplane(Vector3f rayOrigin, Vector3f rayDirection, Vector3f planeOrigin, Vector3f planeNormal, Vector3f& point) {
    // Point P(x, y, z)
    // Normal N(A, B, C);
    // Ax + By + Cz + D = 0
    // D, d = -Ax - By - Cz
    float d = -(planeNormal(0) * planeOrigin(0)) - (planeNormal(1) * planeOrigin(1)) - (planeNormal(2) * planeOrigin(2));

    // n . Rd ; if n . Rd = 0, ray is 90 degrees away from normal, meanign it will never touch
    // denominator
    float nrd = planeNormal.dot(rayDirection);
    if (nrd == 0)
        return -1;
    
    // numerator = -(N . R0 + D)
    float nr0 = planeNormal.dot(rayOrigin);
    float numerator = -(nr0 + d);
    
    // the distance between the ray origin and plane hit
    float t = numerator/nrd;
    point = rayOrigin + t*rayDirection;
    return t;
}

// check for intersections between ray and the box
std::list<float> SceneObject::raybox(Vector3f rayOrigin, Vector3f rayDirection) {
    std::list<float> intersections; 
    // vars to store results of rayplane intersections
    float dist;
    Vector3f point;

    Vector3f planeOrigin;
    Vector3f planeNormal;

    // checking intersection with Top surface
    getPlaneData(Top, planeOrigin, planeNormal);
    // check for any collisions between ray and the top plane
    dist = rayplane(rayOrigin, rayDirection, planeOrigin, planeNormal, point);
    // if does intersect with plane, and is within the XZ plane of the cube
    if (dist != -1 && compareXZ(point)) {
        intersections.push_back(dist);
    }

    // checking intersection with Bottom surface
    getPlaneData(Bottom, planeOrigin, planeNormal);
    dist = rayplane(rayOrigin, rayDirection, planeOrigin, planeNormal, point);
    if (dist != -1 && compareXZ(point)) {
        intersections.push_back(dist);
    }

    // checking intersection with Right surface
    getPlaneData(Right, planeOrigin, planeNormal);
    dist = rayplane(rayOrigin, rayDirection, planeOrigin, planeNormal, point);
    if (dist != -1 && compareYZ(point)) {
        intersections.push_back(dist);
    }

    // checking intersection with Left surface
    getPlaneData(Left, planeOrigin, planeNormal);
    dist = rayplane(rayOrigin, rayDirection, planeOrigin, planeNormal, point);
    if (dist != -1 && compareYZ(point)) {
        intersections.push_back(dist);
    }

    // checking intersection with Front surface
    getPlaneData(Front, planeOrigin, planeNormal);
    dist = rayplane(rayOrigin, rayDirection, planeOrigin, planeNormal, point);
    if (dist != -1 && compareXY(point)) {
        intersections.push_back(dist);
    }
    // checking intersection with Back surface
    getPlaneData(Back, planeOrigin, planeNormal);
    dist = rayplane(rayOrigin, rayDirection, planeOrigin, planeNormal, point);
    if (dist != -1 && compareXY(point)) {
        intersections.push_back(dist);
    }
    return intersections;
}

// updates planeOrigin and planeNormal values to the appropriate face chosen
void SceneObject::getPlaneData(SceneObject::PlaneFace face, Vector3f &planeOrigin, Vector3f &planeNormal) {
    // handling scaling


    // we first find the plane origin (which is scale/2 in a dirn)
    // Atm, we assume that our object is at 0,0,0 ; we change this later
    // we also get the plane normal
    switch(face) {
        case Top:
            planeOrigin = Vector3f(0, scale(1)/2, 0);
            planeNormal = Vector3f(0, 1, 0);
            break;
        case Bottom:
            planeOrigin = Vector3f(0, -scale(1)/2, 0);
            planeNormal = Vector3f(0, -1, 0);
            break;
        case Right:
            planeOrigin = Vector3f(scale(0)/2, 0, 0);
            planeNormal = Vector3f(1, 0, 0);
            break;
        case Left:
            planeOrigin = Vector3f(-scale(0)/2, 0, 0);
            planeNormal = Vector3f(-1, 0, 0);
            break;
        case Front:
            planeOrigin = Vector3f(0, 0, +scale(0)/2);
            planeNormal = Vector3f(0, 0, 1);
            break;
        case Back:
            planeOrigin = Vector3f(0, 0, -scale(0)/2);
            planeNormal = Vector3f(0, 0, -1);
            break;
    }


    // Commenting this out for now; makes it hard to check if point is within bounds
        // Now, we apply the rotation
        // rotateVector(planeOrigin, this->rotation);
        // rotateVector(planeNormal, this->rotation);
        // planeNormal = planeNormal.normalized();
        // planeOrigin contains the dist from center of object to the new rotated plane origin

    // now, we just add our the position to get the new planeOrigin;
    planeOrigin = position + planeOrigin;
}


// Rotates a Vector3f by another Vector3f
// Source: https://stackoverflow.com/questions/21412169/creating-a-rotation-matrix-with-pitch-yaw-roll-using-eigen
void SceneObject::rotateVector(Vector3f &vec, Vector3f rotVector) {
    // first turn rotation vector into quaternion
    
    AngleAxisf xAngle(rotVector(0) *M_PI / 180, Vector3f::UnitX());
    AngleAxisf yAngle(rotVector(1) *M_PI / 180, Vector3f::UnitY());
    AngleAxisf zAngle(rotVector(2) *M_PI / 180, Vector3f::UnitZ());
    Quaternion<float> q = xAngle * yAngle * zAngle;

    // Now, turn into a rotation matrix
    Matrix3f rotationMatrix = q.toRotationMatrix();
    
    // And multiply the two
    vec = rotationMatrix * vec;
    // and now vec is rotated;
}


// TODO: Change something here such that rotation is now taken into account
// This is really hard; now you have to worry about all 3 dimensions when making sure point is within a face
// You also now have to get the new 'scale', i.e. the width/height of the face in terms of xyz

// Checks if a point is within bounds of the XY plane of this cube
// Front and Back planes, usually
bool SceneObject::compareXY(Vector3f point) {
    return (
        point(0) >= this->position(0) - this->scale(0)/2 && 
        point(0) <= this->position(0) + this->scale(0)/2 && 
        point(1) >= this->position(1) - this->scale(1)/2 && 
        point(1) <= this->position(1) + this->scale(1)/2
        );
}

// Checks if a point is within bounds of the XZ plane of this cube
// Top and bottom planes
bool SceneObject::compareXZ(Vector3f point) {
    return (
        point(0) >= this->position(0) - this->scale(0)/2 && 
        point(0) <= this->position(0) + this->scale(0)/2 && 
        point(2) >= this->position(2) - this->scale(2)/2 && 
        point(2) <= this->position(2) + this->scale(2)/2
        );
}


// Checks if a point is within bounds of the YZ plane of this cube
// Right and Left planes
bool SceneObject::compareYZ(Vector3f point) {
    return (
        point(1) >= this->position(1) - this->scale(1)/2 && 
        point(1) <= this->position(1) + this->scale(1)/2 && 
        point(2) >= this->position(2) - this->scale(2)/2 && 
        point(2) <= this->position(2) + this->scale(2)/2
        );
}



// checks for intersection between ray and sphere
// works pretty weirdly if something is scaled, and not all xyz scales are even
std::list<float> SceneObject::raysphere(Vector3f rayOrigin, Vector3f rayDirection) {
    std::list<float> intersections;

    // using a scummy method that averages scale (i.e. assuming scale is always 111, 222, etc.) for perfect sphere
    float currentScale = (this->scale(0) + this->scale(1) + this->scale(2))/3;
    float radius = 0.5 * currentScale; // a 1-unit glutSolidSphere has radius of 0.5

    float A = rayDirection.dot(rayDirection);   
    float B = 2 * ((rayOrigin - this->position).dot(rayDirection));
    float C = ((rayOrigin - this->position).dot(rayOrigin-this->position)) - radius*radius;

    // quadratic eqn
// (-b +- sqrt(B*B - 4*A*C)) / 2A
    float discriminant = B*B - 4*A*C;
    if (discriminant < 0) {
        // no intersection!
        return intersections;
    }

    // otherwise, we find the two points of t1, t2 from quadratic
    float t1 = (-B + sqrt(discriminant))/(2*A);
    float t2 = (-B - sqrt(discriminant))/(2*A);

    intersections.push_back(t1);
    intersections.push_back(t2);
    return intersections;
}