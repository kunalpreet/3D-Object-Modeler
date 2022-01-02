#ifndef PPM_H_
#define PPM_H_

#ifdef __APPLE__
#  define GL_SILENCE_DEPRECATION
#  include <OpenGL/gl.h>
#  include <OpenGL/glu.h>
#  include <GLUT/glut.h>
#else
#  include <GL/gl.h>
#  include <GL/glu.h>
#  include <GL/freeglut.h>
#endif


GLubyte * LoadPPM(
    char * file,
    int * width,
    int * height
);

#endif
