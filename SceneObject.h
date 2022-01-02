#ifndef SCENE_OBJECT_H
#define SCENE_OBJECT_H

#include "material.h"
#include <list>
#include <string>
#include "Libraries/Eigen/Dense"


using namespace std; 
using namespace Eigen;


enum ObjType {
    Cube, Sphere, Cone, Torus, Teapot, Other
};

class SceneObject {
public:
    string id;
    Vector3f position;
    Vector3f rotation;
    Vector3f scale;
    Material material;
    ObjType type;

    // bounding box?
    SceneObject();
    SceneObject(string name, Vector3f pos, Vector3f rot, Vector3f sca, Material mat, ObjType type);
    SceneObject(Vector3f pos, Vector3f rot, Vector3f sca, Material mat, ObjType type);
    static ObjType getObjType(int num);
    static std::string toString(ObjType type);
    static int getObjTypeInt(ObjType type);

    list<float> handleRay(Vector3f rayOrigin, Vector3f rayDirection);
private:
    float rayplane(Vector3f rayOrigin, Vector3f rayDirection, Vector3f planeOrigin, Vector3f planeNormal, Vector3f &point);
    list<float> raybox(Vector3f rayOrigin, Vector3f rayDirection);
    list<float> raysphere(Vector3f rayOrigin, Vector3f rayDirection);
    // setBoundary(); // called during initalization to set the boundary of this object?

    bool compareXY(Vector3f point);
    bool compareXZ(Vector3f point);
    bool compareYZ(Vector3f point);

    enum PlaneFace {
        Top, Bottom, Left, Right, Front, Back
    };
    void getPlaneData(PlaneFace face, Vector3f &planeOrigin, Vector3f &planeNormal);

    void rotateVector(Vector3f &vec, Vector3f rotVector);
};

#endif