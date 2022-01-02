#ifndef MATERIAL_H
#define MATERIAL_H
#include <string>
class Material {
public:
    int id;
    float ambMat[4];
    float diffMat[4];
    float specMat[4];  
    float shine;
    float colour[3]; // in cases of no lighting
    Material();
    Material(int id, float amb[4], float diff[4], float spec[4], float shine);  


    static Material Tan();
    static Material Purple();
    static Material Water();
    static Material Fire();
    static Material Smoke();

    static Material RandomMat();
    static Material getMaterial(int num);
    static std::string toString(Material mat);
    static int getMaterialInt(Material mat);
};

#endif