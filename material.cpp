#include "material.h"
#include <algorithm>
#include <iostream>

// default white colour
Material::Material() {
    float amb[4] = {100.0, 100.0, 100.0, 100.0};
    float diff[4] = {100.0, 100.0, 100.0, 100.0};
    float spec[4] = {100.0, 100.0, 100.0, 100.0};
    Material(-1, amb, diff, spec, 100.0f);
}

Material::Material(int id, float amb[4], float diff[4], float spec[4], float shine){
    this->id = id;
    for (int i = 0; i < 4; i++) {
        ambMat[i] = amb[i];
        diffMat[i] = diff[i];
        specMat[i] = spec[i];
    }
    for (int i = 0; i < 3; i++)
        colour[i] = diff[i];

    this->shine = shine;
}

Material Material::Water() {
    float amb[4] = {0.5f, 0.5f, 0.7f, 1.0f};
    float diff[4] = {0.2f, 0.2f, 1.0f, 1.0f};
    float spec[4] = {0.2f, 0.4f, 1.0f, 1.0f};
    float shine = 127;
    return Material(1, amb, diff, spec, shine);
}

Material Material::Fire() {
    float amb[4] = {0.8f, 0.5f, 0.0f, 1.0f};
    float diff[4] = {1.0f, 0.1f, 0.2f, 1.0f};
    float spec[4] = {1.0f, 0.2f, 0.2f , 1.0f};
    float shine = 10;
    return Material(2, amb, diff, spec, shine);
}

Material Material::Smoke() {
    float amb[4] = {0.2f, 0.2f, 0.2f, 0.04f};
    float diff[4] = {0.2f, 0.2f, 0.2f, 0.04f};
    float spec[4] = {0.2f, 0.2f, 0.2f, 0.04f};
    float shine = 2;
    return Material(3, amb, diff, spec, shine);
}

Material Material::Tan() {
    float amb[] = {0.6, .6, 0.6, 1.0};
    float diff[] = {0.78, 0.57, 0.11, 1};
    float spec[] = {0.99, 0.91, 0.81, 1.0};
    float shine = 27;
    return Material(4, amb, diff, spec, shine);
}

Material Material::Purple() {
    float amb[] = {0.5, 0.26, 0.5, 1.0};
    float diff[] = {0.43, 0.2, 0.43, 1};
    float spec[] = {0.6, 0.71, 0.81, 1.0};
    float shine = 120;
    return Material(5, amb, diff, spec, shine);
}

Material Material::RandomMat() {
    int i = rand() % 5 + 1;
    return getMaterial(i);
}

int Material::getMaterialInt(Material mat) {
    return mat.id;
}

Material Material::getMaterial(int num) {
    switch(num) {
        case 1:  
            return Water();
        case 2:
            return Fire();
        case 3:
            return Smoke();
        case 4:
            return Tan();
        case 5:
            return Purple();
        default:
            return Material();
    }
}

std::string Material::toString(Material mat) {
    switch(mat.id) {
        case 1:  
            return "Water";
        case 2:
            return "Fire";
        case 3:
            return "Smoke";
        case 4:
            return "Tan";
        case 5:
            return "Maroon";
        default:
            return "White";
    }
}
