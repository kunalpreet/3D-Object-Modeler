#include "PPM.h"
#include <stdio.h>
#include <stdlib.h>

GLubyte * LoadPPM(char* file, int* width, int* height) {
    GLubyte* img;
    FILE *fd;
    int n, m;
    int  k, nm;
    char c;
    int i;
    char b[100];
    float s;
    int red, green, blue;

    /* first open file and check if it's an ASCII PPM (indicated by P3 at the start) */
    fd = fopen(file, "r");
    fscanf(fd,"%[^\n] ",b);
    if (b[0]!='P'|| b[1] != '3') {
        printf("%s is not a PPM file!\n",file);
        exit(0);
    }
    fscanf(fd, "%c",&c);

    /* next, skip past the comments - any line starting with #*/
    while (c == '#') {
        fscanf(fd, "%[^\n] ", b);
        fscanf(fd, "%c",&c);
    }
    ungetc(c,fd);

    /* now get the dimensions and max colour value from the image */
    fscanf(fd, "%d %d %d", &n, &m, &k);

    /* calculate number of pixels and allocate storage for this */
    nm = n*m;
    img = (GLubyte*)malloc(3*sizeof(GLuint)*nm);
    s=255.0/k;

    /* for every pixel, grab the read green and blue values, storing them in the image data array */
    for (i = 0; i < nm; i++) {
        fscanf(fd, "%d %d %d", &red, &green, &blue);
        img[3 * nm - 3 * i - 3] = red * s;
        img[3 * nm - 3 * i - 2] = green * s;
        img[3 * nm - 3 * i - 1] = blue * s;
    }

    /* finally, set the "return parameters" (width, height, max) and return the image array */
    *width = n;
    *height = m;

    return img;
}
