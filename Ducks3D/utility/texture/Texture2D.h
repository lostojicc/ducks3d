#ifndef TEXTURE_H
#define TEXTURE_H

#include <glad/glad.h>

class Texture2D {
public:
    unsigned int id;
    unsigned int width, height; 
    // texture Format
    unsigned int internalFormat; // format of texture object
    unsigned int imageFormat; // format of loaded image
    // texture configuration
    unsigned int wrapS, wrapT; // wrapping mode on S and T axis
    unsigned int filterMin, filterMax; // filtering mode
    
    Texture2D();
    
    void Generate(unsigned int width, unsigned int height, unsigned char* data);
    void Bind() const;
};

#endif
